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

#ifndef __COMMON_OB_MYSQL_CONNECTION__
#define __COMMON_OB_MYSQL_CONNECTION__

#include "lib/allocator/ob_malloc.h"
#include "lib/container/ob_se_array.h"
#include "lib/mysqlclient/ob_isql_connection.h"
#include "lib/mysqlclient/ob_mysql_statement.h"
#include "lib/net/ob_addr.h"
#include <mysql.h>

namespace oceanbase {
namespace common {
namespace sqlclient {
namespace ContFuncDefID {
enum ContFuncDefIDEnum {
  CONT_FUNC_START = -1,
#define CONT_FUNC_DEF(id, ...) id,
#include "ob_cont_func_define.h"
#undef CONT_FUNC_DEF
  CONT_FUNC_END,
  END = 256
};
} // namespace ContFuncDefID
class ObServerConnectionPool;
class ObMySQLStatement;
class ObMySQLPreparedStatement;
class ObMySQLConnectionPool;
class ObContFunc;

class ObMySQLConnection : public ObISQLConnection //, ObIDbLinkConnection
{
  friend ObContFunc;
#define CONT_FUNC_DEF(id, type) friend class type;
#include "ob_cont_func_define.h"
#undef CONT_FUNC_DEF
  friend class ObServerConnectionPool;
  friend class ObMySQLPreparedStatement;

public:
  enum {
    OB_MYSQL_CONNECTION_ERROR = 1,
    // OB_MYSQL_CONNECTION_WARN = 2,
    OB_MYSQL_CONNECTION_SUCC = 3
  };
  enum Mode { DEBUG_MODE = 0, MYSQL_MODE = 1, OCEANBASE_MODE = 2 };
  enum ConnStatus { INIT = 0, SUCCESS = 1, ERROR = 2, PENDING = 3 };

public:
  ObMySQLConnection();
  ~ObMySQLConnection();
  int connect(const char *user, const char *pass, const char *db,
              oceanbase::common::ObAddr &addr, int64_t timeout,
              bool read_write_no_timeout = false, int64_t sql_req_level = 0,
              bool async = false);
  int connect(const char *user, const char *pass, const char *db,
              const bool use_ssl, bool read_write_no_timeout = false,
              int64_t sql_req_level = 0, bool async = false);
  void close();
  virtual bool is_closed() const;
  // use user provided the statement
  int create_statement(ObMySQLStatement &stmt, const char *sql);
  int create_statement(ObMySQLStatement &stmt, const uint64_t tenant_id,
                       const char *sql);
  int prepare_statement(ObMySQLPreparedStatement &stmt, const char *sql);
  int escape(const char *from, const int64_t from_size, char *to,
             const int64_t to_size, int64_t &out_size);
  int init(ObServerConnectionPool *root);
  void reset();
  const common::ObAddr &get_server(void) const;
  ObServerConnectionPool *get_root();
  virtual ObCommonServerConnectionPool *get_common_server_pool() override;
  MYSQL *get_handler();
  void set_last_error(int err_code);
  int get_last_error(void) const;

  virtual int
  execute_read(const uint64_t tenant_id, const char *sql,
               ObISQLClient::ReadResult &res, bool is_user_sql = false,
               const common::ObAddr *sql_exec_addr = nullptr) override;

  virtual int
  execute_read(const int64_t cluster_id, const uint64_t tenant_id,
               const ObString &sql, ObISQLClient::ReadResult &res,
               bool is_user_sql = false,
               const common::ObAddr *sql_exec_addr = nullptr) override;

  virtual int
  execute_write(const uint64_t tenant_id, const ObString &sql,
                int64_t &affected_rows, bool is_user_sql = false,
                const common::ObAddr *sql_exec_addr = nullptr) override;

  virtual int
  execute_write(const uint64_t tenant_id, const char *sql,
                int64_t &affected_rows, bool is_user_sql = false,
                const common::ObAddr *sql_exec_addr = nullptr) override;

  int execute_read(const char *sql, ObISQLClient::ReadResult &res);
  int execute_read(ObMySQLPreparedStatement &stmt,
                   ObISQLClient::ReadResult &res);
  int execute_read_async(ObMySQLPreparedStatement &stmt);
  int get_async_read_result(ObMySQLPreparedStatement &stmt, ObISQLClient::ReadResult &res);
  int execute_write(const char *sql, int64_t &affected_rows);
  int execute_write(ObMySQLPreparedStatement &stmt, int64_t &affected_rows);
  int execute_write_async(ObMySQLPreparedStatement &stmt);
  int get_async_write_result(int64_t &affected_rows);
  virtual int start_transaction(const uint64_t &tenant_id,
                                bool with_snap_shot = false) override;
  virtual int rollback() override;
  virtual int commit() override;
  int start_transaction_async(bool with_snap_shot = false);
  int rollback_async();
  int commit_async();
  int wait_for_async_status(int timeout = -1);
  inline ConnStatus get_conn_status() { return conn_status_; }

  // session environment
  virtual int get_session_variable(const ObString &name, int64_t &val) override;
  virtual int set_session_variable(const ObString &name, int64_t val) override;
  int set_session_variable(const ObString &name, const ObString &val);

