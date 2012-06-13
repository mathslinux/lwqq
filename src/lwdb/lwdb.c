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

static LwqqErrorCode lwdb_globaldb_add_new_user(
    struct LwdbGlobalDB *db, const char *qqnumber);
static LwdbGlobalUserEntry *lwdb_globaldb_query_user_info(
    struct LwdbGlobalDB *db, const char *qqnumber);
static LwqqErrorCode lwdb_globaldb_update_user_info(
    struct LwdbGlobalDB *db, const char *key, const char *value);

static LwqqBuddy *lwdb_userdb_query_buddy_info(
    struct LwdbUserDB *db, const char *qqnumber);
static LwqqErrorCode lwdb_userdb_update_buddy_info(
    struct LwdbUserDB *db, LwqqBuddy *buddy);

static char *database_path;
static char *global_database_name;

#define LWDB_INIT_VERSION 1001

static const char *create_global_db_sql =
    "create table if not exists configs("
    "    id integer primary key asc autoincrement,"
    "    family default '',"
    "    key default '',"
    "    value default '');"
    "create table if not exists users("
    "    qqnumber primary key,"
    "    db_name default '',"
    "    password default '',"
    "    status default 'offline',"
    "    rempwd default '1');";

static const char *create_user_db_sql =
    "create table if not exists buddies("
    "    qqnumber primary key,"
    "    category default '',"
    "    vip_info default '',"
    "    nick default '',"
    "    markname default '',"
    "    face default '',"
    "    flag default '');"
    "create table if not exists categories("
    "    name primary key,"
    "    cg_index default '',"
    "    sort default '');";

/** 
 * LWDB initialization
 * 
 */
void lwdb_init()
{
    char buf[256];
    char *home;

    home = getenv("HOME");
    if (!home) {
        lwqq_log(LOG_ERROR, "Cant get $HOME, exit\n");
        exit(1);
    }
    snprintf(buf, sizeof(buf), "%s/.config/lwqq", home);
    database_path = s_strdup(buf);
    
    snprintf(buf, sizeof(buf), "%s/lwqq.db", database_path);
    global_database_name = s_strdup(buf);
}

/** 
 * LWDB final
 * 
 */
void lwdb_final()
{
    s_free(database_path);
    s_free(global_database_name);
}

/** 
 * Create database for lwqq
 * 
 * @param filename 
 * @param db_type Type of database you want to create. 0 means
 *        global database, 1 means user database
 * 
 * @return 0 if everything is ok, else return -1
 */
static int lwdb_create_db(const char *filename, int db_type)
{
    int ret;
    char *errmsg = NULL;
    
    if (!filename) {
        return -1;
    }

    if (access(filename, F_OK) == 0) {
        lwqq_log(LOG_WARNING, "Find a file whose name is same as file "
                 "we want to create, delete it.\n");
        unlink(filename);
    }
    if (db_type == 0) {
        ret = sws_exec_sql_directly(filename, create_global_db_sql, &errmsg);
    } else if (db_type == 1) {
        ret = sws_exec_sql_directly(filename, create_user_db_sql, &errmsg);
    } else {
        ret = -1;
    }

    if (errmsg) {
        lwqq_log(LOG_ERROR, "%s\n", errmsg);
        s_free(errmsg);
    }
    return ret;
}

/** 
 * Check whether db is valid
 * 
 * @param filename The db name
 * @param type 0 means db is a global db, 1 means db is a user db
 * 
 * @return 1 if db is valid, else return 0
 */
static int db_is_valid(const char *filename, int type)
{
    int ret;
    SwsStmt *stmt = NULL;
    SwsDB *db = NULL;
    char *sql;

    /* Check whether file exists */
    if (!filename || access(filename, F_OK)) {
        goto invalid;
    }
    
    /* Open DB */
    db = sws_open_db(filename, NULL);
    if (!db) {
        goto invalid;
    }
    
    /* Query DB */
    if (type == 0) {
        sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='configs';";
    } else {
        sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='buddies';";
    }
    ret = sws_query_start(db, sql, &stmt, NULL);
    if (ret) {
        goto invalid;
    }
    if (sws_query_next(stmt, NULL)) {
        goto invalid;
    }
    sws_query_end(stmt, NULL);
    
    /* Close DB */
    sws_close_db(db, NULL);

    /* OK, it is a valid db */
    return 1;
    
invalid:
    sws_close_db(db, NULL);
    return 0;
}

/** 
 * Create a global DB object
 * 
 * @return A new global DB object, or NULL if somethins wrong, and store
 * error code in err
 */

