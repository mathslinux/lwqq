/**
 * @file   msgloop.h
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Wed Jul  4 22:54:16 2012
 *
 * @brief  This file is written by HuangCongyu.
 * Change code style by mathslinux
 *
 *
 */

#ifndef __GQQ_MSGLOOP_H_
#define __GQQ_MSGLOOP_H_

#include <glib.h>
#include <stdarg.h>

/**
 * The wrapper of the main loop.
 */
typedef struct __GQQMessageLoop {
    GMainLoop *loop;
    GMainContext *ctx;
    gchar *name;
} GQQMessageLoop;

/**
 * Start a new main loop.
 * The new main loop will run in a new thread.
 *
 * @param name
 *
 * @return
 */
GQQMessageLoop* gqq_msgloop_start(const gchar *name);

void gqq_msgloop_stop(GQQMessageLoop *loop);

/**
 * Invoke the method in the main loop `loop`.
 * NOTE: The types of the parameters of the method must be pointer.
 *
 * @param loop
 * @param method : the address of the method which will be called in the
 *        message loop.
 * @param par_num the number of the parameter of the method. <= 5
 *
 * @return
 */
gint gqq_mainloop_attach(GQQMessageLoop* loop, gpointer method, gint par_num, ...);

#endif  /* __GQQ_MSGLOOP_H_ */
