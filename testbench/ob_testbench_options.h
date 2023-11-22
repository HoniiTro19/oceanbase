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
    class ObIWorkloadOptions
    {
    public:
      ObIWorkloadOptions(const char *src_opt_str);
      virtual ~ObIWorkloadOptions();
      virtual int parse_options();
      virtual int fill_options(const char *key, const char *value) = 0;
      virtual WorkloadType get_type() = 0;
      VIRTUAL_TO_STRING_KV(K(start_time_), K(duration_), K(src_opt_str_));
      
      static const int MAX_OPTS_CNT = 16;

    protected:
      int64_t start_time_;
      int64_t duration_;
      const char *src_opt_str_;
    };

    class ObDistributedTransactionOptions : public ObIWorkloadOptions
    {
    public:
      ObDistributedTransactionOptions(const char *opt_str);
      virtual ~ObDistributedTransactionOptions() override;
      virtual int fill_options(const char *key, const char *value) override;
      virtual WorkloadType get_type() override;
      INHERIT_TO_STRING_KV("ObIWorkloadOptions", ObIWorkloadOptions, K(participants_), K(operations_));

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
      INHERIT_TO_STRING_KV("ObIWorkloadOptions", ObIWorkloadOptions, K(concurrency_), K(operations_));

    private:
      int64_t concurrency_;
      int64_t operations_;
    };

    class ObDeadlockTransactionOptions : public ObIWorkloadOptions
    {
    public:
      ObDeadlockTransactionOptions(const char *opt_str);
      virtual ~ObDeadlockTransactionOptions() override;
      virtual int fill_options(const char *key, const char *value) override;
      virtual WorkloadType get_type() override;
      INHERIT_TO_STRING_KV("ObIWorkloadOptions", ObIWorkloadOptions, K(concurrency_), K(chains_));
    
    private:
      int64_t concurrency_;
      int64_t chains_;
    };
  }
}
#endif