#include <loginpanel.h>
#include <mainpanel.h>
#include <mainwindow.h>
#include <tray.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <statusbutton.h>
#include <msgloop.h>
#include <string.h>
#include "type.h"
#include "lwdb.h"
#include "logger.h"
#include "login.h"
#include "info.h"
#include "smemory.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*
 * The global value
 * in main.c
 */
extern LwqqClient *lwqq_client;
extern char *lwqq_install_dir;
extern char *lwqq_icons_dir;
extern char *lwqq_buddy_status_dir;

extern GQQMessageLoop *get_info_loop;
extern GQQMessageLoop *send_loop;

static void qq_loginpanelclass_init(QQLoginPanelClass *c);
static void qq_loginpanel_init(QQLoginPanel *obj);
static void qq_loginpanel_destroy(GtkWidget *obj);
static void qqnumber_combox_changed(GtkComboBox *widget, gpointer data);
//static void update_face_image(LwqqClient *lc, QQMainPanel *panel);
static void update_buddy_qq_number(LwqqClient *lc, QQMainPanel *panel);

typedef struct LoginPanelUserInfo {
    char *qqnumber;
    char *password;
    char *status;
    char *rempwd;
} LoginPanelUserInfo;
static LoginPanelUserInfo login_panel_user_info;

/*
 * The main event loop context of Gtk.
 */
static GQQMessageLoop gtkloop;

//static GPtrArray* login_users = NULL;

