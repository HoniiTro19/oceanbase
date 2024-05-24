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

#define USING_LOG_PREFIX SERVER

#include "observer/table_load/ob_table_load_instance.h"
#include "observer/table_load/ob_table_load_coordinator.h"
#include "observer/table_load/ob_table_load_exec_ctx.h"
#include "observer/table_load/ob_table_load_redef_table.h"
#include "observer/table_load/ob_table_load_service.h"
#include "observer/table_load/ob_table_load_table_ctx.h"
#include "share/table/ob_table_load_define.h"
#include "sql/engine/ob_exec_context.h"

namespace oceanbase
{
namespace observer
{
using namespace sql;
using namespace storage;
using namespace table;

ObTableLoadInstance::ObTableLoadInstance()
  : execute_ctx_(nullptr),
    allocator_(nullptr),
    table_ctx_(nullptr),
    job_stat_(nullptr),
    is_inited_(false)
{
}

ObTableLoadInstance::~ObTableLoadInstance() { destroy(); }

void ObTableLoadInstance::destroy()
{
  int ret = OB_SUCCESS;
  trans_ctx_.reset();
  if (nullptr != table_ctx_) {
    if (OB_FAIL(end_direct_load(false /*commit*/))) {
      LOG_WARN("fail to end direct load", KR(ret));
    }
  }
  if (stmt_ctx_.is_started()) {
    if (OB_FAIL(end_stmt(false /*commit*/))) {
      LOG_WARN("fail to end stmt", KR(ret));
    }
  }
}

int ObTableLoadInstance::init(ObTableLoadParam &param, const ObIArray<uint64_t> &column_ids,
                              ObTableLoadExecCtx *execute_ctx)
{
  int ret = OB_SUCCESS;
  if (IS_INIT) {
    ret = OB_INIT_TWICE;
    LOG_WARN("ObTableLoadInstance init twice", KR(ret), KP(this));
  } else if (OB_UNLIKELY(!param.is_valid() || column_ids.empty() || !execute_ctx->is_valid())) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid args", KR(ret), K(param), K(column_ids), KPC(execute_ctx));
  } else {
    execute_ctx_ = execute_ctx;
    allocator_ = execute_ctx->get_allocator();
    if (OB_FAIL(param.normalize())) {
      LOG_WARN("fail to normalize param", KR(ret));
    }
    // check tenant
    else if (OB_FAIL(ObTableLoadService::check_tenant())) {
      LOG_WARN("fail to check tenant", KR(ret), K(param.tenant_id_));
    }
    // start stmt
    else if (OB_FAIL(start_stmt(param))) {
      LOG_WARN("fail to start stmt", KR(ret), K(param));
    }
    // double check support for concurrency of direct load and ddl
    else if (OB_FAIL(ObTableLoadService::check_support_direct_load(param.table_id_))) {
      LOG_WARN("fail to check support direct load", KR(ret), K(param.table_id_));
    }
    // start direct load
    else if (OB_FAIL(start_direct_load(param, column_ids))) {
      LOG_WARN("fail to start direct load", KR(ret));
    }
    // start trans
    else if (!param.px_mode_ && OB_FAIL(start_trans())) {
      LOG_WARN("fail to start trans", KR(ret));
    }
    // init succ
    else {
      job_stat_ = table_ctx_->job_stat_;
      is_inited_ = true;
    }
  }
  return ret;
}

int ObTableLoadInstance::write(int32_t session_id, const table::ObTableLoadObjRowArray &obj_rows)
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("ObTableLoadInstance not init", KR(ret), KP(this));
  } else {
    if (OB_FAIL(write_trans(session_id, obj_rows))) {
      LOG_WARN("fail to write trans", KR(ret));
    }
  }
  return ret;
}

// commit() = px_commit_data() + px_commit_ddl()
// used in non px_mode
int ObTableLoadInstance::commit(ObTableLoadResultInfo &result_info)
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("ObTableLoadInstance not init", KR(ret), KP(this));
  } else {
    if (OB_FAIL(commit_trans())) {
      LOG_WARN("fail to commit trans", KR(ret));
    }
    // end direct load
    else if (OB_FAIL(end_direct_load(true /*commit*/))) {
      LOG_WARN("fail to end direct load", KR(ret));
    }
    // end stmt
    else if (OB_FAIL(end_stmt(true /*commit*/))) {
      LOG_WARN("fail to end stmt", KR(ret));
    } else {
      result_info = result_info_;
    }
  }
  return ret;
}

