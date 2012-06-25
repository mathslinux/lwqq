/**
 * @file   loginpanel.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Mon May 28 22:33:57 2012
 * 
 * @brief  
 * 
 * 
 */

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "loginpanel.h"
#include "mainwindow.h"
#include "statusbutton.h"
#include "lwdb.h"
#include "logger.h"

extern char *lwqq_install_dir;
extern char *lwqq_icons_dir;
extern char *lwqq_buddy_status_dir;

static void qq_loginpanelclass_init(QQLoginPanelClass *c);
static void qq_loginpanel_init(QQLoginPanel *obj);
static void qq_loginpanel_destroy(GtkWidget *obj);
static void login_btn_cb(GtkButton *btn, gpointer data);

typedef struct LoginPanelUserInfo {
    char *qqnumber;
    char *password;
    char *status;
    char *rempwd;
} LoginPanelUserInfo;
static LoginPanelUserInfo login_panel_user_info;

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
    return FALSE;
}

static void free_login_panel_user_info()
{
    LoginPanelUserInfo *info = &login_panel_user_info;
    g_free(info->password);
    g_free(info->qqnumber);
    g_free(info->rempwd);
    g_free(info->status);
}

static void login_panel_update_user_info(QQLoginPanel* loginpanel)
{
    QQLoginPanel *panel = NULL;
    gboolean active;
    LoginPanelUserInfo *info;
    
    /* Free old data */
    free_login_panel_user_info();
    
    panel = QQ_LOGINPANEL(loginpanel);

    info = &login_panel_user_info;

    info->qqnumber = gtk_combo_box_text_get_active_text(
        GTK_COMBO_BOX_TEXT(panel->uin_entry));

    info->password = g_strdup(gtk_entry_get_text(
                                  GTK_ENTRY(panel->passwd_entry)));
    
    info->status = g_strdup(qq_statusbutton_get_status_string(
                                loginpanel->status_comb));

    active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
                                              loginpanel->rempwcb));
    if (active == TRUE) {
        info->rempwd = g_strdup("1");
    } else {
        info->rempwd = g_strdup("0");
    }
}

static void update_gdb(QQLoginPanel *lp)
{
#define UPDATE_GDB_MACRO() {                                            \
        gdb->update_user_info(gdb, info->qqnumber, "password", info->password); \
        gdb->update_user_info(gdb, info->qqnumber, "status", info->status); \
        gdb->update_user_info(gdb, info->qqnumber, "rempwd", info->rempwd); \
    }
    LwdbGlobalDB *gdb = lp->gdb;
    LwdbGlobalUserEntry *e;
    LoginPanelUserInfo *info = &login_panel_user_info;
    
    LIST_FOREACH(e, &gdb->head, entries) {
        if (!g_strcmp0(e->qqnumber, info->qqnumber)) {
            UPDATE_GDB_MACRO();
            return ;
        }
    }

    gdb->add_new_user(gdb, e->qqnumber);
    UPDATE_GDB_MACRO();
    
#undef UPDATE_GDB_MACRO
}

/** 
 * login_cb(QQLoginPanel *panel)
 * show the splashpanel and start the login procedure.
 * 
 * @param panel 
 */
static void login_cb(QQLoginPanel* panel)
{
    LoginPanelUserInfo *info = &login_panel_user_info;
    GtkWidget *win = panel->container;
    qq_mainwindow_show_splashpanel(win);
    
    /* Get user information from the login panel */
    login_panel_update_user_info(panel);
    lwqq_log(LOG_NOTICE, "Start login... qqnum: %s, status: %s\n",
             info->qqnumber, info->status);

    /* Update database */
    update_gdb(panel);
    free_login_panel_user_info();
#if 0

    /* *
     * run the login state machine
     * we have a login state machine for login
     */
    g_debug("Run login state machine...(%s, %d)", __FILE__, __LINE__);
    state = LOGIN_SM_CHECKVC;
    run_login_state_machine(panel);

    g_object_set(cfg, "qqnum", panel -> uin, NULL);
	if (panel->rempw)
		g_object_set(cfg, "passwd", panel -> passwd, NULL);
	else
		g_object_set(cfg, "passwd", "", NULL);
    g_object_set(cfg, "status", panel -> status, NULL);
	g_object_set(cfg, "rempw", panel -> rempw, NULL);

    qq_buddy_set(info -> me, "status", panel -> status);

	/* Set mute status */
	GQQLoginUser *usr = get_current_login_user(login_users);
	if (usr)
		qq_tray_set_mute_item(tray, usr->mute);

    //clear the error message.
    gtk_label_set_text(GTK_LABEL(panel -> err_label), "");
    gqq_config_save_last_login_user(cfg);
#endif
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
    QQLoginPanel *panel = QQ_LOGINPANEL(data);
	login_cb(panel);
}

