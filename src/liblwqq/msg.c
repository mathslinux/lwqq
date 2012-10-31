/**
 * @file   msg.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Thu Jun 14 14:42:17 2012
 * 
 * @brief  Message receive and send API
 * 
 * 
 */

#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "type.h"
#include "smemory.h"
#include "json.h"
#include "http.h"
#include "url.h"
#include "logger.h"
#include "msg.h"
#include "queue.h"
#include "unicode.h"

static void *start_poll_msg(void *msg_list);
static void lwqq_recvmsg_poll_msg(struct LwqqRecvMsgList *list);
static json_t *get_result_json_object(json_t *json);
static void parse_recvmsg_from_json(LwqqRecvMsgList *list, const char *str);

static void lwqq_msg_message_free(void *opaque);
static void lwqq_msg_status_free(void *opaque);

/**
 * Create a new LwqqRecvMsgList object
 * 
 * @param client Lwqq Client reference
 * 
 * @return NULL on failure
 */
LwqqRecvMsgList *lwqq_recvmsg_new(void *client)
{
    LwqqRecvMsgList *list;

    list = s_malloc0(sizeof(*list));
    list->lc = client;
    pthread_mutex_init(&list->mutex, NULL);
    SIMPLEQ_INIT(&list->head);
    list->poll_msg = lwqq_recvmsg_poll_msg;
    
    return list;
}

/**
 * Free a LwqqRecvMsgList object
 * 
 * @param list 
 */
void lwqq_recvmsg_free(LwqqRecvMsgList *list)
{
    LwqqRecvMsg *recvmsg;
    
    if (!list)
        return ;

    pthread_mutex_lock(&list->mutex);
    while ((recvmsg = SIMPLEQ_FIRST(&list->head))) {
        SIMPLEQ_REMOVE_HEAD(&list->head, entries);
        lwqq_msg_free(recvmsg->msg);
        s_free(recvmsg);
    }
    pthread_mutex_unlock(&list->mutex);

    s_free(list);
    return ;
}

LwqqMsg *lwqq_msg_new(LwqqMsgType type)
{
    LwqqMsg *msg = NULL;

    msg = s_malloc0(sizeof(*msg));
    msg->type = type;

    switch (type) {
    case LWQQ_MT_BUDDY_MSG:
    case LWQQ_MT_GROUP_MSG:
        msg->opaque = s_malloc0(sizeof(LwqqMsgMessage));
        TAILQ_INIT(&((LwqqMsgMessage*)msg->opaque)->content);
        break;
    case LWQQ_MT_STATUS_CHANGE:
        msg->opaque = s_malloc0(sizeof(LwqqMsgStatusChange));
        break;
    case LWQQ_MT_KICK_MESSAGE:
        msg->opaque = s_malloc0(sizeof(LwqqMsgKickMessage));
        break;
    default:
        lwqq_log(LOG_ERROR, "No such message type\n");
        goto failed;
        break;
    }

    return msg;
failed:
    lwqq_msg_free(msg);
    return NULL;
}

static void lwqq_msg_message_free(void *opaque)
{
    LwqqMsgMessage *msg = opaque;
    if (!msg) {
        return ;
    }

    s_free(msg->from);
    s_free(msg->to);
    s_free(msg->f_name);
    s_free(msg->f_color);

    LwqqMsgContent *c;
    TAILQ_FOREACH(c, &msg->content, entries) {
        if (c->type == LWQQ_CONTENT_STRING) {
            s_free(c->data.str);
        }
        s_free(c);
    }
    
    s_free(msg);
}

static void lwqq_msg_status_free(void *opaque)
{
    LwqqMsgStatusChange *s = opaque;
    if (!s) {
        return ;
    }

    s_free(s->who);
    s_free(s);
}

static void lwqq_msg_kick_free(void *opaque)
{
    LwqqMsgKickMessage *kick = opaque;

    if (!kick) {
        return ;
    }

    s_free(kick->reason);
    s_free(kick->way);
    s_free(kick);
}

/**
 * Free a LwqqMsg object
 * 
 * @param msg 
 */
