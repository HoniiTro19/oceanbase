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
  parameters_(config.parameters),
  dblinks_(config.dblinks),
  row_id_start_(config.row_id_start),
  allocator_("TxnTask")
{}

ObIWorkloadTransactionTask::~ObIWorkloadTransactionTask() 
{
  latencys_.release_array(allocator_);
  cumulative_latencys_.release_array(allocator_);
  commits_.release_array(allocator_);
  connections_.release_array(allocator_);
  pollfds_.release_array(allocator_);
}

void ObIWorkloadTransactionTask::begin_trace_latency(int64_t &latency) 
{
  latency = common::ObTimeUtility::current_time();
}

void ObIWorkloadTransactionTask::end_trace_latency(int64_t &latency) 
{
  int64_t end_time = common::ObTimeUtility::current_time();
  latency = end_time - latency;
}

int ObIWorkloadTransactionTask::prepare_arrays() 
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(connection_count_ <= 0)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "the connection count is not inited", KR(ret), K(connection_count_));
  } else if (OB_FAIL(latencys_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate latencys array failed", KR(ret));
  } else if (OB_FAIL(cumulative_latencys_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate cumulative latencys array failed", KR(ret));
  } else if (OB_FAIL(commits_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate commits array failed", KR(ret));
  } else if (OB_FAIL(connections_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate connections array failed", KR(ret));
  } else {
    for (int64_t i = 0; i < connection_count_; ++i) {
      latencys_.at(i) = 0;
      cumulative_latencys_.at(i) = 0;
      commits_.at(i) = true;
      if (OB_FAIL(mysql_proxy_->get_mysql_conn(dblinks_.at(i), 0, connections_.at(i)))) {
        TESTBENCH_LOG(ERROR, "acquire dblink connection failed", KR(ret), K_(dblinks));
      }
    }
  }
  return ret;
}

int ObIWorkloadTransactionTask::init() 
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(mysql_proxy_)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "the connection pool is not inited", KR(ret), KP(mysql_proxy_));
  } else if (OB_ISNULL(latency_collector_)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "the latency collector is not inited", KR(ret), KP(latency_collector_));
  } else if (OB_FAIL(prepare_arrays())) {
    TESTBENCH_LOG(ERROR, "acquire mysql connections failed", KR(ret));
  }
  return ret;
}

int ObIWorkloadTransactionTask::init_pollfds()
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(pollfds_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate pollfds array failed", KR(ret));
  } else {
    for (int64_t i = 0; i < connection_count_; ++i) {
      ObMySQLConnection *conn = connections_.at(i);
      pollfds_.at(i).fd = mysql_get_socket(conn->get_handler());
    }
  }
  return ret;
}

int ObIWorkloadTransactionTask::wait_for_connection_block(int64_t conn_idx)
{
  int ret = OB_SUCCESS;
  ObMySQLConnection *conn = connections_.at(conn_idx);
  while (conn->get_async_status()) {
    if (OB_FAIL(conn->wait_for_async_status()) && ret != OB_NEED_RETRY) {
      TESTBENCH_LOG(ERROR, "get error while wait for connection async status", KR(ret));
      // the conenction status should be set to ConnStatus::ERROR now
    }
  }
  return ret;
}

int ObIWorkloadTransactionTask::find_connection_ready_round_robin(int64_t &conn_idx)
{
  int ret = OB_SUCCESS;
  ObMySQLConnection *conn = nullptr;
  while (true) {
    conn = connections_.at(conn_idx);
    if (ObMySQLConnection::ConnStatus::PENDING == conn->get_conn_status()) {
      if (OB_FAIL(conn->wait_for_async_status(0))) {
        if (OB_UNLIKELY(ret != OB_NEED_RETRY)) {
          TESTBENCH_LOG(ERROR, "get error while wait for connection async status", KR(ret));
        }
      } else {
        break;
      }
    }
    conn_idx = (conn_idx + 1) % connection_count_;
  }
  return ret;
}

int ObIWorkloadTransactionTask::find_connection_ready_multiplexing(int64_t &conn_idx)
{
  int ret = OB_SUCCESS;
  ObMySQLConnection *conn = nullptr;
  if (OB_FAIL(find_mysql_ready(conn_idx))) {
    TESTBENCH_LOG(ERROR, "poll IO muliplexing timeout", KR(ret));
  } else if (FALSE_IT(conn = connections_.at(conn_idx))) {
    // impossible
  } else if (OB_FAIL(conn->run_contfunc_and_update_status())) {
    TESTBENCH_LOG(ERROR, "get error while update connection async status", KR(ret));
  }
  return ret;
}

int ObIWorkloadTransactionTask::find_mysql_ready(int64_t &conn_idx)
{
  int ret = OB_SUCCESS;
  unsigned int poll_timeout = 0;
  for (int64_t i = 0; i < connection_count_; ++i) {
    ObMySQLConnection *conn = connections_.at(i);
    int async_status = conn->get_async_status();
    MYSQL *mysql = conn->get_handler();
    poll_timeout = max(poll_timeout, mysql_get_timeout_value_ms(mysql));
    pollfds_.at(i).events = (async_status & MYSQL_WAIT_READ ? POLLIN : 0) |
                            (async_status & MYSQL_WAIT_WRITE ? POLLOUT : 0) |
                            (async_status & MYSQL_WAIT_EXCEPT ? POLLPRI : 0);
  }
  int res;
  res = poll(pollfds_.get_data(), connection_count_, poll_timeout);
  if (res <= 0) {
    ret = OB_TIMEOUT;
    for (int64_t i = 0; i < connection_count_; ++i) {
      ObMySQLConnection *conn = connections_.at(i);
      conn->set_async_status(MYSQL_WAIT_TIMEOUT);
    }
  } else {
    for (int64_t i = 0; i < connection_count_; ++i) {
      int async_status = (pollfds_.at(i).revents & POLLIN ? MYSQL_WAIT_READ : 0) |
                         (pollfds_.at(i).revents & POLLOUT ? MYSQL_WAIT_WRITE : 0) | 
                         (pollfds_.at(i).revents & POLLPRI ? MYSQL_WAIT_EXCEPT : 0);
      if (async_status) {
        ObMySQLConnection *conn = connections_.at(i);
        conn->set_async_status(async_status);
        conn_idx = i;
        break;
      }
    }
  }
  return ret;
}

