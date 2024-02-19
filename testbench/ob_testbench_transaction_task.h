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
 *
 */

#ifndef _OCEANBASE_WORKLOAD_TRANSACTION_TASK_H_
#define _OCEANBASE_WORKLOAD_TRANSACTION_TASK_H_

#include "ob_testbench_options.h"
#include "ob_testbench_mysql_proxy.h"
#include "ob_testbench_statistics_collector.h"
#include "lib/string/ob_sql_string.h"
#include "lib/mysqlclient/ob_mysql_prepared_statement.h"
#include "lib/time/ob_time_utility.h"
#include <poll.h>

namespace oceanbase
{
  namespace testbench
  {
    typedef ObArrayWrap<ObMySQLConnection *> Connections;
    typedef ObArrayWrap<ObMySQLPreparedStatement> PreparedStatements;
    typedef ObArrayWrap<int64_t> Latencys;
    typedef ObArrayWrap<bool> Commits;
    typedef ObArrayWrap<struct pollfd> Pollfds;
    typedef ObArray<int64_t> Parameters;
    typedef ObArray<uint64_t> Dblinks;
    typedef ObArray<bool> Readonlys;

    struct BasicTaskConfig {
      const char *table_name;
      int64_t connections;
      ObTestbenchStatisticsCollector *collector;
      ObTestbenchMySQLProxy *mysql_proxy;
      Parameters parameters;
      Dblinks dblinks;
      int64_t row_id_start;
      TO_STRING_KV(K(table_name), K(connections), KP(collector), KP(mysql_proxy), K(parameters), K(dblinks), K(row_id_start));
    };

    class ObIWorkloadTransactionTask
    {
    public:
      ObIWorkloadTransactionTask(BasicTaskConfig config);
      virtual ~ObIWorkloadTransactionTask();
      virtual WorkloadType get_type() = 0;
      virtual int init();
      virtual int init_pollfds();
      // NOTE: errors duration execution should be reflected through rollback, the return code should almostly be OB_SUCCESS
      virtual int execute_transactions() = 0;
      virtual int release_dblinks() = 0;
      virtual int collect_statistics() = 0;
      inline Latencys *get_latencys() { return &latencys_; }
      inline Latencys *get_cumulative_latencys() { return &cumulative_latencys_; }
      inline Commits *get_commits() { return &commits_; }

    protected:
      virtual void begin_trace_latency(int64_t &latency);
      virtual void end_trace_latency(int64_t &latency);
      virtual int prepare_arrays();
      virtual int wait_for_connection_block(int64_t conn_idx);
      virtual int find_connection_ready_round_robin(int64_t &conn_idx);
      virtual int find_connection_ready_multiplexing(int64_t &conn_idx);
      virtual int find_mysql_ready(int64_t &conn_idx);
      virtual int record_latency(ObLatencyTaskType type, int64_t latency);
      virtual int bind_params(int64_t &partition_id, int64_t &row_id, ObMySQLPreparedStatement &stmt);
      virtual int bind_multi_params(int64_t &partition_id0, int64_t &row_id0, int64_t &partition_id1, int64_t &row_id1, ObMySQLPreparedStatement &stmt);
      virtual int bind_result(int64_t &col, ObMySQLPreparedStatement &stmt);

    protected:
      const char *table_name_;
      int64_t connection_count_;
      Connections connections_;
      ObTestbenchMySQLProxy *mysql_proxy_;
      ObTestbenchStatisticsCollector *latency_collector_;
      Parameters parameters_;
      Dblinks dblinks_;
      int64_t row_id_start_;
      Latencys latencys_;
      Latencys cumulative_latencys_;
      Commits commits_;
      Pollfds pollfds_;
      ObArenaAllocator allocator_;
      VIRTUAL_TO_STRING_KV(K_(table_name), K_(connection_count), K_(parameters), K_(dblinks), K_(row_id_start), K_(latencys), K_(cumulative_latencys), K_(commits));
    };

    class ObDistributedTransactionTask : public ObIWorkloadTransactionTask
    {
    public:
      ObDistributedTransactionTask(BasicTaskConfig basic_config, int64_t participants, int64_t operations);
      virtual ~ObDistributedTransactionTask() override;
      virtual WorkloadType get_type() override;
      virtual int init() override;
      virtual int execute_transactions() override;
      virtual int release_dblinks() override;
      virtual int collect_statistics() override;
      INHERIT_TO_STRING_KV("ObIWorkloadTransactionTask", ObIWorkloadTransactionTask, K_(participants), K_(operations), K_(lock_dist_sql));

