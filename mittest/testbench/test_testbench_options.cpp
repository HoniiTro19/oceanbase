#include "gtest/gtest.h"
#include "testbench/ob_instance_testbench.h"

namespace oceanbase
{
namespace testbench
{
class TestOptionsParser : public ::testing::Test {
public:
  TestOptionsParser() {}
  ~TestOptionsParser() {}

  void SetUp() {}
  void Tear() {}

public:
  int argc = 19;
  const char *argv_const[19] = {
    "dummy",
    "-l", "INFO",
    "-t", "5",
    "-c", "host=127.0.0.1,user=root,port=2881,database=test,table=testbench",
    "-d", "partitions=9,rows=10000",
    "-s", "capacity=500,minimum=10,maximum=80,threads=1,tasks=99999",
    "-T", "threads=50,tasks=100,participants=1,operations=10",
    "-C", "threads=50,tasks=100,concurrency=1,operations=10,aborts=0",
    "-D", "threads=50,tasks=100,concurrency=2,chains=2"
  };
  ObTestbenchOptions opts;
};

TEST_F(TestOptionsParser, parse_all_options) {
  char *argv[19];
  for (int i = 0; i < 19; ++i) {
    argv[i] = const_cast<char*>(argv_const[i]);
  }
  parse_opts(argc, argv, opts);
  ASSERT_EQ(OB_SUCCESS, opts.parse_options());
  EXPECT_EQ(3, opts.workload_opts_cnt_);
  EXPECT_EQ(OB_LOG_LEVEL_WDIAG, opts.log_level_);
  EXPECT_EQ(0, strcmp("log", opts.log_dir_));
  EXPECT_EQ(0, strcmp("log/scheduler.log", opts.log_file_));
  EXPECT_EQ(5, opts.duration_);
  EXPECT_EQ(0, strcmp("127.0.0.1", opts.connection_opts_->get_cluster_host()));
  EXPECT_EQ(0, strcmp("root", opts.connection_opts_->get_cluster_user()));
  EXPECT_EQ(2881, opts.connection_opts_->get_cluster_port());
  EXPECT_EQ(0, strcmp("", opts.connection_opts_->get_cluster_pass()));
  EXPECT_EQ(0, strcmp("test", opts.connection_opts_->get_cluster_db_name()));
  EXPECT_EQ(0, strcmp("testbench", opts.connection_opts_->get_cluster_table_name()));
  EXPECT_EQ(9, opts.dataset_opts_->get_partitions());
  EXPECT_EQ(10000, opts.dataset_opts_->get_rows());
  EXPECT_EQ(500, opts.statistics_opts_->get_bucket_capacity());
  EXPECT_EQ(10, opts.statistics_opts_->get_bucket_min_percentage());
  EXPECT_EQ(80, opts.statistics_opts_->get_bucket_max_percentage());
  EXPECT_EQ(1, opts.statistics_opts_->get_thread_num());
  EXPECT_EQ(99999, opts.statistics_opts_->get_task_queue_limit());
  ObDistributedTransactionOptions *dtxn_opts = static_cast<ObDistributedTransactionOptions*>(opts.workloads_opts_.at(0));
  ObContentionTransactionOptions *contention_opts = static_cast<ObContentionTransactionOptions*>(opts.workloads_opts_.at(1));
  ObDeadlockTransactionOptions *deadlock_opts = static_cast<ObDeadlockTransactionOptions*>(opts.workloads_opts_.at(2));
  ASSERT_NE(nullptr, dtxn_opts);
  ASSERT_NE(nullptr, contention_opts);
  ASSERT_NE(nullptr, deadlock_opts);
  EXPECT_EQ(50, dtxn_opts->get_thread_num());
  EXPECT_EQ(100, dtxn_opts->get_task_queue_limit());
  EXPECT_EQ(1, dtxn_opts->get_participants());
  EXPECT_EQ(10, dtxn_opts->get_operations());
  EXPECT_EQ(50, contention_opts->get_thread_num());
  EXPECT_EQ(100, contention_opts->get_task_queue_limit());
  EXPECT_EQ(1, contention_opts->get_concurrency());
  EXPECT_EQ(10, contention_opts->get_operations());
  EXPECT_EQ(0, contention_opts->get_aborts());
  EXPECT_EQ(50, deadlock_opts->get_thread_num());
  EXPECT_EQ(100, deadlock_opts->get_task_queue_limit());
  EXPECT_EQ(2, deadlock_opts->get_concurrency());
  EXPECT_EQ(2, deadlock_opts->get_chains());
}
} // namespace testbench
} // namespace oceanbase

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  const char *file_name = "test_testbench_options.log";
  OB_LOGGER.set_log_level("INFO");
  remove(file_name);
  OB_LOGGER.set_file_name(file_name, true, false);
  return RUN_ALL_TESTS();
}