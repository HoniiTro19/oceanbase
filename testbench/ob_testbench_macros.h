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

#ifndef _OCEANBASE_TESTBENCH_MACROS_
#define _OCEANBASE_TESTBENCH_MACROS_

namespace oceanbase
{
namespace testbench 
{
/*
 *                                      log printer of instance
 */ 
#define MPRINT(format, ...) fprintf(stdout, format "\n", ##__VA_ARGS__)
#define MPRINTx(format, ...)     \
  MPRINT(format, ##__VA_ARGS__); \
  exit(1)

/*
 *                                        workload type macros
 */ 
#define WORKLOAD_TYPES                                  \
  X(DISTRIBUTED_TRANSACTION, "distributed_transaction") \
  X(CONTENTION, "contention")                           \
  X(DEADLOCK, "deadlock")

#define ADD_WORKLOAD_OPTS(opts, id, src_opt_str)                                                    \
  ({                                                                                                \
    int ret = OB_SUCCESS;                                                                           \
    switch (id) {                                                                                   \
      case DISTRIBUTED_TRANSACTION:                                                                 \
        opts.push_back(OB_NEW(ObDistributedTransactionOptions, "WorkloadOptions", src_opt_str));    \
        break;                                                                                      \
      case CONTENTION:                                                                              \
        opts.push_back(OB_NEW(ObContentionTransactionOptions, "WorkloadOptions", src_opt_str));     \
        break;                                                                                      \
      case DEADLOCK:                                                                                \
        opts.push_back(OB_NEW(ObDeadlockTransactionOptions, "WorkloadOptions", src_opt_str));       \
        break;                                                                                      \
      default:                                                                                      \
        TESTBENCH_LOG(ERROR, "undefined workload type", K(id));                                     \
        break;                                                                                      \
    }                                                                                               \
  })

#define X(key, value) key,
  enum WorkloadType
  {
    WORKLOAD_TYPES
  };
#undef X

#define X(key, value) value,
  const char *const wl_types[] = {
      WORKLOAD_TYPES};
#undef X

/*
 *                                        workload options macros
 */ 

// global workload options macros
#define GLOBAL_OPTIONS       \
  X(STARTTIME, "start_time") \
  X(DURATION, "duration")

// distributed transaction workload options macros
#define DISTRIBUTED_TXN_OPTIONS              \
  GLOBAL_OPTIONS                             \
  X(PARTICIPANTS, "participants")            \
  X(OPERATIONS, "operations")                \
  X(END, "end")

// contention transaction workload options macros
#define CONTENTION_TXN_OPTIONS              \
  GLOBAL_OPTIONS                            \
  X(CONCURRENCY, "concurrency")             \
  X(OPERATIONS, "operations")               \
  X(END, "end")

// deadlock transaction workload options macros
#define DEADLOCK_TXN_OPTIONS                \
  GLOBAL_OPTIONS                            \
  X(CONCURRENCY, "concurrency")             \
  X(CHAINS, "chains")                       \
  X(END, "end")  

#define X(key, value) key,
  enum class DistributedTxnOptions
  {
    DISTRIBUTED_TXN_OPTIONS
  };

  enum class ContentionTxnOptions
  {
    CONTENTION_TXN_OPTIONS
  };

  enum class DeadlockTxnOptions
  {
    DEADLOCK_TXN_OPTIONS
  };
#undef X

#define X(key, value) value,
  const char *const distributed_txn_opts[] = { DISTRIBUTED_TXN_OPTIONS };
  const char *const contention_txn_opts[] = { CONTENTION_TXN_OPTIONS };
  const char *const deadlock_txn_opts[] = { DEADLOCK_TXN_OPTIONS };
#undef X
} // namespace testbench
} // namespace oceanbase

#endif