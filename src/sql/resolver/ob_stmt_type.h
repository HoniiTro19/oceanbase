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

#ifdef OB_STMT_TYPE_DEF
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_NONE, err_stmt_type_priv, 0)
OB_STMT_TYPE_DEF(T_SELECT, get_dml_stmt_need_privs, 1, ACTION_TYPE_SELECT)
OB_STMT_TYPE_DEF(T_INSERT, get_dml_stmt_need_privs, 2, ACTION_TYPE_INSERT)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_REPLACE, get_dml_stmt_need_privs, 3)
OB_STMT_TYPE_DEF(T_DELETE, get_dml_stmt_need_privs, 4, ACTION_TYPE_DELETE)
OB_STMT_TYPE_DEF(T_UPDATE, get_dml_stmt_need_privs, 5, ACTION_TYPE_UPDATE)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_MERGE, get_dml_stmt_need_privs, 6)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_EXPLAIN, err_stmt_type_priv, 7)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CREATE_TENANT, get_sys_tenant_super_priv, 8)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DROP_TENANT, get_drop_tenant_stmt_need_privs, 9)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_LOCK_TENANT, get_lock_tenant_stmt_need_privs, 10)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_MODIFY_TENANT, get_modify_tenant_stmt_need_privs, 11)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CHANGE_TENANT, get_change_tenant_privs, 12)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CREATE_RESOURCE_POOL, get_sys_tenant_create_resource_pool_priv, 13)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DROP_RESOURCE_POOL, get_sys_tenant_create_resource_pool_priv, 14)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ALTER_RESOURCE_POOL, get_sys_tenant_create_resource_pool_priv, 15)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SPLIT_RESOURCE_POOL, get_sys_tenant_create_resource_pool_priv, 16)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CREATE_RESOURCE_UNIT, get_sys_tenant_create_resource_unit_priv, 17)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ALTER_RESOURCE_UNIT, get_sys_tenant_create_resource_unit_priv, 18)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DROP_RESOURCE_UNIT, get_sys_tenant_create_resource_unit_priv, 19)
OB_STMT_TYPE_DEF(T_CREATE_TABLE, get_create_table_stmt_need_privs, 20, ACTION_TYPE_CREATE_TABLE)
OB_STMT_TYPE_DEF(T_DROP_TABLE, get_drop_table_stmt_need_privs, 21, ACTION_TYPE_DROP_TABLE)
OB_STMT_TYPE_DEF(T_ALTER_TABLE, get_alter_table_stmt_need_privs, 22, ACTION_TYPE_ALTER_TABLE)
OB_STMT_TYPE_DEF(T_CREATE_INDEX, get_create_index_stmt_need_privs, 23, ACTION_TYPE_CREATE_INDEX)
OB_STMT_TYPE_DEF(T_DROP_INDEX, get_drop_index_stmt_need_privs, 24, ACTION_TYPE_DROP_INDEX)
OB_STMT_TYPE_DEF(T_CREATE_VIEW, err_stmt_type_priv, 25, ACTION_TYPE_CREATE_VIEW)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ALTER_VIEW, err_stmt_type_priv, 26)
OB_STMT_TYPE_DEF(T_DROP_VIEW, err_stmt_type_priv, 27, ACTION_TYPE_DROP_VIEW)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_HELP, no_priv_needed, 28)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_TABLES, err_stmt_type_priv, 29)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_DATABASES, err_stmt_type_priv, 30)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_COLUMNS, err_stmt_type_priv, 31)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_VARIABLES, err_stmt_type_priv, 32)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_TABLE_STATUS, err_stmt_type_priv, 33)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_SCHEMA, err_stmt_type_priv, 34)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_CREATE_DATABASE, err_stmt_type_priv, 35)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_CREATE_TABLE, err_stmt_type_priv, 36)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_CREATE_VIEW, err_stmt_type_priv, 37)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_CREATE_PROCEDURE, err_stmt_type_priv, 38)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_CREATE_FUNCTION, err_stmt_type_priv, 39)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_PARAMETERS, err_stmt_type_priv, 40)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_SERVER_STATUS, err_stmt_type_priv, 41)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_INDEXES, err_stmt_type_priv, 42)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_WARNINGS, err_stmt_type_priv, 43)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_ERRORS, err_stmt_type_priv, 44)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_PROCESSLIST, err_stmt_type_priv, 45)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_CHARSET, err_stmt_type_priv, 46)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_COLLATION, err_stmt_type_priv, 47)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_TABLEGROUPS, err_stmt_type_priv, 48)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_STATUS, err_stmt_type_priv, 49)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_TENANT, err_stmt_type_priv, 50)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_CREATE_TENANT, err_stmt_type_priv, 51)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_TRACE, err_stmt_type_priv, 52)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_ENGINES, err_stmt_type_priv, 53)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_PRIVILEGES, err_stmt_type_priv, 54)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_PROCEDURE_STATUS, err_stmt_type_priv, 55)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_FUNCTION_STATUS, err_stmt_type_priv, 56)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_GRANTS, err_stmt_type_priv, 57)
OB_STMT_TYPE_DEF(T_CREATE_USER, get_create_user_privs, 58, ACTION_TYPE_CREATE_USER)
OB_STMT_TYPE_DEF(T_DROP_USER, get_create_user_privs, 59, ACTION_TYPE_DROP_USER)
OB_STMT_TYPE_DEF(T_SET_PASSWORD, get_create_user_privs, 60, ACTION_TYPE_PASSWORD_CHANGE)
OB_STMT_TYPE_DEF(T_LOCK_USER, get_create_user_privs, 61, ACTION_TYPE_LOCK)
OB_STMT_TYPE_DEF(T_RENAME_USER, get_create_user_privs, 62, ACTION_TYPE_RENAME)
OB_STMT_TYPE_DEF(T_GRANT, get_grant_stmt_need_privs, 63, ACTION_TYPE_GRANT_OBJECT)
OB_STMT_TYPE_DEF(T_REVOKE, get_revoke_stmt_need_privs, 64, ACTION_TYPE_REVOKE_OBJECT)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_PREPARE, no_priv_needed, 65)
OB_STMT_TYPE_DEF(T_VARIABLE_SET, get_variable_set_stmt_need_privs, 66, ACTION_TYPE_ALTER_SYSTEM)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_EXECUTE, no_priv_needed, 67)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DEALLOCATE, no_priv_needed, 68)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_START_TRANS, no_priv_needed, 69)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_END_TRANS, no_priv_needed, 70)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KILL, no_priv_needed, 71)
OB_STMT_TYPE_DEF(T_ALTER_SYSTEM, get_sys_tenant_super_priv, 72, ACTION_TYPE_ALTER_SYSTEM)
OB_STMT_TYPE_DEF(T_ALTER_SYSTEM_SETTP, get_sys_tenant_alter_system_priv, 73, ACTION_TYPE_ALTER_SYSTEM)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CHANGE_OBI, err_stmt_type_priv, 74)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SWITCH_MASTER, get_sys_tenant_super_priv, 75)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SERVER_ACTION, get_sys_tenant_super_priv, 76)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_BOOTSTRAP, get_boot_strap_stmt_need_privs, 77)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CS_DISKMAINTAIN, err_stmt_type_priv, 78)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_TABLET_CMD, err_stmt_type_priv, 79)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_REPORT_REPLICA, get_sys_tenant_alter_system_priv, 80)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SWITCH_ROOTSERVER, err_stmt_type_priv, 81)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SWITCH_UPDATESERVER, err_stmt_type_priv, 82)
OB_STMT_TYPE_DEF(T_CLUSTER_MANAGER, err_stmt_type_priv, 83, ACTION_TYPE_ALTER_CLUSTER)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_FREEZE, get_sys_tenant_alter_system_priv, 84)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_FLUSH_CACHE, get_sys_tenant_alter_system_priv, 85)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_FLUSH_KVCACHE, get_sys_tenant_alter_system_priv, 86)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_FLUSH_ILOGCACHE, get_sys_tenant_alter_system_priv, 87)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DROP_MEMTABLE, err_stmt_type_priv, 88)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CLEAR_MEMTABLE, err_stmt_type_priv, 89)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_PRINT_ROOT_TABLE, err_stmt_type_priv, 90)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ADD_UPDATESERVER, err_stmt_type_priv, 91)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DELETE_UPDATESERVER, err_stmt_type_priv, 92)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CHECK_ROOT_TABLE, err_stmt_type_priv, 93)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CLEAR_ROOT_TABLE, get_sys_tenant_alter_system_priv, 94)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_REFRESH_SCHEMA, get_sys_tenant_alter_system_priv, 95)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CREATE_DATABASE, get_create_database_stmt_need_privs, 96)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_USE_DATABASE, no_priv_needed, 97)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ADMIN_SERVER, get_sys_tenant_alter_system_priv, 98)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ADMIN_ZONE, get_sys_tenant_alter_system_priv, 99)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SWITCH_REPLICA_ROLE, get_sys_tenant_alter_system_priv, 100)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_RECYCLE_REPLICA, get_sys_tenant_alter_system_priv, 103)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ADMIN_MERGE, get_sys_tenant_alter_system_priv, 104)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ALTER_DATABASE, get_alter_database_stmt_need_privs, 105)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DROP_DATABASE, get_drop_database_stmt_need_privs, 106)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CREATE_TABLEGROUP, get_create_tablegroup_stmt_need_privs, 107)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DROP_TABLEGROUP, get_drop_tablegroup_stmt_need_privs, 108)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ALTER_TABLEGROUP, get_alter_tablegroup_stmt_need_privs, 109)
OB_STMT_TYPE_DEF(T_TRUNCATE_TABLE, get_truncate_table_stmt_need_privs, 110, ACTION_TYPE_TRUNCATE_TABLE)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_RENAME_TABLE, get_rename_table_stmt_need_privs, 111)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CREATE_TABLE_LIKE, get_create_table_like_stmt_need_privs, 112)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SET_NAMES, no_priv_needed, 113)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CLEAR_LOCATION_CACHE, get_sys_tenant_alter_system_priv, 114)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_RELOAD_GTS, get_sys_tenant_alter_system_priv, 115)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_RELOAD_UNIT, get_sys_tenant_alter_system_priv, 116)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_RELOAD_SERVER, get_sys_tenant_alter_system_priv, 117)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_RELOAD_ZONE, get_sys_tenant_alter_system_priv, 118)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CLEAR_MERGE_ERROR, get_sys_tenant_alter_system_priv, 119)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_MIGRATE_UNIT, get_sys_tenant_alter_system_priv, 120)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_UPGRADE_VIRTUAL_SCHEMA, get_sys_tenant_alter_system_priv, 121)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_RUN_JOB, get_sys_tenant_alter_system_priv, 122)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_EMPTY_QUERY, no_priv_needed, 123)
OB_STMT_TYPE_DEF(T_CREATE_OUTLINE, no_priv_needed, 124, ACTION_TYPE_CREATE_OUTLINE)
OB_STMT_TYPE_DEF(T_ALTER_OUTLINE, no_priv_needed, 125, ACTION_TYPE_ALTER_OUTLINE)
OB_STMT_TYPE_DEF(T_DROP_OUTLINE, no_priv_needed, 126, ACTION_TYPE_DROP_OUTLINE)
// stmt type 127, 128 used by plan baseline, and is been delete
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SWITCH_RS_ROLE, get_sys_tenant_alter_system_priv, 129)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_FLASHBACK_TENANT, get_flashback_tenant_stmt_need_privs, 130)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_FLASHBACK_DATABASE, get_flashback_database_stmt_need_privs, 131)
OB_STMT_TYPE_DEF(T_FLASHBACK_TABLE_FROM_RECYCLEBIN, get_flashback_table_stmt_need_privs, 132, ACTION_TYPE_FLASHBACK_TABLE)
OB_STMT_TYPE_DEF(T_FLASHBACK_INDEX, get_flashback_index_stmt_need_privs, 133, ACTION_TYPE_FLASHBACK)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_PURGE_RECYCLEBIN, get_purge_recyclebin_stmt_need_privs, 134)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_PURGE_TENANT, get_purge_tenant_stmt_need_privs, 135)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_PURGE_DATABASE, get_purge_database_stmt_need_privs, 136)
OB_STMT_TYPE_DEF(T_PURGE_TABLE, get_purge_table_stmt_need_privs, 137, ACTION_TYPE_PURGE_TABLE)
OB_STMT_TYPE_DEF(T_PURGE_INDEX, get_purge_index_stmt_need_privs, 138, ACTION_TYPE_PURGE_INDEX)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_RECYCLEBIN, err_stmt_type_priv, 139)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ADMIN_UPGRADE_CMD, get_sys_tenant_alter_system_priv, 140)
OB_STMT_TYPE_DEF(T_CREATE_ROUTINE, no_priv_needed, 141, ACTION_TYPE_OB_CREATE_ROUTINE)
OB_STMT_TYPE_DEF(T_DROP_ROUTINE, no_priv_needed, 142, ACTION_TYPE_OB_DROP_ROUTINE)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ALTER_ROUTINE, no_priv_needed, 143)
OB_STMT_TYPE_DEF(T_CALL_PROCEDURE, no_priv_needed, 144, ACTION_TYPE_EXECUTE_PROCEDURE)
OB_STMT_TYPE_DEF(T_ANONYMOUS_BLOCK, no_priv_needed, 145, ACTION_TYPE_EXECUTE_PROCEDURE)
OB_STMT_TYPE_DEF(T_CREATE_PACKAGE, no_priv_needed, 146, ACTION_TYPE_CREATE_PACKAGE)
OB_STMT_TYPE_DEF(T_CREATE_PACKAGE_BODY, no_priv_needed, 147, ACTION_TYPE_CREATE_PACKAGE_BODY)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ALTER_PACKAGE, no_priv_needed, 148)
OB_STMT_TYPE_DEF(T_DROP_PACKAGE, no_priv_needed, 149, ACTION_TYPE_DROP_PACKAGE)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_REFRESH_TIME_ZONE_INFO, get_sys_tenant_alter_system_priv, 150)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CANCEL_TASK, get_sys_tenant_alter_system_priv, 151)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SET_DISK_VALID, get_sys_tenant_alter_system_priv, 154)
OB_STMT_TYPE_DEF(T_CREATE_SYNONYM, get_create_synonym_priv, 155, ACTION_TYPE_OB_CREATE_SYNONYM)
OB_STMT_TYPE_DEF(T_DROP_SYNONYM, get_drop_synonym_priv, 156, ACTION_TYPE_OB_DROP_SYNONYM)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CLEAR_BALANCE_TASK, get_sys_tenant_alter_system_priv, 157)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_BUILD_INDEX_SSTABLE, get_sys_tenant_super_priv, 158)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ANALYZE, no_priv_needed, 159)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_CREATE_TABLEGROUP, err_stmt_type_priv, 160)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_LOAD_DATA, get_load_data_stmt_need_privs, 161)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_STMT_TYPE_MAX, get_sys_tenant_super_priv, 162)
OB_STMT_TYPE_DEF(T_CREATE_FUNC, no_priv_needed, 163, ACTION_TYPE_CREATE_FUNCTION)
OB_STMT_TYPE_DEF(T_DROP_FUNC, no_priv_needed, 164, ACTION_TYPE_DROP_FUNCTION)
OB_STMT_TYPE_DEF(T_CREATE_SEQUENCE, no_priv_needed, 165, ACTION_TYPE_CREATE_SEQUENCE)
OB_STMT_TYPE_DEF(T_ALTER_SEQUENCE, no_priv_needed, 166, ACTION_TYPE_ALTER_SEQUENCE)
OB_STMT_TYPE_DEF(T_DROP_SEQUENCE, no_priv_needed, 167, ACTION_TYPE_DROP_SEQUENCE)
OB_STMT_TYPE_DEF(T_SET_TABLE_COMMENT, no_priv_needed, 168, ACTION_TYPE_COMMENT)
OB_STMT_TYPE_DEF(T_SET_COLUMN_COMMENT, no_priv_needed, 169, ACTION_TYPE_COMMENT)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SWITCHOVER, get_sys_tenant_super_priv, 170)
OB_STMT_TYPE_DEF(T_CREATE_TYPE, no_priv_needed, 171, ACTION_TYPE_CREATE_TYPE)
OB_STMT_TYPE_DEF(T_DROP_TYPE, no_priv_needed, 172, ACTION_TYPE_DROP_TYPE)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ALTER_DISKGROUP_ADD_DISK, get_sys_tenant_super_priv, 173)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ALTER_DISKGROUP_DROP_DISK, get_sys_tenant_super_priv, 174)
OB_STMT_TYPE_DEF(T_REMOVE_CLUSTER, get_sys_tenant_super_priv, 175, ACTION_TYPE_DROP_CLUSTER)
OB_STMT_TYPE_DEF(T_ADD_CLUSTER, get_sys_tenant_super_priv, 176, ACTION_TYPE_CREATE_CLUSTER)
OB_STMT_TYPE_DEF(T_ALTER_SYSTEM_SET_PARAMETER, get_sys_tenant_alter_system_priv, 177, ACTION_TYPE_ALTER_SYSTEM)
OB_STMT_TYPE_DEF(T_OPTIMIZE_TABLE, no_priv_needed, 178, ACTION_TYPE_ALTER_TABLE)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_OPTIMIZE_TENANT, no_priv_needed, 179)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_OPTIMIZE_ALL, get_sys_tenant_super_priv, 180)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CREATE_SAVEPOINT, no_priv_needed, 181)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ROLLBACK_SAVEPOINT, no_priv_needed, 182)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_RELEASE_SAVEPOINT, no_priv_needed, 183)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CREATE_KEYSTORE, no_priv_needed, 184)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ALTER_KEYSTORE, no_priv_needed, 185)
OB_STMT_TYPE_DEF(T_CREATE_TABLESPACE, no_priv_needed, 186, ACTION_TYPE_CREATE_TABLESPACE)
OB_STMT_TYPE_DEF(T_DROP_TABLESPACE, no_priv_needed, 187, ACTION_TYPE_DROP_TABLESPACE)
OB_STMT_TYPE_DEF(T_CREATE_TRIGGER, no_priv_needed, 188, ACTION_TYPE_CREATE_TRIGGER)
OB_STMT_TYPE_DEF(T_DROP_TRIGGER, no_priv_needed, 189, ACTION_TYPE_DROP_TRIGGER)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_FLASHBACK_TABLE_TO_SCN, get_flashback_table_stmt_need_privs, 190)
OB_STMT_TYPE_DEF(T_CREATE_ROLE, no_priv_needed, 191, ACTION_TYPE_CREATE_ROLE)
OB_STMT_TYPE_DEF(T_DROP_ROLE, no_priv_needed, 192, ACTION_TYPE_DROP_ROLE)
OB_STMT_TYPE_DEF(T_ALTER_ROLE, no_priv_needed, 193, ACTION_TYPE_ALTER_ROLE)
OB_STMT_TYPE_DEF(T_SET_ROLE, no_priv_needed, 194, ACTION_TYPE_SET_ROLE)
OB_STMT_TYPE_DEF(T_SYSTEM_GRANT, no_priv_needed, 195, ACTION_TYPE_SYSTEM_GRANT)
OB_STMT_TYPE_DEF(T_SYSTEM_REVOKE, no_priv_needed, 196, ACTION_TYPE_SYSTEM_REVOKE)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_USER_PROFILE, no_priv_needed, 197)
OB_STMT_TYPE_DEF(T_ALTER_USER_PROFILE, get_create_user_privs, 198, ACTION_TYPE_ALTER_USER)
OB_STMT_TYPE_DEF(T_AUDIT, no_priv_needed, 199, ACTION_TYPE_OB_AUDIT)
OB_STMT_TYPE_DEF(T_LOGIN, no_priv_needed, 200, ACTION_TYPE_LOGON)
OB_STMT_TYPE_DEF(T_LOGOFF, no_priv_needed, 201, ACTION_TYPE_LOGOFF)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ALTER_CLUSTER, get_sys_tenant_super_priv, 202)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ARCHIVE_LOG, get_sys_tenant_alter_system_priv, 203)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_BACKUP_DATABASE, get_sys_tenant_alter_system_priv, 204) 
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_BACKUP_MANAGE, get_sys_tenant_alter_system_priv, 205)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_RESTORE_TENANT_2, get_sys_tenant_alter_system_priv, 206)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_REFRESH_MEMORY_STAT, get_sys_tenant_super_priv, 207)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_PHYSICAL_RESTORE_TENANT, get_sys_tenant_alter_system_priv, 208)
OB_STMT_TYPE_DEF(T_ALTER_TABLESPACE, no_priv_needed, 209, ACTION_TYPE_ALTER_TABLESPACE)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ADMIN_ROLLING_UPGRADE_CMD, get_sys_tenant_super_priv, 210)
OB_STMT_TYPE_DEF(T_GRANT_ROLE, no_priv_needed, 211, ACTION_TYPE_GRANT_ROLE)
OB_STMT_TYPE_DEF(T_REVOKE_ROLE, no_priv_needed, 212, ACTION_TYPE_REVOKE_ROLE)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_MERGE_RESOURCE_POOL, get_sys_tenant_super_priv, 213)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_TRIGGERS, err_stmt_type_priv, 214)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CREATE_DBLINK, get_create_dblink_priv, 215)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DROP_DBLINK, get_drop_dblink_priv, 216)
OB_STMT_TYPE_DEF(T_CREATE_PUB_SYNONYM, get_create_synonym_priv, 217, ACTION_TYPE_CREATE_PUBLIC_SYNONYM)
OB_STMT_TYPE_DEF(T_DROP_PUB_SYNONYM, get_drop_synonym_priv, 218, ACTION_TYPE_DROP_PUBLIC_SYNONYM)
OB_STMT_TYPE_DEF(T_CREATE_PROFILE, no_priv_needed, 219, ACTION_TYPE_CREATE_PROFILE)
OB_STMT_TYPE_DEF(T_ALTER_PROFILE, no_priv_needed, 220, ACTION_TYPE_ALTER_PROFILE)
OB_STMT_TYPE_DEF(T_DROP_PROFILE, no_priv_needed, 221, ACTION_TYPE_DROP_PROFILE)
OB_STMT_TYPE_DEF(T_ALTER_USER, get_create_user_privs, 222, ACTION_TYPE_ALTER_USER)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_XA_START, no_priv_needed, 223)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_XA_END, no_priv_needed, 224)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_XA_PREPARE, no_priv_needed, 225)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_XA_COMMIT, no_priv_needed, 226)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_XA_ROLLBACK, no_priv_needed, 227)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_BACKUP_BACKUPSET, get_sys_tenant_alter_system_priv, 228)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_BACKUP_ARCHIVELOG, get_sys_tenant_alter_system_priv, 229)
OB_STMT_TYPE_DEF(T_ALTER_TRIGGER, no_priv_needed, 230, ACTION_TYPE_ALTER_TRIGGER)
OB_STMT_TYPE_DEF(T_ALTER_USER_PRIMARY_ZONE, get_create_user_privs, 231, ACTION_TYPE_ALTER_USER)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_CREATE_TRIGGER, err_stmt_type_priv, 232)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DIAGNOSTICS, no_priv_needed, 233)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_BACKUP_CLEAN, get_sys_tenant_alter_system_priv, 234)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DELETE_POLICY, get_sys_tenant_alter_system_priv, 235)
OB_STMT_TYPE_DEF(T_SHOW_QUERY_RESPONSE_TIME, err_stmt_type_priv, 236, ACTION_TYPE_ALTER_USER)

OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ENABLE_SQL_THROTTLE, no_priv_needed, 250)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DISABLE_SQL_THROTTLE, no_priv_needed, 251)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_BACKUP_SET_ENCRYPTION, no_priv_needed, 252)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_BACKUP_SET_DECRYPTION, get_sys_tenant_super_priv, 253)

OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ADMIN_RECOVERY, get_sys_tenant_alter_system_priv, 254)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CREATE_RESTORE_POINT, get_restore_point_priv, 255)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DROP_RESTORE_POINT, get_restore_point_priv, 256)

OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ADMIN_RUN_UPGRADE_JOB, get_sys_tenant_alter_system_priv, 257)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ADMIN_STOP_UPGRADE_JOB, get_sys_tenant_alter_system_priv, 258)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_FLUSH_DAG_WARNINGS, get_sys_tenant_super_priv, 259)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_BACKUP_BACKUPPIECE, get_sys_tenant_alter_system_priv, 261)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ADD_RESTORE_SOURCE, get_sys_tenant_alter_system_priv, 262)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CLEAR_RESTORE_SOURCE, get_sys_tenant_alter_system_priv, 263)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_RESTORE_PREVIEW, err_stmt_type_priv, 264)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SET_REGION_NETWORK_BANDWIDTH, get_sys_tenant_super_priv, 265)
// TODO
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CREATE_DIRECTORY, no_priv_needed, 266)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DROP_DIRECTORY, no_priv_needed, 267)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_LOCK_TABLE, no_priv_needed, 268)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ALTER_RESOURCE_TENANT, get_sys_tenant_create_resource_pool_priv, 269)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ARCHIVE_TENANT, get_sys_tenant_alter_system_priv, 270)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DISCONNECT_CLUSTER, get_sys_tenant_super_priv, 271)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_WASH_MEMORY_FRAGMENTATION, get_sys_tenant_super_priv, 272)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CREATE_CONTEXT, no_priv_needed, 273)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_DROP_CONTEXT, no_priv_needed, 274)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CHECKPOINT_SLOG, get_sys_tenant_alter_system_priv, 275)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_REFRESH_IO_CALIBRATION, get_sys_tenant_alter_system_priv, 276)
OB_STMT_TYPE_DEF(T_INSERT_ALL, get_dml_stmt_need_privs, 277, ACTION_TYPE_INSERT)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_PHYSICAL_RESTORE_STANDBY_TENANT, get_sys_tenant_alter_system_priv, 278)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_RECOVER, get_sys_tenant_alter_system_priv, 279)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ADD_ARBITRATION_SERVICE, get_sys_tenant_alter_system_priv, 280)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_REMOVE_ARBITRATION_SERVICE, get_sys_tenant_alter_system_priv, 281)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_REPLACE_ARBITRATION_SERVICE, get_sys_tenant_alter_system_priv, 282)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SHOW_SEQUENCES, err_stmt_type_priv, 283)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_BACKUP_KEY, get_sys_tenant_alter_system_priv, 284)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CREATE_STANDBY_TENANT, get_sys_tenant_super_priv, 285)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CANCEL_RESTORE, get_sys_tenant_alter_system_priv, 286)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_TABLE_TTL, get_sys_tenant_alter_system_priv, 287)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_RECOVER_TABLE, get_sys_tenant_alter_system_priv, 288)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_CANCEL_RECOVER_TABLE, get_sys_tenant_alter_system_priv, 289)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_TRANSFER_PARTITION, get_sys_tenant_alter_system_priv, 297)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_ALTER_LS_REPLICA, get_sys_tenant_alter_system_priv, 299)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_SERVICE_NAME, get_sys_tenant_alter_system_priv, 315)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_INSERT, err_stmt_type_priv, 322)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_GET, err_stmt_type_priv, 323)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_DELETE, err_stmt_type_priv, 324)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_UPDATE, err_stmt_type_priv, 325)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_INSERT_OR_UPDATE, err_stmt_type_priv, 326)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_REPLACE, err_stmt_type_priv, 327)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_INCREMENT, err_stmt_type_priv, 328)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_APPEND, err_stmt_type_priv, 329)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_PUT, err_stmt_type_priv, 330)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_MULTI_GET, err_stmt_type_priv, 331)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_MULTI_INSERT, err_stmt_type_priv, 332)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_MULTI_DELETE, err_stmt_type_priv, 333)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_MULTI_UPDATE, err_stmt_type_priv, 334)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_MULTI_INSERT_OR_UPDATE, err_stmt_type_priv, 335)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_MULTI_REPLACE, err_stmt_type_priv, 336)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_MULTI_INCREMENT, err_stmt_type_priv, 337)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_MULTI_APPEND, err_stmt_type_priv, 338)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_MULTI_PUT, err_stmt_type_priv, 339)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_QUERY, err_stmt_type_priv, 340)

OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_QUERY_AND_MUTATE, err_stmt_type_priv, 347)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_KV_OTHER, err_stmt_type_priv, 348)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_HBASE_SCAN, err_stmt_type_priv, 349)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_HBASE_PUT, err_stmt_type_priv, 350)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_HBASE_DELETE, err_stmt_type_priv, 351)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_HBASE_APPEND, err_stmt_type_priv, 352)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_HBASE_INCREMENT, err_stmt_type_priv, 353)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_HBASE_CHECK_AND_PUT, err_stmt_type_priv, 354)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_HBASE_CHECK_AND_DELETE, err_stmt_type_priv, 355)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_HBASE_HYBRID_BATCH, err_stmt_type_priv, 356)
OB_STMT_TYPE_DEF_UNKNOWN_AT(T_HBASE_OTHER, err_stmt_type_priv, 357)

OB_STMT_TYPE_DEF_UNKNOWN_AT(T_MAX, err_stmt_type_priv, 500)
#endif

#ifndef OCEANBASE_SQL_RESOLVER_OB_STMT_TYPE_
#define OCEANBASE_SQL_RESOLVER_OB_STMT_TYPE_

// default set the action_type to ACTION_TYPE_UNKNOWN
#define OB_STMT_TYPE_DEF_UNKNOWN_AT(stmt_type, priv_check_func, id) OB_STMT_TYPE_DEF(stmt_type, priv_check_func, id, ACTION_TYPE_UNKNOWN)

