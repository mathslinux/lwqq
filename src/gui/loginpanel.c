/**
 * @file   loginpanel.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Mon May 28 22:33:57 2012
 * 
 * @brief  
 * 
 * 
 */

#include <gtk/gtk.h>
#include "loginpanel.h"

static void qq_loginpanelclass_init(QQLoginPanelClass *c);
static void qq_loginpanel_init(QQLoginPanel *obj);
static void qq_loginpanel_destroy(GtkWidget *obj);
static void login_btn_cb(GtkButton *btn, gpointer data);

GType qq_loginpanel_get_type()
{
    static GType t = 0;
    if (!t) {
        static const GTypeInfo info = {
            sizeof(QQLoginPanelClass),
            NULL,
            NULL,
            (GClassInitFunc)qq_loginpanelclass_init,
            NULL,
            NULL,
            sizeof(QQLoginPanel),
            0,
            (GInstanceInitFunc)qq_loginpanel_init,
            NULL
        };
        t = g_type_register_static(GTK_TYPE_VBOX, "QQLoginPanel", &info, 0);
    }
    return t;
}

GtkWidget* qq_loginpanel_new(GtkWidget *container)
{
    QQLoginPanel *panel = g_object_new(qq_loginpanel_get_type(), NULL);
    panel->container = container;

    return GTK_WIDGET(panel);
}

static void qq_loginpanelclass_init(QQLoginPanelClass *c)
{
    GtkWidgetClass *object_class = NULL;
    object_class = GTK_WIDGET_CLASS(c);

    object_class->destroy = qq_loginpanel_destroy;
}

static void qqnumber_combox_changed(GtkComboBox *widget, gpointer data)
{
    /* TODO */
}

static gboolean quick_login(GtkWidget* widget,GdkEvent* e,gpointer data)
{
    return TRUE;
}

/** 
 * Callback of login_btn button
 * 
 * @param btn 
 * @param data 
 */
static void login_btn_cb(GtkButton *btn, gpointer data)
{
    /* FIXME */
}

static void qq_loginpanel_init(QQLoginPanel *obj)
{
#if 0
    GQQLoginUser *usr, *tmp;
    login_users = gqq_config_get_all_login_user(cfg);
    
    /* Put the last login user at the first of the array */
    for(i = 0; i < login_users->len; ++i){
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, i);
        if(usr == NULL){
            continue;
        }
        if(usr->last == 1){
            break;
        }
    }
    if(i < login_users->len){
        tmp = login_users->pdata[0];
        login_users->pdata[0] = login_users->pdata[i];
        login_users->pdata[i] = tmp;
    }
#endif

    obj->uin_label = gtk_label_new("QQ Number:");
    obj->uin_entry = gtk_combo_box_text_new_with_entry();
#if 0
    for (i = 0; i < login_users->len; ++i) {
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(obj->uin_entry),
                                       usr->qqnumber);
    }
#endif
    gtk_combo_box_set_active(GTK_COMBO_BOX(obj->uin_entry), 0);

    obj->passwd_label = gtk_label_new("Password:");
    obj->passwd_entry = gtk_entry_new();
#if 0
    if(login_users->len > 0){
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, 0);
		if (usr->rempw)
			gtk_entry_set_text(GTK_ENTRY(obj->passwd_entry), usr->passwd);
    }
#endif
    g_signal_connect(G_OBJECT(obj->uin_entry), "changed",
                     G_CALLBACK(qqnumber_combox_changed), obj);
	g_signal_connect(G_OBJECT(obj->uin_entry),"key-press-event",
                     G_CALLBACK(quick_login), (gpointer)obj);
	g_signal_connect(G_OBJECT(obj->passwd_entry), "key-press-event",
                     G_CALLBACK(quick_login), (gpointer)obj);
    /* not visibily */
    gtk_entry_set_visibility(GTK_ENTRY(obj->passwd_entry), FALSE);
    gtk_widget_set_size_request(obj->uin_entry, 200, -1);
    gtk_widget_set_size_request(obj->passwd_entry, 220, -1);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    /* uin label and entry */
    GtkWidget *uin_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(uin_hbox), obj->uin_label, FALSE, FALSE, 0);
    GtkWidget *uin_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_box_pack_start(GTK_BOX(uin_vbox), uin_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(uin_vbox), obj->uin_entry, FALSE, FALSE, 0);
    
    /* password label and entry */
    GtkWidget *passwd_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(passwd_hbox), obj->passwd_label
                       , FALSE, FALSE, 0);
    GtkWidget *passwd_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_box_pack_start(GTK_BOX(passwd_vbox), passwd_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(passwd_vbox), obj->passwd_entry, FALSE, FALSE, 0);

    /* put uin and password in a vbox */
    gtk_box_pack_start(GTK_BOX(vbox), uin_vbox, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), passwd_vbox, FALSE, FALSE, 2);

    /* rember password check box */
    obj->rempwcb = gtk_check_button_new_with_label("Remeber Password");
#if 0
	if(login_users->len > 0){
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, 0);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj->rempwcb), usr->rempw);
    }
#endif
    /* g_signal_connect(G_OBJECT(obj->rempwcb), "toggled" */
    /*                     , G_CALLBACK(qqnumber_combox_changed), obj); */
    GtkWidget *hbox4 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(hbox4), obj->rempwcb, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox4, FALSE, TRUE, 2);

    /* login button */
    obj->login_btn = gtk_button_new_with_label("Login");
    gtk_widget_set_size_request(obj->login_btn, 90, -1);
    g_signal_connect(G_OBJECT(obj->login_btn), "clicked", G_CALLBACK(login_btn_cb), (gpointer)obj);
#if 0
    /* status combo box */
    obj->status_comb = qq_statusbutton_new();
    if (login_users->len > 0) {
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, 0);
        qq_statusbutton_set_status_string(obj->status_comb, usr->status);
    }
#endif 

    GtkWidget *hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox, TRUE, FALSE, 0);

    GtkWidget *hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *hbox3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#if 0
    gtk_box_pack_start(GTK_BOX(hbox2), obj->status_comb, FALSE, FALSE, 0);
#endif
    gtk_box_pack_start(GTK_BOX(hbox2), obj->login_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox3), hbox2, TRUE, FALSE, 0);

    /* error informatin label */
    obj->err_label = gtk_label_new("");
    GdkRGBA color;
    gdk_rgba_parse(&color, "#fff000000"); /* red */
    gtk_widget_override_color(GTK_WIDGET(obj-> err_label), GTK_STATE_NORMAL, &color);

    hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), obj -> err_label, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox2, TRUE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox3, FALSE, FALSE, 0);

    gtk_box_set_homogeneous(GTK_BOX(obj), FALSE);
    GtkWidget *logo = gtk_image_new_from_file("/tmp/webqq_icon.png");
    gtk_widget_set_size_request(logo, -1, 150);
    gtk_box_pack_start(GTK_BOX(obj), logo, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(obj), hbox1, FALSE, FALSE, 15);
}

/*
 * Destroy the instance of QQLoginPanel
 */
static void qq_loginpanel_destroy(GtkWidget *obj)
{
    /*
     * Child widgets will be destroied by their parents.
     * So, we should not try to unref the Child widgets here.
     */
}
