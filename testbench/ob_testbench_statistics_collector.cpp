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

#include "ob_testbench_statistics_collector.h"
#include "share/ob_thread_define.h"
#include "share/ob_thread_mgr.h"

namespace oceanbase {
namespace testbench {

/*
                              ObLatencyTask
*/
ObLatencyTask::ObLatencyTask(ObLatencyTaskType type, const common::ObObj &latency) : type_(type), latency_(latency) {}

ObLatencyTask::~ObLatencyTask() {}

ObLatencyTask *ObLatencyTask::__get_class_address(common::ObLink *ptr) {
  return nullptr != ptr ? CONTAINER_OF(ptr, ObLatencyTask, __next_) : nullptr;
}

common::ObLink *ObLatencyTask::__get_member_address(ObLatencyTask *ptr) {
  return nullptr != ptr ? reinterpret_cast<common::ObLink*>(ADDRESS_OF(ptr, ObLatencyTask, __next_)) : nullptr;
}

/*
                              ObStatisticsTask
*/
ObStatisticsTask::ObStatisticsTask() {}

ObStatisticsTask::~ObStatisticsTask() {}

/*
                              ObStatisticsSubmitTask
*/
ObStatisticsSubmitTask::ObStatisticsSubmitTask() {}

ObStatisticsSubmitTask::~ObStatisticsSubmitTask() {}

int ObStatisticsSubmitTask::init() {
  int ret = OB_SUCCESS;
  type_ = ObStatisticsTaskType::STATISTICS_SUBMIT_TASK;
  TESTBENCH_LOG(INFO, "statistics submit task init succeed", K(type_));
  return ret;
}

/*
                              ObStatisticsQueueTask
*/
ObStatisticsQueueTask::ObStatisticsQueueTask()
  : total_submit_cnt_(0),
    total_apply_cnt_(0),
    index_(-1),
    histogram_inited_(false)
{}

ObStatisticsQueueTask::~ObStatisticsQueueTask() {}

int ObStatisticsQueueTask::init(int64_t index) {
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(index < 0)) {
    ret = OB_INVALID_ARGUMENT;
    TESTBENCH_LOG(WARN, "statistics queue task invalid argument", K(type_), K(index), K(ret));
  } else {
    type_ = ObStatisticsTaskType::STATISTICS_QUEUE_TASK;
    index_ = index;
    TESTBENCH_LOG(INFO, "statistics queue task init succeed", K(type_));
  }
  return ret;
}

int ObStatisticsQueueTask::top(ObLatencyTask *&task) {
  int ret = OB_SUCCESS;
  common::ObLink *link = nullptr;
  if (OB_FAIL(queue_.top(link))) {
    TESTBENCH_LOG(WARN, "get statistics queue link top failed", K(ret));
  } else {
    task = ObLatencyTask::__get_class_address(link);
  }
  return ret;
}

int ObStatisticsQueueTask::pop() {
  common::ObLink *link = nullptr;
  return queue_.pop(link);
}

int ObStatisticsQueueTask::push(ObLatencyTask *task) {
  int ret = OB_SUCCESS;
  common::ObObj latency = task->get_latency();
  common::ObLink *link = ObLatencyTask::__get_member_address(task);
  if (OB_ISNULL(link)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(WARN, "get oblink from latency task failed", K(ret), KPC(task));
  } else if (OB_UNLIKELY(latency.is_null())) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(WARN, "get null latency value", K(ret), KPC(task));
  } else if (OB_FAIL(queue_.push(link))) {
    TESTBENCH_LOG(WARN, "push oblink to the queue failed", K(ret));
  }
  return ret;
}

void ObStatisticsQueueTask::inc_total_submit_cnt() {
  ATOMIC_INC(&total_submit_cnt_);
}

void ObStatisticsQueueTask::inc_total_apply_cnt() {
  ATOMIC_INC(&total_apply_cnt_);
}

int64_t ObStatisticsQueueTask::get_total_submit_cnt() const {
  return ATOMIC_LOAD(&total_submit_cnt_);
}

int64_t ObStatisticsQueueTask::get_total_apply_cnt() const {
  return ATOMIC_LOAD(&total_apply_cnt_);
}

