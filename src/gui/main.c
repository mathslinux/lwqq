/**
 * @file   main.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Mon May 21 23:29:20 2012
 * 
 * @brief  
 * 
 * 
 */

#include <stdlib.h>
#include <gtk/gtk.h>
#include "mainwindow.h"
#include "msgloop.h"
#include "lwdb.h"

/* Directory where lwqq installed in */
char *lwqq_install_dir = NULL;
char *lwqq_icons_dir = NULL;
char *lwqq_buddy_status_dir = NULL;

/**
 * The main loop used to get information from the server.
 * Such as face images, buddy information.
 */
GQQMessageLoop *get_info_loop = NULL;

static void gui_init()
{
    get_info_loop = gqq_msgloop_start("Get informain");
    if (get_info_loop == NULL) {
        exit(1);
    }

    /* initialize lwdb */
    lwdb_init();
    lwqq_install_dir = g_strdup(LWQQ_INSTALL_DIR);
    lwqq_icons_dir = g_strdup_printf("%s/icons", lwqq_install_dir);
    lwqq_buddy_status_dir = g_strdup_printf("%s/status", lwqq_icons_dir);
}

static void gui_finalize()
{
    lwdb_finalize();
    g_free(lwqq_install_dir);
    g_free(lwqq_icons_dir);
    g_free(lwqq_buddy_status_dir);
}

int main(int argc, char *argv[])
{
    GtkWidget *main_win = NULL;

    gui_init();
    
    gtk_init(&argc, &argv);

    main_win = qq_mainwindow_new();
    gtk_widget_show_all(main_win);
    gtk_main();

    gui_finalize();
    
    return 0;
}
