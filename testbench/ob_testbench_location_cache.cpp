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
ObTestbenchLocationCache::ObTestbenchLocationCache() : partition_info_(), svr_addrs_(), random_() {}

ObTestbenchLocationCache::~ObTestbenchLocationCache() {}

void ObTestbenchLocationCache::destroy() {
  partition_info_.destroy();
  svr_addrs_.destroy();
}

int ObTestbenchLocationCache::init(const char *database_name, const char *table_name, ObTestbenchSystableHelper *systable_helper) {
  int ret = OB_SUCCESS;
  database_name_ = database_name;
  table_name_ = table_name;
  systable_helper_ = systable_helper;
  if (OB_FAIL(partition_info_.create(16, ObModIds::OB_PARTITION_META))) {
    TESTBENCH_LOG(ERROR, "create partition info map failed", KR(ret), "bucket_cnt", 16);
  }
  return ret;
}

int ObTestbenchLocationCache::refresh_locations() {
  int ret = OB_SUCCESS;
  common::SpinWLockGuard guard(lock_);
  if (OB_ISNULL(systable_helper_)) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "systable helper is not inited", KR(ret));
  } else if (OB_UNLIKELY(systable_helper_->is_not_inited())) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "systable helper is not inited", KR(ret));
  } else if (OB_FAIL(systable_helper_->query_partition_info(database_name_, table_name_, partition_info_))) {
    TESTBENCH_LOG(ERROR, "get partition info failed", KR(ret));
  } else if (OB_FAIL(systable_helper_->query_tenant_list(tenant_id_list_, tenant_name_list_))) {
    TESTBENCH_LOG(ERROR, "get tenant list failed", KR(ret));
  } else {
    // get server list from partition info
    svr_addrs_.reset();
    auto bucket_iter = partition_info_.begin();
    while (bucket_iter != partition_info_.end()) {
      TESTBENCH_LOG(TRACE, "check partition info after refresh locations", "svr_ip", bucket_iter->first, "partitions", bucket_iter->second->count());
      if (OB_FAIL(svr_addrs_.push_back(bucket_iter->first))) {
        TESTBENCH_LOG(ERROR, "push server ip into svr_ips from link hash map failed", K(ret));
      }
      bucket_iter++;
    }
    // get user tenant from tenant name list
    int64_t tenant_count = tenant_name_list_.count();
    for (int64_t i = 0; i < tenant_count; ++i) {
      if (tenant_id_list_.at(i) % 2 == 0) {
        user_tenant_ = tenant_name_list_.at(i);
        break;
      }
    }
    if (OB_UNLIKELY(user_tenant_.is_empty())) {
      ret = OB_ERR_UNEXPECTED;
      TESTBENCH_LOG(ERROR, "get user tenant failed", KR(ret));
    } else if (OB_UNLIKELY(tenant_count > 3)) {
      TESTBENCH_LOG(WARN, "might get multiple user tenant", "count", tenant_name_list_.count());
    }
  }
  return ret;
}

int ObTestbenchLocationCache::gen_distributed_txn_params(int64_t svrs, ParametersGroup &pgroup, DblinksGroup &dgroup)
{
  int ret = OB_SUCCESS;
  common::SpinWLockGuard guard(lock_);
  int64_t svr_count = svr_addrs_.count();
  if (OB_UNLIKELY(svr_count < svrs)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "the number of servers is smaller than svrs", KR(ret), K_(svr_addrs), K(svrs));
  } else {
    Parameters parameters;
    Dblinks dblinks;
    for (int64_t i = 0; i < svrs; ++i) {
      PartitionList *partition_ids = nullptr;
      int64_t swap_idx = random_.rand(i, svr_count - 1);
      std::swap(svr_addrs_.at(i), svr_addrs_.at(swap_idx));
      if (OB_FAIL(partition_info_.get_refactored(svr_addrs_.at(i), partition_ids))) {
        TESTBENCH_LOG(ERROR, "get partition ids from partition info failed", KR(ret), "addr", svr_addrs_.at(i));
      } else {
        int64_t partition_count = partition_ids->count();
        int64_t sample_idx = random_.rand(0, partition_count - 1);
        if (OB_FAIL(parameters.push_back(partition_ids->at(sample_idx)))) {
          TESTBENCH_LOG(ERROR, "push back partition id failed", KR(ret));
        } else if (OB_FAIL(dblinks.push_back(DblinkKey(user_tenant_.str(), svr_addrs_.at(i)).hash()))) {
          TESTBENCH_LOG(ERROR, "push back dblink id failed", KR(ret));
        }
      }
    }
    pgroup.reset();
    dgroup.reset();
    if (OB_FAIL(pgroup.push_back(parameters))) {
      TESTBENCH_LOG(ERROR, "push back parameters failed", KR(ret));
    } else if (OB_FAIL(dgroup.push_back(dblinks))) {
      TESTBENCH_LOG(ERROR, "push back dblinks failed", KR(ret));
    }
    TESTBENCH_LOG(TRACE, "generate distributed transaction parameters", KR(ret), K(pgroup), K(dgroup));
  }
  return ret;
}

