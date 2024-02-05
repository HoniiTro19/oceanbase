#include "test_testbench_config_base.h"
#include "testbench/ob_testbench_location_cache.h"

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
}

TEST_F(TestTestbenchLocationCache, generate_partitions) {
  ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
  testbench::Parameters parameters;
  for (int64_t i = 0; i < 1000; ++i) {
    ASSERT_EQ(OB_SUCCESS, location_cache.generate_different_partitions(1, parameters));
    ASSERT_EQ(1, parameters.count());
  }
  ASSERT_EQ(OB_ERR_UNEXPECTED, location_cache.generate_different_partitions(1000, parameters));
}

TEST_F(TestTestbenchLocationCache, random_partition) {
  ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
  int64_t parameter = -1;
  common::ObArray<ObString> svr_ips = location_cache.get_svr_ips();
  int64_t svr_count = svr_ips.count();
  ASSERT_GT(svr_count, 0);
  for (int64_t i = 0; i < svr_count; ++i) {
    for (int64_t j = 0; j < 1000; ++j) {
      ASSERT_EQ(OB_SUCCESS, location_cache.generate_random_partition(svr_ips.at(i), parameter));
      ASSERT_GT(parameter, -1);
    }
  }
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