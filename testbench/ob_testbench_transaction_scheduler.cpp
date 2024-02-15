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
    cur_row_id_(0),
    allocator_("TxnTask")
{}

ObTestbenchTransactionScheduler::~ObTestbenchTransactionScheduler() {}

int ObTestbenchTransactionScheduler::init(ObTestbenchMySQLProxy *mysql_proxy, ObTestbenchTransactionExecutorPool *executor_pool, ObTestbenchStatisticsCollector *statistics_collector, ObIWorkloadOptions *workload_options, ObDatasetOptions *dataset_options, ObConnectionOptions *connection_options) 
{
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

int ObTestbenchTransactionScheduler::start() 
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "transaction scheduler is not inited", KR(ret));
  } else if (OB_FAIL(TG_CREATE(lib::TGDefIDs::TxnScheduler, tg_id_))) {
    TESTBENCH_LOG(ERROR, "transaction scheduler create thread pool failed", KR(ret), K(tg_id_));
  } else if (OB_FAIL(TG_SET_RUNNABLE_AND_START(tg_id_, *this))) {
    TESTBENCH_LOG(ERROR, "transaction scheduler set runnable and start failed", KR(ret));
  } else {
    TESTBENCH_LOG(INFO, "transaction scheduler start success");
  }
  return ret;
}

void ObTestbenchTransactionScheduler::stop() 
{
  TESTBENCH_LOG(INFO, "transaction scheduler stop start");
  if (-1 != tg_id_) {
    TG_STOP(tg_id_);
  }
  TESTBENCH_LOG(INFO, "transaction scheduler stop finish");
}

void ObTestbenchTransactionScheduler::wait() 
{
  TESTBENCH_LOG(INFO, "transaction scheduler wait start");
  if (-1 != tg_id_) {
    TG_WAIT(tg_id_);
  }
  TESTBENCH_LOG(INFO, "transaction scheduler wait finish");
}

void ObTestbenchTransactionScheduler::destroy() 
{
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

void ObTestbenchTransactionScheduler::run1() 
{
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
        case CONCURRENT:
          generate_concurrent_txn_task();
          break;
        default:
          TESTBENCH_LOG(ERROR, "unknown transaction task type");
      }
    }
  }
}

int ObTestbenchTransactionScheduler::generate_distributed_txn_task() 
{
  int ret = OB_SUCCESS;
  ObDistributedTransactionOptions *options = static_cast<ObDistributedTransactionOptions*>(workload_options_);
  if (OB_ISNULL(options)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "cast ObIWorkloadOptions to ObDistributedTransactionOptions failed", KR(ret), KP(workload_options_)); 
  } else {
    int64_t partitions = options->get_participants();
    int64_t operations = options->get_operations();
    int64_t concurrency = 1;
    Parameters parameters;
    Dblinks dblinks;
    if (OB_FAIL(location_cache_.gen_distributed_txn_params(partitions, parameters, dblinks))) {
      TESTBENCH_LOG(ERROR, "generate parameters and dblinks for distributed transaction failed", KR(ret), K(partitions));
    } else {
      ObDistributedTransactionTask *task = nullptr;
      BasicTaskConfig config{ table_name_, concurrency, statistics_collector_, mysql_proxy_, parameters, dblinks, cur_row_id_ };
      cur_row_id_ = (cur_row_id_ + operations * concurrency) % dataset_options_->get_rows();
      void *buf = nullptr;
      if (OB_ISNULL(buf = allocator_.alloc(sizeof(ObDistributedTransactionTask)))) {
        ret = OB_ALLOCATE_MEMORY_FAILED;
        TESTBENCH_LOG(ERROR, "allocate memory failed", KR(ret));
      } else if (OB_ISNULL(task = new(buf)ObDistributedTransactionTask(config, partitions, operations))) {
        ret = OB_ERR_UNEXPECTED;
        TESTBENCH_LOG(ERROR, "create new distributed transaction task failed", KR(ret));
      } else if (OB_FAIL(executor_pool_->push_task(task))) {
        TESTBENCH_LOG(ERROR, "push new distributed transaction task failed", KR(ret));
      }
    }
  }
  return ret;
}

int ObTestbenchTransactionScheduler::generate_contention_txn_task() 
{
  int ret = OB_SUCCESS;
  ObContentionTransactionOptions *options = static_cast<ObContentionTransactionOptions*>(workload_options_);
  if (OB_ISNULL(options)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "cast ObIWorkloadOptions to ObContentionTransactionOptions failed", KR(ret), KP(workload_options_));
  } else {
    int64_t concurrency = options->get_concurrency();
    int64_t operations = options->get_operations();
    int64_t aborts = options->get_aborts();
    Parameters parameters;
    Dblinks dblinks;
    if (OB_FAIL(location_cache_.gen_contention_txn_params(concurrency, parameters, dblinks))) {
      TESTBENCH_LOG(ERROR, "generate parameters and dblinks for contention transaction failed", KR(ret), K(concurrency));
    } else {
      ObContentionTransactionTask *task = nullptr;
      BasicTaskConfig config{ table_name_, concurrency, statistics_collector_, mysql_proxy_, parameters, dblinks, cur_row_id_ };
      cur_row_id_ = (cur_row_id_ + concurrency * operations) % dataset_options_->get_rows();
      void *buf = nullptr;
      if (OB_ISNULL(buf = allocator_.alloc(sizeof(ObContentionTransactionTask)))) {
        ret = OB_ALLOCATE_MEMORY_FAILED;
        TESTBENCH_LOG(ERROR, "allocate memory failed", KR(ret));
      } else if (OB_ISNULL(task = new(buf)ObContentionTransactionTask(config, aborts, operations))) {
        ret = OB_ERR_UNEXPECTED;
        TESTBENCH_LOG(ERROR, "create new contention transaction task failed", KR(ret));
      } else if (OB_FAIL(executor_pool_->push_task(task))) {
        TESTBENCH_LOG(ERROR, "push new contention transaction task failed", KR(ret));
      }
    }
  }
  return ret;
}

