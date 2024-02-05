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

#include "testbench/ob_testbench_location_cache.h"
#include "lib/function/ob_function.h"

namespace oceanbase
{
namespace testbench
{
ObTestbenchLocationCache::ObTestbenchLocationCache() : partition_info_(), svr_ips_(), random_() {}

ObTestbenchLocationCache::~ObTestbenchLocationCache() {}

void ObTestbenchLocationCache::destroy() {
  partition_info_.destroy();
  svr_ips_.destroy();
}

int ObTestbenchLocationCache::init(const char *database_name, const char *table_name, ObTestbenchSystableHelper *systable_helper) {
  int ret = OB_SUCCESS;
  database_name_ = database_name;
  table_name_ = table_name;
  systable_helper_ = systable_helper;
  common::ObSEArray<common::ObAddr, 16> sql_server_list;
  sql_server_list.reset();
  if (OB_FAIL(systable_helper_->query_sql_server_list(sql_server_list))) {
    TESTBENCH_LOG(ERROR, "query server list failed", KR(ret));
  } else if (OB_FAIL(partition_info_.create(sql_server_list.count(), ObModIds::OB_PARTITION_META))) {
    TESTBENCH_LOG(ERROR, "create partition info map failed", KR(ret), "bucket_cnt", sql_server_list.count());
  }
  return ret;
}

int ObTestbenchLocationCache::refresh_locations() {
  int ret = OB_SUCCESS;
  if (OB_ISNULL(systable_helper_)) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "systable helper is not inited", KR(ret));
  } else if (OB_UNLIKELY(systable_helper_->is_not_inited())) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "systable helper is not inited", KR(ret));
  } else if (OB_FAIL(systable_helper_->query_partition_info(database_name_, table_name_, partition_info_))) {
    TESTBENCH_LOG(ERROR, "get partition info failed", KR(ret));
  } else {
    auto bucket_iter = partition_info_.begin();
    while (bucket_iter != partition_info_.end()) {
      TESTBENCH_LOG(TRACE, "check partition info after refresh locations", "svr_ip", bucket_iter->first, "partitions", bucket_iter->second->count());
      if (OB_FAIL(svr_ips_.push_back(bucket_iter->first))) {
        TESTBENCH_LOG(ERROR, "push server ip into svr_ips from link hash map failed", K(ret));
      }
      bucket_iter++;
    }
  }
  return ret;
}

int ObTestbenchLocationCache::generate_different_partitions(int64_t target, Parameters &parameters) {
  int ret = OB_SUCCESS;
  int64_t svr_count = svr_ips_.count();
  parameters.reset();
  if (OB_UNLIKELY(svr_count < target)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "the number of servers is smaller than that of partitions", K(ret));
  } else {
    for (int64_t i = 0; i < target; ++i) {
      PartitionList *partition_ids = nullptr;
      int64_t swap_idx = random_.rand(i, svr_count - 1);
      std::swap(svr_ips_.at(i), svr_ips_.at(swap_idx));
      if (OB_FAIL(partition_info_.get_refactored(svr_ips_.at(i), partition_ids))) {
        TESTBENCH_LOG(ERROR, "get partition ids from partition info failed", K(ret));
      } else if (OB_ISNULL(partition_ids)) {
        ret = OB_ERR_UNEXPECTED;
        TESTBENCH_LOG(ERROR, "get empty partition id list", KR(ret));
      } else {
        int64_t partition_count = partition_ids->count();
        int64_t sample_idx = random_.rand(0, partition_count - 1);
        if (OB_FAIL(parameters.push_back(partition_ids->at(sample_idx)))) {
          TESTBENCH_LOG(ERROR, "push back partition ids failed", K(ret));
        }
      }
    }
    TESTBENCH_LOG(TRACE, "generate different partitions", "partitions", parameters);
  }
  return ret;
}

int ObTestbenchLocationCache::generate_random_partition(const ObString &svr_ip, int64_t &parameter) {
  int ret = OB_SUCCESS;
  int64_t svr_count = svr_ips_.count();
  PartitionList *partition_ids = nullptr;
  if (OB_FAIL(partition_info_.get_refactored(svr_ip, partition_ids))) {
    TESTBENCH_LOG(ERROR, "get partition ids from partition info failed", KR(ret));
  } else if (OB_ISNULL(partition_ids)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "get empty partition id list", KR(ret));
  } else {
    int64_t partition_count = partition_ids->count();
    int64_t sample_idx = random_.rand(0, partition_count - 1);
    parameter = partition_ids->at(sample_idx);
    TESTBENCH_LOG(TRACE, "generate random partition", "partition", parameter);
  }
  return ret;
}

common::ObArray<ObString> &ObTestbenchLocationCache::get_svr_ips() {
  return svr_ips_;
}
} // namespace testbench
} // namespace oceanbase