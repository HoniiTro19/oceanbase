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

#ifndef _OCEANBASE_WORKLOAD_OPTIONS_H_
#define _OCEANBASE_WORKLOAD_OPTIONS_H_

#include "ob_testbench_macros.h"
#include "lib/utility/utility.h"

namespace oceanbase
{
  namespace testbench
  {
    class ObIOptions
    {
    public:
      ObIOptions(const char *src_opt_str);
      virtual ~ObIOptions();
      virtual int parse_options();
      virtual int fill_options(const char *key, const char *value) = 0;
      VIRTUAL_TO_STRING_KV(K_(src_opt_str));
      
      static const int MAX_OPTS_CNT = 16;

    protected:
      const char *src_opt_str_;
    };

    class ObIWorkloadOptions : public ObIOptions
    {
    public:
      ObIWorkloadOptions(const char *src_opt_str);
      virtual ~ObIWorkloadOptions();
      virtual int fill_options(const char *key, const char *value) = 0;
      virtual WorkloadType get_type() = 0;
      inline int64_t get_thread_num() { return thread_num_; }
      inline int64_t get_task_queue_limit() { return task_queue_limit_; }
    
    protected:
      int64_t thread_num_;
      int64_t task_queue_limit_;
    };

    class ObDistributedTransactionOptions : public ObIWorkloadOptions
    {
    public:
      ObDistributedTransactionOptions(const char *opt_str);
      virtual ~ObDistributedTransactionOptions() override;
      virtual int fill_options(const char *key, const char *value) override;
      virtual WorkloadType get_type() override;
      inline int64_t get_participants() { return participants_; }
      inline int64_t get_operations() { return operations_; }
      INHERIT_TO_STRING_KV("ObIWorkloadOptions", ObIWorkloadOptions, K_(participants), K_(operations));

    private:
      int64_t participants_;
      int64_t operations_;
    };

    class ObContentionTransactionOptions : public ObIWorkloadOptions
    {
    public:
      ObContentionTransactionOptions(const char *opt_str);
      virtual ~ObContentionTransactionOptions() override;
      virtual int fill_options(const char *key, const char *value) override;
      virtual WorkloadType get_type() override;
      inline int64_t get_concurrency() { return concurrency_; }
      inline int64_t get_operations() { return operations_; }
      inline int64_t get_aborts() { return aborts_; }
      INHERIT_TO_STRING_KV("ObIWorkloadOptions", ObIWorkloadOptions, K_(concurrency), K_(operations), K_(aborts));

    private:
      int64_t concurrency_;
      int64_t operations_;
      int64_t aborts_;
    };

    class ObDeadlockTransactionOptions : public ObIWorkloadOptions
    {
    public:
      ObDeadlockTransactionOptions(const char *opt_str);
      virtual ~ObDeadlockTransactionOptions() override;
      virtual int fill_options(const char *key, const char *value) override;
      virtual WorkloadType get_type() override;
      inline int64_t get_concurrency() { return concurrency_; }
      inline int64_t get_chains() { return chains_; }
      INHERIT_TO_STRING_KV("ObIWorkloadOptions", ObIWorkloadOptions, K_(concurrency), K_(chains));
    
    private:
      int64_t concurrency_;
      int64_t chains_;
    };

    class ObConcurrentTransactionOptions : public ObIWorkloadOptions
    {
    public:
      ObConcurrentTransactionOptions(const char *opt_str);
      virtual ~ObConcurrentTransactionOptions() override;
      virtual int fill_options(const char *key, const char *value) override;
      virtual WorkloadType get_type() override;
      inline int64_t get_concurrency() { return concurrency_; }
      inline int64_t get_operations() { return operations_; }
      inline int64_t get_readonly() { return readonly_; }
      INHERIT_TO_STRING_KV("ObIWorkloadOptions", ObIWorkloadOptions, K_(concurrency), K_(operations), K_(readonly));
      
    private:
      int64_t concurrency_;
      int64_t operations_;
      int64_t readonly_;
    };

    class ObDatasetOptions : public ObIOptions
    {
    public:
      ObDatasetOptions(const char *opt_str);
      virtual ~ObDatasetOptions() override;
      virtual int fill_options(const char *key, const char *value) override;
      inline int64_t get_partitions() { return partitions_; }
      inline int64_t get_rows() { return rows_; }
      INHERIT_TO_STRING_KV("ObIOptions", ObIOptions, K_(partitions), K_(rows));
    
    private:
      int64_t partitions_;
      int64_t rows_;
    };

    class ObStatisticsCollectorOptions : public ObIOptions
    {
    public:
      ObStatisticsCollectorOptions(const char *opt_str);
      virtual ~ObStatisticsCollectorOptions() override;
      virtual int fill_options(const char *key, const char *value) override;
      inline int64_t get_bucket_capacity() { return bucket_capacity_; }
      inline int64_t get_bucket_min_percentage() { return bucket_min_percentage_; }
      inline int64_t get_bucket_max_percentage() { return bucket_max_percentage_; }
      inline int64_t get_thread_num() { return thread_num_; }
      inline int64_t get_task_queue_limit() { return task_queue_limit_; }
      INHERIT_TO_STRING_KV("ObIOptions", ObIOptions, K_(bucket_capacity), K_(bucket_min_percentage), K_(bucket_max_percentage), K_(thread_num), K_(task_queue_limit));

    private:
      int64_t bucket_capacity_;
      int64_t bucket_min_percentage_;
      int64_t bucket_max_percentage_;
      int64_t thread_num_;
      int64_t task_queue_limit_;
    };

    class ObConnectionOptions : public ObIOptions
    {
    public:
      ObConnectionOptions(const char *opt_str);
      virtual ~ObConnectionOptions() override;
      virtual int fill_options(const char *key, const char *value) override;
      inline const char *get_cluster_host() { return cluster_host_; }
      inline const char *get_cluster_user() { return cluster_user_; }
      inline int32_t get_cluster_port() { return cluster_port_; }
      inline const char *get_cluster_pass() { return cluster_pass_; }
      inline const char *get_cluster_db_name() { return cluster_db_name_; }
      inline const char *get_cluster_table_name() { return cluster_table_name_; }
      INHERIT_TO_STRING_KV("ObIOptions", ObIOptions, KCSTRING_(cluster_host), KCSTRING_(cluster_user), K_(cluster_port), KCSTRING_(cluster_pass), KCSTRING_(cluster_db_name), KCSTRING_(cluster_table_name));
    
    private:
      char cluster_host_[OB_MAX_HOST_NAME_LENGTH];
      char cluster_user_[OB_MAX_USER_NAME_BUF_LENGTH];
      int32_t cluster_port_;
      char cluster_pass_[OB_MAX_PASSWORD_BUF_LENGTH];
      char cluster_db_name_[OB_MAX_DATABASE_NAME_BUF_LENGTH];
      char cluster_table_name_[OB_MAX_TABLE_NAME_BUF_LENGTH];
    };
  } // namespace testbench
} // namespace oceanbase
#endif