GType qq_loginpanel_get_type()
{
    static GType t = 0;
    if(!t){
        static const GTypeInfo info =
            {
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
        t = g_type_register_static(GTK_TYPE_VBOX, "QQLoginPanel"
                                   , &info, 0);
    }
    return t;
}

GtkWidget* qq_loginpanel_new(GtkWidget *container)
{
    QQLoginPanel *panel = g_object_new(qq_loginpanel_get_type(), NULL);
    panel -> container = container;

    return GTK_WIDGET(panel);
}

static void qq_loginpanelclass_init(QQLoginPanelClass *c)
{
    GtkWidgetClass *object_class = NULL;
    object_class = GTK_WIDGET_CLASS(c);

    object_class -> destroy = qq_loginpanel_destroy;

    /*
     * get the default main evet loop context.
     *
     * I think this will work...
     */
    gtkloop.ctx = g_main_context_default();
    gtkloop.name = "LoginPanel Gtk";
}

//
// Run in message loop
//
#if 0
static gint do_login(QQLoginPanel *panel)
{
    const gchar *uin = panel -> uin;
    const gchar *passwd = panel -> passwd;
    const gchar *status = panel -> status;

    GError *err = NULL;
    gint ret = qq_login(info, uin, passwd, status, &err);
    const gchar *msg;
    switch(ret)
    {
        case NO_ERR:
            // going on
            return 0;
        case NETWORK_ERR:
            msg = "Network error. Please try again.";
            break;
        case WRONGPWD_ERR:
            msg = "Wrong Password.";
            break;
        case WRONGUIN_ERR:
            msg = "Wrong QQ Number.";
            break;
        case WRONGVC_ERR:
            msg = "Wrong Verify Code.";
            break;
        default:
            msg = "Error. Please try again.";
            break;
    }
    g_warning("Login error! %s (%s, %d)", err -> message, __FILE__, __LINE__);
    g_error_free(err);
    //show err message
    gqq_mainloop_attach(&gtkloop, gtk_label_set_text, 2,
		    GTK_LABEL(panel -> err_label), msg);
    return -1;
}
#endif

/**
 * Update the details.
 */
static void update_details(LwqqClient *lc, QQLoginPanel *panel)
{
#if 0
    // update my information
    qq_get_buddy_info(info, info -> me, NULL);
    gqq_mainloop_attach(&gtkloop
                        , qq_mainpanel_update_my_info
                        , 1
                        , QQ_MAINWINDOW(panel -> container) -> main_panel);
#endif

    QQMainPanel *mp = QQ_MAINPANEL(QQ_MAINWINDOW(panel->container)->main_panel);
    /* update online buddies */
    lwqq_info_get_online_buddies(lc, NULL);
    gqq_mainloop_attach(&gtkloop, qq_mainpanel_update_online_buddies, 1, mp);

    //update qq number
    update_buddy_qq_number(lc, (QQMainPanel *)QQ_MAINWINDOW(panel->container)->main_panel);
#if 0
    // update group number
    gint i;
    QQGroup *grp;
    gchar num[100];
    for(i = 0; i < info->groups->len; ++i){
        grp = g_ptr_array_index(info -> groups, i);
        if(grp == NULL){
            continue;
        }
        qq_get_qq_number(info, grp -> code -> str, num, NULL);
        qq_group_set(grp, "gnumber", num);
    }
    gqq_mainloop_attach(&gtkloop, qq_mainpanel_update_group_info , 1,
                        QQ_MAINWINDOW(panel -> container) -> main_panel);

    //update face image
    update_face_image(info,
                      (QQMainPanel*)QQ_MAINWINDOW(panel->container)-> main_panel);
#endif
}

//login state machine state.
enum{
    LOGIN_SM_CHECKVC,
    LOGIN_SM_LOGIN,
    LOGIN_SM_GET_DATA,
    LOGIN_SM_DONE,
    LOGIN_SM_ERR
};

//static void read_verifycode(gpointer p);
//static gint state = LOGIN_SM_ERR;
#if 0
static void login_state_machine(gpointer data)
{
    QQLoginPanel *panel = (QQLoginPanel*)data;
    while(TRUE){
        switch(state)
        {
            case LOGIN_SM_CHECKVC:
                if(qq_check_verifycode(info, panel -> uin, NULL) != 0){
                    state = LOGIN_SM_ERR;
                    break;
                }
                state = LOGIN_SM_LOGIN;
                if(info -> need_vcimage){
                    gqq_mainloop_attach(&gtkloop, read_verifycode, 1, panel);
                    // Quit the state machine.
                    // The state machine will restart in the read verify code
                    // dialog.
                    return;
                }
            case LOGIN_SM_LOGIN:
                if(do_login(panel) != 0){
                    state = LOGIN_SM_ERR;
                }else{
                    state = LOGIN_SM_GET_DATA;
                }
                break;
            case LOGIN_SM_GET_DATA:
                //Read cached data from db
                gqq_config_load(cfg, panel -> uin);
                qq_get_buddies_and_categories(info, NULL);
                qq_get_groups(info, NULL);
                state = LOGIN_SM_DONE;
                break;
            case LOGIN_SM_DONE:
                g_debug("Login done. show main panel!(%s, %d)", __FILE__, __LINE__);
                //
                // Start poll message
                //
                qq_start_poll(info, qq_poll_message_callback, &gtkloop, NULL);
                // update main panel
                gqq_mainloop_attach(&gtkloop, qq_mainpanel_update
                                    , 1, QQ_MAINWINDOW(panel -> container)
                                    -> main_panel);
                // show main panel
                gqq_mainloop_attach(&gtkloop, qq_mainwindow_show_mainpanel
                                    , 1, panel -> container);
                update_details(lc, panel);

                return;
            case LOGIN_SM_ERR:
                g_debug("Login error... (%s, %d)", __FILE__, __LINE__);
                gqq_mainloop_attach(&gtkloop, qq_mainwindow_show_loginpanel
                                    , 1, panel -> container);
                g_debug("Show login panel.(%s, %d)", __FILE__, __LINE__);
                return;
            default:
                break;
        }
    }
    return;
}
#endif

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

    gdb->add_new_user(gdb, info->qqnumber);
    UPDATE_GDB_MACRO();
    
#undef UPDATE_GDB_MACRO
}

/* FIXME, it's just a temporary function */
static char *get_vc(char *vc_file)
{
    char vc[128] = {0};
    int vc_len;
    FILE *f;

    if ((f = fopen(vc_file, "r")) == NULL) {
        return NULL;
    }

    if (!fgets(vc, sizeof(vc), f)) {
        fclose(f);
        return NULL;
    }
    
    vc_len = strlen(vc);
    if (vc[vc_len - 1] == '\n') {
        vc[vc_len - 1] = '\0';
    }
    return g_strdup(vc);
}

