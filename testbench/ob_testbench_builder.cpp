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

#include "lib/oblog/ob_log.h"
#include "ob_testbench_builder.h"

namespace oceanbase
{
namespace testbench
{
ObTestbenchOptions::ObTestbenchOptions() 
  : workload_opts_cnt_(0), 
    log_level_(OB_LOG_LEVEL_DEBUG),
    log_file_size_(256 * 1024 * 1024),
    duration_(1), 
    workloads_opts_() 
{
  snprintf(log_dir_, OB_MAX_CONTEXT_STRING_LENGTH, "%s", "log");
  snprintf(log_file_, OB_MAX_CONTEXT_STRING_LENGTH, "%s", "log/scheduler.log");
}

ObTestbenchOptions::~ObTestbenchOptions() {}

int ObTestbenchOptions::parse_options() {
  int ret = OB_SUCCESS;
  if (OB_FAIL(dataset_opts_->parse_options())) {
    TESTBENCH_LOG(WARN, "parse_options for dataset failed", KR(ret));
  } else if (OB_FAIL(statistics_opts_->parse_options())) {
    TESTBENCH_LOG(WARN, "parse_options for statistics collector failed", KR(ret));
  } else if (OB_FAIL(connection_opts_->parse_options())) {
    TESTBENCH_LOG(WARN, "parse_options for connection parameters failed", KR(ret));
  } else {
    for (int i = 0; i < workload_opts_cnt_; ++i) {
      if (OB_FAIL(workloads_opts_[i]->parse_options())) {
        TESTBENCH_LOG(WARN, "parse_options for workload fail", K(ret));
      }
    }
  }  
  return ret;
}

void ObTestbenchOptions::destroy_options() {
  for (int i = 0; i < workload_opts_cnt_; ++i) {
    ObIWorkloadOptions *&opt = workloads_opts_[i];
    if (opt != nullptr) {
      OB_DELETE(ObIWorkloadOptions, "WorkloadOptions", opt);
    }
  }
  if (dataset_opts_ != nullptr) {
    OB_DELETE(ObDatasetOptions, "WorkloadOptions", dataset_opts_);
  }
  if (statistics_opts_ != nullptr) {
    OB_DELETE(ObStatisticsCollectorOptions, "WorkloadOptions", statistics_opts_);
  }
  if (connection_opts_ != nullptr) {
    OB_DELETE(ObConnectionOptions, "WorkloadOptions", connection_opts_);
  }
}

ObTestbenchBuilder::ObTestbenchBuilder() 
  : is_inited_(false), 
    sql_proxy_(),
    statistics_collector_(),
    executor_pool_()
{}

ObTestbenchBuilder::~ObTestbenchBuilder() {}

int ObTestbenchBuilder::init(ObTestbenchOptions &opts) {
  int ret = OB_SUCCESS;
  opts_ = &opts;
  ObPLogWriterCfg log_cfg;
  if (IS_INIT) {
    ret = OB_INIT_TWICE;
    TESTBENCH_LOG(WARN, "ObTestbenchBuilder init twice", KR(ret));
  } else if (OB_FAIL(OB_LOGGER.init(log_cfg, false))) {
    TESTBENCH_LOG(WARN, "async log init error", KR(ret));
  } else if (OB_FAIL(init_config())) {
    TESTBENCH_LOG(WARN, "ObTestbenchBuilder init_config fail, use default config", KR(ret));
  } else if (OB_FAIL(init_sql_proxy())) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder init_sql_proxy fail", KR(ret));
  } else if (OB_FAIL(init_statistics_collector())) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder init_workload_executor fail", KR(ret));
  } else if (OB_FAIL(init_transaction_executor())) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder init_transaction_executor fail", KR(ret));
  } else if (OB_FAIL(init_transaction_scheduler())) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder init_transaction_scheduler fail", KR(ret));
  } else {
    is_inited_ = true;
    TESTBENCH_LOG(INFO, "ObTestbenchBuilder init success");
  }
  return ret;
}

int ObTestbenchBuilder::init_config() {
  int ret = OB_SUCCESS;
  if (OB_FAIL(opts_->parse_options())) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder parser options failed", KR(ret));
  } else {
    TESTBENCH_LOG(INFO, "ObTestbenchBuilder init config succeed");
  }
  return ret;
}

