/**
 * Copyright (c) 2021 OceanBase
 * OceanBase CE is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan PubL v2.
 * You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#include "ob_testbench_transaction_task.h"

namespace oceanbase
{
namespace testbench
{
/*
                                        ObIWorkloadTransactionTask
*/
ObIWorkloadTransactionTask::ObIWorkloadTransactionTask(BasicTaskConfig config) 
: table_name_(config.table_name), 
  connection_count_(config.connections),
  mysql_proxy_(config.mysql_proxy),
  latency_collector_(config.collector),
  dblink_id_(config.dblink_id),
  partition_id_(config.partition_id),
  row_id_start_(config.row_id_start),
  allocator_("TxnTask")
{}

ObIWorkloadTransactionTask::~ObIWorkloadTransactionTask() {
  latencys_.release_array();
  cumulative_latencys_.release_array();
  commits_.release_array();
  connections_.release_array();
}

int ObIWorkloadTransactionTask::prepare_arrays() {
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(connection_count_ <= 0)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "the connection count is not inited", K(ret), K(connection_count_));
  } else if (OB_FAIL(latencys_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate latencys array failed", K(ret));
  } else if (OB_FAIL(cumulative_latencys_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate cumulative latencys array failed", K(ret));
  } else if (OB_FAIL(commits_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate commits array failed", K(ret));
  } else if (OB_FAIL(connections_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate connections array failed", K(ret));
  } else {
    for (int64_t i = 0; i < connection_count_; ++i) {
      latencys_.at(i) = 0;
      cumulative_latencys_.at(i) = 0;
      commits_.at(i) = true;
      if (OB_FAIL(mysql_proxy_->get_mysql_conn(dblink_id_.at(i), 0, connections_.at(i)))) {
        TESTBENCH_LOG(ERROR, "acquire dblink connection failed", K(ret), K(dblink_id_));
      }
    }
  }
  return ret;
}

int ObIWorkloadTransactionTask::init() {
  int ret = OB_SUCCESS;
  if (OB_ISNULL(mysql_proxy_)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "the connection pool is not inited", K(ret), KP(mysql_proxy_));
  } else if (OB_ISNULL(latency_collector_)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "the latency collector is not inited", K(ret), KP(latency_collector_));
  } else if (OB_FAIL(prepare_arrays())) {
    TESTBENCH_LOG(ERROR, "acquire mysql connections failed", K(ret));
  } else {
    TESTBENCH_LOG(INFO, "ObIWorkloadTransactionTask init succeed");
  }
  return ret;
}

int ObIWorkloadTransactionTask::wait_for_connection(ObMySQLConnection *conn) {
  int ret = OB_SUCCESS;
  if (OB_FAIL(conn->get_conn_status())) {
    TESTBENCH_LOG(ERROR, "get connection status failed", KR(ret));
  } else if (OB_FAIL(conn->update_conn_status())) {
    TESTBENCH_LOG(ERROR, "update connection status failed", KR(ret));
  }
  return ret;
}

int ObIWorkloadTransactionTask::record_latency(ObLatencyTaskType type, int64_t latency) {
  int ret = OB_SUCCESS;
  ObLatencyTask *task = nullptr;
  void *buf = nullptr;
  common::ObObj latency_obj;
  latency_obj.set_double(latency);
  if (OB_ISNULL(buf = allocator_.alloc(sizeof(ObLatencyTask)))) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    TESTBENCH_LOG(ERROR, "allocate memory failed", K(ret));
  } else {
    task = new(buf)ObLatencyTask(type, latency_obj);
    latency_collector_->push_latency_task(task);
  }
  return ret;
}