int ObIWorkloadTransactionTask::record_latency(ObLatencyTaskType type, int64_t latency) 
{
  int ret = OB_SUCCESS;
  if (latency == 0) {
    return ret;
  }
  ObLatencyTask *task = nullptr;
  common::ObObj latency_obj;
  latency_obj.set_int(latency);
  // TODO: use the right allocator
  // void *buf = nullptr;
  // if (OB_ISNULL(buf = allocator_.alloc(sizeof(ObLatencyTask)))) {
  //   ret = OB_ALLOCATE_MEMORY_FAILED;
  //   TESTBENCH_LOG(ERROR, "allocate memory failed", KR(ret));
  // } else if (OB_ISNULL(task = new(buf)ObLatencyTask(type, latency_obj))) {
  //   TESTBENCH_LOG(ERROR, "create new latency task failed", KR(ret), K(type));
  if (OB_ISNULL(task = new ObLatencyTask(type, latency_obj))) {
    TESTBENCH_LOG(ERROR, "create new latency task failed", KR(ret), K(type));
  } else if (OB_FAIL(latency_collector_->push_latency_task(task))) {
    TESTBENCH_LOG(ERROR, "push latency task failed", KR(ret), K(latency_obj));
  }
  return ret;
}

int ObIWorkloadTransactionTask::bind_params(int64_t &partition_id, int64_t &row_id, ObMySQLPreparedStatement &stmt)
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(stmt.bind_param_int(0, &partition_id))) {
    TESTBENCH_LOG(WARN, "bind the first parameter of partition id failed", KR(ret), K(partition_id));
  } else if (OB_FAIL(stmt.bind_param_int(1, &row_id))) {
    TESTBENCH_LOG(WARN, "bind the first parameter of row id failed", KR(ret), K(row_id));
  }
  return ret;
}

int ObIWorkloadTransactionTask::bind_multi_params(int64_t &partition_id0, int64_t &row_id0, int64_t &partition_id1, int64_t &row_id1, ObMySQLPreparedStatement &stmt)
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(stmt.bind_param_int(0, &partition_id0))) {
    TESTBENCH_LOG(WARN, "bind the first parameter of partition id failed", KR(ret), K(partition_id0));
  } else if (OB_FAIL(stmt.bind_param_int(1, &row_id0))) {
    TESTBENCH_LOG(WARN, "bind the first parameter of row id failed", KR(ret), K(row_id0));
  } else if (OB_FAIL(stmt.bind_param_int(2, &partition_id1))) {
    TESTBENCH_LOG(WARN, "bind the second parameter of partition id failed", KR(ret), K(partition_id1));
  } else if (OB_FAIL(stmt.bind_param_int(3, &row_id1))) {
    TESTBENCH_LOG(WARN, "bind the second parameter of row id failed", KR(ret), K(row_id1));
  }
  return ret;
}

int ObIWorkloadTransactionTask::bind_result(int64_t &col, ObMySQLPreparedStatement &stmt)
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(stmt.bind_result_int(0, &col))) {
    TESTBENCH_LOG(WARN, "bind the result column failed", KR(ret));
  }
  return ret;
}

/*
                                        ObDistributedTransactionTask
*/
ObDistributedTransactionTask::ObDistributedTransactionTask(BasicTaskConfig basic_config, int64_t participants, int64_t operations)
: ObIWorkloadTransactionTask(basic_config), participants_(participants), operations_(operations), lock_dist_sql_()
{
  lock_dist_sql_format_ = "UPDATE %s SET lock_dist = lock_dist + 1 WHERE partition_id = ? AND row_id = ?;";
}

ObDistributedTransactionTask::~ObDistributedTransactionTask() 
{
  lock_dist_stmts_.release_array(allocator_);
}

int ObDistributedTransactionTask::release_dblinks() 
{
  int ret = OB_SUCCESS;
  for (int64_t i = 0; i < connection_count_; ++i) {
    if (OB_FAIL(lock_dist_stmts_.at(i).close())) {
      TESTBENCH_LOG(WARN, "close distributed transaction statement failed", "conn_idx", i, KR(ret));
    } else if (OB_FAIL(mysql_proxy_->release_conn(0, commits_.at(i), connections_.at(i)))) {
      TESTBENCH_LOG(ERROR, "release dblink failed", KR(ret));
    }
  }
  lock_dist_stmts_.release_array(allocator_);
  return ret;
}

int ObDistributedTransactionTask::init() 
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(ObIWorkloadTransactionTask::init())) {
    TESTBENCH_LOG(ERROR, "init base transaction task failed", KR(ret));
  } else if (OB_FAIL(lock_dist_stmts_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate statements array failed", KR(ret));
  } else if (OB_FAIL(lock_dist_sql_.assign_fmt(lock_dist_sql_format_, table_name_))) {
    TESTBENCH_LOG(ERROR, "get format lock_txn_sql failed", KR(ret), K(lock_dist_sql_));
  } else {
    for (int64_t i = 0; i < connection_count_; ++i) {
      if (OB_FAIL(connections_.at(i)->prepare_statement(lock_dist_stmts_.at(i), lock_dist_sql_.ptr()))) {
        TESTBENCH_LOG(ERROR, "prepare statement failed", KR(ret), "sql", lock_dist_sql_.ptr());
      }
    }
  }
  return ret;
}

