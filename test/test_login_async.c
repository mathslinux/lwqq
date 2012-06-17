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
#include "msg.h"
#include "async.h"

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
pthread_mutex_t mutex;
void wake_up(LwqqClient*lc,LwqqHttpRequest*request,void*data)
{
    pthread_mutex_unlock(&mutex);
}

static void test_login(const char *qqnumber, const char *password)
{
    LwqqClient *lc = lwqq_client_new(qqnumber, password);
    if (!lc)
        return ;

    LwqqErrorCode err;
    pthread_mutex_init(&mutex,NULL);
    lwqq_async_set(lc,1);
    lwqq_async_add_listener(lc,LOGIN_COMPLETE,wake_up,&mutex);
    lwqq_login(lc, &err);
    printf("hi\n");
    pthread_mutex_lock(&mutex);
    pthread_mutex_lock(&mutex);

    if (err == LWQQ_EC_LOGIN_NEED_VC) {
        unlink("/tmp/test.txt");
        /*while (1) {
            if (!access("/tmp/test.txt", F_OK)) {
                sleep(1);
                break;
            }
            sleep(1);
        }*/
        char vc[5];
        scanf("%s",vc);
        lc->vc->str = get_vc();
        printf ("get vc: %s\n", lc->vc->str);

        lwqq_login(lc, &err);
    } else if (err != LWQQ_EC_OK) {
        lwqq_log(LOG_ERROR, "Login error, exit\n");
        goto done;
    }
    lwqq_log(LOG_NOTICE, "Login successfully\n");

#if 0
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
            char buf[256] = {0};

            if (group->name) {
                strcat(buf, "name:");
                strcat(buf, group->name);
                strcat(buf, ", ");
            }
            
            if (group->gid) {
                strcat(buf, "gid:");
                strcat(buf, group->gid);
                strcat(buf, ", ");
            }

            if (group->code) {
                strcat(buf, "code:");
                strcat(buf, group->code);
                strcat(buf, ", ");
            }

            if (group->markname) {
                strcat(buf, "markname:");
                strcat(buf, group->markname);
                strcat(buf, ", ");
            }

            if (group->account) {
                strcat(buf, "account:");
                strcat(buf, group->account);
                strcat(buf, ", ");
            }

            if (group->owner) {
                strcat(buf, "owner:");
                strcat(buf, group->owner);
                strcat(buf, ", ");
            }
            
            if (group->memo) {
                strcat(buf, "memo:");
                strcat(buf, group->memo);
                strcat(buf, ", ");
            }

            if (group->fingermemo) {
                strcat(buf, "fingermemo:");
                strcat(buf, group->fingermemo);
                strcat(buf, ", ");
            }

            if (group->level) {
                strcat(buf, "level:");
                strcat(buf, group->level);
                strcat(buf, ", ");
            }

            if (group->createtime) {
                strcat(buf, "createtime:");
                strcat(buf, group->createtime);
                strcat(buf, ", ");
            }

            if (group->face) {
                strcat(buf, "face:");
                strcat(buf, group->face);
                strcat(buf, ", ");
            }

            if (group->flag) {
                strcat(buf, "flag:");
                strcat(buf, group->flag);
                strcat(buf, ", ");
            }
            
         lwqq_log(LOG_DEBUG, "Group info: %s\n", buf);
        }
    }

    lwqq_info_get_friend_detail_info(lc, lc->myself, &err);
#endif 

    lc->msg_list->poll_msg(lc->msg_list);

    while (1) {
        usleep(100);
        LwqqRecvMsg *msg;
        pthread_mutex_lock(&lc->msg_list->mutex);
        if (!SIMPLEQ_EMPTY(&lc->msg_list->head)) {
            msg = SIMPLEQ_FIRST(&lc->msg_list->head);
            if (msg->msg->content) {
                printf ("########################content: %s\n", msg->msg->content);
            }
            SIMPLEQ_REMOVE_HEAD(&lc->msg_list->head, entries);
        }
        pthread_mutex_unlock(&lc->msg_list->mutex);
    }
//    lwqq_msg_poll(lc);
    
    /* Logout test */
    sleep(2);
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
    if (argc != 3) {
        return -1;
    }
    test_login(argv[1], argv[2]);
    return 0;
}