int ObIWorkloadTransactionTask::wait_and_bind_param(int64_t conn_idx, int64_t &partition_id, int64_t &row_id, ObMySQLPreparedStatement &stmt) {
  int ret = OB_SUCCESS;
  // check the status of the last command
  if (OB_FAIL(wait_for_connection(connections_.at(conn_idx)))) {
    TESTBENCH_LOG(WARN, "wait for the connection get error", K(ret));
  } else if (OB_FAIL(stmt.bind_param_int(0, &partition_id))) {
    TESTBENCH_LOG(WARN, "bind the first parameter of distributed transaction failed", K(ret), K(partition_id));
  } else if (OB_FAIL(stmt.bind_param_int(1, &row_id))) {
    TESTBENCH_LOG(WARN, "bind the second parameter of distributed transaction failed", K(ret), K(row_id));
  }
  return ret;
}
/*
                                        ObDistributedTransactionTask
*/
ObDistributedTransactionTask::ObDistributedTransactionTask(BasicTaskConfig basic_config, int64_t participants, int64_t operations)
: ObIWorkloadTransactionTask(basic_config), participants_(participants), operations_(operations), lock_txn_sql_()
{
  lock_txn_sql_format_ = "UPDATE %s SET lock_txn = lock_txn + 1 WHERE partition_id = ? AND row_id = ?;";
}

ObDistributedTransactionTask::~ObDistributedTransactionTask() {}

int ObDistributedTransactionTask::release_dblinks() {
  int ret = OB_SUCCESS;
  for (int64_t i = 0; i < connection_count_; ++i) {
    if (OB_FAIL(lock_txn_stmts_.at(i).close())) {
      TESTBENCH_LOG(WARN, "close lock transaction statement failed", "conn_idx", i, K(ret));
    } else if (OB_FAIL(mysql_proxy_->release_conn(0, commits_.at(i), connections_.at(i)))) {
      TESTBENCH_LOG(ERROR, "release dblink failed", K(ret));
    }
  }
  return ret;
}

