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

#ifndef _OCEANBASE_INSTANCE_TESTBENCH_H
#define _OCEANBASE_INSTANCE_TESTBENCH_H

#include "testbench/ob_testbench_builder.h"
#include "lib/file/file_directory_utils.h"
#include "getopt.h"

namespace oceanbase
{
namespace testbench
{
static void print_help() {
  MPRINT("testbench [OPTIONS]");
  MPRINT("-h, --help, print this help");
  MPRINT("-H, --home_path, home path of the benchmark process");
  MPRINT("-l, --log_level, log level of the benchmark process, candicates are ERROR, WARN, INFO, TRACE, DEBUG");
  MPRINT("-t, --time, time duration of the benchmark process");
  MPRINT("-c, --connection, option string of cluster connection, for example host=127.0.0.1,user=root,port=2881,password=,database=test,table=testbench");
  MPRINT("-d, --dataset, option string of dataset parameters, for example partitions=9,rows=10000");
  MPRINT("-s, --statistics, option string of statistics collector, for example capacity=500,minimum=10,maximum=80");
  MPRINT("-T, --distributed, option string of distributed transaction workload, for example threads=50,tasks=100,participants=1,operations=10");
  MPRINT("-C, --contention, option string of contention workload, for example threads=50,tasks=100,concurrency=1,operations=10,aborts=0");
  MPRINT("-D, --deadlock, option string of deadlock workload, for example threads=50,tasks=100,concurrency=2,chains=2");
  MPRINT("-M, --concurrent, option string of concurrent workload, for example threads=50,tasks=100,concurrency=2,operations=10,readonly=50");
}

static bool to_int64(const char *sval, int64_t &ival) {
  char *endp = NULL;
  ival = static_cast<int64_t>(strtol(sval, &endp, 10));
  return NULL != endp && *endp == '\0';
}

static int to_port_x(const char *sval) {
  int64_t port = 0;
  if (!to_int64(sval, port)) {
    MPRINTx("need port number: %s", sval);
  } else if (port <= 1024 || port >= 65536) {
    MPRINTx("port number should greater than 1024 and less than 65536: %ld", port);
  }
  return static_cast<int>(port);
}

static int to_duration(const char *sval) {
  int64_t duration = 0;
  if (!to_int64(sval, duration)) {
    MPRINTx("need benchmark time duration: %s", sval);
  } else if (duration <= 0) {
    MPRINTx("benchmark time duration should greater than 0: %ld", duration);
  }
  return static_cast<int>(duration);
}

static void get_opts_setting(struct option long_opts[], char short_opts[], const size_t size) {
  static struct {
    const char *long_name_;
    char short_name_;
    bool has_arg_;
  } tb_opts[] = {
      {"help", 'h', 0},
      {"home_path", 'H', 1},
      {"log_level", 'l', 1},
      {"time", 't', 1},
      {"connection", 'c', 1},
      {"dataset", 'd', 1},
      {"statistics", 's', 1},
      {"distributed", 'T', 1},
      {"contention", 'C', 1},
      {"deadlock", 'D', 1},
      {"concurrent", 'M', 1},
  };
  size_t opts_cnt = sizeof(tb_opts) / sizeof(tb_opts[0]);

  if (opts_cnt > size) {
    MPRINTx("parse option fail: opts array is too small");
  }

  int short_idx = 0;
  for (size_t i = 0; i < opts_cnt; ++i) {
    long_opts[i].name = tb_opts[i].long_name_;
    long_opts[i].has_arg = tb_opts[i].has_arg_;
    long_opts[i].flag = NULL;
    long_opts[i].val = tb_opts[i].short_name_;

    short_opts[short_idx++] = tb_opts[i].short_name_;
    if (tb_opts[i].has_arg_) {
      short_opts[short_idx++] = ':';
    }
  }
}

// process long only option
static void
parse_long_opt(const char *name, const char *value, ObTestbenchOptions &opts) {
  MPRINT("long: %s %s", name, value);
  UNUSED(name);
  UNUSED(value);
  UNUSED(opts);
}

static void
parse_short_opt(const int c, const char *value, ObTestbenchOptions &opts) {
  int ret = OB_SUCCESS;
  int w_len = 0;
  switch (c) {
  case 'h':
    print_help();
    break;
  case 'H':
    if (OB_ISNULL(value)) {
      MPRINTx("invalid home path param");
    } else if (FALSE_IT(w_len = snprintf(opts.home_path_, OB_MAX_CONTEXT_STRING_LENGTH, "%s", value))) {
      // impossible
    } else if (OB_UNLIKELY(w_len <= 0) || OB_UNLIKELY(w_len > OB_MAX_CONTEXT_STRING_LENGTH)) {
      MPRINTx("set home_path error: %s", value);
    }
    MPRINT("set home path: %s", value);
    break;
  case 'l':
    if (OB_FAIL(OB_LOGGER.level_str2int(value, opts.log_level_))) {
      MPRINT("malformed log level, candicates are: "
             "    ERROR,USER_ERR,WARN,INFO,TRACE,DEBUG");
      MPRINT("!! Back to INFO log level.");
      opts.log_level_ = OB_LOG_LEVEL_WARN;
    }
    MPRINT("log level: %s", value);
    break;
  case 't':
    if (OB_ISNULL(value)) {
      MPRINTx("invalid time duration param");
    } else if (FALSE_IT(opts.duration_ = to_duration(value))) {
      MPRINTx("set benchmark time duration error: %s", value);
    }
    MPRINT("set benchmark time duration: %s", value);
    break;
  case 'c':
    MPRINT("cluster connection options: %s", value);
    opts.connection_opts_ = OB_NEW(ObConnectionOptions, ObModIds::OB_TESTBENCH_OPTIONS, value);
    break;
  case 'd':
    MPRINT("dataset parameter options: %s", value);
    opts.dataset_opts_ = OB_NEW(ObDatasetOptions, ObModIds::OB_TESTBENCH_OPTIONS, value);
    break;
  case 's':
    MPRINT("statistics collector options: %s", value);
    opts.statistics_opts_ = OB_NEW(ObStatisticsCollectorOptions, ObModIds::OB_TESTBENCH_OPTIONS, value);
    break;
  case 'T': {
    MPRINT("distributed transaction options: %s", value);
    ObDistributedTransactionOptions *options = nullptr;
    if (OB_ISNULL(options = OB_NEW(ObDistributedTransactionOptions, ObModIds::OB_TESTBENCH_OPTIONS, value))) {
      MPRINTx("generate distributed transaction options error: %s", value);
    } else if (OB_FAIL(opts.workloads_opts_.push_back(options))) {
      MPRINTx("push back distributed transaction options error: %s", value);
    } else {
      opts.workload_opts_cnt_ += 1;
    }
  }
    break;
  case 'C': {
    MPRINT("contention transaction options: %s", value);
    ObContentionTransactionOptions *options = nullptr;
    if (OB_ISNULL(options = OB_NEW(ObContentionTransactionOptions, ObModIds::OB_TESTBENCH_OPTIONS, value))) {
      MPRINTx("generate contention transaction options error: %s", value);
    } else if (OB_FAIL(opts.workloads_opts_.push_back(options))) {
      MPRINTx("push back contention transaction options error: %s", value);
    } else {
      opts.workload_opts_cnt_ += 1;
    }
  }
    break;
  case 'D': {
    MPRINT("deadlock transaction options: %s", value);
    ObDeadlockTransactionOptions *options = nullptr;
    if (OB_ISNULL(options = OB_NEW(ObDeadlockTransactionOptions, ObModIds::OB_TESTBENCH_OPTIONS, value))) {
      MPRINTx("generate deadlock transaction options error: %s", value);
    } else if (OB_FAIL(opts.workloads_opts_.push_back(options))) {
      MPRINTx("push back deadlock transaction options error: %s", value);
    } else {
      opts.workload_opts_cnt_ += 1;
    }
  }
    break;
  case 'M': {
    MPRINT("concurrent transaction options: %s", value);
    ObConcurrentTransactionOptions *options = nullptr;
    if (OB_ISNULL(options = OB_NEW(ObConcurrentTransactionOptions, ObModIds::OB_TESTBENCH_OPTIONS, value))) {
      MPRINTx("generate concurrent transaction options error: %s", value);
    } else if (OB_FAIL(opts.workloads_opts_.push_back(options))) {
      MPRINTx("push back concurrent transaction options error: %s", value);
    } else {
      opts.workload_opts_cnt_ += 1;
    }
  }
    break;
  default:
    print_help();
    MPRINTx("parse workload with unknown type: %s", value);
    break;
  }
}

static void parse_opts(int argc, char *argv[], ObTestbenchOptions &opts) {
  static const int MAX_OPTS_CNT = 16;
  static char short_opts[MAX_OPTS_CNT * 2 + 1];
  static struct option long_opts[MAX_OPTS_CNT];
  get_opts_setting(long_opts, short_opts, MAX_OPTS_CNT);
  int long_opts_idx = 0;

  bool end = false;
  while (!end) {
    int c = getopt_long(argc, argv, short_opts, long_opts, &long_opts_idx);
    if (-1 == c || long_opts_idx >= MAX_OPTS_CNT) {
      end = true;
    } else if (0 == c) {
      parse_long_opt(long_opts[long_opts_idx].name, optarg, opts);
    } else {
      parse_short_opt(c, optarg, opts);
    }
  }
}
} // namespace testbench
} // namespace oceanbase

#endif