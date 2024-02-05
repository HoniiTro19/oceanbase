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
  int generate_different_partitions(int64_t target, Parameters &parameters);
  int generate_random_partition(const ObString &svr_ip, int64_t &parameter);
  common::ObArray<ObString> &get_svr_ips();

private:
  bool is_inited_;
  const char *database_name_;
  const char *table_name_;
  ObTestbenchSystableHelper *systable_helper_;
  PartitionInfo partition_info_;
  common::ObArray<ObString> svr_ips_;
  ObRandom random_;
};
} // namespace testbench
} // namespace oceanbase
#endif