// used in insert /*+ append */ into select clause
int ObTableLoadInstance::px_commit_data()
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("ObTableLoadInstance not init", KR(ret), KP(this));
  } else {
    if (OB_FAIL(end_direct_load(true /*commit*/))) {
      LOG_WARN("fail to end direct load", KR(ret));
    }
  }
  return ret;
}

int ObTableLoadInstance::px_commit_ddl()
{
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    LOG_WARN("ObTableLoadInstance not init", KR(ret), KP(this));
  } else {
    if (OB_FAIL(end_stmt(true /*commit*/))) {
      LOG_WARN("fail to end stmt", KR(ret));
    }
  }
  return ret;
}

int ObTableLoadInstance::start_stmt(const ObTableLoadParam &param)
{
  int ret = OB_SUCCESS;
  stmt_ctx_.reset();
  stmt_ctx_.tenant_id_ = param.tenant_id_;
  stmt_ctx_.table_id_ = param.table_id_;
  stmt_ctx_.session_info_ = execute_ctx_->get_session_info();
  if (OB_FAIL(start_redef_table(param))) {
    LOG_WARN("fail to start redef table", KR(ret), K(param));
  }
  if (OB_SUCC(ret)) {
    stmt_ctx_.is_started_ = true;
    LOG_INFO("start stmt succeed", KR(ret), K(stmt_ctx_));
  }
  return ret;
}

int ObTableLoadInstance::end_stmt(const bool commit)
{
  int ret = OB_SUCCESS;
  int tmp_ret = OB_SUCCESS;
  if (commit && OB_FAIL(commit_redef_table())) {
    LOG_WARN("fail to commit redef table", KR(ret));
  }
  if (OB_FAIL(ret) || !commit) {
    if (OB_TMP_FAIL(abort_redef_table())) {
      LOG_WARN("fail to abort redef table", KR(tmp_ret));
      ret = COVER_SUCC(tmp_ret);
    }
  }
  stmt_ctx_.is_started_ = false;
  LOG_INFO("end stmt succeed", KR(ret), K(stmt_ctx_));
  return ret;
}

int ObTableLoadInstance::start_redef_table(const ObTableLoadParam &param)
{
  int ret = OB_SUCCESS;
  ObTableLoadDDLParam &ddl_param = stmt_ctx_.ddl_param_;
  ObTableLoadRedefTableStartArg start_arg;
  ObTableLoadRedefTableStartRes start_res;
  uint64_t data_version = 0;
  start_arg.tenant_id_ = param.tenant_id_;
  start_arg.table_id_ = param.table_id_;
  start_arg.parallelism_ = param.parallel_;
  start_arg.is_load_data_ = !param.px_mode_;
  if (OB_FAIL(GET_MIN_DATA_VERSION(param.tenant_id_, data_version))) {
    LOG_WARN("fail to get tenant data version", KR(ret));
  } else if (OB_FAIL(
               ObTableLoadRedefTable::start(start_arg, start_res, *stmt_ctx_.session_info_))) {
    LOG_WARN("fail to start redef table", KR(ret), K(start_arg));
    // rewrite error code for concurrency of direct load and offline ddl
    if (OB_TABLE_NOT_EXIST == ret) {
      ret = OB_SCHEMA_NOT_UPTODATE;
    }
  } else {
    ddl_param.dest_table_id_ = start_res.dest_table_id_;
    ddl_param.task_id_ = start_res.task_id_;
    ddl_param.schema_version_ = start_res.schema_version_;
    ddl_param.snapshot_version_ = start_res.snapshot_version_;
    ddl_param.data_version_ = data_version;
    LOG_INFO("start redef table succeed", K(ddl_param));
  }
  return ret;
}

int ObTableLoadInstance::commit_redef_table()
{
  int ret = OB_SUCCESS;
  ObTableLoadRedefTableFinishArg arg;
  arg.tenant_id_ = stmt_ctx_.tenant_id_;
  arg.table_id_ = stmt_ctx_.table_id_;
  arg.dest_table_id_ = stmt_ctx_.ddl_param_.dest_table_id_;
  arg.task_id_ = stmt_ctx_.ddl_param_.task_id_;
  arg.schema_version_ = stmt_ctx_.ddl_param_.schema_version_;
  if (OB_FAIL(ObTableLoadRedefTable::finish(arg, *stmt_ctx_.session_info_))) {
    LOG_WARN("fail to finish redef table", KR(ret), K(arg));
  } else {
    LOG_INFO("commit redef table succeed", K(arg));
  }
  return ret;
}