void lwqq_msg_free(LwqqMsg *msg)
{
    if (!msg)
        return;

    switch (msg->type) {
    case LWQQ_MT_BUDDY_MSG:
    case LWQQ_MT_GROUP_MSG:
        lwqq_msg_message_free(msg->opaque);
        break;
    case LWQQ_MT_STATUS_CHANGE:
        lwqq_msg_status_free(msg->opaque);
        break;
    case LWQQ_MT_KICK_MESSAGE:
        lwqq_msg_kick_free(msg->opaque);
        break;
    default:
        lwqq_log(LOG_ERROR, "No such message type\n");
        break;
    }

    s_free(msg);
}

/**
 * Get the result object in a json object.
 * 
 * @param str
 * 
 * @return result object pointer on success, else NULL on failure.
 */
static json_t *get_result_json_object(json_t *json)
{
    json_t *json_tmp;
    char *value;

    /**
     * Frist, we parse retcode that indicate whether we get
     * correct response from server
     */
    value = json_parse_simple_value(json, "retcode");
    if (!value || strcmp(value, "0")) {
        goto failed ;
    }

    /**
     * Second, Check whether there is a "result" key in json object
     */
    json_tmp = json_find_first_label_all(json, "result");
    if (!json_tmp) {
        goto failed;
    }
    
    return json_tmp;

failed:
    return NULL;
}

static LwqqMsgType parse_recvmsg_type(json_t *json)
{
    LwqqMsgType type = LWQQ_MT_UNKNOWN;
    char *msg_type = json_parse_simple_value(json, "poll_type");
    if (!msg_type) {
        return type;
    }
    if (!strncmp(msg_type, "message", strlen("message"))) {
        type = LWQQ_MT_BUDDY_MSG;
    } else if (!strncmp(msg_type, "group_message", strlen("group_message"))) {
        type = LWQQ_MT_GROUP_MSG;
    } else if (!strncmp(msg_type, "buddies_status_change",
                        strlen("buddies_status_change"))) {
        type = LWQQ_MT_STATUS_CHANGE;
    } else if (!strncmp(msg_type,"kick_message",strlen("kick_message"))){
        type = LWQQ_MT_KICK_MESSAGE;
    }
    return type;
}

static int parse_content(json_t *json, void *opaque)
{
    json_t *tmp, *ctent;
    LwqqMsgMessage *msg = opaque;

    tmp = json_find_first_label_all(json, "content");
    if (!tmp || !tmp->child || !tmp->child) {
        return -1;
    }
    tmp = tmp->child->child;
    for (ctent = tmp; ctent != NULL; ctent = ctent->next) {
        if (ctent->type == JSON_ARRAY) {
            /* ["font",{"size":10,"color":"000000","style":[0,0,0],"name":"\u5B8B\u4F53"}] */
            char *buf;
            /* FIXME: ensure NULL access */
            buf = ctent->child->text;
            if (!strcmp(buf, "font")) {
                const char *name, *color, *size;
                int sa, sb, sc;
                /* Font name */
                name = json_parse_simple_value(ctent, "name");
                name = name ?: "Arial";
                msg->f_name = ucs4toutf8(name);

                /* Font color */
                color = json_parse_simple_value(ctent, "color");
                color = color ?: "000000";
                msg->f_color = s_strdup(color);

                /* Font size */
                size = json_parse_simple_value(ctent, "size");
                size = size ?: "12";
                msg->f_size = atoi(size);

                /* Font style: style":[0,0,0] */
                tmp = json_find_first_label_all(ctent, "style");
                if (tmp) {
                    json_t *style = tmp->child->child;
                    const char *stylestr = style->text;
                    sa = (int)strtol(stylestr, NULL, 10);
                    style = style->next;
                    stylestr = style->text;
                    sb = (int)strtol(stylestr, NULL, 10);
                    style = style->next;
                    stylestr = style->text;
                    sc = (int)strtol(stylestr, NULL, 10);
                } else {
                    sa = 0;
                    sb = 0;
                    sc = 0;
                }
                msg->f_style.a = sa;
                msg->f_style.b = sb;
                msg->f_style.b = sc;
            } else if (!strcmp(buf, "face")) {
                /* ["face", 107] */
                /* FIXME: ensure NULL access */
                int facenum = (int)strtol(ctent->child->next->text, NULL, 10);
                LwqqMsgContent *c = s_malloc0(sizeof(*c));
                c->type = LWQQ_CONTENT_FACE;
                c->data.face = facenum; 
                TAILQ_INSERT_TAIL(&msg->content, c, entries);
            }
        } else if (ctent->type == JSON_STRING) {
            LwqqMsgContent *c = s_malloc0(sizeof(*c));
            c->type = LWQQ_CONTENT_STRING;
            c->data.str = ucs4toutf8(ctent->text);
            TAILQ_INSERT_TAIL(&msg->content, c, entries);
        }
    }

    /* Make msg valid */
    if (!msg->f_name || !msg->f_color || TAILQ_EMPTY(&msg->content)) {
        return -1;
    }
    if (msg->f_size < 10) {
        msg->f_size = 10;
    }

    return 0;
}

