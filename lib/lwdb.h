/**
 * @file   lwdb.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Tue May 29 15:24:42 2012
 * 
 * @brief  LWQQ Datebase API
 * We use Sqlite3 in this project
 * 
 * 
 */

#ifndef LWDB_H
#define LWDB_H

#include "type.h"
#include "swsqlite.h"

/** return default database store dir */
const char* lwdb_get_config_dir();

//========================= USER DB API =======================================/
#define LWDB_CACHE_LEN 15
typedef struct LwdbUserDB {
    SwsDB *db;
    struct{
        SwsStmt* stmt;
        char* sql;
    }cache[LWDB_CACHE_LEN];
    LwqqBuddy * (*query_buddy_info)(struct LwdbUserDB *db, const char *qqnumber);
    LwqqErrorCode (*update_buddy_info)(struct LwdbUserDB *db, LwqqBuddy *buddy);
} LwdbUserDB;

/** 
 * Create a user DB object
 * 
 * @param qqnumber : The qq number
 * @param dir      : the database file store directory,NULL to use default path
 * @param flags    : 0
 * 
 * @return A new user DB object, or NULL if somethins wrong
 */
LwdbUserDB *lwdb_userdb_new(const char *qqnumber,const char* dir,int flags);

/** 
 * Free a LwdbUserDB object
 * 
 * @param db 
 */
void lwdb_userdb_free(LwdbUserDB *db);
/**
 * maybe it is better recognisation
 */
#define lwdb_userdb_close(db) (lwdb_userdb_free(db))

/** 
 * insert a buddy info to database
 * if there is no database entry, create one and update it
 */
LwqqErrorCode lwdb_userdb_insert_buddy_info(LwdbUserDB* db,LwqqBuddy* buddy);
/** 
 * insert a group info to database
 * if there is no database entry, create one and update it
 */
LwqqErrorCode lwdb_userdb_insert_group_info(LwdbUserDB* db,LwqqGroup* group);
#define lwdb_userdb_insert_discu_info(db,discu) lwdb_userdb_insert_group_info(db,discu)

//LwqqErrorCode lwdb_userdb_migrate_discu_info(LwdbUserDB* db,LwqqGroup* discu,int old_account);
/** 
 * update a buddy info to database
 * if there is no database entry, error occurs
 */
LwqqErrorCode lwdb_userdb_update_buddy_info(LwdbUserDB* db,LwqqBuddy* buddy);
/** 
 * update a group info to database
 * if there is no database entry, error occurs
 */
LwqqErrorCode lwdb_userdb_update_group_info(LwdbUserDB* db,LwqqGroup* group);
#define lwdb_userdb_update_discu_info(db,discu) lwdb_userdb_update_group_info(db,discu)

/** erase infomation older than @day and at most @last entries , 
 * when query buddy, LwqqBuddy::last_modify puts LWQQ_LAST_MODIFY_RESET
 */
void lwdb_userdb_flush_buddies(LwdbUserDB* db,int last,int day);
/** erase infomation older than @day and at most @last entries , 
 * when query group, LwqqGroup::last_modify puts LWQQ_LAST_MODIFY_RESET
 */
void lwdb_userdb_flush_groups(LwdbUserDB* db,int last,int day);
/**
 * query all buddies and groups qqnumber from database
 */
void lwdb_userdb_query_qqnumbers(LwdbUserDB* db,LwqqClient* lc);
/** 
 * get data from database, only be sure buddy has qqnumber already 
 */
LwqqErrorCode lwdb_userdb_query_buddy(LwdbUserDB* db,LwqqBuddy* buddy);
/** 
 * get data from database, only be sure group has qqnumber already 
 */
LwqqErrorCode lwdb_userdb_query_group(LwdbUserDB* db,LwqqGroup* group);

/** begin a transaction*/
void lwdb_userdb_begin(LwdbUserDB* db);
/** end a transaction*/
void lwdb_userdb_commit(LwdbUserDB* db);
/** read a custom key in database */
const char* lwdb_userdb_read(LwdbUserDB* db,const char* key);
/** write a custom key,value in database */
int lwdb_userdb_write(LwdbUserDB* db,const char* key,const char* value);
/* LwdbUserDB API end */


/************************************************************************/
/* Initialization and final API */
/** 
 * LWDB initialization
 * 
 */
void lwdb_global_free();

/** 
 * LWDB final
 * 
 */
void lwdb_finalize();

/* Initialization and final API end */

/************************************************************************/
/* LwdbGlobalDB API */
typedef struct LwdbGlobalUserEntry {
    char *qqnumber;
    char *db_name;
    char *password;
    char *status;
    char *rempwd;
    LIST_ENTRY(LwdbGlobalUserEntry) entries;
} LwdbGlobalUserEntry;

typedef struct LwdbGlobalDB {
    SwsDB *db;                  /**< Pointer sqlite3 db */
    LwqqErrorCode (*add_new_user)(struct LwdbGlobalDB *db, const char *qqnumber);
    LwdbGlobalUserEntry * (*query_user_info)(struct LwdbGlobalDB *db,
                                             const char *qqnumber);
    LIST_HEAD(, LwdbGlobalUserEntry) head; /**< QQ friends */
    LwqqErrorCode (*update_user_info)(struct LwdbGlobalDB *db, const char *qqnumber,
                                      const char *key, const char *value);
} LwdbGlobalDB;

/** 
 * Create a global DB object
 * 
 * @return A new global DB object, or NULL if somethins wrong, and store
 * error code in err
 */
LwdbGlobalDB *lwdb_globaldb_new(void);

/** 
 * Free a LwdbGlobalDb object
 * 
 * @param db 
 */
void lwdb_globaldb_free(LwdbGlobalDB *db);

/** 
 * Free LwdbGlobalUserEntry object
 * 
 * @param e 
 */
void lwdb_globaldb_free_user_entry(LwdbGlobalUserEntry *e);

/* LwdbGlobalDB API end */

/************************************************************************/
/* LwdbUserDB API */



/************************************************************************/
#if 0
void lwdb_userdb_write_to_client(LwdbUserDB* from,LwqqClient* to);
void lwdb_userdb_read_from_client(LwqqClient* from,LwdbUserDB* to);
#endif

#endif
