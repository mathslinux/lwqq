/**
 * @file   login.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 20 02:21:43 2012
 * 
 * @brief  Linux WebQQ Login API
 * 
 * 
 */

#include "login.h"
#include "logger.h"

/** 
 * WebQQ login function
 * 
 * @param client Lwqq Client 
 * @param err Error code
 */
void lwqq_login(LwqqClient *client, LwqqLoginCode *err)
{
    if (!client || !err) {
        lwqq_log(LOG_WARNING, "Invalid pointer\n");
        goto failed;
    }

    /**
     * 0. check_verify_code
     * 1. calculate_password_md5
     * 2. do_login
     * 3. check whether logining successfully
     * 
     */

    return ;
    
failed:
    if (err) {
        *err = LWQQ_LOGIN_ERROR;
    }
}