/**
 * {"poll_type":"message","value":{"msg_id":5244,"from_uin":570454553,
 * "to_uin":75396018,"msg_id2":395911,"msg_type":9,"reply_ip":176752041,
 * "time":1339663883,"content":[["font",{"size":10,"color":"000000",
 * "style":[0,0,0],"name":"\u5B8B\u4F53"}],"hello\n "]}}
 * 
 * @param json
 * @param opaque
 * 
 * @return
 */
static int parse_new_msg(json_t *json, void *opaque)
{
    LwqqMsgMessage *msg = opaque;
    char *time;
    
    msg->from = s_strdup(json_parse_simple_value(json, "from_uin"));
    if (!msg->from) {
        return -1;
    }

    time = json_parse_simple_value(json, "time");
    time = time ?: "0";
    msg->time = (time_t)strtoll(time, NULL, 10);

    msg->to = s_strdup(json_parse_simple_value(json, "to_uin"));
    if (!msg->to) {
        return -1;
    }
    
    if (parse_content(json, opaque)) {
        return -1;
    }

    return 0;
}

/**
 * {"poll_type":"buddies_status_change",
 * "value":{"uin":570454553,"status":"offline","client_type":1}}
 * 
 * @param json
 * @param opaque
 * 
 * @return 
 */
static int parse_status_change(json_t *json, void *opaque)
{
    LwqqMsgStatusChange *msg = opaque;
    char *c_type;

    msg->who = s_strdup(json_parse_simple_value(json, "uin"));
    if (!msg->who) {
        return -1;
    }
    msg->status = s_strdup(json_parse_simple_value(json, "status"));
    if (!msg->status) {
        return -1;
    }
    c_type = s_strdup(json_parse_simple_value(json, "client_type"));
    c_type = c_type ?: "1";
    msg->client_type = atoi(c_type);

    return 0;
}
/**
 * {"poll_type":"kick_message","value":
 * {"way":"do_poll","show_reason":1,"reason":
"\u60A8\u7684\u8D26\u53F7\u5728\u53E6\u4E00\u5730\u70B9\u767B\u5F55\uFF0C\u60A8\u5DF2\u88AB\u8FEB\u4E0B\u7EBF\u3002\u5982\u6709\u7591\u95EE\uFF0C\u8BF7\u767B\u5F55 safe.qq.com \u4E86\u89E3\u66F4\u591A\u3002"}
 * 
 * @param json
 * @param opaque
 * 
 * @return 
 */
static int parse_kick_message(json_t *json,void *opaque)
{
    LwqqMsgKickMessage *msg = opaque;
    char* show;
    show = json_parse_simple_value(json,"show_reason");
    if(show)msg->show_reason = atoi(show);
    msg->reason = ucs4toutf8(json_parse_simple_value(json,"reason"));
    if(!msg->reason){
        if(!show) msg->show_reason = 0;
        else return -1;
    }
    return 0;
}

