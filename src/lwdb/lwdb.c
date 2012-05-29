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
#include "logger.h"

static const char *global_table_sql =
    "create table if not exists config("
    "   id integer primary key asc autoincrement,family,key,value);"
    "create table if not exists user("
    "   qqnumber primary key,database_name);";

static const char *user_table_sql =
    "create table if not exists ("
    "   id integer primary key asc autoincrement,family,key,value);"
    "create table if not exists user("
    "   qqnumber primary key,database_name);";

/** 
 * Create user's database
 * 
 * @param filename 
 * @param db_type Type of database you want to create. 0 means
 *        global database, 1 means user database
 * 
 * @return 
 */
int lwdb_create_user_db(const char *filename, int db_type)
{
    sqlite3 *db;
    char *err = NULL;
    int ret;
    
    if (!filename) {
        return -1;
    }

    ret = sqlite3_open(filename, &db);
    if (db != SQLITE_OK) {
        goto failed;
    }

    if (db_type == 0) {
        ret = sqlite3_exec(db, global_table_sql, NULL, NULL, &err);
    } else if (db_type == 1) {
        ret = sqlite3_exec(db, user_table_sql, NULL, NULL, &err);
    } else {
        lwqq_log(LOG_ERROR, "No such database type\n");
        goto failed;
    }
    
    if (ret != SQLITE_OK) {
        goto failed;
    }

    sqlite3_close(db);
    return 0;

failed:
    lwqq_log(LOG_ERROR, "Create global database failed\n");
    if (err) {
        sqlite3_free(err);
    }
    sqlite3_close(db);
    return -1;
}
