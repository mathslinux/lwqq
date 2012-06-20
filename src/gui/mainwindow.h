/**
 * @file   mainwindow.h
 * @author HuangCongyu <huangcongyu2006@gmail.com>,
 *         mathslinux <riegamaths@gmail.com>
 * @date   Mon May 28 19:24:53 2012
 * 
 * @brief  This file is based on gtkqq written by HuangCongyu.
 * I rewrite some module for compatible with GTK3
 * 
 * 
 */

#ifndef LWQQ_MAINWINDOW_H
#define LWQQ_MAINWINDOW_H

#include <gtk/gtk.h>

#define QQ_MAINWINDOW(obj) \
    G_TYPE_CHECK_INSTANCE_CAST(obj, qq_mainwindow_get_type(), QQMainWindow)
#define QQ_MAINWINDOWCLASS(c) \
    G_TYPE_CHECK_CLASS_CAST(c, qq_mainwindow_get_type(), QQMainWindowClass)
#define QQ_IS_MAINWINDOW(obj) G_TYPE_CHECK_INSTANCE_TYPE(obj, qq_mainwindow_get_type())

typedef struct __QQMainWindow {
    GtkWindow parent;

    GtkWidget *notebook;

    GtkWidget *login_panel;
    GtkWidget *main_panel;
    GtkWidget *splash_panel;

    gboolean showed;
} QQMainWindow;

typedef struct __QQMainWindowClass {
    GtkWindowClass parent;
} QQMainWindowClass;

/** 
 * Create a new LWQQ window.
 * 
 * 
 * @return 
 */
GtkWidget *qq_mainwindow_new();

GType qq_mainwindow_get_type();

/** 
 * Show the main window.
 * 
 * @param win 
 */
void qq_mainwindow_show(GtkWidget *win);

/** 
 * Show the main window.
 * 
 * @param win 
 */
void qq_mainwindow_show(GtkWidget *win);

/** 
 * Hide the main window
 * 
 * @param win 
 */
void qq_mainwindow_hide(GtkWidget *win);

/** 
 * If the window now is shown, hide it, else show it.
 * 
 * @param win 
 */
void qq_mainwindow_show_hide(GtkWidget *win);

/** 
 * If the window now is shown, hide it, else show it
 * 
 * @param win 
 */
void qq_mainwindow_show_hide(GtkWidget *win);

/**
 * Show different panels
 */
void qq_mainwindow_show_loginpanel(GtkWidget *win);
void qq_mainwindow_show_splashpanel(GtkWidget *win);
void qq_mainwindow_show_mainpanel(GtkWidget *win);

#endif  /* LWQQ_MAINWINDOW_H */
