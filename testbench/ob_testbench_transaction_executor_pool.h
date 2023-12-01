/**
 * Copyright (c) 2021 OceanBase
 * OceanBase CE is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */

#ifndef _OCEANBASE_TESTBENCH_TRANSACTION_EXECUTOR_POOL_H_
#define _OCEANBASE_TESTBENCH_TRANSACTION_EXECUTOR_POOL_H_

#include "lib/thread/thread_mgr_interface.h"
#include "testbench/ob_testbench_transaction_task.h"

namespace oceanbase {
namespace testbench {
class ObTestbenchTransactionExecutorPool : public lib::TGTaskHandler {
public:
  ObTestbenchTransactionExecutorPool();
  ~ObTestbenchTransactionExecutorPool();

public:
  int init(int64_t thread_num, int64_t task_queue_limit, ObTestbenchMySQLProxy *sql_proxy);
  int start();
  void stop();
  void wait();
  void destroy();
  virtual void handle(void *task) override;
  int push_task(ObIWorkloadTransactionTask *task);
  inline int get_tg_id() const { return tg_id_; }
  void inc_start_task();
  int64_t load_start_task();
  void inc_finish_task();
  int64_t load_finish_task();
  inline int64_t get_thread_num() { return thread_num_; }

private:
  int tg_id_;
  bool is_inited_;
  ObTestbenchMySQLProxy *sql_proxy_;
  int64_t start_task_;
  int64_t finish_task_;
  int64_t thread_num_;
  int64_t task_queue_limit_;
  ObArenaAllocator allocator_;

private:
  DISALLOW_COPY_AND_ASSIGN(ObTestbenchTransactionExecutorPool);
};
} // end namespace testbench
} // end namespace oceanbase

#endif