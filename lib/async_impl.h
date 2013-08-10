#ifndef LWQQ_ASYNC_IMPL_H_H
#define LWQQ_ASYNC_IMPL_H_H

#include "async.h"

#define LWQQ__ASYNC_IMPL(impl) lwqq__async_impl_.impl
struct LwqqAsyncTimer{
    LwqqAsyncTimerCallback func;
    void* data;
};

struct LwqqAsyncIo{
    LwqqAsyncIoCallback func;
    void* data;
    int fd;
    int action;
};
typedef struct LwqqAsyncImpl{
    void (*loop_create)();
    void (*loop_run)();
    void (*loop_stop)();
    void (*loop_free)();

    void* (*io_new)();
    void  (*io_free)(void *h);
    void  (*io_start)(void *h,int fd,int act);
    void  (*io_stop)(void *h);

    void* (*timer_new)();
    void  (*timer_free)(void* h);
    void  (*timer_start)(void* h,unsigned int ms);
    void  (*timer_stop)(void* h);
    void  (*timer_again)(void* h);
}LwqqAsyncImpl;

extern LwqqAsyncImpl lwqq__async_impl_;
void lwqq_async_implement(LwqqAsyncImpl*);
#define LWQQ_ASYNC_IMPLEMENT(impl) lwqq_async_implement(&impl);
/*
void LWQQ__ASYNC_IMPL(loop_create)();
void LWQQ__ASYNC_IMPL(loop_run)();
void LWQQ__ASYNC_IMPL(loop_stop)();
void LWQQ__ASYNC_IMPL(loop_free)();

void* LWQQ__ASYNC_IMPL(io_new)();
void  LWQQ__ASYNC_IMPL(io_free)(void* handler);
void  LWQQ__ASYNC_IMPL(io_start)(void* io,int fd,int action);
void  LWQQ__ASYNC_IMPL(io_stop)(void* handler);

void* LWQQ__ASYNC_IMPL(timer_new)();
void  LWQQ__ASYNC_IMPL(timer_free)(void* handler);
void  LWQQ__ASYNC_IMPL(timer_start)(void* handler,unsigned int ms);
void  LWQQ__ASYNC_IMPL(timer_stop)(void* handler);
void  LWQQ__ASYNC_IMPL(timer_again)(void* handler);
*/

#endif