  virtual int ping() override;
  int set_trace_id();
  void set_timeout(const int64_t timeout);
  virtual int set_timeout_variable(const int64_t query_timeout,
                                   const int64_t trx_timeout);
  virtual bool is_busy(void) const;
  virtual void set_busy(const bool busy);
  void set_connection_version(const int64_t version);
  int64_t connection_version() const;
  virtual void set_timestamp(const int64_t timestamp) {
    timestamp_ = timestamp;
  }
  int64_t get_timestamp() const { return timestamp_; }
  void set_mode(const Mode mode) { mode_ = mode; }
  int init_oceanbase_connection();
  void set_read_consistency(const int64_t read_consistency) {
    read_consistency_ = read_consistency;
  }
  void set_read_consistency_strong() {
    set_read_consistency(READ_CONSISTENCY_STRONG);
  }

  VIRTUAL_TO_STRING_KV(K_(db_name), K_(busy));

  // dblink.
  virtual int connect_dblink(const bool use_ssl, int64_t sql_request_level, bool async = false);
  int get_async_status() const; 
  void set_async_status(int async_status);
  int run_contfunc_and_update_status();

private:
  int switch_tenant(const uint64_t tenant_id);
  int reset_read_consistency();
  int wait_for_mysql(int timeout = -1);

private:
  const static int64_t READ_CONSISTENCY_STRONG = 3;
  ObServerConnectionPool *root_; // each connection belongs to ONE pool
  MYSQL mysql_;
  int last_error_code_;
  bool busy_;
  int64_t timestamp_;
  int64_t error_times_;
  int64_t succ_times_;
  int64_t connection_version_;
  bool closed_;
  int64_t timeout_;
  int64_t last_trace_id_;
  Mode mode_;
  const char *db_name_;
  uint64_t tenant_id_;
  int64_t read_consistency_;

  // Async Mode
  MYSQL *mysql_ret_;
  MYSQL_STMT *mysql_stmt_;
  my_bool mysql_bool_err_;
  int mysql_int_err_;
  int async_status_;
  ObContFunc *cont_funcs_[ContFuncDefID::END];
  int cur_cont_func_;
  ObArenaAllocator alloc_;
  ConnStatus conn_status_;

private:
  DISALLOW_COPY_AND_ASSIGN(ObMySQLConnection);
};

inline int ObMySQLConnection::get_async_status() const { return async_status_; }
inline void ObMySQLConnection::set_async_status(int async_status) { async_status_ = async_status; }
inline bool ObMySQLConnection::is_busy() const { return busy_; }
inline void ObMySQLConnection::set_busy(const bool busy) { busy_ = busy; }
inline bool ObMySQLConnection::is_closed() const { return closed_; }
inline MYSQL *ObMySQLConnection::get_handler() { return &mysql_; }
inline void ObMySQLConnection::set_last_error(int err_code) {
  this->last_error_code_ = err_code;
}
inline int ObMySQLConnection::get_last_error(void) const {
  return this->last_error_code_;
}
inline void ObMySQLConnection::set_connection_version(const int64_t version) {
  connection_version_ = version;
}
inline int64_t ObMySQLConnection::connection_version() const {
  return connection_version_;
}

class ObContFunc {
public:
  ObContFunc(ObMySQLConnection *conn) : conn_(conn) {}
  virtual ~ObContFunc() {}
  virtual void run_func() = 0;
  virtual int update_status() = 0;
  int update_bool_status();
  int update_int_status();
  int update_stmt_status();
  int update_mysql_status();

protected:
  ObMySQLConnection *conn_;
  DISALLOW_COPY_AND_ASSIGN(ObContFunc);
};

class ObConnectContFunc : public ObContFunc {
public:
  ObConnectContFunc(ObMySQLConnection *conn) : ObContFunc(conn) {}
  virtual ~ObConnectContFunc() {}
  virtual void run_func() override;
  virtual int update_status() override;
};

class ObQueryContFunc : public ObContFunc {
public:
  ObQueryContFunc(ObMySQLConnection *conn) : ObContFunc(conn) {}
  virtual ~ObQueryContFunc() {}
  virtual void run_func() override;
  virtual int update_status() override;
};

class ObStmtQueryContFunc : public ObContFunc {
public:
  ObStmtQueryContFunc(ObMySQLConnection *conn) : ObContFunc(conn) {}
  virtual ~ObStmtQueryContFunc() {}
  virtual void run_func() override;
  virtual int update_status() override;
};

class ObStmtStoreContFunc : public ObContFunc {
public:
  ObStmtStoreContFunc(ObMySQLConnection *conn) : ObContFunc(conn) {}
  virtual ~ObStmtStoreContFunc() {}
  virtual void run_func() override;
  virtual int update_status() override;
};

class ObStartContFunc : public ObContFunc {
public:
  ObStartContFunc(ObMySQLConnection *conn) : ObContFunc(conn) {}
  virtual ~ObStartContFunc() {}
  virtual void run_func() override;
  virtual int update_status() override;
};

class ObCommitContFunc : public ObContFunc {
public:
  ObCommitContFunc(ObMySQLConnection *conn) : ObContFunc(conn) {}
  virtual ~ObCommitContFunc() {}
  virtual void run_func() override;
  virtual int update_status() override;
};

class ObRollbackContFunc : public ObContFunc {
public:
  ObRollbackContFunc(ObMySQLConnection *conn) : ObContFunc(conn) {}
  virtual ~ObRollbackContFunc() {}
  virtual void run_func() override;
  virtual int update_status() override;
};
} // namespace sqlclient
} // namespace common
} // namespace oceanbase

#endif // __COMMON_OB_MYSQL_CONNECTION__