int64_t ObStatisticsQueueTask::get_snapshot_queue_cnt() const {
  return ATOMIC_LOAD(&total_submit_cnt_) - ATOMIC_LOAD(&total_apply_cnt_);
}

bool ObStatisticsQueueTask::is_histogram_inited() const {
  return ATOMIC_LOAD(&histogram_inited_);
}

void ObStatisticsQueueTask::set_histogram_inited() {
  ATOMIC_STORE(&histogram_inited_, true);
}

/*
                          ObTestbenchStatisticsCollector
*/
ObTestbenchStatisticsCollector::ObTestbenchStatisticsCollector()
  : tg_id_(-1),
    is_inited_(false),
    thread_num_(1),
    task_queue_limit_(99999),
    submit_(),
    submit_queues_{},
    histograms_{},
    allocator_("StatCollect")
{
  snprintf(result_file_, OB_MAX_CONTEXT_STRING_LENGTH, "%s", "log/scheduler.result");
}

ObTestbenchStatisticsCollector::~ObTestbenchStatisticsCollector() {}

int ObTestbenchStatisticsCollector::init(ObStatisticsCollectorOptions *opts) {
  int ret = OB_SUCCESS;
  if (IS_INIT) {
    ret = OB_INIT_TWICE;
    TESTBENCH_LOG(WARN, "statistics collector has already been inited", K(ret));
  } else if (OB_ISNULL(opts)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "statistics collector options are not inited", KR(ret));
  } else if (OB_FAIL(submit_.init())) {
    TESTBENCH_LOG(ERROR, "statistics submit task init failed", K(ret));
  } else {
    bucket_capacity_ = opts->get_bucket_capacity();
    for (int64_t i = 0; OB_SUCC(ret) && i < TASK_QUEUE_SIZE; ++i) {
      if (OB_FAIL(submit_queues_[i].init(i))) {
        TESTBENCH_LOG(ERROR, "statistics submit queue task init failed", K(ret), K(i));
      } else if (OB_FAIL(histograms_[i].prepare_allocate_buckets(allocator_, bucket_capacity_))) {
        TESTBENCH_LOG(ERROR, "histogram allocate new buckets failed", K(ret), K(bucket_capacity_));
      } else {
        histograms_[i].set_sample_size(0);
      }
    }
  }
  if (OB_SUCC(ret)) {
    bucket_min_ratio_ = (double)opts->get_bucket_min_percentage() / 100.0;
    bucket_max_ratio_ = (double)opts->get_bucket_max_percentage() / 100.0;
    thread_num_ = opts->get_thread_num();
    task_queue_limit_ = opts->get_task_queue_limit();
    is_inited_ = true;
  }
  return ret;
}

int ObTestbenchStatisticsCollector::start() {
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "statistics collector is not inited", K(ret));
  } else if (OB_FAIL(TG_CREATE(lib::TGDefIDs::StatisticsCollectorPool, tg_id_))) {
    TESTBENCH_LOG(ERROR, "statistics collector create threadpool failed", K(ret), K(tg_id_));
  } else if (OB_FAIL(TG_SET_HANDLER_AND_START(tg_id_, *this))) {
    TESTBENCH_LOG(ERROR, "start statistics collector failed", K(tg_id_), K(ret));
  } else if (OB_FAIL(TG_SET_THREAD_CNT(tg_id_, thread_num_))) {
    TESTBENCH_LOG(WARN, "set ObTestbenchStatisticsCollector thread cnt fail", KR(ret), K_(thread_num));
  } else if (OB_FAIL(TG_SET_QUEUE_SIZE(tg_id_, task_queue_limit_))) {
    TESTBENCH_LOG(WARN, "set ObTestbenchStatisticsCollector queue size fail", KR(ret), K_(task_queue_limit));
  } else {
    TESTBENCH_LOG(INFO, "start statistics collector succeed", K(tg_id_), K(ret));
  }
  return ret;
}

void ObTestbenchStatisticsCollector::stop() {
  TESTBENCH_LOG(INFO, "statistics collector stop start");
  if (-1 != tg_id_) {
    TG_STOP(tg_id_);
  }
  TESTBENCH_LOG(INFO, "statistics collector stop finish");
}

