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

#include "lwqq.h"

#ifdef WIN32
#include <windows.h>
#endif

#define LWQQ_CLI_VERSION "0.0.1"


static int help_f(int argc, char **argv);
static int quit_f(int argc, char **argv);
static int list_f(int argc, char **argv);
static int send_f(int argc, char **argv);

typedef int (*cfunc_t)(int argc, char **argv);

typedef struct CmdInfo {
	const char	*name;
	const char	*altname;
	cfunc_t		cfunc;
} CmdInfo;

static LwqqClient *lc = NULL;

static char *progname;

static CmdInfo cmdtab[] = {
    {"help", "h", help_f},
    {"quit", "q", quit_f},
    {"list", "l", list_f},
    {"send", "s", send_f},
    {NULL, NULL, NULL},
};
#ifdef WIN32
char *strtok_r(char *str, const char *delim, char **save)
{
    char *res, *last;

    if( !save )
        return strtok(str, delim);
    if( !str && !(str = *save) )
        return NULL;
    last = str + strlen(str);
    if( (*save = res = strtok(str, delim)) )
    {
        *save += strlen(res);
        if( *save < last )
            (*save)++;
        else
            *save = NULL;
    }
    return res;
}
const char* iconv(unsigned int from,unsigned int to,const char* str,size_t sz)
{
	static char buf[2048];
	wchar_t wbuf[2048];
	MultiByteToWideChar(from,0,str,-1,wbuf,sizeof(wbuf));
	WideCharToMultiByte(to,0,wbuf,-1,buf,sizeof(buf),NULL,NULL);
	return buf;
}
#define charset(str) iconv(CP_UTF8,CP_OEMCP,str,-1)
#else
#define charset(str) str
#endif


static int help_f(int argc, char **argv)
{
    printf(
        "\n"
        " Excute a command\n"
        "\n"
        " help/h, -- Output help\n"
        " list/l, -- List buddies\n"
        "            You can use \"list all\" to list all buddies\n"
        "            or use \"list uin\" to list certain buddy\n"
        " send/s, -- Send message to buddy\n"
        "            You can use \"send uin message\" to send message\n"
        "            to buddy"
        "\n");
    
    return 0;
}

static int quit_f(int argc, char **argv)
{
    return 1;
}

static int list_f(int argc, char **argv)
{
    char buf[1024] = {0};

    /** argv may like:
     * 1. {"list", "all"}
     * 2. {"list", "244569070"}
     */
    if (argc != 2) {
        return 0;
    }

    if (!strcmp(argv[1], "all")) {
        /* List all buddies */
        LwqqBuddy *buddy;
        LIST_FOREACH(buddy, &lc->friends, entries) {
            if (!buddy->uin) {
                /* BUG */
                return 0;
            }
            snprintf(buf, sizeof(buf), "uin:%s, ", buddy->uin);
            if (buddy->nick) {
                strcat(buf, "nick:");
                strcat(buf, buddy->nick);
                strcat(buf, ", ");
            }
            printf("Buddy info: %s\n", buf);
        }
    } else {
        /* Show buddies whose uin is argv[1] */
        LwqqBuddy *buddy;
        LIST_FOREACH(buddy, &lc->friends, entries) {
            if (buddy->uin && !strcmp(buddy->uin, argv[1])) {
                snprintf(buf, sizeof(buf), "uin:%s, ", argv[1]);
                if (buddy->nick) {
                    strcat(buf, "nick:");
                    strcat(buf, buddy->nick);
                    strcat(buf, ", ");
                }
                if (buddy->markname) {
                    strcat(buf, "markname:");
                    strcat(buf, buddy->markname);
                }
                printf("Buddy info: %s\n", buf);
                break;
            }
        }
    }
	

    return 0;
}

static int send_f(int argc, char **argv)
{
    /* argv look like: {"send", "74357485" "hello"} */
    if (argc != 3) {
        return 0;
    }
    
    lwqq_msg_send_simple(lc,LWQQ_MS_BUDDY_MSG, argv[1], argv[2]);
    
    return 0;
}

static char *get_prompt(void)
{
	static char	prompt[256];

	if (!prompt[0])
		snprintf(prompt, sizeof(prompt), "%s> ", progname);
	return prompt;
}

