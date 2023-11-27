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

#include "testbench/ob_instance_testbench.h"
#include "observer/ob_server_utils.h"

using namespace oceanbase;

int main(int argc, char *argv[]) {
  int ret = OB_SUCCESS;
  testbench::ObTestbenchOptions opts;
  parse_opts(argc, argv, opts);
  if (OB_FAIL(FileDirectoryUtils::create_full_path(opts.log_dir_))) {
    MPRINT("create log dir fail: %s", opts.log_dir_);
  }
  const char *syslog_file_info = observer::ObServerUtils::build_syslog_file_info(ObAddr());
  int64_t version = common::ObTimeUtility::current_time();
  OB_LOGGER.set_log_level(opts.log_level_, version);
  OB_LOGGER.set_max_file_size(opts.log_file_size_);
  OB_LOGGER.set_new_file_info(syslog_file_info);
  OB_LOGGER.set_file_name(opts.log_file_, true, false);

  testbench::ObTestbenchBuilder &builder =
      testbench::ObTestbenchBuilder::get_instance();
  if (OB_FAIL(builder.init(opts))) {
    TESTBENCH_LOG(ERROR, "testbench builder init failed", KR(ret));
  } else if (OB_FAIL(builder.start_service())) {
    TESTBENCH_LOG(ERROR, "testbench builder start service failed", KR(ret));
  } else {
    TESTBENCH_LOG(INFO, "testbench builder start service");
  }
  ob_usleep(opts.duration_ * 60 * 1000 * 1000);
  builder.stop_service();
  TESTBENCH_LOG(INFO, "testbench builder stop service");
  OB_LOGGER.set_stop_append_log();
  OB_LOGGER.set_enable_async_log(false);
  OB_LOGGER.set_enable_log_limit(false);
  return ret;
}