/**
 * Parse message received from server
 * Buddy message:
 * {"retcode":0,"result":[{"poll_type":"message","value":{"msg_id":5244,"from_uin":570454553,"to_uin":75396018,"msg_id2":395911,"msg_type":9,"reply_ip":176752041,"time":1339663883,"content":[["font",{"size":10,"color":"000000","style":[0,0,0],"name":"\u5B8B\u4F53"}],"hello\n "]}}]}
 * 
 * Message for Changing online status:
 * {"retcode":0,"result":[{"poll_type":"buddies_status_change","value":{"uin":570454553,"status":"offline","client_type":1}}]}
 * 
 * 
 * @param list 
 * @param response 
 */
static void parse_recvmsg_from_json(LwqqRecvMsgList *list, const char *str)
{
    int ret;
    json_t *json = NULL, *json_tmp, *cur;

    ret = json_parse_document(&json, (char *)str);
    if (ret != JSON_OK) {
        lwqq_log(LOG_ERROR, "Parse json object of friends error: %s\n", str);
        goto done;
    }

    json_tmp = get_result_json_object(json);
    if (!json_tmp) {
        lwqq_log(LOG_ERROR, "Parse json object error: %s\n", str);
        goto done;
    }

    if (!json_tmp->child || !json_tmp->child->child) {
        goto done;
    }

    /* make json_tmp point to first child of "result" */
    json_tmp = json_tmp->child->child;
    for (cur = json_tmp; cur != NULL; cur = cur->next) {
        LwqqMsg *msg = NULL;
        LwqqMsgType msg_type;
        int ret;
        
        msg_type = parse_recvmsg_type(cur);
        msg = lwqq_msg_new(msg_type);
        if (!msg) {
            continue;
        }

        switch (msg_type) {
        case LWQQ_MT_BUDDY_MSG:
        case LWQQ_MT_GROUP_MSG:
            ret = parse_new_msg(cur, msg->opaque);
            break;
        case LWQQ_MT_STATUS_CHANGE:
            ret = parse_status_change(cur, msg->opaque);
            break;
        case LWQQ_MT_KICK_MESSAGE:
            ret = parse_kick_message(cur,msg->opaque);
            break;
        default:
            ret = -1;
            lwqq_log(LOG_ERROR, "No such message type\n");
            break;
        }

        if (ret == 0) {
            LwqqRecvMsg *rmsg = s_malloc0(sizeof(*rmsg));
            rmsg->msg = msg;
            /* Parse a new message successfully, link it to our list */
            pthread_mutex_lock(&list->mutex);
            SIMPLEQ_INSERT_TAIL(&list->head, rmsg, entries);
            pthread_mutex_unlock(&list->mutex);
        } else {
            lwqq_msg_free(msg);
        }
    }
    
done:
    if (json) {
        json_free_value(&json);
    }
}

/**
 * Poll to receive message.
 * 
 * @param list
 */
static void *start_poll_msg(void *msg_list)
{
    LwqqClient *lc;
    LwqqHttpRequest *req = NULL;  
    int ret;
    char *cookies;
    char *s;
    char msg[1024];
    LwqqRecvMsgList *list;

    list = (LwqqRecvMsgList *)msg_list;
    lc = (LwqqClient *)(list->lc);
    if (!lc) {
        goto failed;
    }
    snprintf(msg, sizeof(msg), "{\"clientid\":\"%s\",\"psessionid\":\"%s\"}",
             lc->clientid, lc->psessionid);
    s = url_encode(msg);
    snprintf(msg, sizeof(msg), "r=%s", s);
    s_free(s);

    /* Create a POST request */
    char url[512];
    snprintf(url, sizeof(url), "%s/channel/poll2", "http://d.web2.qq.com");
    req = lwqq_http_create_default_request(url, NULL);
    if (!req) {
        goto failed;
    }
    req->set_header(req, "Referer", "http://d.web2.qq.com/proxy.html?v=20101025002");
    req->set_header(req, "Content-Transfer-Encoding", "binary");
    req->set_header(req, "Content-type", "application/x-www-form-urlencoded");
    cookies = lwqq_get_cookies(lc);
    if (cookies) {
        req->set_header(req, "Cookie", cookies);
        s_free(cookies);
    }
    while(1) {
        ret = req->do_request(req, 1, msg);
        if (ret || req->http_code != 200) {
            continue;
        }
        parse_recvmsg_from_json(list, req->response);
    }
failed:
    pthread_exit(NULL);
}

