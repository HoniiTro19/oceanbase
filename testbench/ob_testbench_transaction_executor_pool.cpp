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

#include "ob_testbench_transaction_executor_pool.h"
#include "share/ob_thread_define.h"
#include "share/ob_thread_mgr.h"

namespace oceanbase
{
namespace testbench
{
ObTestbenchTransactionExecutorPool::ObTestbenchTransactionExecutorPool() 
  : tg_id_(-1), 
    is_inited_(false), 
    sql_proxy_(NULL), 
    start_task_(0), 
    finish_task_(0),
    thread_num_(0), 
    task_queue_limit_(0), 
    allocator_("TxnTask") 
{}

ObTestbenchTransactionExecutorPool::~ObTestbenchTransactionExecutorPool() {}

int ObTestbenchTransactionExecutorPool::init(int64_t thread_num, int64_t task_queue_limit, ObTestbenchMySQLProxy *sql_proxy) {
  int ret = OB_SUCCESS;
  if (IS_INIT) {
    ret = OB_INIT_TWICE;
    TESTBENCH_LOG(ERROR, "ObTestbenchTransactionExecutorPool init twice", KR(ret));
  } else if (NULL == sql_proxy) {
    ret = OB_INVALID_ARGUMENT;
    TESTBENCH_LOG(ERROR, "invalid argument", KR(ret));
  } else {
    is_inited_ = true;
    sql_proxy_ = sql_proxy;
    thread_num_ = thread_num;
    task_queue_limit_ = task_queue_limit;
    TESTBENCH_LOG(INFO, "ObTestbenchTransactionExecutorPool init success", K(thread_num), K(task_queue_limit));
  }

  return ret;
}

int ObTestbenchTransactionExecutorPool::start() {
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "ObTestbenchTransactionExecutorPool not init");
  } else if (OB_FAIL(TG_CREATE(lib::TGDefIDs::TransactionExecutorPool, tg_id_))) {
    TESTBENCH_LOG(ERROR, "init ObTestbenchTransactionExecutorPool fail", KR(ret), K_(tg_id));
  } else if (OB_FAIL(TG_SET_HANDLER_AND_START(tg_id_, *this))) {
    TESTBENCH_LOG(ERROR, "ObTestbenchTransactionExecutorPool set handle and start fail", KR(ret));
  } else if (OB_FAIL(TG_SET_THREAD_CNT(tg_id_, thread_num_))) {
    TESTBENCH_LOG(WARN, "set ObTestbenchTransactionExecutorPool thread cnt fail", KR(ret), K_(thread_num));
  } else if (OB_FAIL(TG_SET_QUEUE_SIZE(tg_id_, task_queue_limit_))) {
    TESTBENCH_LOG(WARN, "set ObTestbenchTransactionExecutorPool queue size fail", KR(ret), K_(task_queue_limit));
  } else {
    TESTBENCH_LOG(INFO, "ObTestbenchTransactionExecutorPool start success", K_(thread_num), K_(task_queue_limit));
  }
  return ret;
}

void ObTestbenchTransactionExecutorPool::stop() {
  TESTBENCH_LOG(INFO, "ObTestbenchTransactionExecutorPool stop begin");
  if (-1 != tg_id_) {
    TG_STOP(tg_id_);
  }
  TESTBENCH_LOG(INFO, "ObTestbenchTransactionExecutorPool stop end");
}

void ObTestbenchTransactionExecutorPool::wait() {
  TESTBENCH_LOG(INFO, "ObTestbenchTransactionExecutorPool wait begin");
  if (-1 != tg_id_) {
    TG_WAIT(tg_id_);
  }
  TESTBENCH_LOG(INFO, "ObTestbenchTransactionExecutorPool wait end");
}

void ObTestbenchTransactionExecutorPool::destroy() {
  stop();
  wait();
  is_inited_ = false;
  if (-1 != tg_id_) {
    TG_DESTROY(tg_id_);
    tg_id_ = -1;
  }
  TESTBENCH_LOG(INFO, "ObTestbenchTransactionExecutorPool destroy");
}

void ObTestbenchTransactionExecutorPool::handle(void *task) {
  int ret = OB_SUCCESS;
  ObIWorkloadTransactionTask *task_to_handle = static_cast<ObIWorkloadTransactionTask*>(task);
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "transaction executor pool is not inited", KR(ret));
  } else if (OB_ISNULL(task_to_handle)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "transaction task is null", KR(ret));
  } else if (OB_FAIL(task_to_handle->init())) {
    TESTBENCH_LOG(ERROR, "transaction task init failed", KR(ret));
  } else if (OB_FAIL(task_to_handle->execute_transactions())) {
    TESTBENCH_LOG(ERROR, "transaction task execute failed", KR(ret));
  } else if (OB_FAIL(task_to_handle->release_dblinks())) {
    TESTBENCH_LOG(ERROR, "transaction task release dblinks failed", KR(ret));
  }
  inc_finish_task();
  allocator_.free(task_to_handle);
}

int ObTestbenchTransactionExecutorPool::push_task(ObIWorkloadTransactionTask *task) {
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "transaction executor pool is not inited", KR(ret));
  } else if (OB_ISNULL(task)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "task is null", KR(ret));
  } else {
    while (OB_FAIL(TG_PUSH_TASK(tg_id_, task)) && OB_EAGAIN == ret) {
      ob_usleep(1000);
      TESTBENCH_LOG(ERROR, "transaction executor pool push task failed", KR(ret));
    }
    inc_start_task();
  }
  return ret;
}

void ObTestbenchTransactionExecutorPool::inc_start_task() {
  ATOMIC_INC(&start_task_);
}

int64_t ObTestbenchTransactionExecutorPool::load_start_task() {
  return ATOMIC_LOAD(&start_task_);
}

void ObTestbenchTransactionExecutorPool::inc_finish_task() {
  ATOMIC_INC(&finish_task_);
}

int64_t ObTestbenchTransactionExecutorPool::load_finish_task() {
  return ATOMIC_LOAD(&finish_task_);
}
} // namespace testbench
} // namespace oceanbase