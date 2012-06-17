
/**
 * @file   async.c
 * @author xiehuc<xiehuc@gmail.com>
 * @date   Sun May 20 02:21:43 2012
 * 
 * @brief  Linux WebQQ Async API
 * use libev
 * 
 */

#include <ev.h>
#include <ghttp.h>
#include <stdlib.h>
#include <string.h>
#include "async.h"
typedef struct async_watch_data{
    LwqqHttpRequest* request;
    ListenerType type;
    LwqqClient* client;
}async_watch_data;

thread_t th;
int running=0;
void* lwqq_async_thread(void* data)
{
    struct ev_loop* loop = EV_DEFAULT;
    running=1;
    while(1){
        ev_run(loop,0);
    }
    running=0;
}
void check_start_thread(){
    if(th==NULL) thread_init(th);
    if(!running)
        thread_create(th,lwqq_async_thread,NULL,"thread");
}

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
static void ev_io_come(EV_P_ ev_io* w,int revent)
{
    async_watch_data* data = (async_watch_data*)w->data;
    ghttp_status status;
    LwqqHttpRequest* request = data->request;
    status = ghttp_process(request->req);
    if(status!=ghttp_done) return ;

    //restore do_request end part
    lwqq_http_do_request_async(request);
    lwqq_async_callback(data);


    ev_io_stop(EV_DEFAULT,w);
    free(data);
    free(w);
}
void lwqq_async_watch(LwqqClient* client,LwqqHttpRequest* request,ListenerType type)
{
    ev_io *watcher = (ev_io*)malloc(sizeof(ev_io));
    ghttp_request* req = (ghttp_request*)request->req;
    ev_io_init(watcher,ev_io_come,ghttp_get_socket(req),EV_READ);
    async_watch_data* data = malloc(sizeof(async_watch_data));
    data->request = request;
    data->type = type;
    data->client = client;
    watcher->data = data;
    ev_io_start(EV_DEFAULT,watcher);
    check_start_thread();
}

void lwqq_async_set(LwqqClient* client,int enabled)
{
    if(enabled&&!lwqq_async_enabled(client)){
        client->async = malloc(sizeof(LwqqAsyncListener));
        memset(client->async,0,sizeof(LwqqAsyncListener));
    }else if(!enabled&&lwqq_async_enabled(client)){
        free(client->async);
        client->async=NULL;
    }

}
