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

#include "ob_testbench_options.h"
#include "ob_testbench_utils.h"

namespace oceanbase {
namespace testbench {
/*
 *                                     ObIOptions     
 */
ObIOptions::ObIOptions(const char *src_opt_str) : src_opt_str_(src_opt_str) {}

ObIOptions::~ObIOptions() {}

int ObIOptions::parse_options() {
  int ret = OB_SUCCESS;
  char **opts = (char **)ob_malloc(MAX_OPTS_CNT * sizeof(char *), ObModIds::OB_TESTBENCH_OPTIONS);
  int opts_cnt = 0;
  if (OB_ISNULL(opts)) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    TESTBENCH_LOG(WARN, "allocate memory for options failed", K(ret));
  } else if (OB_FAIL(split(src_opt_str_, ",", 0, opts, opts_cnt))) {
    TESTBENCH_LOG(WARN, "parse options for distributed transaction workload fail, use default options");
  } else {
    for (int i = 0; i < opts_cnt; ++i) {
      char **kv = (char **)ob_malloc(2 * sizeof(char *), ObModIds::OB_TESTBENCH_OPTIONS);
      int kv_cnt = 0;
      if (OB_FAIL(split(*(opts + i), "=", 2, kv, kv_cnt))) {
        TESTBENCH_LOG(WARN, "parse key value options for distributed transaction workload fail", "opts", *(opts + i));
      } else {
        fill_options(*kv, *(kv + 1));
      }
      ob_free(kv);
    }
  }
  ob_free(opts);
  return ret;
}

/*
 *                                           ObIWorkloadOptions     
 */
ObIWorkloadOptions::ObIWorkloadOptions(const char *opt_str)
  : ObIOptions(opt_str) {}

ObIWorkloadOptions::~ObIWorkloadOptions() {}

/*
 *                                       ObDistributedTransactionOptions  
 */
ObDistributedTransactionOptions::ObDistributedTransactionOptions(const char *opt_str)
  : ObIWorkloadOptions(opt_str),
    participants_(1),
    operations_(1)
{}

ObDistributedTransactionOptions::~ObDistributedTransactionOptions() {}

int ObDistributedTransactionOptions::fill_options(const char *key, const char *value) {
  int ret = OB_SUCCESS;
  if (OB_ISNULL(value)) {
    ret = OB_INVALID_ARGUMENT;
    TESTBENCH_LOG(ERROR, "unexpected option for distributed transaction workload", KCSTRING(key), KCSTRING(value));
  } else {
    DistributedTxnOptions match = DistributedTxnOptions::END;
    for (int64_t i = 0; i < sizeof(distributed_txn_opts) / sizeof(distributed_txn_opts[0]); ++i) {
      if (0 == strcmp(distributed_txn_opts[i], key)) {
        match = static_cast<DistributedTxnOptions>(i);
        break;
      }
    }
    switch (match) {
    case DistributedTxnOptions::THREADS:
      thread_num_ = atoi(value);
      break;
    case DistributedTxnOptions::TASKS:
      task_queue_limit_ = atoi(value);
      break;
    case DistributedTxnOptions::PARTICIPANTS:
      participants_ = atoi(value);
      break;
    case DistributedTxnOptions::OPERATIONS:
      operations_ = atoi(value);
      break;
    default:
      ret = OB_INVALID_ARGUMENT;
      TESTBENCH_LOG(WARN, "unexpected option for distributed transaction workload", KCSTRING(key), KCSTRING(value));
      break;
    }
  }
  return ret;
}

WorkloadType ObDistributedTransactionOptions::get_type() {
  return WorkloadType::DISTRIBUTED_TRANSACTION;
}

/*
 *                                     ObContentionTransactionOptions 
 */
ObContentionTransactionOptions::ObContentionTransactionOptions(const char *opt_str)
  : ObIWorkloadOptions(opt_str),
    concurrency_(1),
    operations_(1),
    aborts_(0)
{}

ObContentionTransactionOptions::~ObContentionTransactionOptions() {}

int ObContentionTransactionOptions::fill_options(const char *key, const char *value) {
  int ret = OB_SUCCESS;
  if (OB_ISNULL(value)) {
    ret = OB_INVALID_ARGUMENT;
    TESTBENCH_LOG(ERROR, "unexpected option for contention transaction workload", KCSTRING(key), KCSTRING(value));
  } else {
    ContentionTxnOptions match = ContentionTxnOptions::END;
    for (int64_t i = 0; i < sizeof(contention_txn_opts) / sizeof(contention_txn_opts[0]); ++i) {
      if (0 == strcmp(contention_txn_opts[i], key)) {
        match = static_cast<ContentionTxnOptions>(i);
        break;
      }
    }
    switch (match) {
    case ContentionTxnOptions::THREADS:
      thread_num_ = atoi(value);
      break;
    case ContentionTxnOptions::TASKS:
      task_queue_limit_ = atoi(value);
      break;
    case ContentionTxnOptions::CONCURRENCY:
      concurrency_ = atoi(value);
      break;
    case ContentionTxnOptions::OPERATIONS:
      operations_ = atoi(value);
      break;
    case ContentionTxnOptions::ABORTS:
      aborts_ = atoi(value);
      break;
    default:
      ret = OB_INVALID_ARGUMENT;
      TESTBENCH_LOG(WARN, "unexpected option for contention transaction workload", KCSTRING(key), KCSTRING(value));
      break;
    }
  }
  return ret;
} 

WorkloadType ObContentionTransactionOptions::get_type() {
  return WorkloadType::CONTENTION;
}

/*
 *                                      ObDeadlockTransactionOptions     
 */
ObDeadlockTransactionOptions::ObDeadlockTransactionOptions(const char *opt_str)
  : ObIWorkloadOptions(opt_str),
    concurrency_(1),
    chains_(1)
{}

ObDeadlockTransactionOptions::~ObDeadlockTransactionOptions() {}

int ObDeadlockTransactionOptions::fill_options(const char *key, const char *value) {
  int ret = OB_SUCCESS;
  if (OB_ISNULL(value)) {
    ret = OB_INVALID_ARGUMENT;
    TESTBENCH_LOG(ERROR, "unexpected option for deadlock transaction workload", KCSTRING(key), KCSTRING(value));
  } else {
    DeadlockTxnOptions match = DeadlockTxnOptions::END;
    for (int64_t i = 0; i < sizeof(deadlock_txn_opts) / sizeof(deadlock_txn_opts[0]); ++i) {
      if (0 == strcmp(deadlock_txn_opts[i], key)) {
        match = static_cast<DeadlockTxnOptions>(i);
        break;
      }
    }
    switch (match) {
    case DeadlockTxnOptions::THREADS:
      thread_num_ = atoi(value);
      break;
    case DeadlockTxnOptions::TASKS:
      task_queue_limit_ = atoi(value);
      break;
    case DeadlockTxnOptions::CONCURRENCY:
      concurrency_ = atoi(value);
      break;
    case DeadlockTxnOptions::CHAINS:
      chains_ = atoi(value);
      break;
    default:
      ret = OB_INVALID_ARGUMENT;
      TESTBENCH_LOG(WARN, "unexpected option for deadlock transaction workload", KCSTRING(key), KCSTRING(value));
      break;
    }
  }
  return ret;
}

WorkloadType ObDeadlockTransactionOptions::get_type() {
  return WorkloadType::DEADLOCK;
}

/*
 *                                      ObConcurrentTransactionOptions     
 */
ObConcurrentTransactionOptions::ObConcurrentTransactionOptions(const char *opt_str)
  : ObIWorkloadOptions(opt_str),
    concurrency_(1),
    operations_(1),
    readonly_(0)
{}

ObConcurrentTransactionOptions::~ObConcurrentTransactionOptions() {}

int ObConcurrentTransactionOptions::fill_options(const char *key, const char *value) {
  int ret = OB_SUCCESS;
  if (OB_ISNULL(value)) {
    ret = OB_INVALID_ARGUMENT;
    TESTBENCH_LOG(ERROR, "unexpected option for deadlock transaction workload", KCSTRING(key), KCSTRING(value));
  } else {
    ConcurrentTxnOptions match = ConcurrentTxnOptions::END;
    for (int64_t i = 0; i < sizeof(concurrent_txn_opts) / sizeof(concurrent_txn_opts[0]); ++i) {
      if (0 == strcmp(concurrent_txn_opts[i], key)) {
        match = static_cast<ConcurrentTxnOptions>(i);
        break;
      }
    }
    switch (match) {
    case ConcurrentTxnOptions::THREADS:
      thread_num_ = atoi(value);
      break;
    case ConcurrentTxnOptions::TASKS:
      task_queue_limit_ = atoi(value);
      break;
    case ConcurrentTxnOptions::CONCURRENCY:
      concurrency_ = atoi(value);
      break;
    case ConcurrentTxnOptions::OPERATIONS:
      operations_ = atoi(value);
      break;
    case ConcurrentTxnOptions::READONLY:
      readonly_ = atoi(value);
      break;
    default:
      ret = OB_INVALID_ARGUMENT;
      TESTBENCH_LOG(WARN, "unexpected option for concurrent transaction workload", KCSTRING(key), KCSTRING(value));
      break;
    }
  }
  return ret;
}

WorkloadType ObConcurrentTransactionOptions::get_type() {
  return WorkloadType::CONCURRENT;
}

/*
 *                                      ObDatasetOptions     
 */
ObDatasetOptions::ObDatasetOptions(const char *opt_str)
  : ObIOptions(opt_str),
    partitions_(0),
    rows_(0)
{}

ObDatasetOptions::~ObDatasetOptions() {}

int ObDatasetOptions::fill_options(const char *key, const char *value) {
  int ret = OB_SUCCESS;
  if (OB_ISNULL(value)) {
    ret = OB_INVALID_ARGUMENT;
    TESTBENCH_LOG(ERROR, "unexpected option for dataset", KCSTRING(key), KCSTRING(value));
  } else {
    DatasetOptions match = DatasetOptions::END;
    for (int64_t i = 0; i < sizeof(dataset_opts) / sizeof(dataset_opts[0]); ++i) {
      if (0 == strcmp(dataset_opts[i], key)) {
        match = static_cast<DatasetOptions>(i);
        break;
      }
    }
    switch (match) {
    case DatasetOptions::PARTITIONS:
      partitions_ = atoi(value);
      break;
    case DatasetOptions::ROWS:
      rows_ = atoi(value);
      break;
    default:
      ret = OB_INVALID_ARGUMENT;
      TESTBENCH_LOG(WARN, "unexpected option for dataset", KCSTRING(key), KCSTRING(value));
      break;
    }
  }
  return ret; 
}

/*
 *                                      ObStatisticsCollectorOptions     
 */
ObStatisticsCollectorOptions::ObStatisticsCollectorOptions(const char *opt_str)
  : ObIOptions(opt_str),
    bucket_capacity_(500),
    bucket_min_percentage_(10),
    bucket_max_percentage_(80),
    thread_num_(1),
    task_queue_limit_(99999)
{}

ObStatisticsCollectorOptions::~ObStatisticsCollectorOptions() {}

int ObStatisticsCollectorOptions::fill_options(const char *key, const char *value) {
  int ret = OB_SUCCESS;
  if (OB_ISNULL(value)) {
    ret = OB_INVALID_ARGUMENT;
    TESTBENCH_LOG(ERROR, "unexpected option for statistics collector", KCSTRING(key), KCSTRING(value));
  } else {
    StatisticsOptions match = StatisticsOptions::END;
    for (int64_t i = 0; i < sizeof(statistics_opts) / sizeof(statistics_opts[0]); ++i) {
      if (0 == strcmp(statistics_opts[i], key)) {
        match = static_cast<StatisticsOptions>(i);
        break;
      }
    }
    switch (match) {
    case StatisticsOptions::CAPACITY:
      bucket_capacity_ = atoi(value);
      break;
    case StatisticsOptions::MAXIMUM:
      bucket_max_percentage_ = atoi(value);
      break;
    case StatisticsOptions::MINIMUM:
      bucket_min_percentage_ = atoi(value);
      break;
    case StatisticsOptions::THREADS:
      thread_num_ = atoi(value);
      break;
    case StatisticsOptions::TASKS:
      task_queue_limit_ = atoi(value);
      break;
    default:
      ret = OB_INVALID_ARGUMENT;
      TESTBENCH_LOG(WARN, "unexpected option for statistics collector", KCSTRING(key), KCSTRING(value));
      break;
    }
  }
  return ret;
}

/*
 *                                      ObConnectionOptions     
 */
ObConnectionOptions::ObConnectionOptions(const char *opt_str)
  : ObIOptions(opt_str),
    cluster_port_(2881)
{
  cluster_host_[0] = '\0';
  cluster_user_[0] = '\0';
  cluster_pass_[0] = '\0';
  cluster_db_name_[0] = '\0';
  cluster_table_name_[0] = '\0';
}

ObConnectionOptions::~ObConnectionOptions() {}

int ObConnectionOptions::fill_options(const char *key, const char *value) {
  int ret = OB_SUCCESS;
  if (OB_ISNULL(value)) {
    ret = OB_INVALID_ARGUMENT;
    TESTBENCH_LOG(ERROR, "unexpected option for connection", K(ret), KCSTRING(key), KCSTRING(value));
  } else {
    int len = 0;
    ConnectionOptions match = ConnectionOptions::END;
    for (int64_t i = 0; i < sizeof(connection_opts) / sizeof(connection_opts[0]); ++i) {
      if (0 == strcmp(connection_opts[i], key)) {
        match = static_cast<ConnectionOptions>(i);
        break;
      }
    }
    switch (match) {
    case ConnectionOptions::HOST:
      if (FALSE_IT(len = snprintf(cluster_host_, OB_MAX_HOST_NAME_LENGTH, "%s", value))) {
        // impossible
      } else if (OB_UNLIKELY(len <= 0) || OB_UNLIKELY(len > OB_MAX_HOST_NAME_LENGTH)) {
        ret = OB_INVALID_ARGUMENT;
        TESTBENCH_LOG(ERROR, "set cluster host error", K(ret), KCSTRING(value));
      }
      break;
    case ConnectionOptions::USER:
      if (FALSE_IT(len = snprintf(cluster_user_, OB_MAX_USER_NAME_BUF_LENGTH, "%s", value))) {
        // impossible
      } else if (OB_UNLIKELY(len <= 0) || OB_UNLIKELY(len > OB_MAX_USER_NAME_BUF_LENGTH)) {
        ret = OB_INVALID_ARGUMENT;
        TESTBENCH_LOG(ERROR, "set cluster user error", K(ret), KCSTRING(value));
      }
      break;
    case ConnectionOptions::PASSWORD:
      if (FALSE_IT(len = snprintf(cluster_pass_, OB_MAX_PASSWORD_BUF_LENGTH, "%s", value))) {
        // impossible
      } else if (OB_UNLIKELY(len <= 0) || OB_UNLIKELY(len > OB_MAX_PASSWORD_BUF_LENGTH)) {
        ret = OB_INVALID_ARGUMENT;
        TESTBENCH_LOG(ERROR, "set cluster password error", K(ret), KCSTRING(value));
      }
      break;
    case ConnectionOptions::DATABASE:
      if (FALSE_IT(len = snprintf(cluster_db_name_, OB_MAX_DATABASE_NAME_BUF_LENGTH, "%s", value))) {
        // impossible
      } else if (OB_UNLIKELY(len <= 0) || OB_UNLIKELY(len > OB_MAX_DATABASE_NAME_BUF_LENGTH)) {
        ret = OB_INVALID_ARGUMENT;
        TESTBENCH_LOG(ERROR, "set cluster database error", K(ret), KCSTRING(value));
      }
      break;
    case ConnectionOptions::TABLE:
      if (FALSE_IT(len = snprintf(cluster_table_name_, OB_MAX_TABLE_NAME_BUF_LENGTH, "%s", value))) {
        // impossible
      } else if (OB_UNLIKELY(len <= 0) || OB_UNLIKELY(len > OB_MAX_TABLE_NAME_BUF_LENGTH)) {
        ret = OB_INVALID_ARGUMENT;
        TESTBENCH_LOG(ERROR, "set test table error", K(ret), KCSTRING(value));
      }
      break;
    case ConnectionOptions::PORT:
      cluster_port_ = atoi(value);
      break;
    default:
      ret = OB_INVALID_ARGUMENT;
      TESTBENCH_LOG(WARN, "unexpected option for connection", KCSTRING(key), KCSTRING(value));
      break;
    }
  }
  return ret;
}
} // namespace testbench
} // namespace oceanbase