void ObTestbenchStatisticsCollector::wait() {
  TESTBENCH_LOG(INFO, "statistics collector wait start");
  if (-1 != tg_id_) {
    int64_t num = 0;
    int ret = OB_SUCCESS;
    while (OB_SUCC(TG_GET_QUEUE_NUM(tg_id_, num)) && num > 0) {
      PAUSE();
    }
    if (OB_FAIL(ret)) {
      TESTBENCH_LOG(WARN, "statistics collector get queue number failed");
    }
    TG_WAIT(tg_id_);
  }
  TESTBENCH_LOG(INFO, "statistics collector wait finish");
}

void ObTestbenchStatisticsCollector::destroy() {
  stop();
  wait();
  is_inited_ = false;
  if (-1 != tg_id_) {
    TG_DESTROY(tg_id_);
    tg_id_ = -1;
  }
  TESTBENCH_LOG(INFO, "statistics collector destroy");
}

int ObTestbenchStatisticsCollector::push_task_(ObStatisticsTask *task) {
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "statistics collector is not inited", K(ret));
  } else if (OB_ISNULL(task)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "task is null", K(ret));
  } else {
    while (OB_FAIL(TG_PUSH_TASK(tg_id_, task)) && OB_EAGAIN == ret) {
      ob_usleep(1000);
      TESTBENCH_LOG(ERROR, "statistics collector push task failed", K(ret));
    }
  }
  return ret;
}

int ObTestbenchStatisticsCollector::push_latency_task(ObLatencyTask *task) {
  int ret = OB_SUCCESS;
  ObLatencyTaskType type = task->get_type();
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "statistics collector is not inited", K(ret));
  } else if (OB_FAIL(submit_queues_[type].push(task))) {
    TESTBENCH_LOG(ERROR, "push latency task failed", K(submit_queues_[type]), K(type), K(ret));
  } else {
    submit_queues_[type].inc_total_submit_cnt();
    int64_t queue_size = submit_queues_[type].get_snapshot_queue_cnt();
    if (queue_size > 10000 && OB_FAIL(sync_latency_task())) {
      TESTBENCH_LOG(ERROR, "push sync latency task failed", KR(ret));
    }
  }
  return ret;
}

int ObTestbenchStatisticsCollector::sync_latency_task() {
  int ret = OB_SUCCESS;
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "statistics collector is not inited", K(ret));
  } else if (OB_FAIL(push_task_(&submit_))) {
    TESTBENCH_LOG(ERROR, "sync submit task failed", K(ret), K(submit_));
  }
  return ret;
}

int ObTestbenchStatisticsCollector::get_percentage_latency(ObLatencyTaskType type, double_t percentage, double_t &latency) {
  int ret = OB_SUCCESS;
  common::ObObj value;
  if (OB_FAIL(histograms_[type].get_percentage_value(percentage, value))) {
    TESTBENCH_LOG(WARN, "get percentage value failed", K(ret));
  } else {
    latency = value.get_double();
  }
  return ret;
}

const ObHistogram &ObTestbenchStatisticsCollector::get_histogram(ObLatencyTaskType type) const {
  return histograms_[type];
}

const ObStatisticsQueueTask &ObTestbenchStatisticsCollector::get_queue_task(ObLatencyTaskType type) const {
  return submit_queues_[type];
}

void ObTestbenchStatisticsCollector::handle(void *task) {
  int ret = OB_SUCCESS;
  ObStatisticsTask *task_to_handle = static_cast<ObStatisticsTask *>(task);
  if (IS_NOT_INIT) {
    ret = OB_NOT_INIT;
    TESTBENCH_LOG(ERROR, "statistics collector is not inited", K(ret));
  } else if (OB_ISNULL(task_to_handle)) {
    ret = OB_ERR_UNEXPECTED;
    TESTBENCH_LOG(ERROR, "statistics task is null", K(ret));
  } else {
    ObStatisticsTaskType task_type = task_to_handle->get_type();
    if (ObStatisticsTaskType::STATISTICS_QUEUE_TASK == task_type) {
      ObStatisticsQueueTask *queue_task = static_cast<ObStatisticsQueueTask*>(task_to_handle);
      ret = handle_queue_task_(queue_task);
    } else if (ObStatisticsTaskType::STATISTICS_SUBMIT_TASK == task_type) {
      ret = handle_submit_task_();
    } else {
      ret = OB_ERR_UNEXPECTED;
      TESTBENCH_LOG(ERROR, "invalid task type", K(ret), K(task_type));
    }
  }
  if (OB_FAIL(ret) && nullptr != task_to_handle) {
    if (OB_FAIL(push_task_(task_to_handle))) {
      TESTBENCH_LOG(ERROR, "push task back after handle failed", K(ret), KPC(task_to_handle));
    }
  }
}

