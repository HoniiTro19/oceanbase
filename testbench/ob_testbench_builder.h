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

#ifndef _OCEANBASE_TESTBENCH_BUILDER_H_
#define _OCEANBASE_TESTBENCH_BUILDER_H_

#include "testbench/ob_testbench_transaction_executor_pool.h"
#include "testbench/ob_testbench_options.h"
#include "lib/container/ob_se_array.h"
#include "testbench/ob_testbench_mysql_proxy.h"
#include "testbench/ob_testbench_transaction_scheduler.h"

namespace oceanbase
{
namespace testbench
{
  struct ObTestbenchOptions
  {
    ObTestbenchOptions();
    ~ObTestbenchOptions();

    int parse_options();
    void destroy_options();

    int workload_opts_cnt_;
    int8_t log_level_;
    int log_file_size_;
    char log_dir_[OB_MAX_CONTEXT_STRING_LENGTH];
    char log_file_[OB_MAX_CONTEXT_STRING_LENGTH];
    int duration_;
    ObArray<ObIWorkloadOptions *> workloads_opts_;
    ObDatasetOptions *dataset_opts_;
    ObStatisticsCollectorOptions *statistics_opts_;
    ObConnectionOptions *connection_opts_;

    TO_STRING_KV(K_(workload_opts_cnt), K_(log_level), K_(log_level), KCSTRING_(log_dir), KCSTRING_(log_file), K_(duration));
  };

  class ObTestbenchBuilder
  {
  public:
    static ObTestbenchBuilder &get_instance();

  public:
    ObTestbenchBuilder();
    ~ObTestbenchBuilder();

    int init(ObTestbenchOptions &opts);
    int init_config();
    int init_sql_proxy();
    int init_transaction_executor();
    int init_statistics_collector();
    int init_transaction_scheduler();
    int start_service();
    void stop_service();

  private:
    bool is_inited_;
    ObTestbenchMySQLProxy sql_proxy_;
    ObTestbenchStatisticsCollector statistics_collector_;
    ObTestbenchTransactionExecutorPool executor_pool_;
    ObTestbenchTransactionScheduler transaction_scheduler_;
    ObTestbenchOptions *opts_;

  private:
    DISALLOW_COPY_AND_ASSIGN(ObTestbenchBuilder);
  };

  inline ObTestbenchBuilder &ObTestbenchBuilder::get_instance()
  {
    static ObTestbenchBuilder builder;
    return builder;
  }
} // namespace testbench
} // namespace oceanbase
#endif