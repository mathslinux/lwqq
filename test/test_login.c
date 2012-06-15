/**
 * @file   test_login.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sat Jun  9 20:06:56 2012
 * 
 * @brief  Lwqq login test module 
 * 
 * 
 */

#include <string.h>
#include <unistd.h>

#include "login.h"
#include "logger.h"
#include "info.h"
#include "smemory.h"

static char *get_vc()
{
    char buf[1024] = {0};
    
    FILE *f = fopen("/tmp/test.txt", "r");
    if (!f)
        return NULL;

    char *i = fgets(buf, sizeof(buf), f);
    if (!i)
        return NULL;
    fclose(f);
    int len = strlen(buf);
    buf[len - 1] = '\0';
    printf ("%s\n", i);
    return s_strdup(buf);
}

static void test_login()
{
    LwqqClient *lc = lwqq_client_new("1421032531", "1234567890");
    if (!lc)
        return ;

    LwqqErrorCode err;
    lwqq_login(lc, &err);
    if (err == LWQQ_EC_LOGIN_NEED_VC) {
        while (1) {
            if (!access("/tmp/test.txt", F_OK)) {
                sleep(1);
                break;
            }
            sleep(1);
        }
        lc->vc->str = get_vc();
        printf ("get vc: %s\n", lc->vc->str);

        lwqq_login(lc, &err);
    } else if (err != LWQQ_EC_OK) {
        lwqq_log(LOG_ERROR, "Login error, exit\n");
        goto done;
    }
    lwqq_log(LOG_NOTICE, "Login successfully\n");
    
    lwqq_info_get_friends_info(lc, &err);

    if (err == LWQQ_EC_OK) {
        LwqqBuddy *buddy;
        LIST_FOREACH(buddy, &lc->friends, entries) {
            char buf[128] = {0};
            if (buddy->qqnumber) {
                strcat(buf, "qqnumber:");
                strcat(buf, buddy->qqnumber);
                strcat(buf, ", ");
            }
            if (buddy->nick) {
                strcat(buf, "nick:");
                strcat(buf, buddy->nick);
                strcat(buf, ", ");
            }
            if (buddy->uin) {
                strcat(buf, "uin:");
                strcat(buf, buddy->uin);
                strcat(buf, ", ");
            }
            lwqq_log(LOG_DEBUG, "Buddy info: %s\n", buf);
        }
    }

    lwqq_info_get_group_name_list(lc, &err);
       
    if (err == LWQQ_EC_OK) {
        LwqqGroup *group;
        LIST_FOREACH(group, &lc->groups, entries) {
            if (group->name)
                lwqq_log(LOG_DEBUG, "Group name: %s\n", group->name);
        }
    }

    lwqq_info_get_friend_detail_info(lc, lc->myself, &err);
    
    /* Logout test */
    sleep(1);
    lwqq_logout(lc, &err);
    if (err != LWQQ_EC_OK) {
        lwqq_log(LOG_DEBUG, "Logout failed\n");        
    } else {
        lwqq_log(LOG_DEBUG, "Logout sucessfully\n");
    }
    
done:
    lwqq_client_free(lc);
}

int main(int argc, char *argv[])
{
    test_login();
    return 0;
}