int ObTestbenchStatisticsCollector::init_histogram_(ObStatisticsQueueTask *task) {
  int ret = OB_SUCCESS;
  return ret;
}

int ObTestbenchStatisticsCollector::handle_queue_task_(ObStatisticsQueueTask *task) {
  int ret = OB_SUCCESS;
  int64_t index = task->get_index();
  ObHistogram &histogram = histograms_[index];
  int64_t task_cnt = task->get_snapshot_queue_cnt();
  if (OB_UNLIKELY(!task->is_histogram_inited())) {
    ObVector<common::ObObj> buffer;
    buffer.reset();
    for (int64_t i = 0; i < task_cnt; ++i) {
        ObLatencyTask *latency_task = nullptr;
      if (OB_FAIL(task->top(latency_task))) {
        TESTBENCH_LOG(WARN, "get top latency task failed", KR(ret));
      } else {
        const common::ObObj &latency = latency_task->get_latency();
        if (OB_FAIL(buffer.push_back(latency))) {
          TESTBENCH_LOG(WARN, "put latency into buffer failed", KR(ret));
        }
        task->inc_total_apply_cnt();
        if (OB_FAIL(task->pop())) {
          TESTBENCH_LOG(WARN, "pop top latency task failed", KR(ret));
        }
        allocator_.free(latency_task);
      }
    }
    std::sort(buffer.begin(), buffer.end());
    int64_t q99_index = 999 * (task_cnt - 1) / 1000;
    int64_t q0_value = buffer[0].get_int();
    int64_t q99_value = buffer[q99_index].get_int();
    double_t bucket_width = static_cast<double_t>(q99_value - q0_value) / (bucket_capacity_ * (bucket_max_ratio_ - bucket_min_ratio_));
    double bucket_min_value = std::max(static_cast<double_t>(0), q0_value - bucket_min_ratio_ * bucket_capacity_ * bucket_width);
    histogram.set_bucket_width(bucket_width);
    for (int64_t i = 0; i < bucket_capacity_; ++i) {
      common::ObObj value;
      value.set_double(bucket_min_value + bucket_width * i);
      ObHistBucket bucket(value, 0, 0, 0);
      if (OB_FAIL(histogram.add_bucket(bucket))) {
        TESTBENCH_LOG(WARN, "histogram initialization add bucket failed", KR(ret), K(bucket));
      }
    }
    task->set_histogram_inited();
    TESTBENCH_LOG(INFO, "init histogram finished", KR(ret), K(q99_value), "name", latency_task_names[index], K(bucket_width));
    for (int64_t i = 0; i < task_cnt; ++i) {
      if (OB_FAIL(histogram.inc_endpoint_repeat_count(buffer.at(i)))) {
        TESTBENCH_LOG(WARN, "increase latency count failed", KR(ret), "type", task->get_type(), "latency", buffer.at(i));
      }
    }
  } else {
    for (int64_t i = 0; i < task_cnt; ++i) {
      ObLatencyTask *latency_task = nullptr;
      if (OB_FAIL(task->top(latency_task))) {
        TESTBENCH_LOG(WARN, "get top latency task failed", K(ret));
      } else {
        const common::ObObj &latency = latency_task->get_latency();
        if (OB_FAIL(histogram.inc_endpoint_repeat_count(latency))) {
          TESTBENCH_LOG(WARN, "increase latency count failed", K(ret), "type", task->get_type(), K(latency));
        }
        task->inc_total_apply_cnt();
        if (OB_FAIL(task->pop())) {
          TESTBENCH_LOG(WARN, "pop top latency task failed", K(ret));
        }
        allocator_.free(latency_task);
      }
    }
  }
  TESTBENCH_LOG(INFO, "process queued tasks finished", KR(ret), "type", task->get_type());
  return ret;
}