int ObTestbenchBuilder::init_sql_proxy() {
  int ret = OB_SUCCESS;
  common::ObAddr addr;
  if (OB_UNLIKELY(!addr.set_ip_addr(opts_->connection_opts_->get_cluster_host(), opts_->connection_opts_->get_cluster_port()))) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder addr set_ip_addr fail", KR(ret));
  } else if (OB_FAIL(sql_proxy_.set_server_provider_param(addr))) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder sql_proxy_ set_server_provider_param fail", KR(ret));
  } else if (OB_FAIL(sql_proxy_.set_connection_pool_param(opts_->connection_opts_->get_cluster_user(), opts_->connection_opts_->get_cluster_pass(), opts_->connection_opts_->get_cluster_db_name()))) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder sql_proxy_ set_connection_pool_param fail", KR(ret));
  } else if (OB_FAIL(sql_proxy_.init())) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder sql_proxy_ init fail", KR(ret));
  } else {
    TESTBENCH_LOG(INFO, "ObTestbenchBuilder init sql proxy succeed");
  }
  return ret;
}

int ObTestbenchBuilder::init_statistics_collector() {
  int ret = OB_SUCCESS;
  if (OB_FAIL(statistics_collector_.init(opts_->statistics_opts_))) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder init statistics collector failed", KR(ret), "statistics_options", statistics_opts);
  } else {
    TESTBENCH_LOG(INFO, "ObTestbenchBuilder init statistics collector succeed");
  }
  return ret;
}

int ObTestbenchBuilder::init_transaction_executor() {
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(opts_->workload_opts_cnt_ < 1)) {
    TESTBENCH_LOG(ERROR, "get empty workload options", KR(ret), "opts_cnt", opts_->workload_opts_cnt_);
  } else {
    ObIWorkloadOptions *options = opts_->workloads_opts_.at(0);
    if (OB_FAIL(executor_pool_.init(options->get_thread_num(), options->get_task_queue_limit(), &sql_proxy_))) {
      TESTBENCH_LOG(ERROR, "ObTestbenchBuilder init transaction executor failed", KR(ret), KP(options));
    } else {
      TESTBENCH_LOG(INFO, "ObTestbenchBuilder init transaction executor pool succeed");
    }
    if (OB_UNLIKELY(opts_->workload_opts_cnt_ > 1)) {
      TESTBENCH_LOG(WARN, "get multiple workload options, use the first one by default", "opts_cnt", opts_->workload_opts_cnt_, KP(options));
    }
  }
  return ret;
}


int ObTestbenchBuilder::init_transaction_scheduler() {
  int ret = OB_SUCCESS;
  ObIWorkloadOptions *options = opts_->workloads_opts_.at(0);
  if (OB_FAIL(transaction_scheduler_.init(&sql_proxy_, &executor_pool_, &statistics_collector_, options, opts_->dataset_opts_, opts_->connection_opts_))) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder init transaction scheduler failed", KR(ret));
  } else {
    TESTBENCH_LOG(INFO, "ObTestbenchBuilder init transaction scheduler succeed");
  }
  return ret;
}

int ObTestbenchBuilder::start_service() {
  int ret = OB_SUCCESS;
  if (OB_FAIL(sql_proxy_.start_service())) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder sql_proxy_ start_service fail", KR(ret));
  } else if (OB_FAIL(statistics_collector_.start())) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder statistics collector start failed", KR(ret));
  } else if (OB_FAIL(executor_pool_.start())) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder transaction executor pool start failed", KR(ret));
  } else if (OB_FAIL(transaction_scheduler_.start())) {
    TESTBENCH_LOG(ERROR, "ObTestbenchBuilder transaction scheduler start failed", KR(ret));
  } else {
    TESTBENCH_LOG(INFO, "ObTestbenchBuilder start service succeed");
  }
  return ret;
}

void ObTestbenchBuilder::stop_service() {
  int ret = OB_SUCCESS;
  transaction_scheduler_.destroy();
  executor_pool_.destroy();
  if (OB_FAIL(statistics_collector_.generate_report())) {
    TESTBENCH_LOG(ERROR, "statistics collector generate report failed", KR(ret));
  }
  statistics_collector_.destroy();
  sql_proxy_.destroy();
  opts_->destroy_options();
  TESTBENCH_LOG(INFO, "ObTestbenchBuilder stop_service success");
}
} // namespace testbench
} // namespace oceanbase