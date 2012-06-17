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
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <libgen.h>
#include <pthread.h>

#include "login.h"
#include "logger.h"
#include "info.h"
#include "smemory.h"
#include "msg.h"

#define LWQQ_CLI_VERSION "0.0.1"

static LwqqClient *lc = NULL;

static char vc_image[128];
static char vc_file[128];

static char *progname;

static char *get_prompt(void)
{
	static char	prompt[256];

	if (!prompt[0])
		snprintf(prompt, sizeof(prompt), "%s> ", progname);
	return prompt;
}

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

static LwqqErrorCode cli_login()
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

static void cli_logout()
{
    LwqqErrorCode err;
    
    lwqq_logout(lc, &err);
    if (err != LWQQ_EC_OK) {
        lwqq_log(LOG_DEBUG, "Logout failed\n");        
    } else {
        lwqq_log(LOG_DEBUG, "Logout sucessfully\n");
    }
}

static void usage()
{
    fprintf(stdout, "Usage: lwqq-cli [options]...\n"
            "lwqq-cli: A qq client based on gtk+ uses webqq protocol\n"
            "  -v, --version\n"
            "      Show version of program\n"
            "  -u, --user\n"
            "      Set username(qqnumer)\n"
            "  -p, --pwd\n"
            "      Set password\n"
            "  -h, --help\n"
            "      Print help and exit\n"
        );
}

void signal_handler(int signum)
{
	if (signum == SIGINT) {
        cli_logout(lc);
        lwqq_client_free(lc);
        exit(0);
	}
}

static void *recvmsg_thread(void *list)
{
    LwqqRecvMsgList *l = (LwqqRecvMsgList *)list;

    /* Poll to receive message */
    l->poll_msg(l);
    
    /* Need to wrap those code so look like more nice */
    while (1) {
        sleep(1);
        LwqqRecvMsg *msg;
        pthread_mutex_lock(&l->mutex);
        if (!SIMPLEQ_EMPTY(&l->head)) {
            msg = SIMPLEQ_FIRST(&l->head);
            if (msg->msg->content) {
                printf("Receive message: %s\n", msg->msg->content);
            }
            SIMPLEQ_REMOVE_HEAD(&l->head, entries);
        }
        pthread_mutex_unlock(&l->mutex);
    }

    pthread_exit(NULL);
}

static char **breakline(char *input, int *count)
{
    int c = 0;
    char **rval = calloc(sizeof(char *), 1);
    char **tmp;
    char *token, *save_ptr;

    token = strtok_r(input, " ", &save_ptr);
	while (token) {
        c++;
        tmp = realloc(rval, sizeof(*rval) * (c + 1));
        rval = tmp;
        rval[c - 1] = strdup(token);
        rval[c] = NULL;
        token = strtok_r(NULL, " ", &save_ptr);
	}
    
    *count = c;
    return rval;
}

static void handle_command(const char *command)
{
    
}

int main(int argc, char *argv[])
{
    char *qqnumber = NULL, *password = NULL;
    LwqqErrorCode err;
    int c, e = 0;
    pthread_t tid;
    pthread_attr_t attr;
    
    if (argc == 1) {
        usage();
        exit(1);
    }

    progname = basename(argv[0]);

    const struct option long_options[] = {
        { "version", 0, 0, 'v' },
        { "help", 0, 0, 'h' },
        { "user", 0, 0, 'u' },
        { "pwd", 0, 0, 'p' },
        { 0, 0, 0, 0 }
    };

    /* Lanuch signal handler when user press down Ctrl-C in terminal */
    signal(SIGINT, signal_handler);
    
    while ((c = getopt_long(argc, argv, "vhu:p:",
                            long_options, NULL)) != EOF) {
        switch (c) {
        case 'v':
            printf("lwqq-cli version %s, Copyright (c) 2012 "
                   "mathslinux\n", LWQQ_CLI_VERSION);
            exit(0);
            
        case 'h':
            usage();
            exit(0);
            
        case 'u':
            qqnumber = optarg;
            break;
            
        case 'p':
            password = optarg;
            break;
            
        default:
            e++;
            break;
        }
    }
    if (e || argc > optind) {
        usage();
        exit(1);
    }
    
    lc = lwqq_client_new(qqnumber, password);
    if (!lc) {
        lwqq_log(LOG_NOTICE, "Create lwqq client failed\n");
        return -1;
    }

    /* Login to server */
    err = cli_login();
    if (err != LWQQ_EC_OK) {
        lwqq_log(LOG_ERROR, "Login error, exit\n");
        lwqq_client_free(lc);
        return -1;
    }

    lwqq_log(LOG_NOTICE, "Login successfully\n");

    /* Create a thread to receive message */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&tid, &attr, recvmsg_thread, lc->msg_list);

    while (1) {
        char buf[128];
        char **v;
        int c = 0;
        fprintf(stdout, "%s", get_prompt());
        fflush(stdout);
        if (!fgets(buf, sizeof(buf), stdin)) {
            /* Avoid gcc warning */
            continue;
        }
        v = breakline(buf, &c);
        handle_command(buf);
    }
    
    /* Logout */
    sleep(3);
    cli_logout(lc);
    lwqq_client_free(lc);
    return 0;
}