static void qq_loginpanel_init(QQLoginPanel *obj)
{
    LwdbGlobalUserEntry *e;
    
    memset(&login_panel_user_info, 0, sizeof(login_panel_user_info));

    obj->gdb = lwdb_globaldb_new();
    if (!obj->gdb) {
        lwqq_log(LOG_ERROR, "Create global db failed, exit\n");
        exit(1);
    }
    obj->uin_label = gtk_label_new("QQ Number:");
    obj->uin_entry = gtk_combo_box_text_new_with_entry();
    LIST_FOREACH(e, &obj->gdb->head, entries) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(obj->uin_entry),
                                       e->qqnumber);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(obj->uin_entry), 0);

    obj->passwd_label = gtk_label_new("Password:");
    obj->passwd_entry = gtk_entry_new();
    e = LIST_FIRST(&obj->gdb->head);
    if (e && e->rempwd) {
        gtk_entry_set_text(GTK_ENTRY(obj->passwd_entry), e->password);
    }
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
    gtk_box_pack_start(GTK_BOX(passwd_hbox), obj->passwd_label,
                       FALSE, FALSE, 0);
    GtkWidget *passwd_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_box_pack_start(GTK_BOX(passwd_vbox), passwd_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(passwd_vbox), obj->passwd_entry, FALSE, FALSE, 0);

    /* put uin and password in a vbox */
    gtk_box_pack_start(GTK_BOX(vbox), uin_vbox, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), passwd_vbox, FALSE, FALSE, 2);

    /* rember password check box */
    obj->rempwcb = gtk_check_button_new_with_label("Remeber Password");
    if (e && e->rempwd) {
        gboolean r;
        r = atoi(e->rempwd) == 1 ? TRUE : FALSE;
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj->rempwcb), r);

    }
    /* g_signal_connect(G_OBJECT(obj->rempwcb), "toggled" */
    /*                     , G_CALLBACK(qqnumber_combox_changed), obj); */
    GtkWidget *hbox4 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(hbox4), obj->rempwcb, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox4, FALSE, TRUE, 2);

    /* login button */
    obj->login_btn = gtk_button_new_with_label("Login");
    gtk_widget_set_size_request(obj->login_btn, 90, -1);
    g_signal_connect(G_OBJECT(obj->login_btn), "clicked", G_CALLBACK(login_btn_cb), (gpointer)obj);
    
    /* status combo box */
    obj->status_comb = qq_statusbutton_new();
#if 0
    if (login_users->len > 0) {
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, 0);
        qq_statusbutton_set_status_string(obj->status_comb, usr->status);
    }
#endif

    GtkWidget *hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox, TRUE, FALSE, 0);

    GtkWidget *hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *hbox3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), obj->status_comb, FALSE, FALSE, 0);
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
    char img[512];
    g_snprintf(img, sizeof(img), "%s/webqq_icon.png", lwqq_icons_dir);
    GtkWidget *logo = gtk_image_new_from_file(img);
    gtk_widget_set_size_request(logo, -1, 150);
    gtk_box_pack_start(GTK_BOX(obj), logo, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(obj), hbox1, FALSE, FALSE, 15);
}

/*
 * Destroy the instance of QQLoginPanel
 */
static void qq_loginpanel_destroy(GtkWidget *obj)
{
    QQLoginPanel *lp = QQ_LOGINPANEL(obj);
    
    /* Free LwdbGlobalDB */
    lwdb_globaldb_free(lp->gdb);
    lp->gdb = NULL;

    /*
     * Child widgets will be destroied by their parents.
     * So, we should not try to unref the Child widgets here.
     */
}
