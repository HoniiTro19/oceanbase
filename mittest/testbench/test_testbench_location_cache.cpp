#include "test_testbench_config_base.h"
#include "testbench/ob_testbench_location_cache.h"
#include "testbench/ob_testbench_systable_helper.h"
#include <thread>

namespace oceanbase
{
namespace unittest
{
class TestTestbenchLocationCache : public TestConfig {
public:
  TestTestbenchLocationCache() : database_name("test"), table_name("testbench") {}
  ~TestTestbenchLocationCache() {}

  virtual void SetUp();
  virtual void Tear();

public:
  const char *database_name;
  const char *table_name;
  testbench::ObTestbenchSystableHelper systable_helper;
  testbench::ObTestbenchLocationCache location_cache;
};

void TestTestbenchLocationCache::SetUp() {
  TestConfig::SetUp();
  ASSERT_EQ(OB_SUCCESS, systable_helper.init_conn(mysql_config));
  ASSERT_EQ(OB_SUCCESS, location_cache.init(database_name, table_name, &systable_helper));
}

void TestTestbenchLocationCache::Tear() {
  systable_helper.destroy();
  location_cache.destroy();
}

TEST_F(TestTestbenchLocationCache, refresh_locations) {
  ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
  ASSERT_EQ(ObString("tb"), location_cache.get_user_tenant().str());
  int64_t key_count = 0;
  auto add_count_cb = [&](hash::HashMapPair<ObAddr, testbench::PartitionList*> &entry) -> int {
    int ret = OB_SUCCESS;
    ++key_count;
    return ret;
  };
  ASSERT_EQ(OB_SUCCESS, location_cache.get_partition_info()->foreach_refactored(add_count_cb));
  ASSERT_EQ(key_count, location_cache.get_svr_addrs().count());
}

TEST_F(TestTestbenchLocationCache, generate_parameters) {
  ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
  int64_t times = 1000;
  testbench::ParametersGroup pgroup;
  testbench::DblinksGroup dgroup;
  ASSERT_NE(OB_SUCCESS, location_cache.gen_distributed_txn_params(1000, pgroup, dgroup));
  for (int64_t i = 0; i < times; ++i) {
    // distributed transaction
    ASSERT_EQ(OB_SUCCESS, location_cache.gen_distributed_txn_params(2, pgroup, dgroup));
    ASSERT_EQ(1, pgroup.count());
    ASSERT_EQ(1, dgroup.count());
    ASSERT_EQ(2, pgroup.at(0).count());
    ASSERT_EQ(2, dgroup.at(0).count());
    // contention transaction
    ASSERT_EQ(OB_SUCCESS, location_cache.gen_contention_txn_params(10, pgroup, dgroup));
    ASSERT_EQ(10, pgroup.count());
    ASSERT_EQ(10, dgroup.count());
    for (int64_t j = 0; j < 10; ++j) {
      ASSERT_EQ(1, pgroup.at(j).count());
      ASSERT_EQ(1, dgroup.at(j).count());
    }
    // deadlock transaction
    ASSERT_EQ(OB_SUCCESS, location_cache.gen_deadlock_txn_params(12, pgroup, dgroup));
    ASSERT_EQ(12, pgroup.count());
    ASSERT_EQ(12, dgroup.count());
    for (int64_t j = 0; j < 12; ++j) {
      ASSERT_EQ(1, pgroup.at(j).count());
      ASSERT_EQ(1, dgroup.at(j).count());
    }
    // concurrent transaction
    ASSERT_EQ(OB_SUCCESS, location_cache.gen_concurrent_txn_params(20, pgroup, dgroup));
    ASSERT_EQ(20, pgroup.count());
    ASSERT_EQ(20, dgroup.count());
    for (int64_t j = 0; j < 20; ++j) {
      ASSERT_EQ(1, pgroup.at(j).count());
      ASSERT_EQ(1, dgroup.at(j).count());
    }
  }
}

TEST_F(TestTestbenchLocationCache, concurrent_request) {
  ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
  testbench::ParametersGroup pgroup;
  testbench::DblinksGroup dgroup;
  auto write_request = [&]() {
    int times = 0;
    while (times < 5) {
      ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
      sleep(1);
      ++times;
    }
  };
  auto read_request = [&]() {
    testbench::Parameters parameters;
    int times = 0;
    int64_t parameter = -1;
    while (times < 100000) {
      ASSERT_EQ(OB_SUCCESS, location_cache.gen_concurrent_txn_params(1, pgroup, dgroup));
      ++times;
    }
  };
  std::thread thread_write = std::thread(write_request);
  std::thread thread_read = std::thread(read_request);
  thread_write.join();
  thread_read.join();
}
} // namespace unittest
} // namespace oceanbase

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  const char *file_name = "test_testbench_location_cache.log";
  OB_LOGGER.set_log_level("INFO");
  remove(file_name);
  OB_LOGGER.set_file_name(file_name, false, false);
  return RUN_ALL_TESTS();
}