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

// concurrent transaction workload options macros
#define CONCURRENT_TXN_OPTIONS              \
  GLOBAL_OPTIONS                            \
  X(CONCURRENCY, "concurrency")             \
  X(OPERATIONS, "operations")               \
  X(READONLY, "readonly")                   \
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

  enum class ConcurrentTxnOptions
  {
    CONCURRENT_TXN_OPTIONS
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
  const char *const concurrent_txn_opts[] = { CONCURRENT_TXN_OPTIONS };
  const char *const dataset_opts[] = { DATASET_OPTIONS };
  const char *const statistics_opts[] = { STATISTICS_OPTIONS };
  const char *const connection_opts[] = { CONNECTION_OPTIONS };
#undef X

/*
                                          latency task type macros
*/
#define LATENCY_TYPE_OPTIONS                          \
  X(INVALID_LATENCY_TASK, "invalid")                  \
  X(RPC_LATENCY_TASK, "rpc")                          \
  X(DISTRIBUTED_TXN_LATENCY_TASK, "distributed_txn")  \
  X(CONTENTION_TXN_LATENCY_TASK, "contention_txn")    \
  X(DEADLOCK_TXN_LATENCY_TASK, "deadlock_txn")        \
  X(CONCURRENT_TXN_LATENCY_TASK, "concurrent_txn")    \
  X(COMMIT_SQL_LATENCY_TASK, "commit")                \
  X(LOCK_SQL_LATENCY_TASK, "lock")                    \
  X(DEADLOCK_SQL_LATENCY_TASK, "deadlock")            \
  X(ELECTION_LATENCY_TASK, "election")                \
  X(ROLLBACK_TXN_LATENCY_TASK, "rollback_txn")        \
  X(LATENCY_TASK_TYPE_CNT, "end")  

#define X(key, value) key,
enum ObLatencyTaskType {
  LATENCY_TYPE_OPTIONS
};
#undef X

#define X(key, value) value,
const char *const latency_task_names[] = { LATENCY_TYPE_OPTIONS };
#undef X

/*
                                          code segment timer macros
*/

#define TIMER(name, seg)                                            \
  int64_t begin_time = common::ObTimeUtility::current_time();       \
  seg;                                                              \
  int64_t end_time = common::ObTimeUtility::current_time();         \
  TESTBENCH_LOG(INFO, name, "time", end_time - begin_time);        \

#define TIMER_RET(name, seg)                                        \
  {                                                                 \
    TIMER(name, seg)                                                \
    ret;                                                            \
  }

} // namespace testbench
} // namespace oceanbase

#endif