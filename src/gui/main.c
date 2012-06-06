/**
 * @file   main.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Mon May 21 23:29:20 2012
 * 
 * @brief  
 * 
 * 
 */

#include <gtk/gtk.h>
#include <unistd.h>
#include "login.h"
#include "logger.h"
#include "info.h"
#include "smemory.h"
#include "mainwindow.h"
#include <string.h>

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

char *s = NULL;
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
            if (buddy->nick)
                lwqq_log(LOG_DEBUG, "Nick: %s\n", buddy->nick);
        }
    }

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
#if 0
    GtkWidget *main_win = NULL;
    
    gtk_init(&argc, &argv);

    main_win = qq_mainwindow_new();
    gtk_widget_show_all(main_win);
    gtk_main();
#endif

    s = argv[1];
    test_login();
    return 0;
}
