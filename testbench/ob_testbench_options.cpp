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
 *                                     ObIWorkloadOptions     
 */
ObIWorkloadOptions::ObIWorkloadOptions(const char *src_opt_str)
  : start_time_(0), duration_(5), src_opt_str_(src_opt_str)
{}

ObIWorkloadOptions::~ObIWorkloadOptions() {}

int ObIWorkloadOptions::parse_options() {
  int ret = OB_SUCCESS;
  char **opts = (char **)calloc(MAX_OPTS_CNT, sizeof(char *));
  int opts_cnt = 0;
  if (OB_FAIL(split(src_opt_str_, ",", 0, opts, opts_cnt))) {
    TESTBENCH_LOG(WARN, "parse options for distributed transaction workload fail, use default options");
  } else {
    for (int i = 0; i < opts_cnt; ++i) {
      char **kv = (char **)calloc(2, sizeof(char *));
      int kv_cnt = 0;
      if (OB_FAIL(split(*(opts + i), "=", 2, kv, kv_cnt))) {
        TESTBENCH_LOG(WARN, "parse key value options for distributed transaction workload fail", KP(*(opts + i)));
      } else {
        fill_options(*kv, *(kv + 1));
      }
      free(kv);
    }
  }
  free(opts);
  return ret;
}

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
  DistributedTxnOptions match = DistributedTxnOptions::END;
  for (int64_t i = 0; i < sizeof(distributed_txn_opts) / sizeof(distributed_txn_opts[0]); ++i) {
    if (0 == strcmp(distributed_txn_opts[i], key)) {
      match = static_cast<DistributedTxnOptions>(i);
      break;
    }
  }
  switch (match) {
  case DistributedTxnOptions::STARTTIME:
    start_time_ = atoi(value);
    break;
  case DistributedTxnOptions::DURATION:
    duration_ = atoi(value);
    break;
  case DistributedTxnOptions::PARTICIPANTS:
    participants_ = atoi(value);
    break;
  case DistributedTxnOptions::OPERATIONS:
    operations_ = atoi(value);
    break;
  default:
    TESTBENCH_LOG(WARN, "unexpected option for distributed transaction workload", KP(key), KP(value));
    ret = OB_INVALID_ARGUMENT;
    break;
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
    operations_(1)
{}

ObContentionTransactionOptions::~ObContentionTransactionOptions() {}

int ObContentionTransactionOptions::fill_options(const char *key, const char *value) {
  int ret = OB_SUCCESS;
  ContentionTxnOptions match = ContentionTxnOptions::END;
  for (int64_t i = 0; i < sizeof(contention_txn_opts) / sizeof(contention_txn_opts[0]); ++i) {
    if (0 == strcmp(contention_txn_opts[i], key)) {
      match = static_cast<ContentionTxnOptions>(i);
      break;
    }
  }
  switch (match) {
  case ContentionTxnOptions::STARTTIME:
    start_time_ = atoi(value);
    break;
  case ContentionTxnOptions::DURATION:
    duration_ = atoi(value);
    break;
  case ContentionTxnOptions::CONCURRENCY:
    concurrency_ = atoi(value);
    break;
  case ContentionTxnOptions::OPERATIONS:
    operations_ = atoi(value);
    break;
  default:
    TESTBENCH_LOG(WARN, "unexpected option for contention transaction workload", KP(key), KP(value));
    break;
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
  DeadlockTxnOptions match = DeadlockTxnOptions::END;
  for (int64_t i = 0; i < sizeof(deadlock_txn_opts) / sizeof(deadlock_txn_opts[0]); ++i) {
    if (0 == strcmp(deadlock_txn_opts[i], key)) {
      match = static_cast<DeadlockTxnOptions>(i);
      break;
    }
  }
  switch (match) {
  case DeadlockTxnOptions::STARTTIME:
    start_time_ = atoi(value);
    break;
  case DeadlockTxnOptions::DURATION:
    duration_ = atoi(value);
    break;
  case DeadlockTxnOptions::CONCURRENCY:
    concurrency_ = atoi(value);
    break;
  case DeadlockTxnOptions::CHAINS:
    chains_ = atoi(value);
    break;
  default:
    TESTBENCH_LOG(WARN, "unexpected option for deadlock transaction workload", KP(key), KP(value));
    ret = OB_INVALID_ARGUMENT;
    break;
  }
  return ret;
}

WorkloadType ObDeadlockTransactionOptions::get_type() {
  return WorkloadType::DEADLOCK;
}
} // namespace testbench
} // namespace oceanbase