static gpointer poll_msg(gpointer data)
{
    LwqqRecvMsgList *l = (LwqqRecvMsgList *)data;
    l->poll_msg(l);
    while (1) {
        LwqqRecvMsg *msg;
        pthread_mutex_lock(&l->mutex);
        if (SIMPLEQ_EMPTY(&l->head)) {
            /* No message now, wait 100ms */
            pthread_mutex_unlock(&l->mutex);
            usleep(100000);
            continue;
        }
        msg = SIMPLEQ_FIRST(&l->head);
        char *msg_type = msg->msg->msg_type;
        if (msg_type && strlen(msg_type) > 0) {
            printf("Receive message type: %s \n", msg->msg->msg_type);
            if (strcmp(msg_type, MT_MESSAGE) == 0) {
                LwqqMsgMessage * m = (LwqqMsgMessage *)(msg->msg);
                if (m->content) {
                    printf("Receive message: %s\n", m->content);
                }
            } else if ( strcmp(msg_type, MT_GROUP_MESSAGE) == 0 ) {
                if (msg->msg->message.content) {
                    printf("Receive group message: %s\n", msg->msg->message.content);
                }
            } else if (strcmp(msg_type, MT_STATUS_CHANGE) == 0) {
                if (msg->msg->status.who
                    && msg->msg->status.status) {
                    printf("Receive status change: %s - > %s\n", 
                           msg->msg->status.who,
                           msg->msg->status.status);
                }
            } else  {
                printf("unknow message\n");
            }

        }
        SIMPLEQ_REMOVE_HEAD(&l->head, entries);
        pthread_mutex_unlock(&l->mutex);
    }
    return NULL;
}

/** 
 * Handle login. In this function, we do login, get friends information,
 * Get group information, and so on.
 * 
 * @param panel 
 */
static void handle_login(QQLoginPanel *panel)
{
    LoginPanelUserInfo *info = &login_panel_user_info;

    lwqq_client = lwqq_client_new(info->qqnumber, info->password);
    if (!lwqq_client) {
        lwqq_log(LOG_NOTICE, "Create lwqq client failed\n");
        return ;
    }
    while (1) {
        LwqqClient *lc = lwqq_client;
        LwqqErrorCode err = LWQQ_EC_ERROR;
        lwqq_login(lwqq_client, &err);
        if (err == LWQQ_EC_OK) {
            lwqq_log(LOG_NOTICE, "Login successfully\n");
            lwqq_info_get_friends_info(lc, NULL);

            /* Start poll message */
            g_thread_new("Poll message", poll_msg, lc->msg_list);

            /* update main panel */
            gqq_mainloop_attach(&gtkloop, qq_mainpanel_update, 1,
                                QQ_MAINPANEL(QQ_MAINWINDOW(panel->container)->main_panel));

            /* show main panel */
            gqq_mainloop_attach(&gtkloop, qq_mainwindow_show_mainpanel,
                                1, panel->container);

            update_details(lc, panel);
            break;
        } else if (err == LWQQ_EC_LOGIN_NEED_VC) {
#if 1                           /* FIXME */
            char vc_image[128];
            char vc_file[128];
            snprintf(vc_image, sizeof(vc_image), "/tmp/lwqq_%s.jpeg", lc->username);
            snprintf(vc_file, sizeof(vc_file), "/tmp/lwqq_%s.txt", lc->username);
            /* Delete old verify image */
            unlink(vc_file);

            lwqq_log(LOG_NOTICE, "Need verify code to login, please check "
                     "image file %s, and input what you see to file %s\n",
                     vc_image, vc_file);
            while (1) {
                if (!access(vc_file, F_OK)) {
                    sleep(1);
                    break;
                }
                sleep(1);
            }
            lc->vc->str = get_vc(vc_file);
            if (!lc->vc->str) {
                continue;
            }
            lwqq_log(LOG_NOTICE, "Get verify code: %s\n", lc->vc->str);
#endif
        } else {
            /* FIXME, need more output */
            lwqq_log(LOG_ERROR, "Login failed\n");
        }
    }
}

