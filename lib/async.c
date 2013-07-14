/**
 * @file   async.c
 * @author xiehuc<xiehuc@gmail.com>
 * @date   Sun May 20 02:21:43 2012
 *
 * @brief  Linux WebQQ Async API
 * use libev
 *
 */

#include <stdlib.h>
#include <string.h>
#include "async.h"
#include "smemory.h"
#include "http.h"
#include "logger.h"
#include "internal.h"

#ifdef WITH_LIBEV
#include "async-libev.c"
#endif

#ifdef WITH_LIBUV
#include "async-libuv.c"
#endif

typedef struct LwqqAsyncEntry {
    void* func;
    LwqqAsyncEvent* ev;
    LIST_ENTRY(LwqqAsyncEntry) entries;
}LwqqAsyncEntry;
typedef struct async_dispatch_data {
    LwqqCommand cmd;
    LwqqAsyncTimer timer;
} async_dispatch_data; 
typedef struct LwqqAsyncEvset_ {
    LwqqAsyncEvset parent;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int cond_waiting;
    int ref_count;
    LwqqCommand cmd;
}LwqqAsyncEvset_;
typedef struct LwqqAsyncEvent_{
    LwqqAsyncEvent parent;
    LwqqAsyncEvset* host_lock;
    LwqqCommand cmd;
    LwqqHttpRequest* req;
}LwqqAsyncEvent_;


static void dispatch_wrap(LwqqAsyncTimerHandle timer,void* p)
{
    async_dispatch_data* data = (async_dispatch_data*)p;
    vp_do(data->cmd,NULL);
    lwqq_async_timer_stop(timer);

    //!!! should we stop first delete later?
    s_free(data);
}
void lwqq_async_dispatch(LwqqCommand cmd)
{
#ifndef WITHOUT_ASYNC
    async_dispatch_data* data = s_malloc0(sizeof(*data));
    data->cmd = cmd;
    lwqq_async_timer_watch(&data->timer, 10, dispatch_wrap, data);
#else
    vp_do(cmd,NULL);
#endif
}

void lwqq_async_init(LwqqClient* lc)
{
    lc->dispatch = lwqq_async_dispatch;
}

LwqqAsyncEvent* lwqq_async_event_new(void* req)
{
    LwqqAsyncEvent* event = s_malloc0(sizeof(LwqqAsyncEvent_));
    LwqqAsyncEvent_* internal = (LwqqAsyncEvent_*)event;
    internal->req = req;
    event->lc = req?internal->req->lc:NULL;
    event->failcode = LWQQ_CALLBACK_VALID;
    event->result = 0;
    return event;
}
LwqqHttpRequest* lwqq_async_event_get_conn(LwqqAsyncEvent* ev)
{
    if(!ev) return NULL;
    LwqqAsyncEvent_* in = (LwqqAsyncEvent_*) ev;
    return in->req;
}
LwqqAsyncEvset* lwqq_async_evset_new()
{
    LwqqAsyncEvset_* l = s_malloc0(sizeof(LwqqAsyncEvset_));
    pthread_mutex_init(&l->lock,NULL);
    pthread_cond_init(&l->cond,NULL);
    return (LwqqAsyncEvset*)l;
}
void lwqq_async_evset_free(LwqqAsyncEvset* set)
{
    if(!set) return;
    LwqqAsyncEvset_* evset_ = (LwqqAsyncEvset_*) set;
    pthread_mutex_destroy(&evset_->lock);
    pthread_cond_destroy(&evset_->cond);
    s_free(evset_);
}
void lwqq_async_event_finish(LwqqAsyncEvent* event)
{
    LwqqAsyncEvent_* internal = (LwqqAsyncEvent_*)event;
    vp_do(internal->cmd,NULL);
    LwqqAsyncEvset_* evset_ = (LwqqAsyncEvset_*)internal->host_lock;
    if(evset_ !=NULL){
        pthread_mutex_lock(&evset_->lock);
        evset_->ref_count--;
        //this store evset err count.
        if(event->result != LWQQ_EC_OK)
            evset_->parent.err_count ++;
        if(evset_->ref_count==0){
            vp_do(evset_->cmd,NULL);
            if(evset_->cond_waiting)
                pthread_cond_signal(&evset_->cond);
            else{
                pthread_mutex_unlock(&evset_->lock);
                lwqq_async_evset_free(internal->host_lock);
                s_free(event);
                return;
            }
        }
        pthread_mutex_unlock(&evset_->lock);
    }
    s_free(event);
}
void lwqq_async_evset_wait(LwqqAsyncEvset* set)
{
    if(!set) return;
    LwqqAsyncEvset_* evset_ = (LwqqAsyncEvset_*) set;
    if(evset_->ref_count == 0) vp_do(evset_->cmd,NULL);
    else{
        evset_->cond_waiting = 1;
        pthread_cond_wait(&evset_->cond, &evset_->lock);
        vp_do(evset_->cmd,NULL);
    }
    lwqq_async_evset_free(set);
}
void lwqq_async_evset_add_event(LwqqAsyncEvset* host,LwqqAsyncEvent *handle)
{
    if(!host || !handle) return;
    LwqqAsyncEvset_* internal = (LwqqAsyncEvset_*) host;
    pthread_mutex_lock(&internal->lock);
    ((LwqqAsyncEvent_*)handle)->host_lock = host;
    internal->ref_count++;
    pthread_mutex_unlock(&internal->lock);
}

