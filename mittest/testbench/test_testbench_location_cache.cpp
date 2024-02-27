#include "test_testbench_config_base.h"
#include "testbench/ob_testbench_location_cache.h"
#include "testbench/ob_testbench_systable_helper.h"
#include <thread>
#include <mutex> 

static std::chrono::milliseconds duration_threshold;
static int64_t thread_num;
static int64_t interval;

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

// TEST_F(TestTestbenchLocationCache, refresh_locations) {
//   ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
//   ASSERT_EQ(ObString("tb"), location_cache.get_user_tenant().str());
//   int64_t key_count = 0;
//   auto add_count_cb = [&](hash::HashMapPair<ObAddr, testbench::PartitionList*> &entry) -> int {
//     int ret = OB_SUCCESS;
//     ++key_count;
//     return ret;
//   };
//   ASSERT_EQ(OB_SUCCESS, location_cache.get_partition_info()->foreach_refactored(add_count_cb));
//   ASSERT_EQ(key_count, location_cache.get_svr_addrs().count());
// }

// TEST_F(TestTestbenchLocationCache, generate_parameters) {
//   ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
//   int64_t times = 1000;
//   testbench::Parameters parameters;
//   testbench::Dblinks dblinks;
//   ASSERT_NE(OB_SUCCESS, location_cache.gen_distributed_txn_params(1000, parameters, dblinks));
//   for (int64_t i = 0; i < times; ++i) {
//     // distributed transaction
//     ASSERT_EQ(OB_SUCCESS, location_cache.gen_distributed_txn_params(1, parameters, dblinks));
//     ASSERT_EQ(1, parameters.count());
//     ASSERT_EQ(1, dblinks.count());
//     ASSERT_EQ(OB_SUCCESS, location_cache.gen_distributed_txn_params(2, parameters, dblinks));
//     ASSERT_EQ(2, parameters.count());
//     ASSERT_EQ(2, dblinks.count());
//     // contention transaction
//     ASSERT_EQ(OB_SUCCESS, location_cache.gen_contention_txn_params(10, parameters, dblinks));
//     ASSERT_EQ(10, parameters.count());
//     ASSERT_EQ(10, dblinks.count());
//     // deadlock transaction
//     ASSERT_EQ(OB_SUCCESS, location_cache.gen_deadlock_txn_params(12, parameters, dblinks));
//     ASSERT_EQ(12, parameters.count());
//     ASSERT_EQ(12, dblinks.count());
//     // concurrent transaction
//     ASSERT_EQ(OB_SUCCESS, location_cache.gen_concurrent_txn_params(20, parameters, dblinks));
//     ASSERT_EQ(20, parameters.count());
//     ASSERT_EQ(20, dblinks.count());
//   }
// }

// TEST_F(TestTestbenchLocationCache, concurrent_request) {
//   ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
//   auto write_request = [&]() {
//     int times = 0;
//     while (times < 5) {
//       ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
//       sleep(1);
//       ++times;
//     }
//   };
//   auto read_request = [&]() {
//     testbench::Parameters parameters;
//     testbench::Dblinks dblinks;
//     int times = 0;
//     int64_t parameter = -1;
//     while (times < 100000) {
//       ASSERT_EQ(OB_SUCCESS, location_cache.gen_concurrent_txn_params(1, parameters, dblinks));
//       ++times;
//     }
//   };
//   std::thread thread_write = std::thread(write_request);
//   std::thread thread_read = std::thread(read_request);
//   thread_write.join();
//   thread_read.join();
// }

// TEST_F(TestTestbenchLocationCache, refresh_performance) {
//   int64_t rounds = 1000;
//   for (int64_t i = 0; i < rounds; ++i) {
//     ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
//   }
// }

TEST_F(TestTestbenchLocationCache, concurrent_performance) {
  auto start_time = std::chrono::high_resolution_clock::now();
  int64_t counter = 0;
  std::mutex mtx;
  ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
  auto read_request = [&]() {
    thread_local int64_t local_counter = 0;
    testbench::Parameters parameters;
    testbench::Dblinks dblinks;
    while (true) {
      auto end_time = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
      if (duration >= duration_threshold) {
        break;
      }
      ASSERT_EQ(OB_SUCCESS, location_cache.gen_concurrent_txn_params(1, parameters, dblinks));
      ++local_counter;
    }
    mtx.lock();
    counter += local_counter;
    mtx.unlock();
  };
  auto write_request = [&]() {
    while (true) {
      auto end_time = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
      if (duration >= duration_threshold) {
        break;
      }
      ASSERT_EQ(OB_SUCCESS, location_cache.refresh_locations());
      sleep(interval);
    }
  };
  std::vector<std::thread> read_threads;
  std::thread write_thread = std::thread(write_request);
  for (int64_t i = 0; i < thread_num; ++i) {
    read_threads.push_back(std::thread(read_request));
  }
  write_thread.join();
  for (int64_t i = 0; i < thread_num; ++i) {
    read_threads[i].join();
  }
  ::testing::Test::RecordProperty("throughput", counter);
}
} // namespace unittest
} // namespace oceanbase

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  const char *file_name = "test_testbench_location_cache.log";
  OB_LOGGER.set_log_level("INFO");
  remove(file_name);
  OB_LOGGER.set_file_name(file_name, false, false);
  duration_threshold = std::chrono::milliseconds(60 * 1000);
  thread_num = 10;
  interval = 1;
  if (argc > 1) {
    for (int i = 1; i < argc; ++i) {
      if (strcmp(argv[i], "-d") == 0) {
        if (i < argc - 1) {
          duration_threshold = std::chrono::milliseconds(std::stoi(argv[i + 1]) * 1000);
        } else {
          std::cerr << "cannot find value for argument: " << argv[i] << std::endl;
        }
        ++i;
      } else if (strcmp(argv[i], "-t") == 0) {
        if (i < argc - 1) {
          thread_num = std::stoi(argv[i + 1]);
        } else {
          std::cerr << "cannot find value for argument: " << argv[i] << std::endl;
        }
        ++i;
      } else if (strcmp(argv[i], "-i") == 0) {
        if (i < argc - 1) {
          interval = std::stoi(argv[i + 1]);
        } else {
          std::cerr << "cannot find value for argument: " << argv[i] << std::endl;
        }
        ++i;
      } else {
        std::cerr << "get unexpect argument: " << argv[i] << std::endl;
      }
    }
  }
  return RUN_ALL_TESTS();
}