    private:
      int64_t participants_;
      int64_t operations_;
      const char *lock_dist_sql_format_;
      ObSqlString lock_dist_sql_;
      PreparedStatements lock_dist_stmts_;
    };

    class ObContentionTransactionTask : public ObIWorkloadTransactionTask
    {
    public:
      ObContentionTransactionTask(BasicTaskConfig basic_config, int64_t aborts, int64_t operations);
      virtual ~ObContentionTransactionTask() override;
      virtual WorkloadType get_type() override;
      virtual int init() override;
      virtual int execute_transactions() override;
      virtual int release_dblinks() override;
      virtual int collect_statistics() override;
      inline int64_t get_finished() { return finished_; }
      INHERIT_TO_STRING_KV("ObIWorkloadTransactionTask", ObIWorkloadTransactionTask, K_(aborts), K_(operations), K_(lock_contention_sql));

    private:
      int push_forward_operations(int64_t conn_idx, int64_t &current_row_id);

    private:
      int64_t aborts_;
      int64_t operations_;
      const char *lock_contention_sql_format_;
      ObSqlString lock_contention_sql_;
      PreparedStatements lock_contention_stmts_;
      ObArrayWrap<int64_t> act_operations_;
      int64_t finished_;
      int64_t act_commits_;
    };

    class ObDeadlockTransactionTask : public ObIWorkloadTransactionTask
    {
    public:
      ObDeadlockTransactionTask(BasicTaskConfig basic_config, int64_t chains);
      virtual ~ObDeadlockTransactionTask() override;
      virtual WorkloadType get_type() override;
      virtual int init() override;
      virtual int execute_transactions() override;
      virtual int release_dblinks() override;
      virtual int collect_statistics() override;
      INHERIT_TO_STRING_KV("ObIWorkloadTransactionTask", ObIWorkloadTransactionTask, K_(chains), K_(lock_deadlock_sql));
    
    private:
      int push_forward_operations(int64_t conn_idx);

    private:
      int64_t chains_;
      int64_t cycle_;
      const char *lock_deadlock_sql_format_;
      const char *lock_multi_deadlock_sql_format_;
      ObSqlString lock_deadlock_sql_;
      ObSqlString lock_multi_deadlock_sql_;
      PreparedStatements lock_deadlock_stmts_;
      PreparedStatements lock_multi_deadlock_stmts_;
      int64_t finished_;
      ObArrayWrap<bool> act_commits_;
    };

    class ObConcurrentTransactionTask : public ObIWorkloadTransactionTask
    {
    public:
      ObConcurrentTransactionTask(BasicTaskConfig basic_config, int64_t operations, Readonlys readonlys);
      virtual ~ObConcurrentTransactionTask() override;
      virtual WorkloadType get_type() override;
      virtual int init() override;
      virtual int execute_transactions() override;
      virtual int release_dblinks() override;
      virtual int collect_statistics() override;
      inline int64_t get_finished() { return finished_; }
      INHERIT_TO_STRING_KV("ObIWorkloadTransactionTask", ObIWorkloadTransactionTask, K_(operations), K_(readonlys), K_(lock_wrtxn_sql), K_(lock_rdtxn_sql));
    
    private:
      int execute_read_async(int64_t conn_idx, int64_t current_row_id);
      int execute_write_async(int64_t conn_idx, int64_t current_row_id);
      int push_forward_operations(int64_t conn_idx, int64_t &current_row_id);

    private:
      int64_t operations_;
      Readonlys readonlys_;
      const char *lock_wrtxn_sql_format_;
      const char *lock_rdtxn_sql_format_;
      ObSqlString lock_wrtxn_sql_;
      ObSqlString lock_rdtxn_sql_;
      PreparedStatements lock_wrtxn_stmts_;
      PreparedStatements lock_rdtxn_stmts_;
      ObArrayWrap<bool> wait_for_reads_;
      ObArrayWrap<int64_t> act_operations_;
      int64_t finished_;
    };
  }
}
#endif