/**
 * @file   main.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Mon May 21 23:29:20 2012
 * 
 * @brief  
 * 
 * 
 */

#include <gtk/gtk.h>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    GtkWidget *main_win = NULL;
    
    gtk_init(&argc, &argv);

    main_win = qq_mainwindow_new();
    gtk_widget_show_all(main_win);
    gtk_main();
    
    return 0;
}
