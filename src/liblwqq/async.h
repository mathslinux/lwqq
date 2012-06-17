/**
 * @file   async.h
 * @author xiehuc<xiehuc@gmail.com>
 * @date   Sun May 20 22:24:30 2012
 * 
 * @brief  Linux WebQQ Async API
 * 
 * 
 */
#ifndef LWQQ_ASYNC_H
#define LWQQ_ASYNC_H
#include "type.h"
#include "http.h"
typedef void (*ASYNC_CALLBACK)(LwqqClient* lc,LwqqHttpRequest* request,void* data);
typedef struct LwqqAsyncListener {
    LwqqErrorCode last_err;
    ASYNC_CALLBACK login_complete[3];
    void* login_data[3];
    int login_len;
    ASYNC_CALLBACK friends_all_complete[3];
    void* friends_all_data[3];
    int friends_all_len;
    ASYNC_CALLBACK msg_complete[3];
    void* msg_data[3];
    int msg_len;
} LwqqAsyncListener;

typedef enum ListenerType{
    LOGIN_COMPLETE,
    FRIENDS_ALL_COMPLETE,
    MSG_COME
} ListenerType;

void lwqq_async_add_listener(LwqqClient* lc,ListenerType type,ASYNC_CALLBACK callback,void* data);
void lwqq_async_watch(LwqqClient* client,LwqqHttpRequest* request,ListenerType type);
void lwqq_async_dispatch(LwqqClient* lc,ListenerType type);
#endif 
