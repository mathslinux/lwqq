/**
 * @file   cli.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sat Jun 16 01:58:17 2012
 * 
 * @brief  Command Line Interface for Lwqq
 * 
 * 
 */

#include <string.h>
#include <unistd.h>

#include "login.h"
#include "logger.h"
#include "info.h"
#include "smemory.h"
#include "msg.h"

static char vc_image[128];
static char vc_file[128];

static char *get_vc()
{
    char vc[128] = {0};
    int vc_len;
    FILE *f;

    if ((f = fopen(vc_file, "r")) == NULL) {
        return NULL;
    }

    if (!fgets(vc, sizeof(vc), f)) {
        fclose(f);
        return NULL;
    }
    
    vc_len = strlen(vc);
    if (vc[vc_len - 1] == '\n') {
        vc[vc_len - 1] = '\0';
    }
    return s_strdup(vc);
}

static LwqqErrorCode cli_login(LwqqClient *lc)
{
    LwqqErrorCode err;

    lwqq_login(lc, &err);
    if (err == LWQQ_EC_LOGIN_NEED_VC) {
        snprintf(vc_image, sizeof(vc_image), "/tmp/lwqq_%s.jpeg", lc->username);
        snprintf(vc_file, sizeof(vc_file), "/tmp/lwqq_%s.txt", lc->username);
        /* Delete old verify image */
        unlink(vc_file);

        lwqq_log(LOG_NOTICE, "Need verify code to login, please check "
                 "image file %s, and input what you see to file %s\n",
                 vc_image, vc_file);
        while (1) {
            if (!access(vc_file, F_OK)) {
                sleep(1);
                break;
            }
            sleep(1);
        }
        lc->vc->str = get_vc();
        if (!lc->vc->str) {
            goto failed;
        }
        lwqq_log(LOG_NOTICE, "Get verify code: %s\n", lc->vc->str);
        lwqq_login(lc, &err);
    } else if (err != LWQQ_EC_OK) {
        goto failed;
    }

    return err;

failed:
    return LWQQ_EC_ERROR;
}

static void cli_logout(LwqqClient *lc)
{
    LwqqErrorCode err;
    
    lwqq_logout(lc, &err);
    if (err != LWQQ_EC_OK) {
        lwqq_log(LOG_DEBUG, "Logout failed\n");        
    } else {
        lwqq_log(LOG_DEBUG, "Logout sucessfully\n");
    }
}

int main(int argc, char *argv[])
{
    char *qqnumber = "75396018", *password = "111119907riega";
    LwqqClient *lc;
    LwqqErrorCode err;

    lc = lwqq_client_new(qqnumber, password);
    if (!lc) {
        lwqq_log(LOG_NOTICE, "Create lwqq client failed\n");
        return -1;
    }

    /* Login to server */
    err = cli_login(lc);
    if (err != LWQQ_EC_OK) {
        lwqq_log(LOG_ERROR, "Login error, exit\n");
        return -1;
    }

    lwqq_log(LOG_NOTICE, "Login successfully\n");

    /* Logout */
    sleep(3);
    cli_logout(lc);
    lwqq_client_free(lc);
    return 0;
}