int ObDistributedTransactionTask::execute_transactions() 
{
  int ret = OB_SUCCESS;
  int64_t current_partition_id = 0;
  int64_t current_row_id = row_id_start_;
  int64_t partition_count = parameters_.count();
  // NOTE: there should be only one connection (transaction) in the distributed transaction task 
  UNUSED(connection_count_);
  int64_t conn_idx = 0;
  // begin transaction
  if (FALSE_IT(begin_trace_latency(cumulative_latencys_.at(conn_idx)))) {
    // impossible
  } else if (OB_FAIL(connections_.at(conn_idx)->start_transaction_async(true))) {
    TESTBENCH_LOG(WARN, "start transaction async failed", KR(ret));
    return ret;
  }
  // execute commands
  for (int64_t i = 0; i < operations_; ++i) {
    ObMySQLPreparedStatement &stmt = lock_dist_stmts_.at(conn_idx);
    if (OB_FAIL(wait_for_connection_block(conn_idx))) {
      TESTBENCH_LOG(WARN, "wait for connection block failed", KR(ret));
    } else if (OB_FAIL(bind_params(parameters_.at(current_partition_id), current_row_id, stmt))) {
      TESTBENCH_LOG(WARN, "bind parameters for statement failed", KR(ret), "partition", parameters_.at(current_partition_id), "row", current_row_id);
    } else if (OB_FAIL(connections_.at(conn_idx)->execute_write_async(stmt))) {
      TESTBENCH_LOG(WARN, "execute write async failed", KR(ret));
    } else {
      current_partition_id = (current_partition_id + 1) % partition_count;
      current_row_id += 1;
    }
    if (OB_FAIL(ret)) {
      commits_.at(conn_idx) = false;
    }
  }
  // commit transaction 
  if (OB_FAIL(wait_for_connection_block(conn_idx))) {
    commits_.at(conn_idx) = false;
    TESTBENCH_LOG(WARN, "wait for the connection block get error", KR(ret));
  }
  begin_trace_latency(latencys_.at(conn_idx));
  if (OB_FAIL(commits_.at(conn_idx) ? connections_.at(conn_idx)->commit_async() : connections_.at(conn_idx)->rollback_async())) {
    TESTBENCH_LOG(WARN, "execute commit/rollback async failed", KR(ret), "is_commit", commits_.at(conn_idx));
  }
  // wait for the transaction finish
  if (OB_FAIL(wait_for_connection_block(conn_idx))) {
    commits_.at(conn_idx) = false;
    TESTBENCH_LOG(WARN, "wait for the connection block get error", KR(ret));
  }
  end_trace_latency(latencys_.at(conn_idx));
  end_trace_latency(cumulative_latencys_.at(conn_idx));
  return ret;
}

int ObDistributedTransactionTask::collect_statistics()
{
  int ret = OB_SUCCESS;
  for (int64_t conn_idx = 0; conn_idx < connection_count_; ++conn_idx) {
    if (OB_UNLIKELY(false == commits_.at(conn_idx))) {
      if (OB_FAIL(record_latency(ObLatencyTaskType::ROLLBACK_TXN_LATENCY_TASK, cumulative_latencys_.at(conn_idx)))) {
        TESTBENCH_LOG(WARN, "record the latency of rollback transaction failed", KR(ret), "txn_latency", cumulative_latencys_.at(conn_idx));          
      }
    } else if (OB_FAIL(record_latency(ObLatencyTaskType::COMMIT_SQL_LATENCY_TASK, latencys_.at(conn_idx)))) {
      TESTBENCH_LOG(WARN, "record the latency of commit sql failed", KR(ret), "commit_latency", latencys_.at(conn_idx));
    } else if (OB_FAIL(record_latency(ObLatencyTaskType::DISTRIBUTED_TXN_LATENCY_TASK, cumulative_latencys_.at(conn_idx)))) {
      TESTBENCH_LOG(WARN, "record the latency of distributed transaction failed", KR(ret), "txn_latency", cumulative_latencys_.at(conn_idx));
    } else {
      TESTBENCH_LOG(TRACE, "distributed transaction finish", "commit_latency", latencys_.at(conn_idx), "txn_latency", cumulative_latencys_.at(conn_idx));
    }
  }
  return ret;
}

WorkloadType ObDistributedTransactionTask::get_type() 
{
  return WorkloadType::DISTRIBUTED_TRANSACTION;
}

/*
                                        ObContentionTransactionTask
*/
ObContentionTransactionTask::ObContentionTransactionTask(BasicTaskConfig basic_config, int64_t aborts, int64_t operations) 
: ObIWorkloadTransactionTask(basic_config), aborts_(aborts), operations_(operations), lock_contention_sql_(), finished_(0), act_commits_(0)
{
  lock_contention_sql_format_ = "UPDATE %s SET lock_elr = lock_elr + 1 WHERE partition_id = ? AND row_id = ?;";
}

ObContentionTransactionTask::~ObContentionTransactionTask() 
{
  lock_contention_stmts_.release_array(allocator_);
  act_operations_.release_array(allocator_);
}

