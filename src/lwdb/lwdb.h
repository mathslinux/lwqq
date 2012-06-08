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
/* LwdbGlobalDB API */

typedef struct LwdbGlobalDB {
    SwsDB *db;                  /**< Pointer sqlite3 db */
    LwqqErrorCode (*add_new_user)(struct LwdbGlobalDB *db, const char *qq_number);
} LwdbGlobalDB;

/** 
 * Create a global DB object
 * 
 * @param filename The database filename
 * @param err Used to store error code
 * 
 * @return A new global DB object, or NULL if somethins wrong, and store
 * error code in err
 */
LwdbGlobalDB *lwdb_globaldb_new(const char *filename, LwqqErrorCode *err);

/** 
 * Free a LwdbGlobalDb object
 * 
 * @param db 
 */
void lwdb_globaldb_free(LwdbGlobalDB *db);

/* LwdbGlobalDB API end */

/************************************************************************/
/* LwdbUserDB API */

typedef struct LwdbUserDB {
} LwdbUserDB;
/** 
 * Create a user DB object
 * 
 * @param filename The database filename
 * @param err Used to store error code
 * 
 * @return A new user DB object, or NULL if somethins wrong, and store
 * error code in err
 */
LwdbUserDB *lwdb_userdb_new(const char *filename, LwqqErrorCode *err);

/** 
 * Free a LwdbUserDB object
 * 
 * @param db 
 */
void lwdb_userdb_free(LwdbUserDB *db);

/* LwdbUserDB API end */

/************************************************************************/
/** 
 * Create database for lwqq
 * 
 * @param filename 
 * @param db_type Type of database you want to create. 0 means
 *        global database, 1 means user database
 * 
 * @return 0 if everything is ok, else return -1
 */
int lwdb_create_db(const char *filename, int db_type);

#endif