int ObTestbenchTransactionScheduler::generate_deadlock_txn_task() 
{
  int ret = OB_SUCCESS;
  ObDeadlockTransactionOptions *options = static_cast<ObDeadlockTransactionOptions*>(workload_options_);
  if (OB_ISNULL(options)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "cast ObIWorkloadOptions to ObDeadlockTransactionOptions failed", KR(ret), KP(workload_options_));
  } else  {
    int64_t concurrency = options->get_concurrency();
    int64_t chains = options->get_chains();
    Parameters parameters;
    Dblinks dblinks;
    if (OB_FAIL(location_cache_.gen_deadlock_txn_params(concurrency, parameters, dblinks))) {
      TESTBENCH_LOG(ERROR, "generate parameters and dblinks for deadlock transaction failed", KR(ret), K(concurrency));
    } else {
      ObDeadlockTransactionTask *task = nullptr;
      BasicTaskConfig config{ table_name_, concurrency, statistics_collector_, mysql_proxy_, parameters, dblinks, cur_row_id_ };
      cur_row_id_ = (cur_row_id_ + concurrency * chains) % dataset_options_->get_rows();
      void *buf = nullptr;
      if (OB_ISNULL(buf = allocator_.alloc(sizeof(ObDeadlockTransactionTask)))) {
        ret = OB_ALLOCATE_MEMORY_FAILED;
        TESTBENCH_LOG(ERROR, "allocate memory failed", KR(ret));
      } else if (OB_ISNULL(task = new(buf)ObDeadlockTransactionTask(config, chains))) {
        ret = OB_ERR_UNEXPECTED;
        TESTBENCH_LOG(ERROR, "create new deadlock transaction task failed", KR(ret));
      } else if (OB_FAIL(executor_pool_->push_task(task))) {
        TESTBENCH_LOG(ERROR, "push new deadlock transaction task failed", KR(ret));
      }
    }
  }
  return ret;
}

int ObTestbenchTransactionScheduler::generate_concurrent_txn_task() 
{
  int ret = OB_SUCCESS;
  ObConcurrentTransactionOptions *options = static_cast<ObConcurrentTransactionOptions*>(workload_options_);
  if (OB_ISNULL(options)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "cast ObIWorkloadOptions to ObConcurrentTransactionOptions failed", KR(ret), KP(workload_options_));
  } else  {
    int64_t concurrency = options->get_concurrency();
    int64_t operations = options->get_operations();
    int64_t readonly = options->get_readonly();
    Parameters parameters;
    Dblinks dblinks;
    if (OB_FAIL(location_cache_.gen_concurrent_txn_params(concurrency, parameters, dblinks))) {
      TESTBENCH_LOG(ERROR, "generate parameters and dblinks for concurrent transaction failed", KR(ret), K(concurrency));
    } else {
      ObConcurrentTransactionTask *task = nullptr;
      Readonlys readonlys;
      readonlys.reset();
      for (int64_t i = 0; i < concurrency; ++i) {
        bool is_readonly = random_.rand(1, 100) < readonly;
        readonlys.push_back(is_readonly);
      }
      BasicTaskConfig config{ table_name_, concurrency, statistics_collector_, mysql_proxy_, parameters, dblinks, cur_row_id_ };
      cur_row_id_ = (cur_row_id_ + concurrency * operations) % dataset_options_->get_rows();
      void *buf = nullptr;
      if (OB_ISNULL(buf = allocator_.alloc(sizeof(ObConcurrentTransactionTask)))) {
        ret = OB_ALLOCATE_MEMORY_FAILED;
        TESTBENCH_LOG(ERROR, "allocate memory failed", KR(ret));
      } else if (OB_ISNULL(task = new(buf)ObConcurrentTransactionTask(config, operations, readonlys))) {
        ret = OB_ERR_UNEXPECTED;
        TESTBENCH_LOG(ERROR, "create new concurrent transaction task failed", KR(ret));
      } else if (OB_FAIL(executor_pool_->push_task(task))) {
        TESTBENCH_LOG(ERROR, "push new concurrent transaction task failed", KR(ret));
      }
    }
  }
  return ret;
}
} // namespace testbench
} // namespace oceanbase