int ObDistributedTransactionTask::init() {
  int ret = OB_SUCCESS;
  if (OB_FAIL(ObIWorkloadTransactionTask::init())) {
    TESTBENCH_LOG(ERROR, "init base transaction task failed", K(ret));
  } else if (OB_FAIL(lock_txn_stmts_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate statements array failed", K(ret));
  } else if (OB_FAIL(lock_txn_sql_.assign_fmt(lock_txn_sql_format_, table_name_))) {
    TESTBENCH_LOG(ERROR, "get format lock_txn_sql failed", K(ret), K(lock_txn_sql_));
  } else {
    TESTBENCH_LOG(INFO, "ObDistributedTransactionTask allocate stmt succeed");
    for (int64_t i = 0; i < connection_count_; ++i) {
      if (OB_SUCC(ret) && OB_FAIL(connections_.at(i)->prepare_statement(lock_txn_stmts_.at(i), lock_txn_sql_.ptr()))) {
        TESTBENCH_LOG(ERROR, "prepare statement failed", KR(ret), "sql", lock_txn_sql_.ptr());
      }
    }
  }
  return ret;
}

int ObDistributedTransactionTask::execute_transactions() {
  int ret = OB_SUCCESS;
  int64_t current_partition_idx = 0;
  int64_t current_row_id = row_id_start_;
  int64_t partition_count = partition_id_.count();
  // begin transactions
  for (int64_t conn_idx = 0; OB_SUCC(ret) && conn_idx < connection_count_; ++conn_idx) {
    cumulative_latencys_.at(conn_idx) = common::ObTimeUtility::current_time();
    if (OB_FAIL(connections_.at(conn_idx)->start_transaction_async())) {
      TESTBENCH_LOG(WARN, "start transaction async failed", K(ret));
      commits_.at(conn_idx) = false;
    }
  }
  // execute commands
  for (int64_t i = 0; i < operations_; ++i) {
    for (int64_t conn_idx = 0; OB_SUCC(ret) && conn_idx < connection_count_; ++conn_idx) {
      ObMySQLPreparedStatement &stmt = lock_txn_stmts_.at(conn_idx);
      if (OB_FAIL(wait_and_bind_param(conn_idx, partition_id_.at(current_partition_idx), current_row_id, stmt))) {
        TESTBENCH_LOG(WARN, "wait and bind parameters failed", K(ret));
      } else if (OB_FAIL(connections_.at(conn_idx)->execute_write_async(stmt))) {
        TESTBENCH_LOG(WARN, "execute write async failed", K(ret));
      } else {
        current_partition_idx = (current_partition_idx + 1) % partition_count;
        current_row_id += 1;
      }
      if (OB_FAIL(ret)) {
        commits_.at(conn_idx) = false;
      }
    }
  }
  // always commit or rollback transactions
  for (int64_t conn_idx = 0; OB_SUCC(ret) && conn_idx < connection_count_; ++conn_idx) {
    if (OB_FAIL(wait_for_connection(connections_.at(conn_idx)))) {
      TESTBENCH_LOG(WARN, "wait for the connection get error", K(ret));
    } else {
      latencys_.at(conn_idx) = common::ObTimeUtility::current_time();
      if (OB_FAIL(commits_.at(conn_idx) ? connections_.at(conn_idx)->commit_async() : connections_.at(conn_idx)->rollback_async())) {
        TESTBENCH_LOG(WARN, "execute commit/rollback async failed", K(ret), "is_commit", commits_.at(conn_idx));
      }
    }
    if (OB_FAIL(ret)) {
      commits_.at(conn_idx) = false;
    }
  }
  // collect statistics
  for (int64_t conn_idx = 0; OB_SUCC(ret) && conn_idx < connection_count_; ++conn_idx) {
    if (OB_FAIL(wait_for_connection(connections_.at(conn_idx)))) {
      TESTBENCH_LOG(WARN, "wait for the connection get error", K(ret));
      commits_.at(conn_idx) = false;
    } else {
      latencys_.at(conn_idx) = common::ObTimeUtility::current_time() - latencys_.at(conn_idx);
      cumulative_latencys_.at(conn_idx) = common::ObTimeUtility::current_time() - cumulative_latencys_.at(conn_idx);
      if (OB_UNLIKELY(false == commits_.at(conn_idx))) {
        if (OB_FAIL(record_latency(ObLatencyTaskType::ROLLBACK_TXN_LATENCY_TASK, cumulative_latencys_.at(conn_idx)))) {
          TESTBENCH_LOG(WARN, "record the latency of rollback transaction failed", KR(ret), "txn_latency", cumulative_latencys_.at(conn_idx));          
        }
      } else if (OB_FAIL(record_latency(ObLatencyTaskType::COMMIT_SQL_LATENCY_TASK, latencys_.at(conn_idx)))) {
        TESTBENCH_LOG(WARN, "record the latency of commit sql failed", KR(ret), "commit_latency", latencys_.at(conn_idx));
      } else if (OB_FAIL(record_latency(ObLatencyTaskType::DISTRIBUTED_TXN_LATENCY_TASK, cumulative_latencys_.at(conn_idx)))) {
        TESTBENCH_LOG(WARN, "record the latency of distributed transaction failed", KR(ret), "txn_latency", cumulative_latencys_.at(conn_idx));
      } else {
        TESTBENCH_LOG(TRACE, "distributed transaction commit success", "commit_latency", latencys_.at(conn_idx), "txn_latency", cumulative_latencys_.at(conn_idx));
      }
    }
  }
  return ret;
}

WorkloadType ObDistributedTransactionTask::get_type() {
  return WorkloadType::DISTRIBUTED_TRANSACTION;
}

/*
                                        ObContentionTransactionTask
*/
ObContentionTransactionTask::ObContentionTransactionTask(BasicTaskConfig basic_config, int64_t aborts, int64_t operations) 
: ObIWorkloadTransactionTask(basic_config), aborts_(aborts), operations_(operations), lock_elr_sql_()
{
  lock_elr_sql_format_ = "UPDATE %s SET lock_elr = lock_elr + 1 WHERE partition_id = ? AND row_id = ?;";
}

ObContentionTransactionTask::~ObContentionTransactionTask() {}

int ObContentionTransactionTask::init() {
  int ret = OB_SUCCESS;
  if (OB_FAIL(ObIWorkloadTransactionTask::init())) {
    TESTBENCH_LOG(ERROR, "init base transaction task failed", K(ret));
  } else if (OB_FAIL(lock_elr_stmts_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate statements array failed", K(ret));
  } else if (OB_FAIL(lock_elr_sql_.assign_fmt(lock_elr_sql_format_, table_name_))) {
    TESTBENCH_LOG(ERROR, "get format lock_elr_sql_ failed", K(ret), K(lock_elr_sql_));
  } else {
    for (int64_t i = 0; i < connection_count_; ++i) {
      if (OB_SUCC(ret) && OB_FAIL(connections_.at(i)->prepare_statement(lock_elr_stmts_.at(i), lock_elr_sql_.ptr()))) {
        TESTBENCH_LOG(ERROR, "prepare statement failed", KR(ret), "sql", lock_elr_sql_.ptr());
      }
    }
  }
  return ret;
}

int ObContentionTransactionTask::execute_transactions() {
  int ret = OB_SUCCESS;
  int64_t partition_id = partition_id_.at(0);
  int64_t contention_row_id = row_id_start_;
  int64_t current_row_id = row_id_start_ + 1;
  // begin transactions
  for (int64_t conn_idx = 0; OB_SUCC(ret) && conn_idx < connection_count_; ++conn_idx) {
    cumulative_latencys_.at(conn_idx) = common::ObTimeUtility::current_time();
    if (OB_FAIL(connections_.at(conn_idx)->start_transaction_async(false))) {
      TESTBENCH_LOG(WARN, "start transaction async failed", K(ret));
      commits_.at(conn_idx) = false;
    }
  }
  // execute non-contention commands
  for (int64_t i = 0; i < operations_ - 1; ++i) {
    for (int64_t conn_idx = 0; OB_SUCC(ret) && conn_idx < connection_count_; ++conn_idx) {
      ObMySQLPreparedStatement &stmt = lock_elr_stmts_.at(conn_idx);
      if (OB_FAIL(wait_and_bind_param(conn_idx, partition_id, current_row_id, stmt))) {
        TESTBENCH_LOG(WARN, "wait and bind parameters failed", K(ret));
      } else if (OB_FAIL(connections_.at(conn_idx)->execute_write_async(stmt))) {
        TESTBENCH_LOG(WARN, "execute write async failed", K(ret));
      } else {
        current_row_id += 1;
      }
    }
  }
  // execute contention command
  for (int64_t conn_idx = 0; OB_SUCC(ret) && conn_idx < connection_count_; ++conn_idx) {
    ObMySQLPreparedStatement &stmt = lock_elr_stmts_.at(conn_idx);
    if (OB_FAIL(wait_and_bind_param(conn_idx, partition_id, contention_row_id, stmt))) {
      TESTBENCH_LOG(WARN, "wait and bind parameters failed", K(ret));
    } else {
      latencys_.at(conn_idx) = common::ObTimeUtility::current_time();
      if (OB_FAIL(connections_.at(conn_idx)->execute_write_async(stmt))) {
        TESTBENCH_LOG(WARN, "execute write async failed", K(ret));
      }
    }
  }
  // abort transactions
  int64_t aborts = aborts_;
  for (int64_t conn_idx = 0; OB_SUCC(ret) && conn_idx < aborts; ++conn_idx) {
    if (OB_FAIL(wait_for_connection(connections_.at(conn_idx)))) {
      TESTBENCH_LOG(WARN, "wait for the connection get error", K(ret));
    } else if (OB_FAIL(connections_.at(conn_idx)->rollback_async())) {
      TESTBENCH_LOG(WARN, "rollback transaction async failed", K(ret));
    } else {
      commits_.at(conn_idx) = false;
    }
  }
  // commit transactions
  for (int64_t conn_idx = aborts; OB_SUCC(ret) && conn_idx < connection_count_; ++conn_idx) {
    if (OB_FAIL(wait_for_connection(connections_.at(conn_idx)))) {
      TESTBENCH_LOG(WARN, "wait for the connection get error", K(ret));
    } else {
      latencys_.at(conn_idx) = common::ObTimeUtility::current_time() - latencys_.at(conn_idx);
      if (OB_FAIL(connections_.at(conn_idx)->commit_async())) {
        TESTBENCH_LOG(WARN, "commit transaction async failed", K(ret));
      }
    }
  }
  // collect statistics
  for (int64_t conn_idx = 0; OB_SUCC(ret) && conn_idx < connection_count_; ++conn_idx) {
    if (OB_FAIL(wait_for_connection(connections_.at(conn_idx)))) {
      TESTBENCH_LOG(WARN, "wait for the connection get error", K(ret));
      commits_.at(conn_idx) = false;
    } else {
      cumulative_latencys_.at(conn_idx) = common::ObTimeUtility::current_time() - cumulative_latencys_.at(conn_idx);
      if (OB_UNLIKELY(false == commits_.at(conn_idx))) {
        if (OB_FAIL(record_latency(ObLatencyTaskType::ROLLBACK_TXN_LATENCY_TASK, cumulative_latencys_.at(conn_idx)))) {
          TESTBENCH_LOG(WARN, "record the latency of rollback transaction failed", KR(ret), "txn_latency", cumulative_latencys_.at(conn_idx));
        }
      } else if (OB_FAIL(record_latency(ObLatencyTaskType::LOCK_SQL_LATENCY_TASK, latencys_.at(conn_idx)))) {
        TESTBENCH_LOG(WARN, "record the latency of commit sql failed", KR(ret), "lock_latency", latencys_.at(conn_idx));
      } else if (OB_FAIL(record_latency(ObLatencyTaskType::CONTENTION_TXN_LATENCY_TASK, latencys_.at(conn_idx)))) {
        TESTBENCH_LOG(WARN, "record the latency of contention transaction failed", KR(ret), "txn_latency", cumulative_latencys_.at(conn_idx));
      } else {
        TESTBENCH_LOG(TRACE, "contention transaction commit success", "lock_latency", latencys_.at(conn_idx), "txn_latency", cumulative_latencys_.at(conn_idx));
      }
    }
  }
  return ret;
}

int ObContentionTransactionTask::release_dblinks() {
  int ret = OB_SUCCESS;
  for (int64_t i = 0; i < connection_count_; ++i) {
    if (OB_FAIL(lock_elr_stmts_.at(i).close())) {
      TESTBENCH_LOG(WARN, "close lock transaction statement failed", "conn_idx", i, K(ret));
    } else if (OB_FAIL(mysql_proxy_->release_conn(0, commits_.at(i), connections_.at(i)))) {
      TESTBENCH_LOG(ERROR, "release dblink failed", K(ret));
    }
  }
  return ret;
}

WorkloadType ObContentionTransactionTask::get_type() {
  return WorkloadType::CONTENTION;
}

/*
                                        ObDeadlockTransactionTask
*/
ObDeadlockTransactionTask::ObDeadlockTransactionTask(BasicTaskConfig basic_config, int64_t chains) 
: ObIWorkloadTransactionTask(basic_config), chains_(chains), lock_lcl_sql_()
{
  lock_lcl_sql_format_ = "UPDATE %s SET lock_lcl = lock_lcl + 1 WHERE partition_id = ? AND row_id = ?;";
}

ObDeadlockTransactionTask::~ObDeadlockTransactionTask() {}

int ObDeadlockTransactionTask::init() {
  int ret = OB_SUCCESS;
  if (OB_FAIL(ObIWorkloadTransactionTask::init())) {
    TESTBENCH_LOG(ERROR, "init base transaction task failed", K(ret));
  } else if (OB_FAIL(lock_lcl_stmts_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate statements array failed", K(ret));
  } else if (OB_FAIL(lock_lcl_sql_.assign_fmt(lock_lcl_sql_format_, table_name_))) {
    TESTBENCH_LOG(ERROR, "get format lock_lcl_sql_ failed", K(ret), K(lock_lcl_sql_));
  } else {
    for (int64_t i = 0; i < connection_count_; ++i) {
      if (OB_SUCC(ret) && OB_FAIL(connections_.at(i)->prepare_statement(lock_lcl_stmts_.at(i), lock_lcl_sql_.ptr()))) {
        TESTBENCH_LOG(ERROR, "prepare statement failed", KR(ret), "sql", lock_lcl_sql_.ptr());
      }
    }
  }
  return ret;
}

int ObDeadlockTransactionTask::execute_transactions() {
  return OB_UNIMPLEMENTED_FEATURE;
}

int ObDeadlockTransactionTask::release_dblinks() {
  int ret = OB_SUCCESS;
  for (int64_t i = 0; i < connection_count_; ++i) {
    if (OB_FAIL(lock_lcl_stmts_.at(i).close())) {
      TESTBENCH_LOG(WARN, "close lock transaction statement failed", "conn_idx", i, K(ret));
    } else if (OB_FAIL(mysql_proxy_->release_conn(0, commits_.at(i), connections_.at(i)))) {
      TESTBENCH_LOG(ERROR, "release dblink failed", K(ret));
    }
  }
  return ret;
}

WorkloadType ObDeadlockTransactionTask::get_type() {
  return WorkloadType::DEADLOCK;
}
} // namespace testbench
} // namespace oceanbase