void lwqq_async_add_event_listener(LwqqAsyncEvent* event,LwqqCommand cmd)
{
    LwqqAsyncEvent_* event_ = (LwqqAsyncEvent_*) event;
    if(event == NULL){
        vp_do(cmd,NULL);
        return ;
    }else if(event_->cmd.func== NULL)
        event_->cmd = cmd;
    else
        vp_link(&event_->cmd,&cmd);
    if(event->failcode == LWQQ_CALLBACK_SYNCED)
        lwqq_async_event_finish(event);
}
static void on_chain(LwqqAsyncEvent* caller,LwqqAsyncEvent* called)
{
    called->result = caller->result;
    called->failcode = caller->failcode;
    called->lc = caller->lc;
    lwqq_async_event_finish(called);
}
void lwqq_async_add_event_chain(LwqqAsyncEvent* caller,LwqqAsyncEvent* called)
{
    /**indeed caller->lc may be NULL when recursor */
    called->lc = caller->lc;
    lwqq_async_add_event_listener(caller,_C_(2p,on_chain,caller,called));
}
void lwqq_async_add_evset_listener(LwqqAsyncEvset* evset,LwqqCommand cmd)
{
    LwqqAsyncEvset_* _evset = (LwqqAsyncEvset_*)evset;
    if(evset == NULL){
        vp_cancel(cmd);
        return ;
    }else if(_evset->cmd.func== NULL)
        _evset->cmd = cmd;
    else
        vp_link(&_evset->cmd,&cmd);
    if(_evset->ref_count == 0) lwqq_async_evset_free(evset);
}

LwqqAsyncEvent* lwqq_async_queue_find(LwqqAsyncQueue* queue,void* func)
{
    if(!queue||!func) return NULL;
    LwqqAsyncEntry* entry;
    LIST_FOREACH(entry,queue,entries){
        if(entry->func == func) return entry->ev;
    }
    return NULL;
}
void lwqq_async_queue_add(LwqqAsyncQueue* queue,void* func,LwqqAsyncEvent* ev)
{
    LwqqAsyncEntry* entry = s_malloc0(sizeof(*entry));
    entry->func = func;
    entry->ev = ev;
    LIST_INSERT_HEAD(queue,entry,entries);
}
void lwqq_async_queue_rm(LwqqAsyncQueue* queue,void* func)
{
    LwqqAsyncEntry* entry;
    LIST_FOREACH(entry,queue,entries){
        if(entry->func == func){
            LIST_REMOVE(entry,entries);
            s_free(entry);
            return;
        }
    }
}