//
// The login state machine is run in the message loop.
//
static void run_login_state_machine(QQLoginPanel *panel)
{
    gqq_mainloop_attach(get_info_loop, handle_login, 1, panel);
}

// In libqq/qqutils.c
extern gint save_img_to_file(const gchar *data, gint len, const gchar *path);
//
// Show the verify code input dialog
// run in gtk main event loop.
//
#if 0
static void read_verifycode(gpointer p)
{
    QQLoginPanel *panel = (QQLoginPanel*)p;
    GtkWidget *w = panel -> container;
    gchar fn[300];
    if(info -> vc_image_data == NULL || info -> vc_image_type == NULL){
        g_warning("No vc image data or type!(%s, %d)" , __FILE__, __LINE__);
        gtk_label_set_text(GTK_LABEL(panel -> err_label)
                           , "Login failed. Please retry.");
        qq_mainwindow_show_loginpanel(w);
        return;
    }
    sprintf(fn, "%s/verifycode.%s", QQ_CFGDIR, info -> vc_image_type -> str);
    save_img_to_file(info -> vc_image_data -> str
                     , info -> vc_image_data -> len
                     , fn);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Information"
                                                    , GTK_WINDOW(w), GTK_DIALOG_MODAL
                                                    , GTK_STOCK_OK, GTK_RESPONSE_OK
                                                    , NULL);

	/**
	 * After GTK+-3.0 GtkDialog struct contains only private fields and
	 * should not be directly accessed.
	 */
#ifndef USE_GTK3
    GtkWidget *vbox = GTK_DIALOG(dialog) -> vbox;
#else
    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), vbox, 2);
#endif /* USE_GTK3 */
    GtkWidget *img = gtk_image_new_from_file(fn);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("VerifyCode：")
                       , FALSE, FALSE, 20);
    gtk_box_pack_start(GTK_BOX(vbox), img, FALSE, FALSE, 0);

    GtkWidget *vc_entry = gtk_entry_new();
    gtk_widget_set_size_request(vc_entry, 200, -1);
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vc_entry, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 10);

    gtk_widget_set_size_request(dialog, 300, 220);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));

    //got the verify code
    info -> verify_code = g_string_new(
        gtk_entry_get_text(GTK_ENTRY(vc_entry)));
    gtk_widget_destroy(dialog);

    //restart the state machine
    run_login_state_machine(panel);
}
#endif

/**
 * login_cb(QQLoginPanel *panel)
 * show the splashpanel and start the login procedure.
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

    run_login_state_machine(panel);

#if 0
    GtkWidget *win = panel -> container;
    qq_mainwindow_show_splashpanel(win);


    /* get user information from the login panel */
    panel -> uin = qq_loginpanel_get_uin(panel);
    panel -> passwd = qq_loginpanel_get_passwd(panel);
    panel -> status = qq_loginpanel_get_status(panel);
    panel -> rempw = qq_loginpanel_get_rempw(panel);

    g_debug("Start login... qqnum: %s, status: %s (%s, %d)", panel -> uin,
		    panel -> status, __FILE__, __LINE__);

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
/*
 * actived when RETURN pressed at uin_entry or passwd_entry
 */
gboolean quick_login(GtkWidget* widget,GdkEvent* e,gpointer data)
{
	GdkEventKey *event = (GdkEventKey*)e;
	if(event -> keyval == GDK_KEY_Return || event -> keyval == GDK_KEY_KP_Enter||
			event -> keyval == GDK_KEY_ISO_Enter){
		if((event -> state & GDK_CONTROL_MASK) != 0
                         || (event -> state & GDK_SHIFT_MASK) != 0){
			return FALSE;
         	}
         	login_cb(QQ_LOGINPANEL(data));
         	return TRUE;
	}
	return FALSE;

}

/**
 * Callback of login_btn button
 */