static LwqqErrorCode cli_login()
{
    LwqqErrorCode err;

    LWQQ_SYNC_BEGIN(lc);
    lwqq_login(lc,LWQQ_STATUS_ONLINE, &err);

    if (err != LWQQ_EC_OK) {
        goto failed;
    }

    LWQQ_SYNC_END(lc);
    return err;

failed:
    LWQQ_SYNC_END(lc);
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

static void usage()
{
    fprintf(stdout, "Usage: lwqq-cli [options]...\n"
            "lwqq-cli: A qq client based on gtk+ uses webqq protocol\n"
            "  -v, --version\n"
            "      Show version of program\n"
            "  -u, --user\n"
            "      Set username(qqnumber)\n"
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

static void handle_new_msg(LwqqRecvMsg *recvmsg)
{
    LwqqMsg *msg = recvmsg->msg;
	char buf[2048] = {0};

    printf("Receive message type: %d\n", msg->type);
    if (msg->type == LWQQ_MS_BUDDY_MSG) {
        
        LwqqMsgContent *c;
        LwqqMsgMessage *mmsg = (LwqqMsgMessage*)msg;
        TAILQ_FOREACH(c, &mmsg->content, entries) {
            if (c->type == LWQQ_CONTENT_STRING) {
                strcat(buf, c->data.str);
            } else {
                printf ("Receive face msg: %d\n", c->data.face);
            }
        }
        printf("Receive message: %s\n", charset(buf));
    } else if (msg->type == LWQQ_MS_GROUP_MSG) {
        LwqqMsgMessage *mmsg = (LwqqMsgMessage*)msg;
        
        LwqqMsgContent *c;
        TAILQ_FOREACH(c, &mmsg->content, entries) {
            if (c->type == LWQQ_CONTENT_STRING) {
                strcat(buf, c->data.str);
            } else {
                printf ("Receive face msg: %d\n", c->data.face);
            }
        }
        printf("Receive message: %s\n", charset(buf));
    } else if (msg->type == LWQQ_MT_STATUS_CHANGE) {
        LwqqMsgStatusChange *status = (LwqqMsgStatusChange*)msg;
        printf("Receive status change: %s - > %s\n", 
               status->who,
               status->status);
    } else {
        printf("unknow message\n");
    }
    
    lwqq_msg_free(recvmsg->msg);
    s_free(recvmsg);
}

static void *recvmsg_thread(void *list)
{
    LwqqRecvMsgList *l = (LwqqRecvMsgList *)list;

    /* Poll to receive message */
    lwqq_msglist_poll(l, 0);

    /* Need to wrap those code so look like more nice */
    while (1) {
        LwqqRecvMsg *recvmsg;
        pthread_mutex_lock(&l->mutex);
        if (TAILQ_EMPTY(&l->head)) {
            /* No message now, wait 100ms */
            pthread_mutex_unlock(&l->mutex);
            usleep(100000);
            continue;
        }
        recvmsg = TAILQ_FIRST(&l->head);
        TAILQ_REMOVE(&l->head,recvmsg, entries);
        pthread_mutex_unlock(&l->mutex);
        handle_new_msg(recvmsg);
		fflush(stdout);
    }

    pthread_exit(NULL);
    return NULL;
}

static void *info_thread(void *lc)
{
    LwqqErrorCode err;
    lwqq_info_get_friends_info(lc,NULL,&err);

    pthread_exit(NULL);
    return NULL;
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
        rval[c - 1] = token;
        rval[c] = NULL;
        token = strtok_r(NULL, " ", &save_ptr);
	}
    
    *count = c;

    if (c == 0) {
        free(rval);
        return NULL;
    }
    
    return rval;
}

const CmdInfo *find_command(const char *cmd)
{
	CmdInfo	*ct;

	for (ct = cmdtab; ct->name; ct++) {
		if (!strcmp(ct->name, cmd) || !strcmp(ct->altname, cmd)) {
			return (const CmdInfo *)ct;
        }
	}
	return NULL;
}

static void command_loop()
{
    static char command[1024];
    int done = 0;

    while (!done) {
        char **v;
        char *p;
        int c = 0;
        const CmdInfo *ct;
        fprintf(stdout, "%s", get_prompt());
        fflush(stdout);
        memset(&command, 0, sizeof(command));
        if (!fgets(command, sizeof(command), stdin)) {
            /* Avoid gcc warning */
            continue;
        }
        p = command + strlen(command);
        if (p != command && p[-1] == '\n') {
            p[-1] = '\0';
        }
        
        v = breakline(command, &c);
        if (v) {
            ct = find_command(v[0]);
            if (ct) {
                done = ct->cfunc(c, v);
            } else {
                fprintf(stderr, "command \"%s\" not found\n", v[0]);
            }
            free(v);
        }
		fflush(stdout);
		fflush(stderr);
    }
}

static void need_verify2(LwqqClient* lc,LwqqVerifyCode* code)
{
    #ifdef WIN32
    const char *dir = NULL;
    
    #else
    const char *dir = "/tmp";
    #endif
    char fname[32];
    char vcode[256] = {0};
    snprintf(fname,sizeof(fname),"%s.jpeg",lc->username);

    lwqq_util_save_img(code->data,code->size,fname,dir);

    lwqq_log(LOG_NOTICE,"Need verify code to login, please check "
            "image file %s%s, and input below.\n",
            dir?:"",fname);
    printf("Verify Code:");
	fflush(stdout);
    scanf("%s",vcode);
    code->str = s_strdup(vcode);
    vp_do(code->cmd,NULL);
}
/**fix mingw and mintty and utf-8 no output */
static void log_direct_flush(int l,const char* str)
{
	fprintf(stderr,"%s\n",str);
	fflush(stderr);
}

static LwqqAction act = {
    .need_verify2 = need_verify2
};

int main(int argc, char *argv[])
{

	lwqq_log_redirect(log_direct_flush);

    char *qqnumber = NULL, *password = NULL;
    LwqqErrorCode err;
    int i, c, e = 0;
    pthread_t tid[2];
    pthread_attr_t attr[2];
    
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
    
    lwqq_log_set_level(4);
    lc = lwqq_client_new(qqnumber, password);
    lc->action = &act;
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

    /* Initialize thread */
    for (i = 0; i < 2; ++i) {
        pthread_attr_init(&attr[i]);
        pthread_attr_setdetachstate(&attr[i], PTHREAD_CREATE_DETACHED);
    }

    /* Create a thread to receive message */
    pthread_create(&tid[0], &attr[0], recvmsg_thread, lc->msg_list);

    /* Create a thread to update friend info */
    pthread_create(&tid[1], &attr[1], info_thread, lc);

    /* Enter command loop  */
    command_loop();
    
    /* Logout */
    cli_logout(lc);
    lwqq_client_free(lc);
    return 0;
}
