/**
 * @file   lwdb.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Tue May 29 15:24:42 2012
 * 
 * @brief  LWQQ Database API
 * We use Sqlite3 in this project.
 *
 * Description: There are has two types database in LWQQ, one is
 * global, other one depends on user. e.g. if we have two qq user(A and B),
 * then we should have three database, lwqq.sqlie (for global),
 * a.sqlite(for user A), b.sqlite(for user B).
 * 
 * 
 */

#include <stdlib.h>
#include <sqlite3.h>
#include <unistd.h>
#include "smemory.h"
#include "logger.h"
#include "swsqlite.h"
#include "lwdb.h"

LwqqErrorCode lwdb_globaldb_add_new_user(struct LwdbGlobalDB *db,
                                         const char *qq_number);
    
#ifndef LWQQ_CONFIG_DIR
#define LWQQ_CONFIG_DIR "~/.config/lwqq"
#endif

#define LWDB_INIT_VERSION 1001

static const char *create_global_db_sql =
    "create table if not exists configs("
    "    id integer primary key asc autoincrement,family,key,value);"
    "create table if not exists users("
    "    qqnumber primary key,database_name,password,status,rempwd);";

#if 0
static const char *create_user_db_sql =
    "create table if not exists buddies("
    "    qqnumber primary key,category,vip_info,nick,markname,face,flag);"
    "create table if not exists categories("
    "    name primary key,index,sort);";
#endif 

/** 
 * Create database for lwqq
 * 
 * @param filename 
 * @param db_type Type of database you want to create. 0 means
 *        global database, 1 means user database
 * 
 * @return 0 if everything is ok, else return -1
 */
int lwdb_create_db(const char *filename, int db_type)
{
    int ret;
    
    if (!filename) {
        return -1;
    }

    if (access(filename, F_OK) == 0) {
        unlink(filename);
    }
    if (db_type == 0) {
        ret = sws_exec_sql_directly(filename, create_global_db_sql, NULL);
    } else if (db_type == 1) {
        ret = sws_exec_sql_directly(filename, create_global_db_sql, NULL);
    } else {
        ret = -1;
    }

    if (ret == 0) {
        /* Update database version */
        ret = sws_exec_sql_directly(
            filename,
            "INSERT INTO configs (family,key,value) "
            "VALUES('lwqq','version',1001);", NULL);
    }
    
    return ret;
}

static int file_is_global_db(const char *filename)
{
    int ret;
    SwsStmt *stmt = NULL;
    char *test_sql = "SELECT value FROM config WHERE key='version'";
    char value[32] = {0};

    
    /* Open DB */
    SwsDB *db = sws_open_db(filename, NULL);
    if (!db) {
        goto invalid;
    }
    
    /* Query DB */
    ret = sws_query_start(db, test_sql, &stmt, NULL);
    if (ret) {
        goto invalid;
    }
    if (sws_query_next(stmt, NULL)) {
        goto invalid;
    }
    sws_query_column(stmt, 0, value, sizeof(value), NULL);
    sws_query_end(stmt, NULL);

    if (atoi(value) < 1000) {
        goto invalid;
    }
    
    /* Close DB */
    sws_close_db(db, NULL);

    return 1;
    
invalid:
    sws_close_db(db, NULL);
    return 0;
}

/** 
 * Create a global DB object
 * 
 * @param filename The database filename
 * @param err Used to store error code
 * 
 * @return A new global DB object, or NULL if somethins wrong, and store
 * error code in err
 */

LwdbGlobalDB *lwdb_globaldb_new(const char *filename, LwqqErrorCode *err)
{
    LwdbGlobalDB *db = NULL;
    int ret;
    
    if (!filename) {
        if (*err)
            *err = LWQQ_EC_NULL_POINTER;
        return NULL;
    }

    if (!file_is_global_db(filename)) {
        ret = lwdb_create_db(filename, 0);
        if (ret) {
            goto failed;
        }
    }

    db = s_malloc0(sizeof(*db));
    db->db = sws_open_db(filename, NULL);
    if (!db->db) {
        goto failed;
    }
    db->add_new_user = lwdb_globaldb_add_new_user;
    
    return db;

failed:
    lwdb_globaldb_free(db);
    return NULL;
}

/** 
 * Free a LwdbGlobalDb object
 * 
 * @param db 
 */
void lwdb_globaldb_free(LwdbGlobalDB *db)
{
    if (db) {
        sws_close_db(db->db, NULL);
        s_free(db);
    }
}

/** 
 * 
 * 
 * @param db 
 * @param qq_number 
 * 
 * @return LWQQ_EC_OK on success, else return LWQQ_EC_DB_EXEC_FAIELD on failure
 */
LwqqErrorCode lwdb_globaldb_add_new_user(struct LwdbGlobalDB *db,
                                         const char *qq_number)
{
    char *errmsg = NULL;
    char sql[256];

    if (!qq_number){
        return LWQQ_EC_NULL_POINTER;
    }
    
    snprintf(sql, sizeof(sql), "INSERT INTO users (qqnumber) VALUES('%s');",
             qq_number);
    sws_exec_sql(db->db, sql, &errmsg);
    if (errmsg) {
        lwqq_log(LOG_ERROR, "Add new user error: %s\n", errmsg);
        s_free(errmsg);
        return LWQQ_EC_DB_EXEC_FAIELD;
    }

    return LWQQ_EC_OK;
}