LwdbGlobalDB *lwdb_globaldb_new()
{
    LwdbGlobalDB *db = NULL;
    int ret;
    
    if (!db_is_valid(global_database_name, 0)) {
        ret = lwdb_create_db(global_database_name, 0);
        if (ret) {
            goto failed;
        }
    }

    db = s_malloc0(sizeof(*db));
    db->db = sws_open_db(global_database_name, NULL);
    if (!db->db) {
        goto failed;
    }
    db->add_new_user = lwdb_globaldb_add_new_user;
    db->query_user_info = lwdb_globaldb_query_user_info;
    db->update_user_info = lwdb_globaldb_update_user_info;
    
    
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
 * Free LwdbGlobalUserEntry object
 * 
 * @param e 
 */
void lwdb_globaldb_free_user_entry(LwdbGlobalUserEntry *e)
{
    if (e) {
        s_free(e->qqnumber);
        s_free(e->db_name);
        s_free(e->password);
        s_free(e->status);
        s_free(e->rempwd);
        s_free(e);
    }
}

/** 
 * 
 * 
 * @param db 
 * @param qqnumber 
 * 
 * @return LWQQ_EC_OK on success, else return LWQQ_EC_DB_EXEC_FAIELD on failure
 */
static LwqqErrorCode lwdb_globaldb_add_new_user(
    struct LwdbGlobalDB *db, const char *qqnumber)
{
    char *errmsg = NULL;
    char sql[256];

    if (!qqnumber){
        return LWQQ_EC_NULL_POINTER;
    }
    
    snprintf(sql, sizeof(sql), "INSERT INTO users (qqnumber,db_name) "
             "VALUES('%s','%s/%s.db');", qqnumber, database_path, qqnumber);
    sws_exec_sql(db->db, sql, &errmsg);
    if (errmsg) {
        lwqq_log(LOG_ERROR, "Add new user error: %s\n", errmsg);
        s_free(errmsg);
        return LWQQ_EC_DB_EXEC_FAIELD;
    }

    return LWQQ_EC_OK;
}

static LwdbGlobalUserEntry *lwdb_globaldb_query_user_info(
    struct LwdbGlobalDB *db, const char *qqnumber)
{
    int ret;
    char sql[256];
    LwdbGlobalUserEntry *e = NULL;
    SwsStmt *stmt = NULL;

    if (!qqnumber) {
        return NULL;
    }

    snprintf(sql, sizeof(sql), "SELECT db_name,password,status,rempwd "
             "FROM users WHERE qqnumber='%s';", qqnumber);
    ret = sws_query_start(db->db, sql, &stmt, NULL);
    if (ret) {
        goto failed;
    }

    if (!sws_query_next(stmt, NULL)) {
        e = s_malloc0(sizeof(*e));
        char buf[256] = {0};
#define GET_MEMBER_VALUE(i, member) {                           \
            sws_query_column(stmt, i, buf, sizeof(buf), NULL);  \
            e->member = s_strdup(buf);                          \
        }
        e->qqnumber = s_strdup(qqnumber);
        GET_MEMBER_VALUE(0, db_name);
        GET_MEMBER_VALUE(1, password);
        GET_MEMBER_VALUE(2, status);
        GET_MEMBER_VALUE(3, rempwd);
#undef GET_MEMBER_VALUE
    }
    sws_query_end(stmt, NULL);

    return e;

failed:
    s_free(e);
    sws_query_end(stmt, NULL);
    return NULL;
}

static LwqqErrorCode lwdb_globaldb_update_user_info(
    struct LwdbGlobalDB *db, const char *key, const char *value)
{
    char sql[256];
    
    if (!key || !value) {
        return LWQQ_EC_NULL_POINTER;
    }

    snprintf(sql, sizeof(sql), "UPDATE users SET %s='%s';", key, value);
    if (!sws_exec_sql(db->db, sql, NULL)) {
        return LWQQ_EC_DB_EXEC_FAIELD;
    }
    
    return LWQQ_EC_OK;
}

LwdbUserDB *lwdb_userdb_new(const char *qqnumber)
{
    LwdbUserDB *udb = NULL;
    LwdbGlobalDB *gdb = NULL;
    LwdbGlobalUserEntry *e = NULL;
    int ret;
    char *db_name;
    
    if (!qqnumber) {
        return NULL;
    }

    /* Get user's db name */
    gdb = lwdb_globaldb_new(global_database_name);
    if (!gdb) {
        goto failed;
    }
    e = gdb->query_user_info(gdb, qqnumber);
    if (!e) {
        goto failed;
    }
    db_name = e->db_name;

    /* If there is no db named "db_name", create it */
    if (!db_is_valid(db_name, 1)) {
        lwqq_log(LOG_WARNING, "db doesnt exist, create it\n");
        ret = lwdb_create_db(db_name, 1);
        if (ret) {
            goto failed;
        }
    }

    udb = s_malloc0(sizeof(*udb));
    udb->db = sws_open_db(db_name, NULL);
    if (!udb->db) {
        goto failed;
    }
    udb->query_buddy_info = lwdb_userdb_query_buddy_info;
    udb->update_buddy_info = lwdb_userdb_update_buddy_info;

    lwdb_globaldb_free(gdb);
    lwdb_globaldb_free_user_entry(e);
    return udb;

failed:
    lwdb_globaldb_free(gdb);
    lwdb_globaldb_free_user_entry(e);
    lwdb_userdb_free(udb);
    return NULL;
}

void lwdb_userdb_free(LwdbUserDB *db)
{
    if (db) {
        sws_close_db(db->db, NULL);
        s_free(db);
    }
}

/** 
 * Query buddy's information
 * 
 * @param db 
 * @param qqnumber The key we used to query info from DB
 * 
 * @return A LwqqBuddy object on success, or NULL on failure
 */
static LwqqBuddy *lwdb_userdb_query_buddy_info(
    struct LwdbUserDB *db, const char *qqnumber)
{
    return NULL;
}

/** 
 * Update buddy's information
 * 
 * @param db 
 * @param buddy 
 * 
 * @return LWQQ_EC_OK on success, or a LwqqErrorCode member
 */
static LwqqErrorCode lwdb_userdb_update_buddy_info(
    struct LwdbUserDB *db, LwqqBuddy *buddy)
{
    return LWQQ_EC_OK;
}
