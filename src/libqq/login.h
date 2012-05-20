/**
 * @file   login.h
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 20 02:25:51 2012
 *
 * @brief  Linux WebQQ Login API
 * 
 * 
 */

#ifndef LWQQ_LOGIN_H
#define LWQQ_LOGIN_H

#include "type.h"

/**
 * Login Error Code
 * 
 */
typedef enum {
    LWQQ_LOGIN_OK,
    LWQQ_LOGIN_ERROR,
} LwqqLoginCode;

/** 
 * WebQQ login function
 * 
 * @param info Client information
 * @param err Error code
 */
void lwqq_login(LwqqInfo *info, LwqqLoginCode *err);

#endif  /* LWQQ_LOGIN_H */