int ObTableLoadInstance::abort_redef_table()
{
  int ret = OB_SUCCESS;
  ObTableLoadRedefTableAbortArg arg;
  arg.tenant_id_ = stmt_ctx_.tenant_id_;
  arg.task_id_ = stmt_ctx_.ddl_param_.task_id_;
  if (OB_FAIL(ObTableLoadRedefTable::abort(arg, *stmt_ctx_.session_info_))) {
    LOG_WARN("fail to abort redef table", KR(ret), K(arg));
  } else {
    LOG_INFO("abort redef table succeed", K(arg));
  }
  return ret;
}

int ObTableLoadInstance::start_direct_load(const ObTableLoadParam &param,
                                           const ObIArray<uint64_t> &column_ids)
{
  int ret = OB_SUCCESS;
  ObTableLoadTableCtx *table_ctx = nullptr;
  ObSQLSessionInfo *session_info = execute_ctx_->get_session_info();
  if (OB_UNLIKELY(nullptr != table_ctx_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected table ctx is not null", KR(ret));
  } else if (OB_ISNULL(table_ctx = ObTableLoadService::alloc_ctx())) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_WARN("fail to alloc table ctx", KR(ret), K(param));
  } else if (OB_FAIL(table_ctx->init(param, stmt_ctx_.ddl_param_, session_info))) {
    LOG_WARN("fail to init table ctx", KR(ret));
  } else if (OB_FAIL(ObTableLoadCoordinator::init_ctx(table_ctx, column_ids, execute_ctx_))) {
    LOG_WARN("fail to coordinator init ctx", KR(ret));
  } else if (OB_FAIL(ObTableLoadService::add_ctx(table_ctx))) {
    LOG_WARN("fail to add ctx", KR(ret));
  } else {
    table_ctx_ = table_ctx;
  }
  if (OB_FAIL(ret)) {
    if (nullptr != table_ctx) {
      ObTableLoadService::free_ctx(table_ctx);
      table_ctx = nullptr;
    }
  }
  if (OB_SUCC(ret)) {
    ObTableLoadCoordinator coordinator(table_ctx_);
    if (OB_FAIL(coordinator.init())) {
      LOG_WARN("fail to init coordinator", KR(ret));
    } else if (OB_FAIL(coordinator.begin())) {
      LOG_WARN("fail to coodrinator begin", KR(ret));
    }
  }
  return ret;
}

int ObTableLoadInstance::end_direct_load(const bool commit)
{
  int ret = OB_SUCCESS;
  int tmp_ret = OB_SUCCESS;
  if (OB_UNLIKELY(nullptr == table_ctx_)) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("unexpected table ctx is null", KR(ret));
  } else {
    if (commit) {
      ObTableLoadStatusType status = ObTableLoadStatusType::NONE;
      int error_code = OB_SUCCESS;
      ObTableLoadCoordinator coordinator(table_ctx_);
      if (OB_FAIL(coordinator.init())) {
        LOG_WARN("fail to init coordinator", KR(ret));
      }
      // finish
      else if (OB_FAIL(coordinator.finish())) {
        LOG_WARN("fail to finish", KR(ret));
      }
      while (OB_SUCC(ret) && ObTableLoadStatusType::MERGED != status &&
             OB_SUCC(execute_ctx_->check_status())) {
        if (OB_FAIL(coordinator.get_status(status, error_code))) {
          LOG_WARN("fail to coordinator get status", KR(ret));
        } else {
          switch (status) {
            case ObTableLoadStatusType::FROZEN:
            case ObTableLoadStatusType::MERGING:
              usleep(WAIT_INTERVAL_US);
              break;
            case ObTableLoadStatusType::MERGED:
              break;
            case ObTableLoadStatusType::ERROR:
              ret = error_code;
              LOG_WARN("table load has error", KR(ret));
              break;
            default:
              ret = OB_ERR_UNEXPECTED;
              LOG_WARN("unexpected status", KR(ret), K(status));
              break;
          }
        }
      }
      if (OB_SUCC(ret)) {
        // commit
        if (OB_FAIL(coordinator.commit(result_info_))) {
          LOG_WARN("fail to commit", KR(ret));
        }
      }
    }
    if (OB_FAIL(ret) || !commit) {
      // must abort here, abort redef table need exec_ctx session_info
      ObTableLoadCoordinator::abort_ctx(table_ctx_);
    }
    if (OB_TMP_FAIL(ObTableLoadService::remove_ctx(table_ctx_))) {
      LOG_WARN("table ctx may remove by service", KR(tmp_ret), KP(table_ctx_));
    }
    ObTableLoadService::put_ctx(table_ctx_);
    table_ctx_ = nullptr;
    job_stat_ = nullptr;
  }
  return ret;
}

