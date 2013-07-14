#include "async.h"
#include <ev.h>

typedef ev_io LwqqAsyncIo_;
typedef struct LwqqAsyncTimer{
    ev_timer h;
    void (*func)(struct LwqqAsyncTimer* timer,void* data);
    void* data;
    int on_call;
}LwqqAsyncTimer;
typedef LwqqAsyncTimer LwqqAsyncTimer_;

typedef struct {
    LwqqAsyncIoCallback callback;
    void* data;
}LwqqAsyncIoWrap;

static enum{
    THREAD_NOT_CREATED,
    THREAD_NOW_WAITING,
    THREAD_NOW_RUNNING,
} ev_thread_status;
//### global data area ###//
pthread_cond_t ev_thread_cond = PTHREAD_COND_INITIALIZER;
pthread_t pid;
static struct ev_loop* ev_default = NULL;
static int global_quit_lock = 0;
static ev_timer bomb;
//### global data area ###//
static void build_global_loop()
{
    if(ev_default) return;
    ev_default = ev_loop_new(EVBACKEND_POLL);
    ev_set_timeout_collect_interval(ev_default, 0.1);
    ev_set_io_collect_interval(ev_default, 0.05);
}
static void *ev_run_thread(void* data)
{
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    //signal(SIGPIPE,SIG_IGN);
    while(1){
        ev_thread_status = THREAD_NOW_RUNNING;
        ev_run(ev_default,0);
        //if(ev_thread_status == THREAD_NOT_CREATED) return NULL;
        if(global_quit_lock) return NULL;
        ev_thread_status = THREAD_NOW_WAITING;
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&ev_thread_cond,&mutex);
        pthread_mutex_unlock(&mutex);
        //if(ev_thread_status == THREAD_NOT_CREATED) return NULL;
        if(global_quit_lock) return NULL;
    }
    return NULL;
}
static void start_ev_thread()
{
    if(global_quit_lock) return;
    if(ev_thread_status == THREAD_NOW_WAITING){
        pthread_cond_signal(&ev_thread_cond);
    }else if(ev_thread_status == THREAD_NOT_CREATED){
        ev_thread_status = THREAD_NOW_RUNNING;
        pthread_create(&pid,NULL,ev_run_thread,NULL);
    }
}
static void event_cb_wrap(EV_P_ ev_io *w,int action)
{
    if(global_quit_lock) return;
    LwqqAsyncIoWrap* wrap = w->data;
    if(wrap->callback)
        wrap->callback(wrap->data,w->fd,action);
}

LwqqAsyncTimerHandle lwqq_async_timer_new()
{
    return s_malloc0(sizeof(LwqqAsyncTimer_));
}
LwqqAsyncIoHandle lwqq_async_io_new()
{
    return s_malloc0(sizeof(LwqqAsyncIo_));
}
void lwqq_async_timer_free(LwqqAsyncTimerHandle timer)
{
    s_free(timer);
}
void lwqq_async_io_free(LwqqAsyncIoHandle io)
{
    s_free(io);
}
void lwqq_async_io_watch(LwqqAsyncIoHandle io,int fd,int action,LwqqAsyncIoCallback fun,void* data)
{
    LwqqAsyncIo_* io_ = (LwqqAsyncIo_*) io;
    if(global_quit_lock) return;
    ev_io_init(io_,event_cb_wrap,fd,action);
    LwqqAsyncIoWrap* wrap = s_malloc0(sizeof(*wrap));
    wrap->callback = fun;
    wrap->data = data;
    io_->data = wrap;
    if(!ev_default) build_global_loop();
    ev_io_start(ev_default,io_);
    if(ev_thread_status!=THREAD_NOW_RUNNING) 
        start_ev_thread();
}
void lwqq_async_io_stop(LwqqAsyncIoHandle io)
{
    LwqqAsyncIo_* io_ = (LwqqAsyncIo_*) io;
    ev_io_stop(ev_default,io_);
    s_free(io_->data);
}
static void timer_cb_wrap(EV_P_ ev_timer* w,int revents)
{
    if(global_quit_lock) return;
    LwqqAsyncTimerHandle timer = (LwqqAsyncTimerHandle)w;
    timer->func(timer,timer->data);
}
void lwqq_async_timer_watch(LwqqAsyncTimerHandle timer,unsigned int timeout_ms,LwqqAsyncTimerCallback fun,void* data)
{
    if(global_quit_lock) return;
    double second = (timeout_ms) / 1000.0;
    ev_timer_init(&timer->h,timer_cb_wrap,second,second);
    timer->func = fun;
    timer->data = data;
    if(!ev_default) build_global_loop();
    ev_timer_start(ev_default,&timer->h);
    if(ev_thread_status!=THREAD_NOW_RUNNING) 
        start_ev_thread();
}
void lwqq_async_timer_stop(LwqqAsyncTimerHandle timer)
{
    ev_timer_stop(ev_default, &timer->h);
}
static void ev_bomb(EV_P_ ev_timer * w,int revents)
{
    lwqq_puts("boom!!");
    ev_timer_stop(loop,w);
    ev_break(loop,EVBREAK_ALL);
}
void lwqq_async_global_quit()
{
    //no need to destroy thread
    if(ev_thread_status == THREAD_NOT_CREATED) return ;
    global_quit_lock = 1;

    if(ev_thread_status == THREAD_NOW_WAITING){
        pthread_cond_signal(&ev_thread_cond);
    }else if(ev_thread_status == THREAD_NOW_RUNNING){
        ev_timer_init(&bomb,ev_bomb,0.001,0.001);
        ev_timer_start(ev_default,&bomb);
    }
    ev_thread_status = THREAD_NOT_CREATED;
    pthread_join(pid,NULL);
    ev_loop_destroy(ev_default);
    ev_default = NULL;
    global_quit_lock = 0;
}
void lwqq_async_timer_repeat(LwqqAsyncTimerHandle timer)
{
    ev_timer_again(ev_default, &timer->h);
}
#if USE_DEBUG
static int lwqq_gdb_count_running()
{
    return ev_pending_count(ev_default);
}
#endif
