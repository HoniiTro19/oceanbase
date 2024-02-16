#include "test_testbench_config_base.h"
#include "testbench/ob_testbench_server_provider.h"
#include "gtest/gtest.h"

namespace oceanbase {
namespace unittest {

class TestServerProvider : public TestConfig {
public:
  TestServerProvider() : systable_helper(), server_provider() {}
  ~TestServerProvider() {}
  virtual void SetUp();
  virtual void Tear();

public:
  testbench::ObTestbenchSystableHelper systable_helper;
  testbench::ObTestbenchServerProvider server_provider;
};

void TestServerProvider::SetUp() {
  TestConfig::SetUp();
  ASSERT_EQ(OB_SUCCESS, systable_helper.init_conn(mysql_config));
  ASSERT_EQ(OB_SUCCESS, server_provider.init(systable_helper));
}

void TestServerProvider::Tear() {
  server_provider.destroy();
  systable_helper.destroy();
}

TEST_F(TestServerProvider, refresh) {
  EXPECT_EQ(OB_SUCCESS, server_provider.prepare_refresh());
  EXPECT_EQ(OB_SUCCESS, server_provider.end_refresh());
  common::ObSEArray<uint64_t, 16> tenant_list;
  common::ObSEArray<testbench::ObTenantName, 16> tenant_name_list;
  EXPECT_EQ(OB_SUCCESS, server_provider.get_tenant_ids(tenant_list));
  EXPECT_EQ(OB_SUCCESS, server_provider.get_tenants(tenant_name_list));
  TESTBENCH_LOG(INFO, "get tenant ids", K(tenant_list));
  TESTBENCH_LOG(INFO, "get tenant names", K(tenant_name_list));
  int64_t server_count = server_provider.get_server_count();
  int64_t tenant_count = tenant_list.count();
  common::ObSEArray<common::ObAddr, 16> server_list(
      "OBMySQLConnPool", OB_MALLOC_NORMAL_BLOCK_SIZE);
  for (int64_t tenant_idx = 0; tenant_idx < tenant_count; ++tenant_idx) {
    uint64_t tenant_id = OB_INVALID_TENANT_ID;
    EXPECT_EQ(OB_SUCCESS, tenant_list.at(tenant_idx, tenant_id));
    EXPECT_EQ(OB_SUCCESS,
              server_provider.get_tenant_servers(tenant_id, server_list));
    EXPECT_EQ(server_count, server_list.count());
    TESTBENCH_LOG(INFO, "tenant get servers", K(tenant_id), K(server_list));
  }
  tenant_list.destroy();
  tenant_name_list.destroy();
  server_list.destroy();
}
} // namespace unittest
} // namespace oceanbase

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  const char *file_name = "test_testbench_server_provider.log";
  OB_LOGGER.set_log_level("INFO");
  remove(file_name);
  OB_LOGGER.set_file_name(file_name, true, false);
  return RUN_ALL_TESTS();
}