int ObTestbenchLocationCache::gen_contention_txn_params(int64_t conns, ParametersGroup &pgroup, DblinksGroup &dgroup)
{
  int ret = OB_SUCCESS;
  common::SpinWLockGuard guard(lock_);
  int64_t svr_count = svr_addrs_.count();
  if (OB_UNLIKELY(svr_count < 1)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR,  "get empty server list", KR(ret), K_(svr_addrs));
  } else {
    pgroup.reset();
    dgroup.reset();
    int64_t rnd_idx = random_.rand(0, svr_count - 1);
    PartitionList *partition_ids = nullptr;
    if (OB_FAIL(partition_info_.get_refactored(svr_addrs_.at(rnd_idx), partition_ids))) {
      TESTBENCH_LOG(ERROR, "get partition ids from partition info failed", KR(ret), "addr", svr_addrs_.at(rnd_idx));
    } else {
      for (int64_t i = 0; i < conns; ++i) {
        Parameters parameters;
        Dblinks dblinks;
        int64_t partition_count = partition_ids->count();
        int64_t sample_idx = random_.rand(0, partition_count - 1);
        if (OB_FAIL(parameters.push_back(partition_ids->at(sample_idx)))) {
          TESTBENCH_LOG(ERROR, "push back partition id failed");
        } else if (OB_FAIL(dblinks.push_back(DblinkKey(user_tenant_.str(), svr_addrs_.at(rnd_idx)).hash()))) {
          TESTBENCH_LOG(ERROR, "push back dblink id failed", KR(ret));
        } else if (OB_FAIL(pgroup.push_back(parameters))) {
          TESTBENCH_LOG(ERROR, "push back parameters failed", KR(ret));
        } else if (OB_FAIL(dgroup.push_back(dblinks))) {
          TESTBENCH_LOG(ERROR, "push back dblinks failed", KR(ret));
        }
      }
    }
    TESTBENCH_LOG(TRACE, "generate contention transaction parameters", KR(ret), K(pgroup), K(dgroup));
  }
  return ret;
}

int ObTestbenchLocationCache::gen_deadlock_txn_params(int64_t conns, ParametersGroup &pgroup, DblinksGroup &dgroup)
{
  int ret = OB_SUCCESS;
  common::SpinWLockGuard guard(lock_);
  int64_t svr_count = svr_addrs_.count();
  if (OB_UNLIKELY(svr_count < 1)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "get empty server list", KR(ret), K_(svr_addrs));
  } else {
    pgroup.reset();
    dgroup.reset();
    for (int64_t i = 0; i < conns; ++i) {
      PartitionList *partition_ids = nullptr;
      Parameters parameters;
      Dblinks dblinks;
      if (OB_FAIL(partition_info_.get_refactored(svr_addrs_.at(i % svr_count), partition_ids))) {
        TESTBENCH_LOG(ERROR, "get partition ids from partition info failed", KR(ret), "addr", svr_addrs_.at(i % svr_count));
      } else {
        int64_t partition_count = partition_ids->count();
        int64_t sample_idx = random_.rand(0, partition_count - 1);
        if (OB_FAIL(parameters.push_back(partition_ids->at(sample_idx)))) {
          TESTBENCH_LOG(ERROR, "push back partition id failed", KR(ret));
        } else if (OB_FAIL(dblinks.push_back(DblinkKey(user_tenant_.str(), svr_addrs_.at(i)).hash()))) {
          TESTBENCH_LOG(ERROR, "push back dblink id failed", KR(ret));
        } else if (OB_FAIL(pgroup.push_back(parameters))) {
          TESTBENCH_LOG(ERROR, "push back parameters failed", KR(ret));
        } else if (OB_FAIL(dgroup.push_back(dblinks))) {
          TESTBENCH_LOG(ERROR, "push back dblinks failed", KR(ret));
        }
      }
    }
    TESTBENCH_LOG(TRACE, "generate global deadlock transaction parameters", KR(ret), K(pgroup), K(dgroup));
  }
  return ret;
}

int ObTestbenchLocationCache::gen_concurrent_txn_params(int64_t conns, ParametersGroup &pgroup, DblinksGroup &dgroup)
{
  int ret = OB_SUCCESS;
  common::SpinWLockGuard guard(lock_);
  int64_t svr_count = svr_addrs_.count();
  if (OB_UNLIKELY(svr_count < 1)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "get empty server list", KR(ret), K_(svr_addrs));
  } else {
    pgroup.reset();
    dgroup.reset();
    for (int64_t i = 0; i < conns; ++i) {
      PartitionList *partition_ids = nullptr;
      int64_t rnd_idx = random_.rand(0, svr_count - 1);
      Parameters parameters;
      Dblinks dblinks;
      if (OB_FAIL(partition_info_.get_refactored(svr_addrs_.at(rnd_idx), partition_ids))) {
        TESTBENCH_LOG(ERROR, "get partition ids from partition info failed", KR(ret), "addr", svr_addrs_.at(rnd_idx));
      } else {
        int64_t partition_count = partition_ids->count();
        int64_t sample_idx = random_.rand(0, partition_count - 1);
        if (OB_FAIL(parameters.push_back(partition_ids->at(sample_idx)))) {
          TESTBENCH_LOG(ERROR, "push back partition id failed", KR(ret));
        } else if (OB_FAIL(dblinks.push_back(DblinkKey(user_tenant_.str(), svr_addrs_.at(i)).hash()))) {
          TESTBENCH_LOG(ERROR, "push back dblink id failed", KR(ret));
        } else if (OB_FAIL(pgroup.push_back(parameters))) {
          TESTBENCH_LOG(ERROR, "push back parameters failed", KR(ret));
        } else if (OB_FAIL(dgroup.push_back(dblinks))) {
          TESTBENCH_LOG(ERROR, "push back dblinks failed", KR(ret));
        }
      }
    }
    TESTBENCH_LOG(TRACE, "generate global deadlock transaction parameters", KR(ret), K(pgroup), K(dgroup));
  }
  return ret;
}
} // namespace testbench
} // namespace oceanbase