#include "share/ob_thread_mgr.h"
#include "test_testbench_config_base.h"
#include "testbench/ob_testbench_server_provider.h"
#include <thread>
#include <chrono>

namespace oceanbase {
namespace unittest {
using namespace common::sqlclient;
class TestAsyncOperation : public TestConfig {
public:
  TestAsyncOperation() : tg_id(-1), sql_conn_pool() {}
  ~TestAsyncOperation() {}

  virtual void SetUp();
  virtual void Tear();

public:
  int tg_id;
  static const int64_t CONCURRENT_LINKS = 50;
  testbench::ObTestbenchSystableHelper systable_helper;
  testbench::ObTestbenchServerProvider server_provider;
  common::sqlclient::ObMySQLConnectionPool sql_conn_pool;
  ObMySQLConnection *conn = nullptr;
  uint64_t dblink_id;
};

void TestAsyncOperation::SetUp() {
  TestConfig::SetUp();
  ASSERT_EQ(OB_SUCCESS, systable_helper.init_conn(mysql_config));
  ASSERT_EQ(OB_SUCCESS, server_provider.init(systable_helper));
  const char *user = "root";
  const char *pass = "";
  const char *db = "test";
  ASSERT_EQ(OB_SUCCESS, sql_conn_pool.set_db_param(user, pass, db));
  conn_pool_config.sqlclient_per_observer_conn_limit_ = CONCURRENT_LINKS;
  sql_conn_pool.update_config(conn_pool_config);
  sql_conn_pool.set_server_provider(&server_provider);
  ASSERT_EQ(OB_SUCCESS, TG_CREATE(lib::TGDefIDs::MysqlProxyPool, tg_id));
  ASSERT_EQ(OB_SUCCESS, TG_START(tg_id));
  ASSERT_EQ(OB_SUCCESS, sql_conn_pool.start(tg_id));
  ASSERT_EQ(OB_SUCCESS, sql_conn_pool.create_all_dblink_pool());
  ObSEArray<ObFixedLengthString<OB_MAX_TENANT_NAME_LENGTH + 1>, 16>
      tenant_name_array("OBMySQLConnPool", OB_MALLOC_NORMAL_BLOCK_SIZE);
  ObSEArray<uint64_t, 16> tenant_array("OBMySQLConnPool",
                                       OB_MALLOC_NORMAL_BLOCK_SIZE);
  ASSERT_EQ(OB_SUCCESS, server_provider.get_tenants(tenant_name_array));
  ASSERT_EQ(OB_SUCCESS, server_provider.get_tenant_ids(tenant_array));
  ASSERT_GE(tenant_array.count(), 0);
  ObString tenant_name_str = tenant_name_array[1].str();
  uint64_t tenant_id = tenant_array[1];
  ObSEArray<ObAddr, 16> server_array("OBMySQLConnPool",
                                     OB_MALLOC_NORMAL_BLOCK_SIZE);
  ASSERT_EQ(OB_SUCCESS,
            server_provider.get_tenant_servers(tenant_id, server_array));
  ObAddr server = server_array[0];
  dblink_id = DblinkKey(tenant_name_str, server).hash();
  ASSERT_EQ(OB_SUCCESS,
            sql_conn_pool.acquire_dblink(dblink_id, dblink_param_ctx(), conn, 0, 0, true));
}

void TestAsyncOperation::Tear() {
  sql_conn_pool.close_all_connection();
  TG_STOP(tg_id);
  TG_WAIT(tg_id);
  server_provider.destroy();
  systable_helper.destroy();
}

TEST_F(TestAsyncOperation, basic_async_api) {
  int ret = OB_SUCCESS;
  common::ObISQLClient::ReadResult res;
  ObMySQLResult *result = nullptr;
  ObMySQLPreparedStatement stmt;
  int64_t affected_rows = 0;
  // test create table;
  const char *sql = "create table test (id integer primary key, col1 integer, col2 integer, col3 integer);";
  EXPECT_EQ(OB_SUCCESS, conn->execute_write(sql, affected_rows));
  EXPECT_EQ(0, affected_rows);
  // test write
  sql = "insert into test values (1, 2, 3, 4);";
  EXPECT_EQ(OB_SUCCESS, conn->execute_write(sql, affected_rows));
  EXPECT_EQ(1, affected_rows);
  // test write async
  sql = "insert into test values (2, 4, 6, ?);";
  int64_t col3 = 8;
  ASSERT_EQ(OB_SUCCESS, conn->prepare_statement(stmt, sql));
  EXPECT_EQ(OB_SUCCESS, stmt.bind_param_int(0, &col3));
  EXPECT_EQ(OB_SUCCESS, conn->execute_write_async(stmt));
  while (OB_FAIL(conn->get_conn_status())) {
    EXPECT_EQ(OB_NEED_RETRY, ret);
  }
  EXPECT_EQ(OB_SUCCESS, conn->get_async_write_result(affected_rows));
  EXPECT_EQ(1, affected_rows);
  // test read
  sql = "select * from test;";
  EXPECT_EQ(OB_SUCCESS, conn->execute_read(sql, res));
  ASSERT_NE(nullptr, result = res.mysql_result());
  EXPECT_EQ(OB_SUCCESS, result->next());
  EXPECT_EQ(OB_SUCCESS, result->next());
  EXPECT_EQ(OB_ITER_END, result->next());
  // test read async
  sql = "select id from test where col1 = ?;";
  int64_t col1 = 4;
  ASSERT_EQ(OB_SUCCESS, conn->prepare_statement(stmt, sql));
  EXPECT_EQ(OB_SUCCESS, stmt.bind_param_int(0, &col1));
  EXPECT_EQ(OB_SUCCESS, conn->execute_read_async(stmt));
  while (OB_FAIL(conn->get_conn_status())) {
    EXPECT_EQ(OB_NEED_RETRY, ret);
  }
  int64_t id = 0;
  EXPECT_EQ(OB_SUCCESS, stmt.bind_result_int(0, &id));
  ASSERT_EQ(OB_SUCCESS, conn->get_async_read_result(stmt, res));
  ASSERT_NE(nullptr, result = res.mysql_result());
  EXPECT_EQ(OB_SUCCESS, result->next());
  EXPECT_EQ(2, id);
  EXPECT_EQ(OB_ITER_END, result->next());
  // test transaction async
  EXPECT_EQ(OB_SUCCESS, conn->start_transaction_async());
  while (OB_FAIL(conn->get_conn_status())) {
    EXPECT_EQ(OB_NEED_RETRY, ret);
  }
  sql = "insert into test values (3, 6, 9, 12);";
  EXPECT_EQ(OB_SUCCESS, conn->prepare_statement(stmt, sql));
  EXPECT_EQ(OB_SUCCESS, conn->execute_write_async(stmt));
  while (OB_FAIL(conn->get_conn_status())) {
    EXPECT_EQ(OB_NEED_RETRY, ret);
  }
  EXPECT_EQ(OB_SUCCESS, conn->get_async_write_result(affected_rows));
  EXPECT_EQ(1, affected_rows);
  EXPECT_EQ(OB_SUCCESS, conn->rollback_async());
  while (OB_FAIL(conn->get_conn_status())) {
    EXPECT_EQ(OB_NEED_RETRY, ret);
  }
  EXPECT_EQ(OB_SUCCESS, conn->get_async_write_result(affected_rows));
  EXPECT_EQ(1, affected_rows);
  sql = "select * from test;";
  EXPECT_EQ(OB_SUCCESS, conn->execute_read(sql, res));
  ASSERT_NE(nullptr, result = res.mysql_result());
  EXPECT_EQ(OB_SUCCESS, result->next());
  EXPECT_EQ(OB_SUCCESS, result->next());
  EXPECT_EQ(OB_ITER_END, result->next());
  // test error async
  sql = "insert into test values (3, 6, 9, 12);";
  EXPECT_EQ(OB_SUCCESS, conn->prepare_statement(stmt, sql));
  EXPECT_EQ(OB_SUCCESS, conn->execute_write_async(stmt));
  EXPECT_EQ(OB_ERR_UNEXPECTED, conn->get_async_write_result(affected_rows));
  sql = "select * from test;";
  EXPECT_EQ(OB_ERR_UNEXPECTED, conn->execute_read(sql, res));
  while (OB_FAIL(conn->get_conn_status())) {
    EXPECT_EQ(OB_NEED_RETRY, ret);
  }
  EXPECT_EQ(OB_SUCCESS, conn->get_async_write_result(affected_rows));
  // clean up
  sql = "drop table test;";
  EXPECT_EQ(OB_SUCCESS, conn->execute_write(sql, affected_rows));
  EXPECT_EQ(0, affected_rows);
}

TEST_F(TestAsyncOperation, baseline_write_performance) {
  int ret = OB_SUCCESS;
  int64_t insert_rows = 100;
  int64_t affected_rows = 0;
  const char *sql = "create table test (id integer primary key, col1 integer, col2 integer, col3 integer);";
  EXPECT_EQ(OB_SUCCESS, conn->execute_write(sql, affected_rows));
  EXPECT_EQ(0, affected_rows);
  auto process_task = [&](int64_t thread_id, double_t &latency) {
    ObMySQLConnection *cur_conn;
    ObMySQLPreparedStatement stmt;
    int64_t affected_rows_local = 0;
    ASSERT_EQ(OB_SUCCESS,
            sql_conn_pool.acquire_dblink(dblink_id, dblink_param_ctx(), cur_conn, 0, 0, true));
    const char *sql = "insert into test values (?, 1, 2, 3);";
    ASSERT_EQ(OB_SUCCESS, cur_conn->prepare_statement(stmt, sql));
    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    for (int64_t i = 0; i < insert_rows; ++i) {
      int64_t id = i + thread_id * insert_rows;
      start_time = std::chrono::high_resolution_clock::now();
      EXPECT_EQ(OB_SUCCESS, stmt.bind_param_int(0, &id));
      EXPECT_EQ(OB_SUCCESS, cur_conn->execute_write(stmt, affected_rows_local));
      EXPECT_EQ(1, affected_rows_local);
      end_time = std::chrono::high_resolution_clock::now();
      duration += std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }
    latency = static_cast<double_t>(duration.count());
  };
  double_t latency_thread1 = 0;
  double_t latency_thread2 = 0;
  std::thread thread1 = std::thread(process_task, 0, std::ref(latency_thread1));
  std::thread thread2 = std::thread(process_task, 1, std::ref(latency_thread2));
  thread1.join();
  thread2.join();
  ::testing::Test::RecordProperty("average insert latency", latency_thread1 / insert_rows + latency_thread2 / insert_rows);
  // clean up
  sql = "drop table test;";
  EXPECT_EQ(OB_SUCCESS, conn->execute_write(sql, affected_rows));
  EXPECT_EQ(0, affected_rows);
}

TEST_F(TestAsyncOperation, async_write_performance) {
  int ret = OB_SUCCESS;
  int64_t affected_rows = 0;
  const char *sql = "create table test (id integer primary key, col1 integer, col2 integer, col3 integer);";
  EXPECT_EQ(OB_SUCCESS, conn->execute_write(sql, affected_rows));
  EXPECT_EQ(0, affected_rows);
  int64_t insert_rows = 100;
  auto process_task = [&](int64_t thread_id, double_t &latency) {
    ObMySQLPreparedStatement stmt;
    ObMySQLConnection *cur_conn;
    int64_t affected_rows_local = 0;
    ASSERT_EQ(OB_SUCCESS,
            sql_conn_pool.acquire_dblink(dblink_id, dblink_param_ctx(), cur_conn, 0, 0, true));
    const char *sql = "insert into test values (?, 1, 2, 3);";
    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    ASSERT_EQ(OB_SUCCESS, cur_conn->prepare_statement(stmt, sql));
    for (int64_t i = 0; i < insert_rows; ++i) {
      int64_t id = i + thread_id * insert_rows;
      start_time = std::chrono::high_resolution_clock::now();
      EXPECT_EQ(OB_SUCCESS, stmt.bind_param_int(0, &id));
      EXPECT_EQ(OB_SUCCESS, cur_conn->execute_write_async(stmt));
      while (OB_FAIL(cur_conn->get_conn_status())) {
        EXPECT_EQ(OB_NEED_RETRY, ret);
      }
      EXPECT_EQ(OB_SUCCESS, cur_conn->get_async_write_result(affected_rows_local));
      EXPECT_EQ(1, affected_rows_local);
      end_time = std::chrono::high_resolution_clock::now();
      duration += std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }
    latency = static_cast<double_t>(duration.count());
  };
  double_t latency_thread1 = 0;
  double_t latency_thread2 = 0;
  std::thread thread1 = std::thread(process_task, 0, std::ref(latency_thread1));
  std::thread thread2 = std::thread(process_task, 1, std::ref(latency_thread2));
  thread1.join();
  thread2.join();
  ::testing::Test::RecordProperty("average insert latency", latency_thread1 / insert_rows + latency_thread2 / insert_rows);
  // clean up
  sql = "drop table test;";
  EXPECT_EQ(OB_SUCCESS, conn->execute_write(sql, affected_rows));
  EXPECT_EQ(0, affected_rows);
}

} // namespace unittest
} // namespace oceanbase

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  OB_LOGGER.set_log_level("INFO");
  return RUN_ALL_TESTS();
}