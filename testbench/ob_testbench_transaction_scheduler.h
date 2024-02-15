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

#ifndef _OCEANBASE_TESTBENCH_TRANSACTION_SCHEDULER_H_
#define _OCEANBASE_TESTBENCH_TRANSACTION_SCHEDULER_H_

#include "testbench/ob_testbench_mysql_proxy.h"
#include "testbench/ob_testbench_location_cache.h"
#include "testbench/ob_testbench_transaction_executor_pool.h"
#include "testbench/ob_testbench_statistics_collector.h"

namespace oceanbase
{
namespace testbench
{
class ObTestbenchTransactionScheduler : public lib::TGRunnable
{
public:
  ObTestbenchTransactionScheduler();
  ~ObTestbenchTransactionScheduler();
  
  int init(ObTestbenchMySQLProxy *mysql_proxy, ObTestbenchTransactionExecutorPool *executor_pool, ObTestbenchStatisticsCollector *statistics_collector, ObIWorkloadOptions *workload_options, ObDatasetOptions *dataset_options, ObConnectionOptions *connection_options);
  int start();
  void stop();
  void wait();
  void destroy();
  virtual void run1() override;
  int generate_distributed_txn_task();
  int generate_contention_txn_task();
  int generate_deadlock_txn_task();
  int generate_concurrent_txn_task();

private:
  int tg_id_;
  bool is_inited_;
  WorkloadType txn_type_;
  int64_t cur_row_id_;
  const char *table_name_;
  ObTestbenchMySQLProxy *mysql_proxy_;
  ObTestbenchLocationCache location_cache_;
  ObTestbenchTransactionExecutorPool *executor_pool_;
  ObTestbenchStatisticsCollector *statistics_collector_;
  ObIWorkloadOptions *workload_options_;
  ObDatasetOptions *dataset_options_;
  ObArenaAllocator allocator_;
  ObRandom random_;
};
} // namespace testbench
} // namesapce oceanbase
#endif