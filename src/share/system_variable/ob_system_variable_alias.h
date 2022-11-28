/**
 * Copyright (c) 2021 OceanBase
 * OceanBase CE is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan PubL v2.
 * You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#ifndef OCEANBASE_SHARE_SYSTEM_VARIABLE_OB_SYSTEM_VARIABLE_ALIAS_
#define OCEANBASE_SHARE_SYSTEM_VARIABLE_OB_SYSTEM_VARIABLE_ALIAS_
namespace oceanbase
{
namespace share
{
  static const char* const OB_SV_AUTO_INCREMENT_INCREMENT = "auto_increment_increment";
  static const char* const OB_SV_AUTO_INCREMENT_OFFSET = "auto_increment_offset";
  static const char* const OB_SV_AUTOCOMMIT = "autocommit";
  static const char* const OB_SV_CHARACTER_SET_CLIENT = "character_set_client";
  static const char* const OB_SV_CHARACTER_SET_CONNECTION = "character_set_connection";
  static const char* const OB_SV_CHARACTER_SET_DATABASE = "character_set_database";
  static const char* const OB_SV_CHARACTER_SET_RESULTS = "character_set_results";
  static const char* const OB_SV_CHARACTER_SET_SERVER = "character_set_server";
  static const char* const OB_SV_CHARACTER_SET_SYSTEM = "character_set_system";
  static const char* const OB_SV_COLLATION_CONNECTION = "collation_connection";
  static const char* const OB_SV_COLLATION_DATABASE = "collation_database";
  static const char* const OB_SV_COLLATION_SERVER = "collation_server";
  static const char* const OB_SV_INTERACTIVE_TIMEOUT = "interactive_timeout";
  static const char* const OB_SV_LAST_INSERT_ID = "last_insert_id";
  static const char* const OB_SV_MAX_ALLOWED_PACKET = "max_allowed_packet";
  static const char* const OB_SV_SQL_MODE = "sql_mode";
  static const char* const OB_SV_TIME_ZONE = "time_zone";
  static const char* const OB_SV_TX_ISOLATION = "tx_isolation";
  static const char* const OB_SV_VERSION_COMMENT = "version_comment";
  static const char* const OB_SV_WAIT_TIMEOUT = "wait_timeout";
  static const char* const OB_SV_BINLOG_ROW_IMAGE = "binlog_row_image";
  static const char* const OB_SV_CHARACTER_SET_FILESYSTEM = "character_set_filesystem";
  static const char* const OB_SV_CONNECT_TIMEOUT = "connect_timeout";
  static const char* const OB_SV_DATADIR = "datadir";
  static const char* const OB_SV_DEBUG_SYNC = "debug_sync";
  static const char* const OB_SV_DIV_PRECISION_INCREMENT = "div_precision_increment";
  static const char* const OB_SV_EXPLICIT_DEFAULTS_FOR_TIMESTAMP = "explicit_defaults_for_timestamp";
  static const char* const OB_SV_GROUP_CONCAT_MAX_LEN = "group_concat_max_len";
  static const char* const OB_SV_IDENTITY = "identity";
  static const char* const OB_SV_LOWER_CASE_TABLE_NAMES = "lower_case_table_names";
  static const char* const OB_SV_NET_READ_TIMEOUT = "net_read_timeout";
  static const char* const OB_SV_NET_WRITE_TIMEOUT = "net_write_timeout";
  static const char* const OB_SV_READ_ONLY = "read_only";
  static const char* const OB_SV_SQL_AUTO_IS_NULL = "sql_auto_is_null";
  static const char* const OB_SV_SQL_SELECT_LIMIT = "sql_select_limit";
  static const char* const OB_SV_TIMESTAMP = "timestamp";
  static const char* const OB_SV_TX_READ_ONLY = "tx_read_only";
  static const char* const OB_SV_VERSION = "version";
  static const char* const OB_SV_SQL_WARNINGS = "sql_warnings";
  static const char* const OB_SV_MAX_USER_CONNECTIONS = "max_user_connections";
  static const char* const OB_SV_INIT_CONNECT = "init_connect";
  static const char* const OB_SV_LICENSE = "license";
  static const char* const OB_SV_NET_BUFFER_LENGTH = "net_buffer_length";
  static const char* const OB_SV_SYSTEM_TIME_ZONE = "system_time_zone";
  static const char* const OB_SV_QUERY_CACHE_SIZE = "query_cache_size";
  static const char* const OB_SV_QUERY_CACHE_TYPE = "query_cache_type";
  static const char* const OB_SV_SQL_QUOTE_SHOW_CREATE = "sql_quote_show_create";
  static const char* const OB_SV_MAX_SP_RECURSION_DEPTH = "max_sp_recursion_depth";
  static const char* const OB_SV_SQL_SAFE_UPDATES = "sql_safe_updates";
  static const char* const OB_SV_CONCURRENT_INSERT = "concurrent_insert";
  static const char* const OB_SV_DEFAULT_AUTHENTICATION_PLUGIN = "default_authentication_plugin";
  static const char* const OB_SV_DISABLED_STORAGE_ENGINES = "disabled_storage_engines";
  static const char* const OB_SV_ERROR_COUNT = "error_count";
  static const char* const OB_SV_GENERAL_LOG = "general_log";
  static const char* const OB_SV_HAVE_OPENSSL = "have_openssl";
  static const char* const OB_SV_HAVE_PROFILING = "have_profiling";
  static const char* const OB_SV_HAVE_SSL = "have_ssl";
  static const char* const OB_SV_HOSTNAME = "hostname";
  static const char* const OB_SV_LC_MESSAGES = "lc_messages";
  static const char* const OB_SV_LOCAL_INFILE = "local_infile";
  static const char* const OB_SV_LOCK_WAIT_TIMEOUT = "lock_wait_timeout";
  static const char* const OB_SV_LONG_QUERY_TIME = "long_query_time";
  static const char* const OB_SV_MAX_CONNECTIONS = "max_connections";
  static const char* const OB_SV_MAX_EXECUTION_TIME = "max_execution_time";
  static const char* const OB_SV_PROTOCOL_VERSION = "protocol_version";
  static const char* const OB_SV_SERVER_ID = "server_id";
  static const char* const OB_SV_SSL_CA = "ssl_ca";
  static const char* const OB_SV_SSL_CAPATH = "ssl_capath";
  static const char* const OB_SV_SSL_CERT = "ssl_cert";
  static const char* const OB_SV_SSL_CIPHER = "ssl_cipher";
  static const char* const OB_SV_SSL_CRL = "ssl_crl";
  static const char* const OB_SV_SSL_CRLPATH = "ssl_crlpath";
  static const char* const OB_SV_SSL_KEY = "ssl_key";
  static const char* const OB_SV_TIME_FORMAT = "time_format";
  static const char* const OB_SV_TLS_VERSION = "tls_version";
  static const char* const OB_SV_TMP_TABLE_SIZE = "tmp_table_size";
  static const char* const OB_SV_TMPDIR = "tmpdir";
  static const char* const OB_SV_UNIQUE_CHECKS = "unique_checks";
  static const char* const OB_SV_VERSION_COMPILE_MACHINE = "version_compile_machine";
  static const char* const OB_SV_VERSION_COMPILE_OS = "version_compile_os";
  static const char* const OB_SV_WARNING_COUNT = "warning_count";
  static const char* const OB_SV_SESSION_TRACK_SCHEMA = "session_track_schema";
  static const char* const OB_SV_SESSION_TRACK_SYSTEM_VARIABLES = "session_track_system_variables";
  static const char* const OB_SV_SESSION_TRACK_STATE_CHANGE = "session_track_state_change";
  static const char* const OB_SV_HAVE_QUERY_CACHE = "have_query_cache";
  static const char* const OB_SV_QUERY_CACHE_LIMIT = "query_cache_limit";
  static const char* const OB_SV_QUERY_CACHE_MIN_RES_UNIT = "query_cache_min_res_unit";
  static const char* const OB_SV_QUERY_CACHE_WLOCK_INVALIDATE = "query_cache_wlock_invalidate";
  static const char* const OB_SV_BINLOG_FORMAT = "binlog_format";
  static const char* const OB_SV_BINLOG_CHECKSUM = "binlog_checksum";
  static const char* const OB_SV_BINLOG_ROWS_QUERY_LOG_EVENTS = "binlog_rows_query_log_events";
  static const char* const OB_SV_LOG_BIN = "log_bin";
  static const char* const OB_SV_SERVER_UUID = "server_uuid";
  static const char* const OB_SV_DEFAULT_STORAGE_ENGINE = "default_storage_engine";
  static const char* const OB_SV_INTERM_RESULT_MEM_LIMIT = "ob_interm_result_mem_limit";
  static const char* const OB_SV_PROXY_PARTITION_HIT = "ob_proxy_partition_hit";
  static const char* const OB_SV_LOG_LEVEL = "ob_log_level";
  static const char* const OB_SV_QUERY_TIMEOUT = "ob_query_timeout";
  static const char* const OB_SV_READ_CONSISTENCY = "ob_read_consistency";
  static const char* const OB_SV_ENABLE_TRANSFORMATION = "ob_enable_transformation";
  static const char* const OB_SV_TRX_TIMEOUT = "ob_trx_timeout";
  static const char* const OB_SV_ENABLE_PLAN_CACHE = "ob_enable_plan_cache";
  static const char* const OB_SV_ENABLE_INDEX_DIRECT_SELECT = "ob_enable_index_direct_select";
  static const char* const OB_SV_PROXY_SET_TRX_EXECUTED = "ob_proxy_set_trx_executed";
  static const char* const OB_SV_ENABLE_AGGREGATION_PUSHDOWN = "ob_enable_aggregation_pushdown";
  static const char* const OB_SV_LAST_SCHEMA_VERSION = "ob_last_schema_version";
  static const char* const OB_SV_GLOBAL_DEBUG_SYNC = "ob_global_debug_sync";
  static const char* const OB_SV_PROXY_GLOBAL_VARIABLES_VERSION = "ob_proxy_global_variables_version";
  static const char* const OB_SV_ENABLE_SHOW_TRACE = "ob_enable_show_trace";
  static const char* const OB_SV_BNL_JOIN_CACHE_SIZE = "ob_bnl_join_cache_size";
  static const char* const OB_SV_PROXY_USER_PRIVILEGE = "ob_proxy_user_privilege";
  static const char* const OB_SV_ORG_CLUSTER_ID = "ob_org_cluster_id";
  static const char* const OB_SV_PLAN_CACHE_PERCENTAGE = "ob_plan_cache_percentage";
  static const char* const OB_SV_PLAN_CACHE_EVICT_HIGH_PERCENTAGE = "ob_plan_cache_evict_high_percentage";
  static const char* const OB_SV_PLAN_CACHE_EVICT_LOW_PERCENTAGE = "ob_plan_cache_evict_low_percentage";
  static const char* const OB_SV_RECYCLEBIN = "recyclebin";
  static const char* const OB_SV_CAPABILITY_FLAG = "ob_capability_flag";
  static const char* const OB_SV_IS_RESULT_ACCURATE = "is_result_accurate";
  static const char* const OB_SV_ERROR_ON_OVERLAP_TIME = "error_on_overlap_time";
  static const char* const OB_SV_COMPATIBILITY_MODE = "ob_compatibility_mode";
  static const char* const OB_SV_SQL_WORK_AREA_PERCENTAGE = "ob_sql_work_area_percentage";
  static const char* const OB_SV_SAFE_WEAK_READ_SNAPSHOT = "ob_safe_weak_read_snapshot";
  static const char* const OB_SV_ROUTE_POLICY = "ob_route_policy";
  static const char* const OB_SV_ENABLE_TRANSMISSION_CHECKSUM = "ob_enable_transmission_checksum";
  static const char* const OB_SV_FOREIGN_KEY_CHECKS = "foreign_key_checks";
  static const char* const OB_SV_STATEMENT_TRACE_ID = "ob_statement_trace_id";
  static const char* const OB_SV_ENABLE_TRUNCATE_FLASHBACK = "ob_enable_truncate_flashback";
  static const char* const OB_SV_TCP_INVITED_NODES = "ob_tcp_invited_nodes";
  static const char* const OB_SV_SQL_THROTTLE_CURRENT_PRIORITY = "sql_throttle_current_priority";
  static const char* const OB_SV_SQL_THROTTLE_PRIORITY = "sql_throttle_priority";
  static const char* const OB_SV_SQL_THROTTLE_RT = "sql_throttle_rt";
  static const char* const OB_SV_SQL_THROTTLE_CPU = "sql_throttle_cpu";
  static const char* const OB_SV_SQL_THROTTLE_IO = "sql_throttle_io";
  static const char* const OB_SV_SQL_THROTTLE_NETWORK = "sql_throttle_network";
  static const char* const OB_SV_SQL_THROTTLE_LOGICAL_READS = "sql_throttle_logical_reads";
  static const char* const OB_SV_AUTO_INCREMENT_CACHE_SIZE = "auto_increment_cache_size";
  static const char* const OB_SV_ENABLE_JIT = "ob_enable_jit";
  static const char* const OB_SV_TEMP_TABLESPACE_SIZE_PERCENTAGE = "ob_temp_tablespace_size_percentage";
  static const char* const OB_SV_PLUGIN_DIR = "plugin_dir";
  static const char* const OB_SV_SQL_AUDIT_PERCENTAGE = "ob_sql_audit_percentage";
  static const char* const OB_SV_ENABLE_SQL_AUDIT = "ob_enable_sql_audit";
  static const char* const OB_SV_OPTIMIZER_USE_SQL_PLAN_BASELINES = "optimizer_use_sql_plan_baselines";
  static const char* const OB_SV_OPTIMIZER_CAPTURE_SQL_PLAN_BASELINES = "optimizer_capture_sql_plan_baselines";
  static const char* const OB_SV_PARALLEL_SERVERS_TARGET = "parallel_servers_target";
  static const char* const OB_SV_EARLY_LOCK_RELEASE = "ob_early_lock_release";
  static const char* const OB_SV_TRX_IDLE_TIMEOUT = "ob_trx_idle_timeout";
  static const char* const OB_SV_BLOCK_ENCRYPTION_MODE = "block_encryption_mode";
  static const char* const OB_SV_NLS_DATE_FORMAT = "nls_date_format";
  static const char* const OB_SV_NLS_TIMESTAMP_FORMAT = "nls_timestamp_format";
  static const char* const OB_SV_NLS_TIMESTAMP_TZ_FORMAT = "nls_timestamp_tz_format";
  static const char* const OB_SV_RESERVED_META_MEMORY_PERCENTAGE = "ob_reserved_meta_memory_percentage";
  static const char* const OB_SV_CHECK_SYS_VARIABLE = "ob_check_sys_variable";
  static const char* const OB_SV_NLS_LANGUAGE = "nls_language";
  static const char* const OB_SV_NLS_TERRITORY = "nls_territory";
  static const char* const OB_SV_NLS_SORT = "nls_sort";
  static const char* const OB_SV_NLS_COMP = "nls_comp";
  static const char* const OB_SV_NLS_CHARACTERSET = "nls_characterset";
  static const char* const OB_SV_NLS_NCHAR_CHARACTERSET = "nls_nchar_characterset";
  static const char* const OB_SV_NLS_DATE_LANGUAGE = "nls_date_language";
  static const char* const OB_SV_NLS_LENGTH_SEMANTICS = "nls_length_semantics";
  static const char* const OB_SV_NLS_NCHAR_CONV_EXCP = "nls_nchar_conv_excp";
  static const char* const OB_SV_NLS_CALENDAR = "nls_calendar";
  static const char* const OB_SV_NLS_NUMERIC_CHARACTERS = "nls_numeric_characters";
  static const char* const OB_SV__NLJ_BATCHING_ENABLED = "_nlj_batching_enabled";
  static const char* const OB_SV_TRACEFILE_IDENTIFIER = "tracefile_identifier";
  static const char* const OB_SV__GROUPBY_NOPUSHDOWN_CUT_RATIO = "_groupby_nopushdown_cut_ratio";
  static const char* const OB_SV__PX_BROADCAST_FUDGE_FACTOR = "_px_broadcast_fudge_factor";
  static const char* const OB_SV_TRANSACTION_ISOLATION = "transaction_isolation";
  static const char* const OB_SV_TRX_LOCK_TIMEOUT = "ob_trx_lock_timeout";
  static const char* const OB_SV_VALIDATE_PASSWORD_CHECK_USER_NAME = "validate_password_check_user_name";
  static const char* const OB_SV_VALIDATE_PASSWORD_LENGTH = "validate_password_length";
  static const char* const OB_SV_VALIDATE_PASSWORD_MIXED_CASE_COUNT = "validate_password_mixed_case_count";
  static const char* const OB_SV_VALIDATE_PASSWORD_NUMBER_COUNT = "validate_password_number_count";
  static const char* const OB_SV_VALIDATE_PASSWORD_POLICY = "validate_password_policy";
  static const char* const OB_SV_VALIDATE_PASSWORD_SPECIAL_CHAR_COUNT = "validate_password_special_char_count";
  static const char* const OB_SV_DEFAULT_PASSWORD_LIFETIME = "default_password_lifetime";
  static const char* const OB_SV__OB_OLS_POLICY_SESSION_LABELS = "_ob_ols_policy_session_labels";
  static const char* const OB_SV_TRACE_INFO = "ob_trace_info";
  static const char* const OB_SV__PX_PARTITION_SCAN_THRESHOLD = "_px_partition_scan_threshold";
  static const char* const OB_SV__OB_PX_BCAST_OPTIMIZATION = "_ob_px_bcast_optimization";
  static const char* const OB_SV__OB_PX_SLAVE_MAPPING_THRESHOLD = "_ob_px_slave_mapping_threshold";
  static const char* const OB_SV__ENABLE_PARALLEL_DML = "_enable_parallel_dml";
  static const char* const OB_SV__PX_MIN_GRANULES_PER_SLAVE = "_px_min_granules_per_slave";
  static const char* const OB_SV_SECURE_FILE_PRIV = "secure_file_priv";
  static const char* const OB_SV_PLSQL_WARNINGS = "plsql_warnings";
  static const char* const OB_SV__ENABLE_PARALLEL_QUERY = "_enable_parallel_query";
  static const char* const OB_SV__FORCE_PARALLEL_QUERY_DOP = "_force_parallel_query_dop";
  static const char* const OB_SV__FORCE_PARALLEL_DML_DOP = "_force_parallel_dml_dop";
  static const char* const OB_SV_PL_BLOCK_TIMEOUT = "ob_pl_block_timeout";
  static const char* const OB_SV_TRANSACTION_READ_ONLY = "transaction_read_only";
  static const char* const OB_SV_RESOURCE_MANAGER_PLAN = "resource_manager_plan";
  static const char* const OB_SV_PERFORMANCE_SCHEMA = "performance_schema";
  static const char* const OB_SV_NLS_CURRENCY = "nls_currency";
  static const char* const OB_SV_NLS_ISO_CURRENCY = "nls_iso_currency";
  static const char* const OB_SV_NLS_DUAL_CURRENCY = "nls_dual_currency";
  static const char* const OB_SV_PLSQL_CCFLAGS = "plsql_ccflags";
  static const char* const OB_SV__OB_PROXY_SESSION_TEMPORARY_TABLE_USED = "_ob_proxy_session_temporary_table_used";
  static const char* const OB_SV__ENABLE_PARALLEL_DDL = "_enable_parallel_ddl";
  static const char* const OB_SV__FORCE_PARALLEL_DDL_DOP = "_force_parallel_ddl_dop";
  static const char* const OB_SV_CURSOR_SHARING = "cursor_sharing";
  static const char* const OB_SV__OPTIMIZER_NULL_AWARE_ANTIJOIN = "_optimizer_null_aware_antijoin";
  static const char* const OB_SV__PX_PARTIAL_ROLLUP_PUSHDOWN = "_px_partial_rollup_pushdown";
  static const char* const OB_SV__PX_DIST_AGG_PARTIAL_ROLLUP_PUSHDOWN = "_px_dist_agg_partial_rollup_pushdown";
  static const char* const OB_SV__CREATE_AUDIT_PURGE_JOB = "_create_audit_purge_job";
  static const char* const OB_SV__DROP_AUDIT_PURGE_JOB = "_drop_audit_purge_job";
  static const char* const OB_SV__SET_PURGE_JOB_INTERVAL = "_set_purge_job_interval";
  static const char* const OB_SV__SET_PURGE_JOB_STATUS = "_set_purge_job_status";
  static const char* const OB_SV__SET_LAST_ARCHIVE_TIMESTAMP = "_set_last_archive_timestamp";
  static const char* const OB_SV__CLEAR_LAST_ARCHIVE_TIMESTAMP = "_clear_last_archive_timestamp";
  static const char* const OB_SV__AGGREGATION_OPTIMIZATION_SETTINGS = "_aggregation_optimization_settings";
  static const char* const OB_SV__PX_SHARED_HASH_JOIN = "_px_shared_hash_join";
  static const char* const OB_SV_SQL_NOTES = "sql_notes";
  static const char* const OB_SV_INNODB_STRICT_MODE = "innodb_strict_mode";
  static const char* const OB_SV__WINDOWFUNC_OPTIMIZATION_SETTINGS = "_windowfunc_optimization_settings";
  static const char* const OB_SV_ENABLE_RICH_ERROR_MSG = "ob_enable_rich_error_msg";

}
}
#endif //OCEANBASE_COMMON_OB_SYSTEM_VARIABLE_ALIAS_H_