static void login_btn_cb(GtkButton *btn, gpointer data)
{
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
    if (e && e->status) {
        qq_statusbutton_set_status_string(obj->status_comb, e->status);
    }

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

#if 0
static void qq_loginpanel_init(QQLoginPanel *obj)
{
    login_users = gqq_config_get_all_login_user(cfg);
    //Put the last login user at the first of the array
    gint i;
    GQQLoginUser *usr, *tmp;
    for(i = 0; i < login_users -> len; ++i){
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, i);
        if(usr == NULL){
            continue;
        }
        if(usr -> last == 1){
            break;
        }
    }
    if(i < login_users -> len){
        tmp = login_users -> pdata[0];
        login_users -> pdata[0] = login_users -> pdata[i];
        login_users -> pdata[i] = tmp;
    }

    obj -> uin_label = gtk_label_new("QQ Number:");
    /**
     * WARNING:
     * 		gtk_combo_box_entry_new_text()
     * 		gtk_combo_box_append_text()
     * 		is mark as deprecated and should not be used in newly-written
     * 		code. It can't port to new GTK+ library.
     */
    obj -> uin_entry = gtk_combo_box_text_new_with_entry();
    for(i = 0; i < login_users -> len; ++i){
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(obj -> uin_entry)
                                  , usr -> qqnumber);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(obj -> uin_entry), 0);

    obj -> passwd_label = gtk_label_new("Password:");
    obj -> passwd_entry = gtk_entry_new();
    if(login_users -> len > 0){
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, 0);
		if (usr->rempw)
			gtk_entry_set_text(GTK_ENTRY(obj -> passwd_entry), usr -> passwd);
    }
    g_signal_connect(G_OBJECT(obj -> uin_entry), "changed"
                     , G_CALLBACK(qqnumber_combox_changed), obj);
	g_signal_connect(G_OBJECT(obj->uin_entry),"key-press-event",G_CALLBACK(quick_login),(gpointer)obj);
	g_signal_connect(G_OBJECT(obj->passwd_entry),"key-press-event",G_CALLBACK(quick_login),(gpointer)obj);
    //not visibily
    gtk_entry_set_visibility(GTK_ENTRY(obj -> passwd_entry), FALSE);
    gtk_widget_set_size_request(obj -> uin_entry, 200, -1);
    gtk_widget_set_size_request(obj -> passwd_entry, 220, -1);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 10);

    //uin label and entry
    GtkWidget *uin_hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(uin_hbox), obj -> uin_label, FALSE, FALSE, 0);
    GtkWidget *uin_vbox = gtk_vbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(uin_vbox), uin_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(uin_vbox), obj -> uin_entry, FALSE, FALSE, 0);
    //password label and entry
    GtkWidget *passwd_hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(passwd_hbox), obj -> passwd_label
                       , FALSE, FALSE, 0);
    GtkWidget *passwd_vbox = gtk_vbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(passwd_vbox), passwd_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(passwd_vbox), obj -> passwd_entry, FALSE, FALSE, 0);

    //put uin and password in a vbox
    gtk_box_pack_start(GTK_BOX(vbox), uin_vbox, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), passwd_vbox, FALSE, FALSE, 2);

    //rember password check box
    obj -> rempwcb = gtk_check_button_new_with_label("Remeber Password");
	if(login_users -> len > 0){
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, 0);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj -> rempwcb), usr->rempw);
    }
    /* g_signal_connect(G_OBJECT(obj -> rempwcb), "toggled" */
    /*                     , G_CALLBACK(qqnumber_combox_changed), obj); */
    GtkWidget *hbox4 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox4), obj -> rempwcb, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox4, FALSE, TRUE, 2);

    //login button
    obj -> login_btn = gtk_button_new_with_label("Login");
    gtk_widget_set_size_request(obj -> login_btn, 90, -1);
    g_signal_connect(G_OBJECT(obj -> login_btn), "clicked"
                     , G_CALLBACK(login_btn_cb), (gpointer)obj);
    //status combo box
    obj -> status_comb = qq_statusbutton_new();
    if(login_users -> len > 0){
        usr = (GQQLoginUser*)g_ptr_array_index(login_users, 0);
        qq_statusbutton_set_status_string(obj -> status_comb, usr -> status);
    }

    GtkWidget *hbox1 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox, TRUE, FALSE, 0);

    GtkWidget *hbox2 = gtk_hbox_new(FALSE, 0);
    GtkWidget *hbox3 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), obj -> status_comb, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), obj -> login_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox3), hbox2, TRUE, FALSE, 0);

    //error informatin label
    obj -> err_label = gtk_label_new("");
