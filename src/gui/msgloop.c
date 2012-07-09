/**
 * @file   msgloop.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Wed Jul  4 22:54:16 2012
 * @brief  This file is written by HuangCongyu.
 * Change code style by mathslinux
 *
 *
 */

#include <msgloop.h>
#include "logger.h"

/* The Invoker type */
typedef void (*Invoker)(gpointer method, gpointer *pars);

/**
 * GQQClosure
 * Contain all the information to invoke a method whoes
 * parameters are all gpointer type.
 * @param data
 *
 * @return
 */
typedef struct {
    Invoker invoker;
    gpointer method;
    /// I think that no method's parameters', number will greater ten...
    gpointer pars[10];
} GQQClosure;

static gpointer thread_main(gpointer data)
{
    GQQMessageLoop *ml= (GQQMessageLoop *)data;
    char name[256];

    ml->ctx = g_main_context_new();
    if (ml->ctx == NULL) {
        lwqq_log(LOG_ERROR, "Create context for %s loop failed\n", ml->name);
        return NULL;
    }

    GMainLoop *loop = g_main_loop_new(ml->ctx, TRUE);
    if (loop == NULL) {
        lwqq_log(LOG_ERROR, "Create %s main loop failed\n", ml->name);
        return NULL;
    }
    ml->loop = loop;

    lwqq_log(LOG_DEBUG, "Start %s main loop ......\n", ml->name);
    g_snprintf(name, sizeof(name), "%s", ml->name);
    g_main_loop_run(ml->loop);

    lwqq_log(LOG_DEBUG, "%s main loop quit\n", name);
    return NULL;
}

GQQMessageLoop* gqq_msgloop_start(const gchar *name)
{
    if (name == NULL) {
        name = "";
    }

    GQQMessageLoop *ml = g_slice_new0(GQQMessageLoop);
    ml->name = g_strdup(name);

    GError *err = NULL;
    if (g_thread_create(thread_main, ml, FALSE, &err) == NULL) {
        lwqq_log(LOG_WARNING, "Create main loop thread failed\n", err->message);
        g_error_free(err);
        g_free(ml->name);
        g_slice_free(GQQMessageLoop, ml);
        return NULL;
    }
    return ml;
}
void gqq_msgloop_stop(GQQMessageLoop *loop)
{
    if (loop == NULL) {
        return;
    }
    g_main_context_unref(loop->ctx);
    g_main_loop_quit(loop->loop);
    g_free(loop->name);
    g_slice_free(GQQMessageLoop, loop);
    return;
}

void gqq_method_invoker_1(gpointer method, gpointer *pars)
{
    typedef void(*MethodType1)(gpointer);
    MethodType1 m = (MethodType1)method;
    m(pars[0]);
}
void gqq_method_invoker_2(gpointer method, gpointer *pars)
{
    typedef void(*MethodType1)(gpointer, gpointer);
    MethodType1 m = (MethodType1)method;
    m(pars[0], pars[1]);
}
void gqq_method_invoker_3(gpointer method, gpointer *pars)
{
    typedef void(*MethodType1)(gpointer, gpointer, gpointer);
    MethodType1 m = (MethodType1)method;
    m(pars[0], pars[1], pars[2]);
}
void gqq_method_invoker_4(gpointer method, gpointer *pars)
{
    typedef void(*MethodType1)(gpointer, gpointer, gpointer, gpointer);
    MethodType1 m = (MethodType1)method;
    m(pars[0], pars[1], pars[2], pars[3]);
}
void gqq_method_invoker_5(gpointer method, gpointer *pars)
{
    typedef void(*MethodType1)(gpointer, gpointer, gpointer,
                               gpointer, gpointer);
    MethodType1 m = (MethodType1)method;
    m(pars[0], pars[1], pars[2], pars[3], pars[4]);
}

/* The table of the addresses of the invokers */
static Invoker invokers[] = {
    NULL,
    gqq_method_invoker_1,
    gqq_method_invoker_2,
    gqq_method_invoker_3,
    gqq_method_invoker_4,
    gqq_method_invoker_5,
    NULL
};

/**
 * The callback called in the main loop.
 *
 * @param data
 *
 * @return
 */
static gboolean gqq_ctx_callback(gpointer data)
{
    GQQClosure *c = (GQQClosure*)data;
    /* invoke the method. */
    c->invoker(c->method, c->pars);
    /* free the GQQClosure. */
    g_slice_free(GQQClosure, c);
    return FALSE;
}

gint gqq_mainloop_attach(GQQMessageLoop *loop, gpointer method, gint par_num, ...)
{
    va_list par;
    va_start(par, par_num);
    GQQClosure *c = g_slice_new0(GQQClosure);
    c->method = method;
    c->invoker = invokers[par_num];
    gpointer p;
    gint i;
    for(i = 0; i < par_num; ++i){
        p = va_arg(par, gpointer);
        c->pars[i] = p;
    }
    va_end(par);

    GSource *src = g_idle_source_new();
    g_source_set_callback(src, gqq_ctx_callback, c, NULL);
    g_source_attach(src, loop->ctx);
    g_source_unref(src);
    return 0;
}
