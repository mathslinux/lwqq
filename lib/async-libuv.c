#include <uv.h>
#include "async.h"


typedef uv_poll_t LwqqAsyncIo_;
typedef struct LwqqAsyncTimer{
    uv_timer_t h;
    void (*func)(struct LwqqAsyncTimer* timer,void* data);
    void* data;
    int on_call;
}LwqqAsyncTimer;
typedef LwqqAsyncTimer LwqqAsyncTimer_;

typedef struct {
    LwqqAsyncIoCallback callback;
    int fd;
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
static uv_loop_t* ev_default = NULL;
static int global_quit_lock = 0;
//### global data area ###//
static void build_global_loop()
{
    if(ev_default) return;
    ev_default = uv_loop_new();
    //ev_set_timeout_collect_interval(ev_default, 0.1);
    //ev_set_io_collect_interval(ev_default, 0.05);
}

static void *ev_run_thread(void* data)
{
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    //signal(SIGPIPE,SIG_IGN);
    while(1){
        ev_thread_status = THREAD_NOW_RUNNING;
        uv_run(ev_default, UV_RUN_DEFAULT);

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
static void event_cb_wrap(uv_poll_t* w,int status,int events)
{
    if(global_quit_lock) return;
    LwqqAsyncIoWrap* wrap = w->data;
    if(wrap->callback)
        wrap->callback(wrap->data,wrap->fd,events);
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
    if(!ev_default) build_global_loop();
    uv_poll_init(ev_default, io_, fd);
    LwqqAsyncIoWrap* wrap = s_malloc0(sizeof(*wrap));
    wrap->callback = fun;
    wrap->data = data;
    wrap->fd = fd;
    io_->data = wrap;
    uv_poll_start(io_, action, event_cb_wrap);
    if(ev_thread_status!=THREAD_NOW_RUNNING) 
        start_ev_thread();
}
void lwqq_async_io_stop(LwqqAsyncIoHandle io)
{
    LwqqAsyncIo_* io_ = (LwqqAsyncIo_*) io;
    uv_poll_stop(io_);
    s_free(io_->data);
}

static void timer_cb_wrap(uv_timer_t* w,int status)
{
    if(global_quit_lock) return;
    LwqqAsyncTimerHandle timer = (LwqqAsyncTimerHandle)w;
    timer->func(timer,timer->data);
}
void lwqq_async_timer_watch(LwqqAsyncTimerHandle timer,unsigned int timeout_ms,LwqqAsyncTimerCallback fun,void* data)
{
    if(global_quit_lock) return;
    if(!ev_default) build_global_loop();
    uv_timer_init(ev_default, &timer->h);
    timer->func = fun;
    timer->data = data;
    uv_timer_start(&timer->h, timer_cb_wrap,timeout_ms, timeout_ms);
    if(ev_thread_status!=THREAD_NOW_RUNNING) 
        start_ev_thread();
}
void lwqq_async_timer_stop(LwqqAsyncTimerHandle timer)
{
    uv_timer_stop(&timer->h);
}
void lwqq_async_global_quit()
{
    //no need to destroy thread
    if(ev_thread_status == THREAD_NOT_CREATED) return ;
    global_quit_lock = 1;

    if(ev_thread_status == THREAD_NOW_WAITING){
        pthread_cond_signal(&ev_thread_cond);
    }else if(ev_thread_status == THREAD_NOW_RUNNING){
        uv_stop(ev_default);
    }
    ev_thread_status = THREAD_NOT_CREATED;
    pthread_join(pid,NULL);
    uv_loop_delete(ev_default);
    ev_default = NULL;
    global_quit_lock = 0;
}
void lwqq_async_timer_repeat(LwqqAsyncTimerHandle timer)
{
    uv_timer_again(&timer->h);
}
#if USE_DEBUG
static int lwqq_gdb_count_running()
{
    return 0;
}
#endif