int ObContentionTransactionTask::init() 
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(ObIWorkloadTransactionTask::init())) {
    TESTBENCH_LOG(ERROR, "init base transaction task failed", KR(ret));
  } else if (OB_FAIL(ObIWorkloadTransactionTask::init_pollfds())) {
    TESTBENCH_LOG(ERROR, "init pollfds failed", KR(ret));
  } else if (OB_FAIL(lock_contention_stmts_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate statements array failed", KR(ret));
  } else if (OB_FAIL(act_operations_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate actual operations array failed", KR(ret));
  } else if (OB_FAIL(lock_contention_sql_.assign_fmt(lock_contention_sql_format_, table_name_))) {
    TESTBENCH_LOG(ERROR, "get format lock_contention_sql_ failed", KR(ret), K_(lock_contention_sql));
  } else {
    for (int64_t i = 0; i < connection_count_; ++i) {
      if (OB_FAIL(connections_.at(i)->prepare_statement(lock_contention_stmts_.at(i), lock_contention_sql_.ptr()))) {
        TESTBENCH_LOG(ERROR, "prepare statement failed", KR(ret), "sql", lock_contention_sql_.ptr());
      } else {
        act_operations_.at(i) = 0;
      }
    }
  }
  return ret;
}

int ObContentionTransactionTask::execute_transactions() 
{
  int ret = OB_SUCCESS;
  int64_t current_row_id = row_id_start_ + 1;
  // begin transactions
  for (int64_t conn_idx = 0; conn_idx < connection_count_; ++conn_idx) {
    begin_trace_latency(cumulative_latencys_.at(conn_idx));
    if (OB_FAIL(connections_.at(conn_idx)->start_transaction_async(false))) {
      TESTBENCH_LOG(WARN, "start transaction async failed", KR(ret));
      return ret;
    }
  }
  // start transaction async may return without async
  for (int64_t conn_idx = 0; conn_idx < connection_count_; ++conn_idx) {
    ObMySQLConnection *conn = connections_.at(conn_idx);
    if (OB_FAIL(push_forward_operations(conn_idx, current_row_id))) {
      commits_.at(conn_idx) = false;
      TESTBENCH_LOG(WARN, "push forward transaction operations failed", KR(ret));
    }
  }
  // execute commands
  int64_t conn_idx = 0;
  while (finished_ < connection_count_) {
    if (OB_FAIL(find_connection_ready_multiplexing(conn_idx))) {
      if (ret == OB_TIMEOUT) {
        TESTBENCH_LOG(WARN, "operations timeout", KR(ret));
        return ret;
      } else if (ret != OB_NEED_RETRY) {
        commits_.at(conn_idx) = false;
        TESTBENCH_LOG(WARN, "find connection ready failed", KR(ret), K(conn_idx));
      }
    }
    if (OB_FAIL(push_forward_operations(conn_idx, current_row_id))) {
      TESTBENCH_LOG(WARN, "push forward transaction operations failed", KR(ret));
    }
    TESTBENCH_LOG(DEBUG, "check connections operations status", K_(act_operations));
  }
  return ret;
}

int ObContentionTransactionTask::collect_statistics()
{
  int ret = OB_SUCCESS;
  for (int64_t conn_idx = 0; conn_idx < connection_count_; ++conn_idx) {
    if (false == commits_.at(conn_idx)) {
      if (OB_FAIL(record_latency(ObLatencyTaskType::ROLLBACK_TXN_LATENCY_TASK, cumulative_latencys_.at(conn_idx)))) {
        TESTBENCH_LOG(WARN, "record the latency of rollback transaction failed", KR(ret), "txn_latency", cumulative_latencys_.at(conn_idx));
      }
    } else if (OB_FAIL(record_latency(ObLatencyTaskType::LOCK_SQL_LATENCY_TASK, latencys_.at(conn_idx)))) {
      TESTBENCH_LOG(WARN, "record the latency of commit sql failed", KR(ret), "lock_latency", latencys_.at(conn_idx));
    } else if (OB_FAIL(record_latency(ObLatencyTaskType::CONTENTION_TXN_LATENCY_TASK, cumulative_latencys_.at(conn_idx)))) {
      TESTBENCH_LOG(WARN, "record the latency of contention transaction failed", KR(ret), "txn_latency", cumulative_latencys_.at(conn_idx));
    } else {
      TESTBENCH_LOG(TRACE, "contention transaction finish", "lock_latency", latencys_.at(conn_idx), "txn_latency", cumulative_latencys_.at(conn_idx));
    }
  }
  return ret;
}

int ObContentionTransactionTask::release_dblinks() 
{
  int ret = OB_SUCCESS;
  for (int64_t i = 0; i < connection_count_; ++i) {
    if (OB_FAIL(lock_contention_stmts_.at(i).close())) {
      TESTBENCH_LOG(WARN, "close contention transaction statement failed", "conn_idx", i, KR(ret));
    } else if (OB_FAIL(mysql_proxy_->release_conn(0, commits_.at(i), connections_.at(i)))) {
      TESTBENCH_LOG(ERROR, "release dblink failed", KR(ret));
    }
  }
  return ret;
}

WorkloadType ObContentionTransactionTask::get_type() 
{
  return WorkloadType::CONTENTION;
}

int ObContentionTransactionTask::push_forward_operations(int64_t conn_idx, int64_t &current_row_id)
{
  int ret = OB_SUCCESS;
  ObMySQLConnection *conn = connections_.at(conn_idx);
  while (!conn->get_async_status()) {
    if (act_operations_.at(conn_idx) == 0) {
      // execute contention command
      int64_t contention_partition_id = 0;
      int64_t contention_row_id = row_id_start_;
      ObMySQLPreparedStatement &stmt = lock_contention_stmts_.at(conn_idx);
      ObMySQLConnection *conn = connections_.at(conn_idx);
      begin_trace_latency(latencys_.at(conn_idx));
      if (OB_FAIL(bind_params(parameters_.at(contention_partition_id), contention_row_id, stmt))) {
        TESTBENCH_LOG(WARN, "bind parameters for statement failed", KR(ret), "partition", parameters_.at(contention_partition_id), "row", contention_row_id);
      } else if (OB_FAIL(connections_.at(conn_idx)->execute_write_async(stmt))) {
        TESTBENCH_LOG(WARN, "execute write async failed", KR(ret));
      }
      TESTBENCH_LOG(TRACE, "execute contention row", K(conn_idx), K_(act_operations));
      ++act_operations_.at(conn_idx);
      if (OB_FAIL(ret)) {
        commits_.at(conn_idx) = false;
      }
    } else if (act_operations_.at(conn_idx) < operations_) {
      if (act_operations_.at(conn_idx) == 1) {
        end_trace_latency(latencys_.at(conn_idx));
      }
      ObMySQLPreparedStatement &stmt = lock_contention_stmts_.at(conn_idx);
      ObMySQLConnection *conn = connections_.at(conn_idx);
      if (OB_FAIL(bind_params(parameters_.at(conn_idx), current_row_id, stmt))) {
        TESTBENCH_LOG(WARN, "bind parameters for statement failed", KR(ret), "partition", parameters_.at(conn_idx), "row", current_row_id);
      } else if (OB_FAIL(connections_.at(conn_idx)->execute_write_async(stmt))) {
        TESTBENCH_LOG(WARN, "execute write async failed", KR(ret));
      }
      TESTBENCH_LOG(TRACE, "execute non-contention row", K(conn_idx), K(current_row_id), K_(act_operations), K_(latencys), K_(cumulative_latencys));
      current_row_id += 1;
      ++act_operations_.at(conn_idx);
      if (OB_FAIL(ret)) {
        commits_.at(conn_idx) = false;
      }
    } else if (act_operations_.at(conn_idx) == operations_) {
      if (act_operations_.at(conn_idx) == 1) {
        end_trace_latency(latencys_.at(conn_idx));
      }
      // commit or abort transactions
      int64_t commits = connection_count_ - aborts_;
      bool commit = commits_.at(conn_idx) && act_commits_ < commits;
      if (commit) {
        if (OB_FAIL(connections_.at(conn_idx)->commit_async())) {
          commits_.at(conn_idx) = false;
          TESTBENCH_LOG(WARN, "execute commit transaction async failed", KR(ret));
        }
        ++act_commits_;
        TESTBENCH_LOG(TRACE, "execute contention transaction commit", K(conn_idx), K_(act_operations), K_(act_commits));
      } else {
        if (OB_FAIL(connections_.at(conn_idx)->rollback_async())) {
          TESTBENCH_LOG(WARN, "execute rollback transaction async failed", KR(ret));
        }
        commits_.at(conn_idx) = false;
        TESTBENCH_LOG(TRACE, "execute contention transaction abort", K(conn_idx), K_(act_operations), K_(act_commits));
      }
      ++act_operations_.at(conn_idx);
      if (OB_FAIL(ret)) {
        commits_.at(conn_idx) = false;
      }
    } else {
      ++finished_;
      end_trace_latency(cumulative_latencys_.at(conn_idx));
      break;
    }
  }
  return ret;
}

/*
                                        ObDeadlockTransactionTask
*/
ObDeadlockTransactionTask::ObDeadlockTransactionTask(BasicTaskConfig basic_config, int64_t chains) 
: ObIWorkloadTransactionTask(basic_config), chains_(chains), cycle_(1), lock_deadlock_sql_(), lock_multi_deadlock_sql_(), finished_(0)
{
  lock_deadlock_sql_format_ = "UPDATE %s SET lock_lcl = lock_lcl + 1 WHERE partition_id = ? AND row_id = ?;";
  lock_multi_deadlock_sql_format_ = "UPDATE %s SET lock_lcl = lock_lcl + 1 WHERE (partition_id = ? AND row_id = ?) OR (partition_id = ? AND row_id = ?);";
}

ObDeadlockTransactionTask::~ObDeadlockTransactionTask() 
{
  lock_deadlock_stmts_.release_array(allocator_);
  lock_multi_deadlock_stmts_.release_array(allocator_);
}

int ObDeadlockTransactionTask::init() {
  int ret = OB_SUCCESS;
  if (OB_FAIL(ObIWorkloadTransactionTask::init())) {
    TESTBENCH_LOG(ERROR, "init base transaction task failed", KR(ret));
  } else if (OB_FAIL(ObIWorkloadTransactionTask::init_pollfds())) {
    TESTBENCH_LOG(ERROR, "init pollfds failed", KR(ret));
  } else if (OB_FAIL(lock_deadlock_stmts_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate statements array failed", KR(ret));
  } else if (OB_FAIL(lock_multi_deadlock_stmts_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate statements array failed", KR(ret));
  } else if (OB_FAIL(act_commits_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate actual commit array failed", KR(ret));
  } else if (OB_FAIL(lock_deadlock_sql_.assign_fmt(lock_deadlock_sql_format_, table_name_))) {
    TESTBENCH_LOG(ERROR, "get format lock_deadlock_sql_ failed", KR(ret), K_(lock_deadlock_sql));
  } else if (OB_FAIL(lock_multi_deadlock_sql_.assign_fmt(lock_multi_deadlock_sql_format_, table_name_))) {
    TESTBENCH_LOG(ERROR, "get format lock_multi_deadlock_sql_ failed", KR(ret));
  } else {
    for (int64_t i = 0; i < connection_count_; ++i) {
      if (OB_FAIL(connections_.at(i)->prepare_statement(lock_deadlock_stmts_.at(i), lock_deadlock_sql_.ptr()))) {
        TESTBENCH_LOG(ERROR, "prepare statement failed", KR(ret), "sql", lock_deadlock_sql_.ptr());
      } else if (OB_FAIL(connections_.at(i)->prepare_statement(lock_multi_deadlock_stmts_.at(i), lock_multi_deadlock_sql_.ptr()))) {
        TESTBENCH_LOG(ERROR, "prepare statement failed", KR(ret), "sql", lock_multi_deadlock_sql_.ptr());
      } else {
        act_commits_.at(i) = false;
      }
    }
  }
  return ret;
}

int ObDeadlockTransactionTask::execute_transactions() 
{
  int ret = OB_SUCCESS;
  // NOTE: connection_count_ should be divisible by chains_
  cycle_ = connection_count_ / chains_;
  int64_t current_row_id = row_id_start_;
  // begin transactions
  for (int64_t conn_idx = 0; conn_idx < connection_count_; ++conn_idx) {
    begin_trace_latency(cumulative_latencys_.at(conn_idx));
    if (OB_FAIL(connections_.at(conn_idx)->start_transaction_async(false))) {
      TESTBENCH_LOG(WARN, "start transaction async failed", KR(ret));
      return ret;
    }
  }
  // execute anchor row
  for (int64_t conn_idx = 0; conn_idx < connection_count_; ++conn_idx) {
    ObMySQLPreparedStatement &stmt = lock_deadlock_stmts_.at(conn_idx);
    if (OB_FAIL(wait_for_connection_block(conn_idx))) {
      TESTBENCH_LOG(WARN, "wait for connection block failed", KR(ret));
    } else if (OB_FAIL(bind_params(parameters_.at(conn_idx), current_row_id, stmt))) {
      TESTBENCH_LOG(WARN, "bind parameters for statement failed", KR(ret), "partition", parameters_.at(conn_idx), "row", current_row_id);
    } else if (OB_FAIL(connections_.at(conn_idx)->execute_write_async(stmt))) {
      TESTBENCH_LOG(WARN, "execute write async failed", KR(ret));
    } else {
      TESTBENCH_LOG(TRACE, "execute anchor row", "partition", parameters_.at(conn_idx), "row", current_row_id);
    }
    ++current_row_id;
    if (OB_FAIL(ret)) {
      commits_.at(conn_idx) = false;
    }
  }
  // execute dependency row
  current_row_id = row_id_start_ + 1;
  for (int64_t conn_idx = 0; conn_idx < connection_count_; ++conn_idx) {
    ObMySQLPreparedStatement &stmt = lock_deadlock_stmts_.at(conn_idx);
    if (conn_idx % cycle_ == cycle_ - 1) {
      // do nothing
    } else if (OB_FAIL(wait_for_connection_block(conn_idx))) {
      TESTBENCH_LOG(WARN, "wait for connection block failed", KR(ret));
    } else if (OB_FAIL(bind_params(parameters_.at(conn_idx + 1), current_row_id, stmt))) {
      TESTBENCH_LOG(WARN, "bind parameters for statement failed", KR(ret), "partition", parameters_.at(conn_idx + 1), "row", current_row_id);
    } else if (OB_FAIL(connections_.at(conn_idx)->execute_write_async(stmt))) {
      TESTBENCH_LOG(WARN, "execute write async failed", KR(ret));
    } else {
      TESTBENCH_LOG(TRACE, "execute dependency row", "partition", parameters_.at(conn_idx + 1), "row", current_row_id);
    }
    ++current_row_id;
    if (OB_FAIL(ret)) {
      commits_.at(conn_idx) = false;
    }
  }
  // execute anti-dependency row
  current_row_id = row_id_start_ + 1;
  int64_t anti_dependency_row_id = 0;
  int64_t cycle_idx = 0;
  for (int64_t conn_idx = 0; conn_idx < connection_count_; ++conn_idx) {
    ObMySQLPreparedStatement &stmt = lock_deadlock_stmts_.at(conn_idx);
    ObMySQLPreparedStatement &multi_stmt = lock_multi_deadlock_stmts_.at(conn_idx);
    if (conn_idx % cycle_ < cycle_ - 1) {
      // do nothing
    } else if (conn_idx == connection_count_ - 1) {
      int64_t anti_dependency_idx = (conn_idx / cycle_) * cycle_;
      anti_dependency_row_id = row_id_start_ + anti_dependency_idx;
      if (OB_FAIL(wait_for_connection_block(conn_idx))) {
        TESTBENCH_LOG(WARN, "wait for connection block failed", KR(ret));
      } else if (FALSE_IT(begin_trace_latency(latencys_.at(cycle_idx++)))) {
        // impossible
      } else if (OB_FAIL(bind_params(parameters_.at(anti_dependency_idx), anti_dependency_row_id, stmt))) {
        TESTBENCH_LOG(WARN, "bind parameters for statement failed", KR(ret), "partition", parameters_.at(anti_dependency_idx), "row", anti_dependency_row_id);
      } else if (OB_FAIL(connections_.at(conn_idx)->execute_write_async(stmt))) {
        TESTBENCH_LOG(WARN, "execute write async failed", KR(ret));
      } else {
        TESTBENCH_LOG(TRACE, "execute anti-dependency row", "partition", parameters_.at(anti_dependency_idx), "row", anti_dependency_row_id);
      }
    } else {
      int64_t anti_dependency_idx = (conn_idx / cycle_) * cycle_;
      anti_dependency_row_id = row_id_start_ + anti_dependency_idx;
      if (OB_FAIL(wait_for_connection_block(conn_idx))) {
        TESTBENCH_LOG(WARN, "wait for connection block failed", KR(ret));
      } else if (FALSE_IT(begin_trace_latency(latencys_.at(cycle_idx++)))) {
        // impossible
      } else if (OB_FAIL(bind_multi_params(parameters_.at(conn_idx + 1), current_row_id, parameters_.at(anti_dependency_idx), anti_dependency_row_id, multi_stmt))) {
        TESTBENCH_LOG(WARN, "bind multiple parameters failed", KR(ret));
      } else if (OB_FAIL(connections_.at(conn_idx)->execute_write_async(multi_stmt))) {
        TESTBENCH_LOG(WARN, "execute write async failed", KR(ret));
      } else {
        TESTBENCH_LOG(TRACE, "execute anti-dependency row", "partition0", parameters_.at(conn_idx + 1), "row0", current_row_id, "partition1", parameters_.at(anti_dependency_idx), "row1", anti_dependency_row_id);
      }
      ++current_row_id;
    }
    if (OB_FAIL(ret)) {
      commits_.at(conn_idx) = false;
    }
  }
  // commit or rollback transactions to release locks
  int64_t conn_idx = 0;
  cycle_idx = 0;
  while (finished_ < connection_count_) {
    if (OB_FAIL(find_connection_ready_multiplexing(conn_idx))) {
      if (ret == OB_TIMEOUT) {
        TESTBENCH_LOG(WARN, "operations timeout", KR(ret));
        return ret;
      } else if (ret == OB_DEADLOCK) {
        commits_.at(conn_idx) = false;
        TESTBENCH_LOG(INFO, "deadlock cycle detected", K(ret), K(cycle_idx));
        end_trace_latency(latencys_.at(cycle_idx++));
      } else if (ret != OB_NEED_RETRY) {
        commits_.at(conn_idx) = false;
        TESTBENCH_LOG(WARN, "find connection ready failed", KR(ret), K(conn_idx));
      }
    }
    if (OB_FAIL(push_forward_operations(conn_idx))) {
      TESTBENCH_LOG(WARN, "push forward transaction operations failed", KR(ret));
    }
  }
  // clear unverified deadlock cycles
  for (int64_t i = cycle_idx; i < connection_count_; ++i) {
    latencys_.at(i) = 0;
  }
  return ret;
}

int ObDeadlockTransactionTask::release_dblinks() 
{
  int ret = OB_SUCCESS;
  for (int64_t i = 0; i < connection_count_; ++i) {
    if (OB_FAIL(lock_deadlock_stmts_.at(i).close())) {
      TESTBENCH_LOG(WARN, "close deadlock transaction statement failed", "conn_idx", i, KR(ret));
    } else if (OB_FAIL(lock_multi_deadlock_stmts_.at(i).close())) {
      TESTBENCH_LOG(WARN, "close deadlock transaction multi statekments failed", "conn_idx", i, KR(ret));
    } else if (OB_FAIL(mysql_proxy_->release_conn(0, commits_.at(i), connections_.at(i)))) {
      TESTBENCH_LOG(ERROR, "release dblink failed", KR(ret));
    }
  }
  return ret;
}

int ObDeadlockTransactionTask::collect_statistics()
{
  int ret = OB_SUCCESS;
  for (int64_t conn_idx = 0; conn_idx < connection_count_; ++conn_idx) {
    if (OB_FAIL(record_latency(ObLatencyTaskType::DEADLOCK_SQL_LATENCY_TASK, latencys_.at(conn_idx)))) {
      TESTBENCH_LOG(WARN, "record the latency of cycle detection and resolution failed", KR(ret), "latency", latencys_.at(conn_idx));
    } else if (false == commits_.at(conn_idx)) {
      if (OB_FAIL(record_latency(ObLatencyTaskType::ROLLBACK_TXN_LATENCY_TASK, cumulative_latencys_.at(conn_idx)))) {
        TESTBENCH_LOG(WARN, "record the latency of rollback transaction failed", KR(ret), "txn_latency", cumulative_latencys_.at(conn_idx));
      }
    } else if (OB_FAIL(record_latency(ObLatencyTaskType::DEADLOCK_TXN_LATENCY_TASK, cumulative_latencys_.at(conn_idx)))) {
      TESTBENCH_LOG(WARN, "record the latency of deadlock transaction failed", KR(ret), "txn_latency", cumulative_latencys_.at(conn_idx));
    } else {
      TESTBENCH_LOG(TRACE, "deadlock transaction finish", "lock_latency", latencys_.at(conn_idx), "txn_latency", cumulative_latencys_.at(conn_idx));
    }
  }
  return ret;
}

WorkloadType ObDeadlockTransactionTask::get_type() 
{
  return WorkloadType::DEADLOCK;
}

int ObDeadlockTransactionTask::push_forward_operations(int64_t conn_idx)
{
  int ret = OB_SUCCESS;
  ObMySQLConnection *conn = connections_.at(conn_idx);
  while (!conn->get_async_status()) {
    if (!act_commits_.at(conn_idx)) {
      if (OB_FAIL(commits_.at(conn_idx) ? connections_.at(conn_idx)->commit_async() : connections_.at(conn_idx)->rollback_async())) {
        commits_.at(conn_idx) = false;
        TESTBENCH_LOG(WARN, "execute rollback async failed", KR(ret), "is_commit", commits_.at(conn_idx));
      }
      act_commits_.at(conn_idx) = true;
    } else {
      ++finished_;
      end_trace_latency(cumulative_latencys_.at(conn_idx));
      break;
    }
  }
  return ret;
}

/*
                                        ObConcurrentTransactionTask
*/
ObConcurrentTransactionTask::ObConcurrentTransactionTask(BasicTaskConfig basic_config, int64_t operations, Readonlys readonlys) 
: ObIWorkloadTransactionTask(basic_config), operations_(operations), readonlys_(readonlys), lock_wrtxn_sql_(), lock_rdtxn_sql_(), finished_(0)
{
  lock_wrtxn_sql_format_ = "UPDATE %s SET lock_txn = lock_txn + 1 WHERE partition_id = ? AND row_id = ?;";
  lock_rdtxn_sql_format_ = "SELECT lock_txn FROM %s WHERE partition_id = ? AND row_id = ?;";
}

ObConcurrentTransactionTask::~ObConcurrentTransactionTask() 
{
  lock_wrtxn_stmts_.release_array(allocator_);
  lock_rdtxn_stmts_.release_array(allocator_);
  wait_for_reads_.release_array(allocator_);
  act_operations_.release_array(allocator_);
}

int ObConcurrentTransactionTask::init() {
  int ret = OB_SUCCESS;
  if (OB_FAIL(ObIWorkloadTransactionTask::init())) {
    TESTBENCH_LOG(ERROR, "init base transaction task failed", KR(ret));
  } else if (OB_FAIL(ObIWorkloadTransactionTask::init_pollfds())) {
    TESTBENCH_LOG(ERROR, "init pollfds failed", KR(ret));
  } else if (OB_FAIL(lock_wrtxn_stmts_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate statements array failed", KR(ret));
  } else if (OB_FAIL(lock_rdtxn_stmts_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate statements array failed", KR(ret));
  } else if (OB_FAIL(wait_for_reads_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate wait for reads array failed", KR(ret));
  } else if (OB_FAIL(act_operations_.allocate_array(allocator_, connection_count_))) {
    TESTBENCH_LOG(ERROR, "allocate actual operations array failed", KR(ret));
  } else if (OB_FAIL(lock_wrtxn_sql_.assign_fmt(lock_wrtxn_sql_format_, table_name_))) {
    TESTBENCH_LOG(ERROR, "get format lock_wrtxn_sql failed", KR(ret), K_(lock_wrtxn_sql));
  } else if (OB_FAIL(lock_rdtxn_sql_.assign_fmt(lock_rdtxn_sql_format_, table_name_))) {
    TESTBENCH_LOG(ERROR, "get format lock_rdtxn_sql failed", KR(ret), K_(lock_rdtxn_sql));
  } else {
    for (int64_t i = 0; i < connection_count_; ++i) {
      if (OB_FAIL(connections_.at(i)->prepare_statement(lock_wrtxn_stmts_.at(i), lock_wrtxn_sql_.ptr()))) {
        TESTBENCH_LOG(ERROR, "prepare statement failed", KR(ret), "sql", lock_wrtxn_sql_.ptr());
      } else if (OB_FAIL(connections_.at(i)->prepare_statement(lock_rdtxn_stmts_.at(i), lock_rdtxn_sql_.ptr()))) {
        TESTBENCH_LOG(ERROR, "prepare statement failed", KR(ret), "sql", lock_rdtxn_sql_.ptr());
      } else {
        wait_for_reads_.at(i) = false;
        act_operations_.at(i) = 0;
      }
    }
  }
  return ret;
}

int ObConcurrentTransactionTask::execute_transactions() 
{
  int ret = OB_SUCCESS;
  int64_t current_row_id = row_id_start_;
  // begin transactions
  for (int64_t conn_idx = 0; conn_idx < connection_count_; ++conn_idx) {
    begin_trace_latency(cumulative_latencys_.at(conn_idx));
    if (OB_FAIL(connections_.at(conn_idx)->start_transaction_async(true))) {
      TESTBENCH_LOG(WARN, "start transaction async failed", KR(ret));
      return ret;
    }
  }
  // start transaction async may return without async
  for (int64_t conn_idx = 0; conn_idx < connection_count_; ++conn_idx) {
    ObMySQLConnection *conn = connections_.at(conn_idx);
    if (OB_FAIL(push_forward_operations(conn_idx, current_row_id))) {
      commits_.at(conn_idx) = false;
      TESTBENCH_LOG(WARN, "push forward transaction operations failed", KR(ret));
    }
  }
  // execute commands
  int64_t conn_idx = 0;
  while (finished_ < connection_count_) {
    if (OB_FAIL(find_connection_ready_multiplexing(conn_idx))) {
      if (ret == OB_TIMEOUT) {
        TESTBENCH_LOG(WARN, "operations timeout", KR(ret));
        return ret;
      } else if (ret != OB_NEED_RETRY) {
        commits_.at(conn_idx) = false;
        TESTBENCH_LOG(WARN, "find connection ready failed", KR(ret), K(conn_idx), K_(finished), K_(act_operations));
      } 
    }
    if (OB_FAIL(push_forward_operations(conn_idx, current_row_id))) {
      TESTBENCH_LOG(WARN, "push forward transaction operations failed", KR(ret));
    }
  }
  return ret;
}

int ObConcurrentTransactionTask::release_dblinks() 
{
  int ret = OB_SUCCESS;
  for (int64_t i = 0; i < connection_count_; ++i) {
    if (OB_FAIL(lock_wrtxn_stmts_.at(i).close())) {
      TESTBENCH_LOG(WARN, "close write transaction statement failed", "conn_idx", i, KR(ret));
    } else if (OB_FAIL(lock_rdtxn_stmts_.at(i).close())) {
      TESTBENCH_LOG(WARN, "close read transaction statement failed", "conn_idx", i, KR(ret));
    } else if (OB_FAIL(mysql_proxy_->release_conn(0, commits_.at(i), connections_.at(i)))) {
      TESTBENCH_LOG(ERROR, "release dblink failed", KR(ret));
    }
  }
  return ret;
}

int ObConcurrentTransactionTask::collect_statistics()
{
  int ret = OB_SUCCESS;
  for (int64_t conn_idx = 0; conn_idx < connection_count_; ++conn_idx) {
    if (false == commits_.at(conn_idx)) {
      if (OB_FAIL(record_latency(ObLatencyTaskType::ROLLBACK_TXN_LATENCY_TASK, cumulative_latencys_.at(conn_idx)))) {
        TESTBENCH_LOG(WARN, "record the latency of rollback transaction failed", KR(ret), "txn_latency", cumulative_latencys_.at(conn_idx));
      }
    } else if (OB_FAIL(record_latency(ObLatencyTaskType::COMMIT_SQL_LATENCY_TASK, latencys_.at(conn_idx)))) {
      TESTBENCH_LOG(WARN, "record the latency of commit command failed", KR(ret), "latency", latencys_.at(conn_idx));
    } else if (OB_FAIL(record_latency(ObLatencyTaskType::CONCURRENT_TXN_LATENCY_TASK, cumulative_latencys_.at(conn_idx)))) {
      TESTBENCH_LOG(WARN, "record the latency of concurrent transaction failed", KR(ret), "txn_latency", cumulative_latencys_.at(conn_idx));
    } else {
      TESTBENCH_LOG(TRACE, "concurrent transaction finish", "lock_latency", latencys_.at(conn_idx), "txn_latency", cumulative_latencys_.at(conn_idx));
    }
  }
  return ret;
}

WorkloadType ObConcurrentTransactionTask::get_type() 
{
  return WorkloadType::CONCURRENT;
}

int ObConcurrentTransactionTask::execute_read_async(int64_t conn_idx, int64_t current_row_id)
{
  int ret = OB_SUCCESS;
  ObMySQLConnection *conn = connections_.at(conn_idx);
  ObMySQLPreparedStatement &rdstmt = lock_rdtxn_stmts_.at(conn_idx);
  int64_t lock_txn = -1;
  if (wait_for_reads_.at(conn_idx)) {
    common::ObISQLClient::ReadResult res;
    if (OB_FAIL(bind_result(lock_txn, rdstmt))) {
      TESTBENCH_LOG(WARN, "bind result columns failed", KR(ret));
    } else if (OB_FAIL(conn->get_async_read_result(rdstmt, res))) {
      TESTBENCH_LOG(WARN, "get async read result failed", KR(ret));
    }
    ++act_operations_.at(conn_idx);
  } else {
    if (OB_FAIL(bind_params(parameters_.at(conn_idx), current_row_id, rdstmt))) {
      TESTBENCH_LOG(WARN, "bind parameters for statement failed", KR(ret), "partition", parameters_.at(conn_idx), "row", current_row_id);
    } else if (OB_FAIL(conn->execute_read_async(rdstmt))) {
      TESTBENCH_LOG(WARN, "execute async read statement failed", KR(ret));
    }
  }
  wait_for_reads_.at(conn_idx) = !wait_for_reads_.at(conn_idx);
  return ret;
}

int ObConcurrentTransactionTask::execute_write_async(int64_t conn_idx, int64_t current_row_id)
{
  int ret = OB_SUCCESS;
  ObMySQLConnection *conn = connections_.at(conn_idx);
  ObMySQLPreparedStatement &wrstmt = lock_wrtxn_stmts_.at(conn_idx);
  if (OB_FAIL(bind_params(parameters_.at(conn_idx), current_row_id, wrstmt))) {
    TESTBENCH_LOG(WARN, "bind parameters for statement failed", KR(ret), "partition", parameters_.at(conn_idx), "row", current_row_id);
  } else if (OB_FAIL(conn->execute_write_async(wrstmt))) {
    TESTBENCH_LOG(WARN, "execute write async failed", KR(ret));
  }
  ++act_operations_.at(conn_idx);
  return ret;
}

int ObConcurrentTransactionTask::push_forward_operations(int64_t conn_idx, int64_t &current_row_id)
{
  int ret = OB_SUCCESS;
  ObMySQLConnection *conn = connections_.at(conn_idx);
  while (!conn->get_async_status()) {
    if (act_operations_.at(conn_idx) < operations_) {
      // execute read write operatios
      if (OB_FAIL(readonlys_.at(conn_idx) ? execute_read_async(conn_idx, current_row_id) : execute_write_async(conn_idx, current_row_id))) {
        commits_.at(conn_idx) = false;
        TESTBENCH_LOG(WARN, "execute read/write async failed", KR(ret), "is_read", readonlys_.at(conn_idx));
      }
      ++current_row_id;
    } else if (act_operations_.at(conn_idx) == operations_) {
      // commit or abort transaction
      begin_trace_latency(latencys_.at(conn_idx));
      if (OB_FAIL(commits_.at(conn_idx) ? connections_.at(conn_idx)->commit_async() : connections_.at(conn_idx)->rollback_async())) {
        commits_.at(conn_idx) = false;
        TESTBENCH_LOG(WARN, "execute commit/rollback async failed", KR(ret), "is_commit", commits_.at(conn_idx));
      }
      ++act_operations_.at(conn_idx);
    } else {
      ++finished_;
      end_trace_latency(latencys_.at(conn_idx));
      end_trace_latency(cumulative_latencys_.at(conn_idx));
      break;
    }
  }
  return ret;
}
} // namespace testbench
} // namespace oceanbase