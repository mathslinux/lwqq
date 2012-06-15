/**
 * @file   msg.h
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Thu Jun 14 14:42:08 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef LWQQ_MSG_H
#define LWQQ_MSG_H

#include <pthread.h>

/**
 * Lwqq Message object, used by receiving and sending message
 * 
 */
typedef struct LwqqMsg {
    char *from;                 /**< Message sender(qqnumber) */
    char *to;                   /**< Message receiver(qqnumber) */
    
    /* Message type. e.g. buddy message or group message */
    char *msg_type;

    char *content;              /**< Message content */
} LwqqMsg;

/** 
 * Create a new LwqqMsg object
 * 
 * @param from 
 * @param to 
 * @param msg_type 
 * @param content 
 * 
 * @return NULL on failure
 */
LwqqMsg *lwqq_msg_new(const char *from, const char *to,
                      const char *msg_type, const char *content);
/** 
 * Free a LwqqMsg object
 * 
 * @param msg 
 */
void lwqq_msg_free(LwqqMsg *msg);

/**
 * Lwqq Receive Message object, used by receiving message
 * 
 */
typedef struct LwqqRecvMsg {
    LwqqMsg *msg;
    SIMPLEQ_ENTRY(LwqqRecvMsg) entries;
} LwqqRecvMsg;

typedef struct LwqqRecvMsgList {
    int count;                  /**< Number of message  */
    pthread_mutex_t mutex;
    SIMPLEQ_HEAD(, LwqqRecvMsg) head;
    void *lc;                   /**< Lwqq Client reference */
    void (*poll_msg)(struct LwqqRecvMsgList *list); /**< Poll to fetch msg */
} LwqqRecvMsgList;

/** 
 * Create a new LwqqRecvMsgList object
 * 
 * @param client Lwqq Client reference
 * 
 * @return NULL on failure
 */
LwqqRecvMsgList *lwqq_recvmsg_new(void *client);

/** 
 * Free a LwqqRecvMsgList object
 * 
 * @param list 
 */
void lwqq_recvmsg_free(LwqqRecvMsgList *list);

#endif  /* LWQQ_MSG_H */
