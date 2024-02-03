#include "share/ob_thread_mgr.h"
#include "test_testbench_config_base.h"
#include "testbench/ob_testbench_statistics_collector.h"
#include "testbench/ob_testbench_server_provider.h"
#include "testbench/ob_testbench_transaction_task.h"
#include "lib/random/ob_random.h"

namespace oceanbase {
namespace unittest {

using namespace common::sqlclient;
using namespace testbench;
class TestTransactionTask : public TestConfig {
public:
  TestTransactionTask() 
    : mysql_tg_id(-1), statistics_collector() {}
  ~TestTransactionTask() {}

  virtual void SetUp();
  virtual void Tear();

public:
  int mysql_tg_id;
  uint64_t dblink_id;
  static const int64_t CONCURRENT_LINKS = 50;
  ObTestbenchMySQLProxy mysql_proxy;
  ObStatisticsCollectorOptions opts {"capacity=500,minimum=10,maximum=80,threads=1,tasks=99999"};
  ObTestbenchStatisticsCollector statistics_collector;
  const char *table_name = "testbench";
  ObRandom random;
  ObArenaAllocator allocator;
};

void TestTransactionTask::SetUp() {
  TestConfig::SetUp();
  const char *user = "root";
  const char *pass = "";
  const char *db = "test";
  ASSERT_EQ(OB_SUCCESS, mysql_proxy.set_server_provider_param(addr));
  ASSERT_EQ(OB_SUCCESS, mysql_proxy.set_connection_pool_param(user, pass, db));
  ASSERT_EQ(OB_SUCCESS, mysql_proxy.init());
  ASSERT_EQ(OB_SUCCESS, mysql_proxy.start_service());
  ASSERT_EQ(OB_SUCCESS, opts.parse_options());
  ASSERT_EQ(OB_SUCCESS, statistics_collector.init(&opts));
  ASSERT_EQ(OB_SUCCESS, statistics_collector.start());
  ObTestbenchServerProvider *server_provider = mysql_proxy.get_server_provider();
  ObSEArray<ObFixedLengthString<OB_MAX_TENANT_NAME_LENGTH + 1>, 16>
      tenant_name_array("OBMySQLConnPool", OB_MALLOC_NORMAL_BLOCK_SIZE);
  ObSEArray<uint64_t, 16> tenant_array("OBMySQLConnPool",
                                       OB_MALLOC_NORMAL_BLOCK_SIZE);
  ASSERT_EQ(OB_SUCCESS, server_provider->get_tenants(tenant_name_array));
  ASSERT_EQ(OB_SUCCESS, server_provider->get_tenant_ids(tenant_array));
  ASSERT_GE(tenant_array.count(), 0);
  ObString tenant_name_str = tenant_name_array[1].str();
  uint64_t tenant_id = tenant_array[1];
  ObSEArray<ObAddr, 16> server_array("OBMySQLConnPool",
                                     OB_MALLOC_NORMAL_BLOCK_SIZE);
  ASSERT_EQ(OB_SUCCESS,
            server_provider->get_tenant_servers(tenant_id, server_array));
  ObAddr server = server_array[0];
  dblink_id = DblinkKey(tenant_name_str, server).hash();
}

void TestTransactionTask::Tear() {
  mysql_proxy.destroy();
  statistics_collector.destroy();
}

TEST_F(TestTransactionTask, distributed_transaction) {
  int64_t transaction_count = 1000;
  for (int i = 0; i < transaction_count; ++i) {
    Parameters partition_ids;
    for (int j = 0; j < 2; ++j) {
      // ensure there are at least 50 partitions numbered from 1 to 50 in the database
      partition_ids.push_back(random.rand(1, 50));
    }
    BasicTaskConfig config{ table_name, 3, &statistics_collector, dblink_id, &mysql_proxy, partition_ids, i * 40};
    ObDistributedTransactionTask distributed_transaction_task(config, 2, 10);
    ASSERT_EQ(OB_SUCCESS, distributed_transaction_task.init());
    TESTBENCH_LOG(TRACE, "distributed_transaction_task init", "index", i);
    ASSERT_EQ(OB_SUCCESS, distributed_transaction_task.execute_transactions());
        TESTBENCH_LOG(TRACE, "distributed_transaction_task execute_transactions", "index", i);
    ASSERT_EQ(OB_SUCCESS, distributed_transaction_task.release_dblinks());
    TESTBENCH_LOG(TRACE, "execute distributed transaction succeed", "index", i);
  }
}

TEST_F(TestTransactionTask, contention_transaction) {
  int64_t transaction_count = 100;
  for (int i = 0; i < transaction_count; ++i) {
    Parameters partition_ids;
    partition_ids.push_back(random.rand(1, 50));
    BasicTaskConfig config { table_name, 1, &statistics_collector, dblink_id, &mysql_proxy, partition_ids, i * 20};
    ObContentionTransactionTask contention_transaction_task(config, 0, 10);
    ASSERT_EQ(OB_SUCCESS, contention_transaction_task.init());
    TESTBENCH_LOG(TRACE, "contention_transaction init", "index", i);
    ASSERT_EQ(OB_SUCCESS, contention_transaction_task.execute_transactions());
    TESTBENCH_LOG(TRACE, "contention_transaction execute_transactions", "index", i);
    ASSERT_EQ(OB_SUCCESS, contention_transaction_task.release_dblinks());
    TESTBENCH_LOG(TRACE, "execute contention_transaction succeed", "index", i);
  }
}

TEST_F(TestTransactionTask, deadlock_transaction) {

}
} // unittest
} // oceanbase

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  const char *file_name = "test_testbench_transaction_task.log";
  OB_LOGGER.set_log_level("INFO");
  remove(file_name);
  OB_LOGGER.set_file_name(file_name, false, false);
  return RUN_ALL_TESTS();
}