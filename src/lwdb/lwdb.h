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

/************************************************************************/
/* Initialization and final API */
/** 
 * LWDB initialization
 * 
 */
void lwdb_init();

/** 
 * LWDB final
 * 
 */
void lwdb_final();

/* Initialization and final API end */

/************************************************************************/
/* LwdbGlobalDB API */
typedef struct LwdbGlobalUserEntry {
    char *qqnumber;
    char *db_name;
    char *password;
    char *status;
    char *rempwd;
} LwdbGlobalUserEntry;

typedef struct LwdbGlobalDB {
    SwsDB *db;                  /**< Pointer sqlite3 db */
    LwqqErrorCode (*add_new_user)(struct LwdbGlobalDB *db, const char *qqnumber);
    LwdbGlobalUserEntry * (*query_user_info)(struct LwdbGlobalDB *db,
                                          const char *qqnumber);
    LwqqErrorCode (*update_user_info)(struct LwdbGlobalDB *db,
                                      const char *key, const char *value);
} LwdbGlobalDB;

/** 
 * Create a global DB object
 * 
 * @return A new global DB object, or NULL if somethins wrong, and store
 * error code in err
 */
LwdbGlobalDB *lwdb_globaldb_new();

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

typedef struct LwdbUserDB {
    SwsDB *db;
    LwqqBuddy * (*query_buddy_info)(struct LwdbUserDB *db, const char *qqnumber);
    LwqqErrorCode (*update_buddy_info)(struct LwdbUserDB *db, LwqqBuddy *buddy);
} LwdbUserDB;
/** 
 * Create a user DB object
 * 
 * @param qqnumber The qq number
 * 
 * @return A new user DB object, or NULL if somethins wrong, and store
 * error code in err
 */
LwdbUserDB *lwdb_userdb_new(const char *qqnumber);

/** 
 * Free a LwdbUserDB object
 * 
 * @param db 
 */
void lwdb_userdb_free(LwdbUserDB *db);

/* LwdbUserDB API end */

/************************************************************************/

#endif
