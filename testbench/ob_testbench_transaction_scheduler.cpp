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

#include "ob_testbench_transaction_scheduler.h"

#include "testbench/ob_testbench_transaction_task.h"
#include "share/ob_thread_define.h"
#include "share/ob_thread_mgr.h"

namespace oceanbase
{
namespace testbench
{
ObTestbenchTransactionScheduler::ObTestbenchTransactionScheduler() 
  : tg_id_(-1),  
    cur_partition_id_(0),
    cur_row_id_(0),
    cur_conn_id_(0),
    allocator_("TxnTask")
{}

ObTestbenchTransactionScheduler::~ObTestbenchTransactionScheduler() {}

int ObTestbenchTransactionScheduler::init(ObTestbenchMySQLProxy *mysql_proxy, ObTestbenchTransactionExecutorPool *executor_pool, ObTestbenchStatisticsCollector *statistics_collector, ObIWorkloadOptions *workload_options, ObDatasetOptions *dataset_options, ObConnectionOptions *connection_options) {
  int ret = OB_SUCCESS;
  ObTestbenchSystableHelper *systable_helper_ = mysql_proxy->get_systable_helper();
  if (IS_INIT) {
    ret = OB_INIT_TWICE;
    TESTBENCH_LOG(ERROR, "location cache init twice", KR(ret));
  } else if (OB_ISNULL(systable_helper_)) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "mysql proxy is not inited", KR(ret));
  } else if (OB_FAIL(location_cache_.init(connection_options->get_cluster_db_name(), connection_options->get_cluster_table_name(), systable_helper_))) {
    TESTBENCH_LOG(ERROR, "location cache init failed", "database_name", connection_options->get_cluster_db_name(), "table_name", connection_options->get_cluster_table_name(), KR(ret));
  } else if (OB_FAIL(location_cache_.refresh_locations())) {
    TESTBENCH_LOG(ERROR, "refresh location cache failed", KR(ret));
  } else {
    mysql_proxy_ = mysql_proxy; 
    executor_pool_ = executor_pool;
    statistics_collector_ = statistics_collector;
    workload_options_ = workload_options;
    txn_type_ = workload_options->get_type();
    dataset_options_ = dataset_options;
    table_name_ = connection_options->get_cluster_table_name();
    is_inited_ = true;
    TESTBENCH_LOG(INFO, "transaction scheduler init success");
  }
  return ret;
}

int ObTestbenchTransactionScheduler::get_dblink_ids() {
  int ret = OB_SUCCESS;
  ObSEArray<ObFixedLengthString<OB_MAX_TENANT_NAME_LENGTH + 1>, 16> tenant_name_array("OBMySQLConnPool", OB_MALLOC_NORMAL_BLOCK_SIZE);
  ObSEArray<uint64_t, 16> tenant_array("OBMySQLConnPool", OB_MALLOC_NORMAL_BLOCK_SIZE);
  ObTestbenchServerProvider *server_provider_ = mysql_proxy_->get_server_provider();
  ObSEArray<ObAddr, 16> server_array("OBMySQLConnPool", OB_MALLOC_NORMAL_BLOCK_SIZE);
  if (OB_ISNULL(server_provider_)) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "mysql proxy is not inited", KR(ret));
  } else if (OB_FAIL(server_provider_->get_tenants(tenant_name_array))) {
    TESTBENCH_LOG(ERROR, "get tenant name from server provider failed", KR(ret), K(tenant_name_array));
  } else if (OB_FAIL(server_provider_->get_tenant_ids(tenant_array))) {
    TESTBENCH_LOG(ERROR, "get tenant id from server provider failed", KR(ret), K(tenant_array));
  } else if (OB_UNLIKELY(2 != tenant_array.count() || tenant_name_array.count() != tenant_array.count())) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "the number of tenants should be two (sys and tb)", KR(ret), "tenant_cnt", tenant_array.count(), "tenant_id_cnt", tenant_name_array.count());
  } else if (OB_FAIL(server_provider_->get_tenant_servers(tenant_array.at(1), server_array))) {
    TESTBENCH_LOG(ERROR, "get servers by tenant id failed", KR(ret), "tenant_id", tenant_array.at(1));
  } else {
    ObString tenant_name_str = tenant_name_array[1].str();
    ARRAY_FOREACH_N(server_array, idx, cnt) {
      ObAddr server = server_array[idx];
      if (OB_SUCC(ret) && OB_FAIL(dblink_ids_.push_back(DblinkKey(tenant_name_str, server).hash()))) {
        TESTBENCH_LOG(WARN, "push back dblink id failed", KR(ret));
      } else {
        TESTBENCH_LOG(DEBUG, "push back dblink id succeed", K(tenant_name_str), K(server));
      }
    }
  }
  return ret;
}