static void lwqq_recvmsg_poll_msg(LwqqRecvMsgList *list)
{
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&tid, &attr, start_poll_msg, list);
}

/* FIXME: So much hard code */
char *create_default_content(const char *content)
{
    char s[2048];

    snprintf(s, sizeof(s), "\"[\\\"%s\\\\n\\\",[\\\"font\\\","
             "{\\\"name\\\":\\\"宋体\\\",\\\"size\\\":\\\"10\\\","
             "\\\"style\\\":[0,0,0],\\\"color\\\":\\\"000000\\\"}]]\"", content);
    return strdup(s);
}

/** 
 * 
 * 
 * @param lc 
 * @param sendmsg 
 * 
 * @return 
 */
int lwqq_msg_send(void *client, LwqqMsg *msg)
{
    int ret;
    LwqqHttpRequest *req = NULL;  
    char *cookies;
    char *s;
    char *content = NULL;
    char data[1024];
    LwqqMsgMessage *mmsg;
    LwqqMsgContent *c;
    char str[1024] = {0};
    LwqqClient *lc = client;

    if (!msg || (msg->type != LWQQ_MT_BUDDY_MSG &&
                 msg->type != LWQQ_MT_GROUP_MSG)) {
        goto failed;
    }
    mmsg = msg->opaque;
    TAILQ_FOREACH(c, &mmsg->content, entries) {
        if (c->type == LWQQ_CONTENT_STRING) {
            strcat(str, c->data.str);
        }
    }
    if (!strlen(str)) {
        goto failed;
    }
    content = create_default_content(str);
    snprintf(data, sizeof(data), "{\"to\":%s,\"face\":0,\"content\":%s,"
             "\"msg_id\":%ld,\"clientid\":\"%s\",\"psessionid\":\"%s\"}",
             mmsg->to, content, lc->msg_id, lc->clientid, lc->psessionid);
    s_free(content);
    s = url_encode(data);
    snprintf(data, sizeof(data), "r=%s", s);
    s_free(s);

    /* Create a POST request */
    char url[512];
    snprintf(url, sizeof(url), "%s/channel/send_buddy_msg2", "http://d.web2.qq.com");
    req = lwqq_http_create_default_request(url, NULL);
    if (!req) {
        goto failed;
    }
    req->set_header(req, "Referer", "http://d.web2.qq.com/proxy.html?v=20101025002");
    req->set_header(req, "Content-Transfer-Encoding", "binary");
    req->set_header(req, "Content-type", "application/x-www-form-urlencoded");
    cookies = lwqq_get_cookies(lc);
    if (cookies) {
        req->set_header(req, "Cookie", cookies);
        s_free(cookies);
    }
    
    ret = req->do_request(req, 1, data);
    if (ret || req->http_code != 200) {
        goto failed;
    }
    return 0;

failed:
    lwqq_http_request_free(req);
    return -1;
}

int lwqq_msg_send2(void *client, const char *to, const char *content)
{
    int ret = 0;
    LwqqMsg *msg = NULL;
    LwqqMsgMessage *mmsg = NULL;
    LwqqMsgContent *c = NULL;
    LwqqClient *lc = client;
    
    if (!lc || !to || !content) {
        return -1;
    }
    
    msg = lwqq_msg_new(LWQQ_MT_BUDDY_MSG);
    if (!msg) {
        return -1;
    }

    mmsg = msg->opaque;
    mmsg->to = s_strdup(to);
    c = s_malloc0(sizeof(*c));
    c->type = LWQQ_CONTENT_STRING;
    c->data.str = s_strdup(content);
    TAILQ_INSERT_HEAD(&mmsg->content, c, entries);

    ret = lwqq_msg_send(lc, msg);
    lwqq_msg_free(msg);
    return ret;
}
