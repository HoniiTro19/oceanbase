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

#ifndef _OCEANBASE_TESTBENCH_LOCATION_CACHE_H
#define _OCEANBASE_TESTBENCH_LOCATION_CACHE_H

#include "ob_testbench_systable_helper.h"
#include "lib/random/ob_random.h"
#include "testbench/ob_testbench_transaction_task.h"

namespace oceanbase
{
namespace testbench
{
class ObTestbenchLocationCache
{
public:
  ObTestbenchLocationCache();
  ~ObTestbenchLocationCache();

  int init(const char *database_name, const char *table_name, ObTestbenchSystableHelper *systable_helper);
  void destroy();
  int refresh_locations();
  int gen_distributed_txn_params(int64_t svrs, ParametersGroup &pgroup, DblinksGroup &dgroup);
  int gen_contention_txn_params(int64_t conns, ParametersGroup &pgroup, DblinksGroup &dgroup);
  int gen_deadlock_txn_params(int64_t conns, ParametersGroup &pgroup, DblinksGroup &dgroup);
  int gen_concurrent_txn_params(int64_t conns, ParametersGroup &pgroup, DblinksGroup &dgroup);

  // for test use
  inline PartitionInfo *get_partition_info() { return &partition_info_; }
  inline ObTenantName get_user_tenant() { return user_tenant_; }
  inline common::ObArray<ObAddr> get_svr_addrs() { return svr_addrs_; }

private:
  bool is_inited_;
  const char *database_name_;
  const char *table_name_;
  ObTestbenchSystableHelper *systable_helper_;
  PartitionInfo partition_info_;
  common::ObArray<ObAddr> svr_addrs_;
  common::ObArray<uint64_t> tenant_id_list_;
  common::ObArray<ObTenantName> tenant_name_list_;
  ObTenantName user_tenant_;
  ObRandom random_;
  common::SpinRWLock lock_;
};
} // namespace testbench
} // namespace oceanbase
#endif