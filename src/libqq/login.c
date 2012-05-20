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
 * @param info Client information
 * @param err Error code
 */
void lwqq_login(LwqqInfo *info, LwqqLoginCode *err)
{
    if (!info || !err) {
        lwqq_log(LOG_WARNING, "Invalid pointer\n");
        goto failed;
    }

    /**
     * 1. get_pwvc_md5
     * 2. get_ptcz_skey
     * 3. check whether logining successfully
     * 
     */

    return ;
    
failed:
    if (err) {
        err = LWQQ_LOGIN_ERROR;
    }
}