int ObTestbenchStatisticsCollector::handle_submit_task_() {
  int ret = OB_SUCCESS;
  for (int64_t i = 0; OB_SUCC(ret) && i < TASK_QUEUE_SIZE; i++) {
    if (submit_queues_[i].get_total_apply_cnt() >= submit_queues_[i].get_total_submit_cnt()) {
      // do nothing
    } else if (OB_FAIL(push_task_(&submit_queues_[i]))) {
      TESTBENCH_LOG(ERROR, "submit queued tasks failed", K(submit_queues_[i]), K(ret));
    }
  }
  return ret;
}

int ObTestbenchStatisticsCollector::generate_report() {
  int ret = OB_SUCCESS;
  unsigned int mode = 0644;
  int32_t fd = -1;
  if (OB_FAIL(sync_latency_task())) {
    TESTBENCH_LOG(ERROR, "push sync latency task failed", KR(ret));
  } else if (OB_UNLIKELY((fd = ::open(result_file_, O_WRONLY | O_CREAT | O_APPEND, mode))) < 0) {
    ret = OB_ERR_UNEXPECTED;
    (void)close(fd);
    TESTBENCH_LOG(ERROR, "open result file failed", KR(ret), K(fd));
  } else {
    int64_t num = 0;
    while (tg_id_ != -1 && OB_SUCC(TG_GET_QUEUE_NUM(tg_id_, num)) && num > 0) {
      PAUSE();
    }
    static const int64_t max_buf_len = 512;
    char buf[max_buf_len];
    for (int64_t i = 0; i < TASK_QUEUE_SIZE; ++i) {
      if (submit_queues_[i].is_histogram_inited()) {
        snprintf(buf, max_buf_len, "histogram type:\n%s\n", latency_task_names[i]);
        ObArray<int64_t> counts;
        ObArray<double_t> values;
        if (OB_UNLIKELY(-1 == ::write(fd, buf, strlen(buf)))) {
          ret = OB_ERR_SYS;
          TESTBENCH_LOG(ERROR, "write histogram type name failed", KR(ret), "name", latency_task_names[i]);
        } else if (OB_FAIL(histograms_[i].get_endpoint_values(counts, values))) {
          TESTBENCH_LOG(ERROR, "get histogram endpoint values failed", KR(ret), "name", latency_task_names[i]);
        } else {
          TESTBENCH_LOG(INFO, "write down histogram information", "name", latency_task_names[i], "submit", submit_queues_[i].get_total_submit_cnt(), "apply", submit_queues_[i].get_total_apply_cnt());
          const char *counts_title = "endpoint counts:\n";
          if (OB_UNLIKELY(-1 == ::write(fd, counts_title, strlen(counts_title)))) {
            ret = OB_ERR_SYS;
            TESTBENCH_LOG(ERROR, "write endpoint counts title failed", KR(ret), KCSTRING(counts_title));
          }
          ARRAY_FOREACH(counts, i) {
            snprintf(buf, max_buf_len, "%ld\n", counts.at(i));
            if (OB_UNLIKELY(-1 == ::write(fd, buf, strlen(buf)))) {
              ret = OB_ERR_SYS;
              TESTBENCH_LOG(ERROR, "write endpoint count line failed", KR(ret), "index", i);
            }
          }
          const char *values_title = "endpoint values:\n";
          if (OB_UNLIKELY(-1 == ::write(fd, values_title, strlen(values_title)))) {
            ret = OB_ERR_SYS;
            TESTBENCH_LOG(ERROR, "write endpoint values title failed", KR(ret), KCSTRING(values_title));
          }
          ARRAY_FOREACH(values, i) {
            snprintf(buf, max_buf_len, "%.3f\n", values.at(i));
            if (OB_UNLIKELY(-1 == ::write(fd, buf, strlen(buf)))) {
              ret = OB_ERR_SYS;
              TESTBENCH_LOG(ERROR, "write endpoint value line failed", KR(ret), "index", i);
            }
          }
          snprintf(buf, max_buf_len, "end\n");
          if (OB_UNLIKELY(-1 == ::write(fd, buf, strlen(buf)))) {
            ret = OB_ERR_SYS;
            TESTBENCH_LOG(ERROR, "write blank line failed", KR(ret));
          }
        }
      }
    }
  }
  (void)close(fd);
  return ret;
}

} // namespace testbench
} // namespace oceanbase