int ObTestbenchTransactionScheduler::start() {
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "transaction scheduler is not inited", KR(ret));
  } else if (OB_FAIL(get_dblink_ids())) {
    TESTBENCH_LOG(ERROR, "transaction scheduler get dblink ids failed", KR(ret));
  } else if (OB_FAIL(TG_CREATE(lib::TGDefIDs::TxnScheduler, tg_id_))) {
    TESTBENCH_LOG(ERROR, "transaction scheduler create thread pool failed", KR(ret), K(tg_id_));
  } else if (OB_FAIL(TG_SET_RUNNABLE_AND_START(tg_id_, *this))) {
    TESTBENCH_LOG(ERROR, "transaction scheduler set runnable and start failed", KR(ret));
  } else {
    TESTBENCH_LOG(INFO, "transaction scheduler start success");
  }
  return ret;
}

void ObTestbenchTransactionScheduler::stop() {
  TESTBENCH_LOG(INFO, "transaction scheduler stop start");
  TG_STOP(tg_id_);
  TESTBENCH_LOG(INFO, "transaction scheduler stop finish");
}

void ObTestbenchTransactionScheduler::wait() {
  TESTBENCH_LOG(INFO, "transaction scheduler wait start");
  TG_WAIT(tg_id_);
  TESTBENCH_LOG(INFO, "transaction scheduler wait finish");
}

void ObTestbenchTransactionScheduler::destroy() {
  stop();
  wait();
  location_cache_.destroy();
  is_inited_ = false;
  if (-1 != tg_id_) {
    TG_DESTROY(tg_id_);
    tg_id_ = -1;
  }
  TESTBENCH_LOG(INFO, "transaction scheduler destroy");
}

void ObTestbenchTransactionScheduler::run1() {
  int ret = OB_SUCCESS;
  int64_t thread_num = executor_pool_->get_thread_num() * 2;
  while (!has_set_stop()) {
    int64_t snap = executor_pool_->load_start_task() - executor_pool_->load_finish_task();
    if (snap < thread_num) {
      TESTBENCH_LOG(DEBUG, "check transaction executor pool status, need new task", K(snap), K(thread_num));
      switch (txn_type_) {
        case DISTRIBUTED_TRANSACTION:
          generate_distributed_txn_task();
          break;
        case CONTENTION:
          generate_contention_txn_task();
          break;
        case DEADLOCK:
          generate_deadlock_txn_task();
          break;
        default:
          TESTBENCH_LOG(ERROR, "unknown transaction task type");
      }
    }
  }
}

void ObTestbenchTransactionScheduler::generate_distributed_txn_task() {
  int ret = OB_SUCCESS;
  ObDistributedTransactionOptions *options = static_cast<ObDistributedTransactionOptions*>(workload_options_);
  Parameters partition_ids;
  if (OB_ISNULL(options)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "cast ObIWorkloadOptions to ObDistributedTransactionOptions failed", KR(ret), KP(workload_options_)); 
  } else {
    // int64_t partitions = options->get_participants();
    // int64_t operations = options->get_operations();
    // if (OB_FAIL(location_cache_.generate_different_partitions(partitions, partition_ids))) {
    //   TESTBENCH_LOG(ERROR, "generate distinct partitions failed", KR(ret), "target", partitions);
    // } else {
    //   ObDistributedTransactionTask *task = nullptr;
    //   void *buf = nullptr;
    //   BasicTaskConfig config{ table_name_, 1, statistics_collector_, dblink_ids_.at(cur_conn_id_), mysql_proxy_, partition_ids, cur_row_id_ };
    //   cur_conn_id_ = (cur_conn_id_ + 1) % dblink_ids_.count();
    //   cur_row_id_ = (cur_row_id_ + operations) % dataset_options_->get_rows();
    //   if (OB_ISNULL(buf = allocator_.alloc(sizeof(ObDistributedTransactionTask)))) {
    //     ret = OB_ALLOCATE_MEMORY_FAILED;
    //     TESTBENCH_LOG(ERROR, "allocate memory failed", KR(ret));
    //   } else if (OB_ISNULL(task = new(buf)ObDistributedTransactionTask(config, partitions, operations))) {
    //     ret = OB_ERR_UNEXPECTED;
    //     TESTBENCH_LOG(ERROR, "create new distributed transaction task failed", KR(ret));
    //   } else if (OB_FAIL(executor_pool_->push_task(task))) {
    //     TESTBENCH_LOG(ERROR, "push new distributed transaction task failed", KR(ret));
    //   }
    // }
  }
}