#ifndef USE_GTK3
    GdkColor color;
    GdkColormap *cmap = gdk_colormap_get_system();
    gdk_colormap_alloc_color(cmap, &color, TRUE, TRUE);
    gdk_color_parse("#fff000000", &color);    //red
    //change text color to red
    //MUST modify fb, not text
    gtk_widget_modify_fg(obj -> err_label, GTK_STATE_NORMAL, &color);
#else
    GdkRGBA color;
    gdk_rgba_parse(&color, "#fff000000"); /* red */
    gtk_widget_override_color(GTK_WIDGET(obj-> err_label), GTK_STATE_NORMAL, &color);
#endif /* USE_GTK3 */

    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), obj -> err_label, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox2, TRUE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox3, FALSE, FALSE, 0);

    gtk_box_set_homogeneous(GTK_BOX(obj), FALSE);
    GtkWidget *logo = gtk_image_new_from_file(IMGDIR"webqq_icon.png");
    gtk_widget_set_size_request(logo, -1, 150);
    gtk_box_pack_start(GTK_BOX(obj), logo, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(obj), hbox1, FALSE, FALSE, 15);
}
#endif

static void qqnumber_combox_changed(GtkComboBox *widget, gpointer data)
{
#if 0
    QQLoginPanel *obj = QQ_LOGINPANEL(data);
    gint idx = gtk_combo_box_get_active(widget);
    if(idx < 0 || idx >= login_users -> len){
        return;
    }
    GQQLoginUser *usr = (GQQLoginUser*)g_ptr_array_index(login_users, idx);
	if (usr->rempw)
		gtk_entry_set_text(GTK_ENTRY(obj -> passwd_entry), usr -> passwd);
    qq_statusbutton_set_status_string(obj -> status_comb, usr -> status);
    return;
#endif
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

const gchar* qq_loginpanel_get_uin(QQLoginPanel *loginpanel)
{
    QQLoginPanel *panel = QQ_LOGINPANEL(loginpanel);
    /* return gtk_combo_box_get_active_text(
        GTK_COMBO_BOX(panel -> uin_entry));     */
    return gtk_combo_box_text_get_active_text(
		    GTK_COMBO_BOX_TEXT(panel->uin_entry));

}
const gchar* qq_loginpanel_get_passwd(QQLoginPanel *loginpanel)
{
    QQLoginPanel *panel = QQ_LOGINPANEL(loginpanel);
    return gtk_entry_get_text(
        GTK_ENTRY(panel -> passwd_entry));
}
const gchar* qq_loginpanel_get_status(QQLoginPanel *loginpanel)
{
    return qq_statusbutton_get_status_string(loginpanel -> status_comb);
}

gint qq_loginpanel_get_rempw(QQLoginPanel *loginpanel)
{
	return  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(loginpanel->rempwcb));
}

typedef struct {
    LwqqClient *lc;
    LwqqBuddy *bdy;
} ThreadFuncPar;

//
// Get buddy qq number thread func
//
static void get_buddy_qqnumber_thread_func(gpointer data, gpointer user_data)
{

    ThreadFuncPar *par = data;
    LwqqClient *lc = par->lc;
    LwqqBuddy *bdy = par->bdy;
    g_slice_free(ThreadFuncPar ,par);

    /* Free old one */
    s_free(bdy->qqnumber);
    bdy->qqnumber = lwqq_info_get_friend_qqnumber(lc, bdy->uin);
}

