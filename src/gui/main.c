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

int main(int argc, char *argv[])
{
    LwqqClient *lc = lwqq_client_new("1421032531", "1234567890");
    if (!lc)
        return -1;

    LwqqErrorCode err;
    lwqq_login(lc, &err);
    
    lwqq_client_free(lc);
    return 0;
}