int ObTableLoadInstance::start_trans()
{
  int ret = OB_SUCCESS;
  ObTableLoadSegmentID segment_id(DEFAULT_SEGMENT_ID);
  ObTableLoadCoordinator coordinator(table_ctx_);
  if (OB_FAIL(coordinator.init())) {
    LOG_WARN("fail to init coordinator", KR(ret));
  } else if (OB_FAIL(coordinator.start_trans(segment_id, trans_ctx_.trans_id_))) {
    LOG_WARN("fail to coordinator start trans", KR(ret));
  } else if (OB_FAIL(trans_ctx_.next_sequence_no_array_.create(table_ctx_->param_.session_count_,
                                                               *allocator_))) {
    LOG_WARN("fail to create next sequence no array", KR(ret));
  } else {
    for (int64_t i = 0; i < table_ctx_->param_.session_count_; ++i) {
      trans_ctx_.next_sequence_no_array_[i] = 1;
    }
  }
  return ret;
}

int ObTableLoadInstance::write_trans(int32_t session_id, const ObTableLoadObjRowArray &obj_rows)
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(session_id < 0 || session_id > table_ctx_->param_.session_count_ ||
                  obj_rows.empty())) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid args", KR(ret), K(session_id), K(obj_rows.count()));
  } else {
    // TODO(suzhi.yt): java客户端调用的时候, 对于相同session_id可能会并发
    uint64_t &next_sequence_no = trans_ctx_.next_sequence_no_array_[session_id - 1];
    ObTableLoadCoordinator coordinator(table_ctx_);
    if (OB_FAIL(coordinator.init())) {
      LOG_WARN("fail to init coordinator", KR(ret));
    } else if (OB_FAIL(coordinator.write(trans_ctx_.trans_id_, session_id, next_sequence_no++,
                                         obj_rows))) {
      LOG_WARN("fail to write coordinator", KR(ret));
    }
  }
  return ret;
}

int ObTableLoadInstance::commit_trans()
{
  int ret = OB_SUCCESS;
  ObTableLoadTransStatusType trans_status = ObTableLoadTransStatusType::NONE;
  int error_code = OB_SUCCESS;
  ObTableLoadCoordinator coordinator(table_ctx_);
  if (OB_FAIL(coordinator.init())) {
    LOG_WARN("fail to init coordinator", KR(ret));
  }
  // finish trans
  else if (OB_FAIL(coordinator.finish_trans(trans_ctx_.trans_id_))) {
    LOG_WARN("fail to finish trans", KR(ret));
  }
  while (OB_SUCC(ret) && ObTableLoadTransStatusType::COMMIT != trans_status &&
         OB_SUCC(execute_ctx_->check_status())) {
    if (OB_FAIL(coordinator.get_trans_status(trans_ctx_.trans_id_, trans_status, error_code))) {
      LOG_WARN("fail to coordinator get trans status", KR(ret));
    } else {
      switch (trans_status) {
        case ObTableLoadTransStatusType::FROZEN:
          usleep(WAIT_INTERVAL_US);
          break;
        case ObTableLoadTransStatusType::COMMIT:
          break;
        case ObTableLoadTransStatusType::ERROR:
          ret = error_code;
          LOG_WARN("trans has error", KR(ret));
          break;
        default:
          ret = OB_ERR_UNEXPECTED;
          LOG_WARN("unexpected trans status", KR(ret), K(trans_status));
          break;
      }
    }
  }
  return ret;
}

} // namespace observer
} // namespace oceanbase