//
// Update qq number
// Run in get_info_loop.
//
static void update_buddy_qq_number(LwqqClient *lc, QQMainPanel *panel)
{
    GThreadPool *thread_pool;
    ThreadFuncPar *par = NULL;
    LwqqBuddy *buddy;

    thread_pool = g_thread_pool_new(get_buddy_qqnumber_thread_func, NULL,
                                    100, TRUE, NULL);

    if (!thread_pool){
        g_debug("can not create new thread pool ...(%s,%d)",
                __FILE__, __LINE__ );
        return;
    }
    LIST_FOREACH(buddy, &lc->friends, entries) {
        par = g_slice_new0(ThreadFuncPar);
        par->lc = lc;
        par->bdy = buddy;
        g_thread_pool_push(thread_pool, (gpointer)par, NULL);
    }

    g_thread_pool_free(thread_pool, 0, 1);

    //update the panel
    qq_mainpanel_update_buddy_info(panel);
//    gqq_mainloop_attach(&gtkloop, qq_mainpanel_update_buddy_faceimg, 1, panel);
    return;
}

#if 0
void get_buddy_face_thread_func(gpointer data, gpointer user_data)
{

    ThreadFuncPar *par = data;
    GPtrArray * imgs = par -> array;
    gint id = par -> id;
    QQInfo * info = par -> info;
    g_slice_free(ThreadFuncPar , par);
    QQFaceImg * img;
    gchar path[500];
    img = g_ptr_array_index(imgs, id);
    qq_get_face_img(info,img, NULL);
    g_snprintf(path, 500, "%s/%s", QQ_FACEDIR, img -> num -> str);
    qq_save_face_img(img,path,NULL);

}
/**
 * Get all buddies' and groups' face images
 * Run in get_info_loop.
 */
static void update_face_image(LwqqClient *lc, QQMainPanel *panel)
{
    if(info == NULL || panel == NULL){
        return;
    }

    GPtrArray *fimgs = g_ptr_array_new();
    gint i;
    QQBuddy *bdy = NULL;
    QQFaceImg *img = NULL;

    // me
    img = qq_faceimg_new();
    qq_faceimg_set(img, "uin", info -> me -> uin);
    qq_faceimg_set(img, "num", info -> me  -> qqnumber);
    g_ptr_array_add(fimgs, img);

    // buddies
    for(i = 0; i < info -> buddies -> len; ++i){
        bdy = g_ptr_array_index(info -> buddies, i);
        if(bdy == NULL){
            continue;
        }
        img = qq_faceimg_new();
        qq_faceimg_set(img, "uin", bdy -> uin);
        qq_faceimg_set(img, "num", bdy -> qqnumber);
        g_ptr_array_add(fimgs, img);
    }
//#if 0
    // groups
    QQGroup *grp = NULL;
    for(i = 0; i < info -> groups -> len; ++i){
        grp = g_ptr_array_index(info -> groups, i);
        if(grp == NULL){
            continue;
        }
        img = qq_faceimg_new();
        qq_faceimg_set(img, "uin", grp -> code);
        qq_faceimg_set(img, "num", grp -> gnumber);
        g_ptr_array_add(fimgs, img);
    }
//#endif

    GThreadPool * thread_pool;

#if !GLIB_CHECK_VERSION(2,31,0)
    g_thread_init(NULL);
#endif
    ThreadFuncPar * par = NULL;

    thread_pool = g_thread_pool_new(get_buddy_face_thread_func, NULL ,100, TRUE, NULL );

    if ( ! thread_pool ){
        g_debug("can not create new thread pool ...(%s,%d)",
                __FILE__, __LINE__ );
        return;
    }

    for ( i =0 ; i < info->buddies ->len ; i ++ )
    {
        par = g_slice_new0(ThreadFuncPar);
        par -> array = NULL;
        par -> id = i;
        par -> info = info;
        par -> array = fimgs;

        g_thread_pool_push( thread_pool , (gpointer) par, NULL);
    }

    g_thread_pool_free(thread_pool, 0,1);


    for(i = 0; i < fimgs -> len; ++i){
        img = g_ptr_array_index(fimgs, i);
        qq_faceimg_free(img);
    }

    //update the buddy info
    gqq_mainloop_attach(&gtkloop, qq_mainpanel_update_buddy_faceimg, 1, panel);
    gqq_mainloop_attach(&gtkloop, qq_mainpanel_update_group_info, 1, panel);
}
#endif
