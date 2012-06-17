/**
 * @file   async.c
 * @author xiehuc<xiehuc@gmail.com>
 * @date   Sun May 20 02:21:43 2012
 * 
 * @brief  Linux WebQQ Async API
 * use libev
 * 
 */

#include <ghttp.h>
#include <stdlib.h>
#include <string.h>
#include <eventloop.h>
#include "async.h"
typedef struct async_watch_data{
    LwqqHttpRequest* request;
    ListenerType type;
    LwqqClient* client;
    gint handle;
}async_watch_data;

void lwqq_async_add_listener(LwqqClient* lc,ListenerType type,ASYNC_CALLBACK callback,void*data)
{
    LwqqAsyncListener* async = lc->async;
    switch(type){
        case LOGIN_COMPLETE:
            async->login_complete[async->login_len] = callback;
            async->login_data[async->login_len++] = data;
            break;
        case FRIENDS_ALL_COMPLETE:
            async->friends_all_complete[async->friends_all_len] = callback;
            async->friends_all_data[async->friends_all_len++] = data;
            break;
        case MSG_COME:
            async->msg_complete[async->msg_len] = callback;
            async->msg_data[async->msg_len++] = data;
            break;
    }
}
#define FOREACH_CALLBACK(prefix) \
    while(async->prefix##_len>0){\
        data = async->prefix##_data[--async->prefix##_len];\
        async->prefix##_complete[async->prefix##_len](lc,request,data);\
        async->prefix##_complete[async->prefix##_len] = NULL;\
    }
void lwqq_async_callback(async_watch_data* data){
    LwqqClient* lc = data->client;
    LwqqAsyncListener* async = lc->async;
    LwqqHttpRequest* request = data->request;
    ListenerType type = data->type;
    int i;
    switch(type){
        case LOGIN_COMPLETE:
            while(async->login_len>0){
                data = async->login_data[--async->login_len];
                async->login_complete[async->login_len](lc,request,data);
                async->login_complete[async->login_len] = NULL;
            }
            break;
        case FRIENDS_ALL_COMPLETE:
            FOREACH_CALLBACK(friends_all);
            break;
        case MSG_COME:
            FOREACH_CALLBACK(msg);
            break;
    }
}
void lwqq_async_dispatch(LwqqClient* lc,ListenerType type)
{
    LwqqAsyncListener* async = lc->async;
    int i=0;
    void* data;
    switch(type){
        case MSG_COME:
            for(i=0;i<async->msg_len;i++){
                data = async->msg_data[i];
                async->msg_complete[i](lc,NULL,data);
            }
            break;
    }
}
int lwqq_async_has_listener(LwqqClient* lc,ListenerType type)
{
    switch(type){
        case LOGIN_COMPLETE:
            return lc->async->login_complete!=NULL;
            break;
    }
    return 0;
}
static void input_come(gpointer p,gint i,PurpleInputCondition cond)
{
    async_watch_data* data = (async_watch_data*)p;
    ghttp_status status;
    LwqqHttpRequest* request = data->request;
    status = ghttp_process(request->req);

    if(status!=ghttp_done) return ;

    //restore do_request end part
    lwqq_http_do_request_async(request);
    lwqq_async_callback(data);


    purple_input_remove(data->handle);
    free(data);
}
void lwqq_async_watch(LwqqClient* client,LwqqHttpRequest* request,ListenerType type)
{
    ghttp_request* req = (ghttp_request*)request->req;
    async_watch_data* data = malloc(sizeof(async_watch_data));
    data->request = request;
    data->type = type;
    data->client = client;
    data->handle = purple_input_add(ghttp_get_socket(req),PURPLE_INPUT_READ,
            input_come,data);
    /*ev_io *watcher = (ev_io*)malloc(sizeof(ev_io));
    ev_io_init(watcher,ev_io_come,ghttp_get_socket(req),EV_READ);
    watcher->data = data;
    ev_io_start(EV_DEFAULT,watcher);
    check_start_thread();*/
}

void lwqq_async_set(LwqqClient* client,int enabled)
{
    purple_debug_info("account","hi","endl");
    if(enabled&&!lwqq_async_enabled(client)){
        client->async = malloc(sizeof(LwqqAsyncListener));
        memset(client->async,0,sizeof(LwqqAsyncListener));
    }else if(!enabled&&lwqq_async_enabled(client)){
        free(client->async);
        client->async=NULL;
    }

}
