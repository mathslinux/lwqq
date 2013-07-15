#include "async_impl.h"
#include "smemory.h"
#include <ev.h>

struct LwqqAsyncTimer_ {
    LwqqAsyncTimer super;
    ev_timer h;
};
struct LwqqAsyncIo_ {
    LwqqAsyncIo super;
    ev_io h;
};
static struct ev_loop* ev_default = NULL;
static void (loop_create)()
{
    if(ev_default) return;
    ev_default = ev_loop_new(EVBACKEND_POLL);
    ev_set_timeout_collect_interval(ev_default, 0.1);
    ev_set_io_collect_interval(ev_default, 0.05);
}
static void (loop_run)()
{
    ev_run(ev_default,0);
}

static void (loop_stop)()
{
    ev_break(ev_default, EVBREAK_ALL);
}
static void (loop_free)()
{
    ev_loop_destroy(ev_default);
    ev_default = NULL;
}
static void* (io_new)()
{
    return s_malloc0(sizeof(struct LwqqAsyncIo_));
}

static void (io_free)(void* h)
{
    s_free(h);
}
static void io_cb_wrap(EV_P_ ev_io* w,int action)
{
    //if(global_quit_lock) return;
    struct LwqqAsyncIo_* io_= w->data;
    if(io_->super.func)
        io_->super.func(w->data,io_->super.fd,action,io_->super.data);
}
static void  (io_start)(void* io,int fd,int action)
{
    struct LwqqAsyncIo_ * io_ = (struct LwqqAsyncIo_*) io;
    io_->h.data = io;
    ev_io_init(&io_->h, io_cb_wrap, fd, action);
    ev_io_start(ev_default, &io_->h);
}

static void (io_stop)(void* io)
{
    struct LwqqAsyncIo_ * io_ = (struct LwqqAsyncIo_*) io;
    ev_io_stop(ev_default, &io_->h);
}

static void* (timer_new)()
{
    return s_malloc0(sizeof(struct LwqqAsyncTimer_));
}

static void (timer_free)(void* t)
{
    s_free(t);
}

static void timer_cb_wrap(EV_P_ ev_timer* w,int action)
{
    //if(global_quit_lock) return;
    struct LwqqAsyncTimer_* io_= w->data;
    if(io_->super.func)
        io_->super.func(w->data,io_->super.data);
}

static void  (timer_start)(void* t,unsigned int ms)
{
    struct LwqqAsyncTimer_ * t_ = (struct LwqqAsyncTimer_*) t;
    t_->h.data = t;
    ev_timer_init(&t_->h, timer_cb_wrap, ms/1000.0, ms/1000.0);
    ev_timer_start(ev_default, &t_->h);
}

static void  (timer_stop)(void* t)
{
    struct LwqqAsyncTimer_ * t_ = (struct LwqqAsyncTimer_*) t;
    ev_timer_stop(ev_default, &t_->h);
}

static void  (timer_again)(void* t)
{
    struct LwqqAsyncTimer_ * t_ = (struct LwqqAsyncTimer_*) t;
    ev_timer_again(ev_default, &t_->h);
}

static LwqqAsyncImpl impl_libev = {
    .loop_create = loop_create,
    .loop_run    = loop_run,
    .loop_stop   = loop_stop,
    .loop_free   = loop_free,

    .io_new      = io_new,
    .io_free     = io_free,
    .io_start    = io_start,
    .io_stop     = io_stop,

    .timer_new   = timer_new,
    .timer_free  = timer_free,
    .timer_start = timer_start,
    .timer_stop  = timer_stop,
    .timer_again = timer_again,
};


