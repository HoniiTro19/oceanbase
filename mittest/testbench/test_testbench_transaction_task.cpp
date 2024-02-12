#include "share/ob_thread_mgr.h"
#include "test_testbench_config_base.h"
#include "testbench/ob_testbench_statistics_collector.h"
#include "testbench/ob_testbench_server_provider.h"
#include "testbench/ob_testbench_transaction_task.h"
#include "testbench/ob_testbench_location_cache.h"
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
  ObString tenant_name_str;
  ObSEArray<ObAddr, 16> server_array;
  static const int64_t CONCURRENT_LINKS = 50;
  ObTestbenchMySQLProxy mysql_proxy;
  ObStatisticsCollectorOptions opts {"capacity=500,minimum=10,maximum=80,threads=1,tasks=99999"};
  ObTestbenchStatisticsCollector statistics_collector;
  ObTestbenchSystableHelper systable_helper;
  ObTestbenchLocationCache location_cache;
  const char *database_name = "test";
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
  tenant_name_str = tenant_name_array[1].str();
  uint64_t tenant_id = tenant_array[1];
  ASSERT_EQ(OB_SUCCESS, server_provider->get_tenant_servers(tenant_id, server_array));
  ASSERT_EQ(OB_SUCCESS, systable_helper.init_conn(mysql_config));
  ASSERT_EQ(OB_SUCCESS, location_cache.init(database_name, table_name, &systable_helper));
  ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
}

void TestTransactionTask::Tear() {
  mysql_proxy.destroy();
  statistics_collector.destroy();
  systable_helper.destroy();
  location_cache.destroy();
}

TEST_F(TestTransactionTask, distributed_transaction) {
  int64_t transaction_count = 1000;
  int64_t connections = 1;
  int64_t svrs = 2;
  for (int64_t i = 0; i < transaction_count; ++i) {
    Parameters parameters;
    Dblinks dblinks;
    ASSERT_EQ(OB_SUCCESS, location_cache.gen_distributed_txn_params(svrs, parameters, dblinks));
    BasicTaskConfig config{ table_name, connections, &statistics_collector, &mysql_proxy, parameters, dblinks, i * 10 };
    ObDistributedTransactionTask distributed_transaction_task(config, svrs, 10);
    ASSERT_EQ(OB_SUCCESS, distributed_transaction_task.init());
    ASSERT_EQ(OB_SUCCESS, distributed_transaction_task.execute_transactions());
    ASSERT_EQ(OB_SUCCESS, distributed_transaction_task.release_dblinks());
    Commits *commits = distributed_transaction_task.get_commits();
    Latencys *latencys = distributed_transaction_task.get_latencys();
    Latencys *cumu_latencys = distributed_transaction_task.get_cumulative_latencys();
    for (int64_t j = 0; j < connections; ++j) {
      ASSERT_EQ(true, commits->at(j));
      ASSERT_GT(latencys->at(j), 0);
      ASSERT_GT(cumu_latencys->at(j), 0);
    }
  }
}

TEST_F(TestTransactionTask, contention_transaction) {
  int64_t transaction_count = 1000;
  int64_t connections = 10;
  for (int64_t i = 0; i < transaction_count; ++i) {
    Parameters parameters;
    Dblinks dblinks;
    int64_t aborts = random.rand(1, connections);
    ASSERT_EQ(OB_SUCCESS, location_cache.gen_contention_txn_params(connections, parameters, dblinks));
    BasicTaskConfig config { table_name, connections, &statistics_collector, &mysql_proxy, parameters, dblinks, i * 100 };
    ObContentionTransactionTask contention_transaction_task(config, aborts, 10);
    ASSERT_EQ(OB_SUCCESS, contention_transaction_task.init());
    ASSERT_EQ(OB_SUCCESS, contention_transaction_task.execute_transactions());
    ASSERT_EQ(OB_SUCCESS, contention_transaction_task.release_dblinks());
    Commits *commits = contention_transaction_task.get_commits();
    Latencys *latencys = contention_transaction_task.get_latencys();
    Latencys *cumu_latencys = contention_transaction_task.get_cumulative_latencys();
    int64_t finished = contention_transaction_task.get_finished();
    ASSERT_EQ(finished, connections);
    int64_t act_aborts = 0;
    for (int64_t j = 0; j < connections; ++j) {
      act_aborts = commits->at(j) ? act_aborts : act_aborts + 1;
      ASSERT_GT(latencys->at(j), 0);
      ASSERT_GT(cumu_latencys->at(j), 0);
    }
    ASSERT_EQ(act_aborts, aborts);
  }
}

TEST_F(TestTransactionTask, deadlock_transaction) {
  int64_t transaction_count = 50;
  int64_t connections = 9;
  int64_t chains = 3;
  int64_t cycle = connections / chains;
  for (int64_t i = 0; i < transaction_count; ++i) {
    Parameters parameters;
    Dblinks dblinks;
    ASSERT_EQ(OB_SUCCESS, location_cache.gen_deadlock_txn_params(connections, parameters, dblinks));
    BasicTaskConfig config { table_name, connections, &statistics_collector, &mysql_proxy, parameters, dblinks, i * 10 };
    ObDeadlockTransactionTask deadlock_transaction_task(config, chains);
    ASSERT_EQ(OB_SUCCESS, deadlock_transaction_task.init());
    ASSERT_EQ(OB_SUCCESS, deadlock_transaction_task.execute_transactions());
    ASSERT_EQ(OB_SUCCESS, deadlock_transaction_task.release_dblinks());
    Commits *commits = deadlock_transaction_task.get_commits();
    Latencys *latencys = deadlock_transaction_task.get_latencys();
    Latencys *cumu_latencys = deadlock_transaction_task.get_cumulative_latencys();
    for (int64_t j = 0; j < connections; ++j) {
      if (j % cycle == cycle - 1) {
        ASSERT_GT(latencys->at(j), 0);        
      }
      ASSERT_GT(cumu_latencys->at(j), 0);
    }
  }
}

TEST_F(TestTransactionTask, concurrent_transaction) {
  int64_t transaction_count = 1000;
  int64_t connections = 10;
  for (int64_t i = 0; i < transaction_count; ++i) {
    Parameters parameters;
    Dblinks dblinks;
    ASSERT_EQ(OB_SUCCESS, location_cache.gen_concurrent_txn_params(connections, parameters, dblinks));
    BasicTaskConfig config { table_name, connections, &statistics_collector, &mysql_proxy, parameters, dblinks, i * 100 };
    Readonlys readonlys;
    readonlys.reset();
    for (int64_t j = 0; j < connections; ++j) {
      bool readonly = random.rand(1, 10) <= 5;
      readonlys.push_back(readonly);
    }
    ObConcurrentTransactionTask concurrent_transaction_task(config, 10, readonlys);
    ASSERT_EQ(OB_SUCCESS, concurrent_transaction_task.init());
    ASSERT_EQ(OB_SUCCESS, concurrent_transaction_task.execute_transactions());
    ASSERT_EQ(OB_SUCCESS, concurrent_transaction_task.release_dblinks());
    Commits *commits = concurrent_transaction_task.get_commits();
    Latencys *latencys = concurrent_transaction_task.get_latencys();
    Latencys *cumu_latencys = concurrent_transaction_task.get_cumulative_latencys();
    int64_t finished = concurrent_transaction_task.get_finished();
    ASSERT_EQ(connections, finished);
    for (int64_t j = 0; j < connections; ++j) {
      ASSERT_EQ(true, commits->at(j));
      ASSERT_GT(cumu_latencys->at(j), 0);
    }
  }
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