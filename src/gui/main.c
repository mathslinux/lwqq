/**
 * @file   main.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Mon May 21 23:29:20 2012
 * 
 * @brief  
 * 
 * 
 */

#include "login.h"
#include "logger.h"
#include "info.h"

int main(int argc, char *argv[])
{
    LwqqClient *lc = lwqq_client_new("1421032531", "1234567890");
    if (!lc)
        return -1;

    LwqqErrorCode err;
    lwqq_login(lc, &err);
    if (err != LWQQ_OK) {
        lwqq_log(LOG_ERROR, "Login error, exit\n");
        goto done;
    }
    lwqq_log(LOG_NOTICE, "Login successfully\n");
    
    lwqq_info_get_friends_info(lc, &err);

done:
    lwqq_client_free(lc);
    return 0;
}
