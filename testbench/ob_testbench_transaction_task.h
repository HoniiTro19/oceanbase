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

namespace oceanbase
{
  namespace testbench
  {
    typedef ObArrayWrap<ObMySQLConnection *> Connections;
    typedef ObArrayWrap<ObMySQLPreparedStatement> PreparedStatements;
    typedef ObArrayWrap<int64_t> Latencys;
    typedef ObArrayWrap<bool> Commits;
    typedef ObArray<int64_t> Parameters;
    typedef ObArray<uint64_t> Dblinks;

    struct BasicTaskConfig {
      const char *table_name;
      int64_t connections;
      ObTestbenchStatisticsCollector *collector;
      Dblinks dblink_id;
      ObTestbenchMySQLProxy *mysql_proxy;
      Parameters partition_id;
      int64_t row_id_start;
      TO_STRING_KV(K(table_name), K(connections), KP(collector), K(dblink_id), KP(mysql_proxy), K(partition_id), K(row_id_start));
    };

    class ObIWorkloadTransactionTask
    {
    public:
      ObIWorkloadTransactionTask(BasicTaskConfig config);
      virtual ~ObIWorkloadTransactionTask();
      virtual WorkloadType get_type() = 0;
      virtual int init();
      virtual int execute_transactions() = 0;
      virtual int release_dblinks() = 0;

    protected:
      virtual int prepare_arrays();
      virtual int wait_for_connection(ObMySQLConnection *conn);
      virtual int record_latency(ObLatencyTaskType type, int64_t latency);
      virtual int wait_and_bind_param(int64_t conn_idx, int64_t &partition_id, int64_t &row_id, ObMySQLPreparedStatement &stmt);

    protected:
      const char *table_name_;
      int64_t connection_count_;
      Connections connections_;
      ObTestbenchMySQLProxy *mysql_proxy_;
      ObTestbenchStatisticsCollector *latency_collector_;
      Dblinks dblink_id_;
      Parameters partition_id_;
      int64_t row_id_start_;
      Latencys latencys_;
      Latencys cumulative_latencys_;
      Commits commits_;
      ObArenaAllocator allocator_;
      VIRTUAL_TO_STRING_KV(K_(table_name), K_(connection_count), K_(dblink_id), K_(partition_id), K_(row_id_start), K_(latencys), K_(cumulative_latencys), K_(commits));
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
      INHERIT_TO_STRING_KV("ObIWorkloadTransactionTask", ObIWorkloadTransactionTask, K(participants_), K(operations_), K(lock_txn_sql_));

    private:
      int64_t participants_;
      int64_t operations_;
      const char *lock_txn_sql_format_;
      ObSqlString lock_txn_sql_;
      PreparedStatements lock_txn_stmts_;
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
      INHERIT_TO_STRING_KV("ObIWorkloadTransactionTask", ObIWorkloadTransactionTask, K(aborts_), K(operations_), K(lock_elr_sql_));

    private:
      int64_t aborts_;
      int64_t operations_;
      const char *lock_elr_sql_format_;
      ObSqlString lock_elr_sql_;
      PreparedStatements lock_elr_stmts_;
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
      INHERIT_TO_STRING_KV("ObIWorkloadTransactionTask", ObIWorkloadTransactionTask, K(chains_), K(lock_lcl_sql_));
    
    private:
      int64_t chains_;
      const char *lock_lcl_sql_format_;
      ObSqlString lock_lcl_sql_;
      PreparedStatements lock_lcl_stmts_;
    };
  }
}
#endif