namespace oceanbase {
namespace sql {
namespace stmt {

enum StmtType : int32_t
{
#define OB_STMT_TYPE_DEF(stmt_type, priv_check_func, id, action_type) stmt_type = id,
#include "sql/resolver/ob_stmt_type.h"
#undef OB_STMT_TYPE_DEF

#define IS_INSERT_OR_REPLACE_STMT(stmt_type) (stmt::T_INSERT == (stmt_type) || stmt::T_REPLACE == (stmt_type))
};

struct StmtTypeIndex
{
public:
  StmtTypeIndex()
    : stmt_type_idx_()
  {
    int i = 0;
    for (int j = 0; j < ARRAYSIZEOF(stmt_type_idx_); j++) {
      stmt_type_idx_[j] = -1;
    }
    #define OB_STMT_TYPE_DEF(stmt_type, priv_check_func, id, action_type) stmt_type_idx_[stmt_type] = i++;
    #include "sql/resolver/ob_stmt_type.h"
    #undef OB_STMT_TYPE_DEF
  }
  int32_t stmt_type_idx_[T_MAX + 1];
};

inline int32_t get_stmt_type_idx(StmtType type)
{
  static StmtTypeIndex inst;
  return inst.stmt_type_idx_[type];
}

}
}
}

#endif /* _OB_STMT_TYPE_H */
