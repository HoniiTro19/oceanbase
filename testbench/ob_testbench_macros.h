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
  X(DEADLOCK, "deadlock")                               \
  X(CONCURRENT, "concurrent")

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
  X(THREADS, "threads") \
  X(TASKS, "tasks")

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
  X(ABORTS, "aborts")                       \
  X(END, "end")

// deadlock transaction workload options macros
#define DEADLOCK_TXN_OPTIONS                \
  GLOBAL_OPTIONS                            \
  X(CONCURRENCY, "concurrency")             \
  X(CHAINS, "chains")                       \
  X(END, "end")  

// dataset options macros
#define DATASET_OPTIONS                      \
  X(PARTITIONS, "partitions")                \
  X(ROWS, "rows")                            \
  X(END, "end")

// statistics collector options macros
#define STATISTICS_OPTIONS                   \
  X(CAPACITY, "capacity")                    \
  X(MINIMUM, "minimum")                      \
  X(MAXIMUM, "maximum")                      \
  X(THREADS, "threads")                      \
  X(TASKS, "tasks")                          \
  X(END, "end")

// connection options macros
#define CONNECTION_OPTIONS                  \
  X(HOST, "host")                           \
  X(USER, "user")                           \
  X(PORT, "port")                           \
  X(PASSWORD, "password")                   \
  X(DATABASE, "database")                   \
  X(TABLE, "table")                         \
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

  enum class DatasetOptions
  {
    DATASET_OPTIONS
  };

  enum class StatisticsOptions
  {
    STATISTICS_OPTIONS
  };

  enum class ConnectionOptions
  {
    CONNECTION_OPTIONS
  };
#undef X

#define X(key, value) value,
  const char *const distributed_txn_opts[] = { DISTRIBUTED_TXN_OPTIONS };
  const char *const contention_txn_opts[] = { CONTENTION_TXN_OPTIONS };
  const char *const deadlock_txn_opts[] = { DEADLOCK_TXN_OPTIONS };
  const char *const dataset_opts[] = { DATASET_OPTIONS };
  const char *const statistics_opts[] = { STATISTICS_OPTIONS };
  const char *const connection_opts[] = { CONNECTION_OPTIONS };
#undef X

/*
                                          code segment timer macros
*/

#define TIMER(name, seg)                                            \
  int64_t begin_time = common::ObTimeUtility::current_time();       \
  seg;                                                              \
  int64_t end_time = common::ObTimeUtility::current_time();         \
  TESTBENCH_LOG(TRACE, name, "time", end_time - begin_time);        \

#define TIMER_RET(name, seg)                                        \
  {                                                                 \
    TIMER(name, seg)                                                \
    ret;                                                            \
  }

} // namespace testbench
} // namespace oceanbase

#endif