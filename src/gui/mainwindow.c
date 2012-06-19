/**
 * @file   mainwindow.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Mon May 28 19:45:07 2012
 * 
 * @brief  This file is based on gtkqq written by HuangCongyu.
 * I rewrite some module for compatible with GTK3
 * 
 * 
 */

#include <stdlib.h>
#include "mainwindow.h"
#include "loginpanel.h"

static void qq_mainwindow_init(QQMainWindow *win);
static void qq_mainwindowclass_init(QQMainWindowClass *wc);

gboolean qq_mainwindow_close(GtkWidget *widget)
{
    qq_mainwindow_hide(widget);
    
    return TRUE;
}

GType qq_mainwindow_get_type()
{
    static GType t = 0;
    if (!t) {
        const GTypeInfo info = {
            sizeof(QQMainWindowClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc)qq_mainwindowclass_init,
            NULL,    /* class finalize*/
            NULL,    /* class data */
            sizeof(QQMainWindow),
            0,    /* n pre allocs */
            (GInstanceInitFunc)qq_mainwindow_init,
            0
        };

        t = g_type_register_static(GTK_TYPE_WINDOW, "QQMainWindow", &info, 0);
    }
    return t;
}

/** 
 * Show the main window.
 * 
 * @param win 
 */
void qq_mainwindow_show(GtkWidget *win)
{
    QQMainWindow *mainwin = (QQMainWindow *)win;
    
    mainwin->showed = TRUE;
    gtk_widget_show(win);
}

/** 
 * Hide the main window
 * 
 * @param win 
 */
void qq_mainwindow_hide(GtkWidget *win)
{
    QQMainWindow *mainwin = (QQMainWindow *)win;
    
    mainwin->showed = FALSE;
    gtk_widget_hide(win);
}

/** 
 * If the window now is shown, hide it, else show it.
 * 
 * @param win 
 */
void qq_mainwindow_show_hide(GtkWidget *win)
{
    QQMainWindow *mainwin = (QQMainWindow *)win;
    
    if (TRUE == mainwin->showed) {
        qq_mainwindow_hide(win);
    } else {
        qq_mainwindow_show(win);
    }
}

/** 
 * Create a new LWQQ window.
 * 
 * 
 * @return 
 */
GtkWidget *qq_mainwindow_new()
{
    return GTK_WIDGET(g_object_new(qq_mainwindow_get_type(), "type",
                                   GTK_WINDOW_TOPLEVEL, NULL));
}

static void qq_mainwindow_init(QQMainWindow *win)
{
    GtkWidget *w = GTK_WIDGET(win);
    
    /* FIXME: the type of window should be configed by some config file */
    gtk_widget_set_size_request(w, 200, 500);
    gtk_window_resize(GTK_WINDOW(w), 250, 550);

    g_signal_connect(w, "delete-event",
					 G_CALLBACK(qq_mainwindow_close), NULL);
	win->showed = FALSE;

    win->login_panel = qq_loginpanel_new(w);
        
    win->notebook = gtk_notebook_new();

#if 0                           /* FIXME */
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(win->notebook), FALSE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(win->notebook), FALSE);
#endif

    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(win->notebook), TRUE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(win->notebook), TRUE);

    gtk_widget_show_all(win->login_panel);

    gtk_notebook_append_page(GTK_NOTEBOOK(win->notebook),
                             win->login_panel, NULL);
    gtk_container_add(GTK_CONTAINER(win), win->notebook);
    
    gtk_window_set_title(GTK_WINDOW(win), "LWQQ");
}

static void qq_mainwindowclass_init(QQMainWindowClass *wc)
{
}