void ObTestbenchTransactionScheduler::generate_contention_txn_task() {
  int ret = OB_SUCCESS;
  ObContentionTransactionOptions *options = static_cast<ObContentionTransactionOptions*>(workload_options_);
  Parameters partition_ids;
  if (OB_ISNULL(options)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "cast ObIWorkloadOptions to ObContentionTransactionOptions failed", KR(ret), KP(workload_options_));
  } else {
    // int64_t concurrency = options->get_concurrency();
    // int64_t operations = options->get_operations();
    // int64_t aborts = options->get_aborts();
    // if (OB_FAIL(location_cache_.generate_different_partitions(1, partition_ids))) {
    //   TESTBENCH_LOG(ERROR, "generate distinct partitions failed", KR(ret), "target", 1);
    // } else {
    //   ObContentionTransactionTask *task = nullptr;
    //   void *buf = nullptr;
    //   BasicTaskConfig config{ table_name_, concurrency, statistics_collector_, dblink_ids_.at(cur_conn_id_), mysql_proxy_, partition_ids, cur_row_id_ };
    //   cur_conn_id_ = (cur_conn_id_ + 1) % dblink_ids_.count();
    //   cur_row_id_ = (cur_row_id_ + concurrency * operations) % dataset_options_->get_rows();
    //   if (OB_ISNULL(buf = allocator_.alloc(sizeof(ObContentionTransactionTask)))) {
    //     ret = OB_ALLOCATE_MEMORY_FAILED;
    //     TESTBENCH_LOG(ERROR, "allocate memory failed", KR(ret));
    //   } else if (OB_ISNULL(task = new(buf)ObContentionTransactionTask(config, aborts, operations))) {
    //     ret = OB_ERR_UNEXPECTED;
    //     TESTBENCH_LOG(ERROR, "create new contention transaction task failed", KR(ret));
    //   } else if (OB_FAIL(executor_pool_->push_task(task))) {
    //     TESTBENCH_LOG(ERROR, "push new contention transaction task failed", KR(ret));
    //   }
    // }
  }
}

void ObTestbenchTransactionScheduler::generate_deadlock_txn_task() {
  int ret = OB_SUCCESS;
  ObDeadlockTransactionOptions *options = static_cast<ObDeadlockTransactionOptions*>(workload_options_);
  Parameters partition_ids;
  if (OB_ISNULL(options)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "cast ObIWorkloadOptions to ObDeadlockTransactionOptions failed", KR(ret), KP(workload_options_));
  } else  {
    // int64_t partitions = options->get_partitions();
    // int64_t concurrency = options->get_concurrency();
    // int64_t chains = options->get_chains();
    // if (OB_FAIL(location_cache_.generate_different_partitions(partitions, partition_ids))) {
    //   TESTBENCH_LOG(ERROR, "generate distinct partitions failed", KR(ret), "target", partitions);
    // } else {
    //   ObDeadlockTransactionTask *task = nullptr;
    //   void *buf = nullptr;
    //   BasicTaskConfig config{ table_name_, concurrency, statistics_collector_, dblink_ids_.at(cur_conn_id_), mysql_proxy_, partition_ids, cur_row_id_ };
    //   cur_conn_id_ = (cur_conn_id_ + 1) % dblink_ids_.count();
    //   cur_row_id_ = (cur_row_id_ + concurrency * chains) % dataset_options_->get_rows();
    //   if (OB_ISNULL(buf = allocator_.alloc(sizeof(ObDeadlockTransactionTask)))) {
    //     ret = OB_ALLOCATE_MEMORY_FAILED;
    //     TESTBENCH_LOG(ERROR, "allocate memory failed", KR(ret));
    //   } else if (OB_ISNULL(task = new(buf)ObDeadlockTransactionTask(config, chains))) {
    //     ret = OB_ERR_UNEXPECTED;
    //     TESTBENCH_LOG(ERROR, "create new deadlock transaction task failed", KR(ret));
    //   } else if (OB_FAIL(executor_pool_->push_task(task))) {
    //     TESTBENCH_LOG(ERROR, "push new deadlock transaction task failed", KR(ret));
    //   }
    // }
  }
}
} // namespace testbench
} // namespace oceanbase