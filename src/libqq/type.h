/**
 * @file   type.h
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 20 22:24:30 2012
 * 
 * @brief  Linux WebQQ Data Struct API
 * 
 * 
 */

#ifndef TYPE_H
#define TYPE_H

#include "queue.h"

/* All Information of our qq user */
typedef struct LwqqUser {
    LIST_ENTRY(LwqqUser) entries;
} LwqqUser;

typedef struct LwqqInfo {
    char *username;
    char *password;             /**< Password */
    LwqqUser *user;             /**< User */
    LIST_HEAD(QCowClusterAlloc, LwqqUser) friends; /**< QQ friends */
    
} LwqqInfo;

#endif
