#include <config.h>
#include <glib/gi18n.h>
#include "tray.h"
#include "mainwindow.h"
#include "type.h"

/*
 * The main event loop context of Gtk.
 */
extern LwqqClient *lwqq_client;
extern char *lwqq_install_dir;
extern char *lwqq_icons_dir;
extern char *lwqq_buddy_status_dir;
extern char *lwqq_user_dir;
extern GtkWidget *main_win;
extern GHashTable *lwqq_chat_window;

//
// Private members
//
typedef struct {
	GQueue *blinking_queue;     // blinking queue
	GQueue *tmp_queue;          // tmp queue

	GtkWidget *popupmenu;       // popup menu
	GtkWidget *mute_item;		/**< mute item. */
} QQTrayPriv;

static void qq_tray_init(QQTray *tray);
static void qq_trayclass_init(QQTrayClass *tc, gpointer data);

GType qq_tray_get_type()
{
    static GType t = 0;
    if (!t) {
        const GTypeInfo info = {
            sizeof(QQTrayClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc)qq_trayclass_init,
            NULL,    /* class finalize*/
            NULL,    /* class data */
            sizeof(QQTray),
            0,    /* n pre allocs */
            (GInstanceInitFunc)qq_tray_init,
            0
        };

        t = g_type_register_static(GTK_TYPE_STATUS_ICON, "QQTray", &info, 0);
    }
    return t;
}

QQTray *qq_tray_new()
{
    gchar img[256];
    g_snprintf(img, sizeof(img), "%s/webqq_icon.png", lwqq_icons_dir);
    return QQ_TRAY(g_object_new(qq_tray_get_type(), "file", img , NULL));
}

/**
 * Blinking uin's face image
 *
 * @param tray
 * @param uin
 */
static void qq_tray_blinking(QQTray *tray, const gchar *uin)
{
    gchar buf[500];
    GdkPixbuf *pb;
    LwqqBuddy *bdy = lwqq_buddy_find_buddy_by_uin(lwqq_client, uin);

    /* blinking */
    if (bdy) {
        g_snprintf(buf, sizeof(buf), "%s/%s", lwqq_user_dir, bdy->qqnumber ?: "");
    } else {
        g_snprintf(buf, sizeof(buf), "%s/webqq_icon.png", lwqq_icons_dir);
    }
    pb = gdk_pixbuf_new_from_file(buf, NULL);
    if (!pb) {
        g_snprintf(buf, sizeof(buf), "%s/webqq_icon.png", lwqq_icons_dir);
        pb = gdk_pixbuf_new_from_file(buf, NULL);
    }
    gtk_status_icon_set_from_pixbuf(GTK_STATUS_ICON(tray), pb);
    g_object_unref(pb);
}

//
// popup-menu event
// Popup the menu
//
static void qq_tray_popup_menu(GtkStatusIcon *tray, guint button,
                               guint active_time, gpointer data)
{
    QQTrayPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(tray, qq_tray_get_type(),
                                                   QQTrayPriv);
    gtk_menu_popup(GTK_MENU(priv->popupmenu), NULL, NULL,
                   gtk_status_icon_position_menu, tray, button, active_time);
}

static gboolean qq_tray_button_press(GtkStatusIcon *tray, GdkEvent *event,
                                     gpointer data)
{
    GdkEventButton *buttonevent = (GdkEventButton *)event;

	/* Only handle left clicked. */
	if (buttonevent->button != 1 || buttonevent->type != GDK_BUTTON_PRESS) {
		return FALSE;
	}
    
    QQTrayPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(tray, qq_tray_get_type(),
                                                   QQTrayPriv);
    gchar *uin = g_queue_pop_tail(priv->blinking_queue);
    if (!uin) {
		/* If there is no new msg, show or hide the main window. */
#if 0
		qq_mainwindow_show_hide(main_win);
#endif
        return FALSE;
    }
    GtkWidget *cw = g_hash_table_lookup(lwqq_chat_window, uin);
    if(cw != NULL){
        gtk_widget_show(cw);
    }
    g_free(uin);

    if (g_queue_is_empty(priv -> blinking_queue)) {
	    /**
	     * WARNING:
	     * 		gtk_status_icon_set_blinking()
	     * 		has been deprecated since version GTK 2.22
	     * 		and will be removed in GTK+ 3
	     * 		maybe we should try libnotify
	     */
        gchar img[256];
        g_snprintf(img, sizeof(img), "%s/webqq_icon.png", lwqq_icons_dir);
        GdkPixbuf *pb = gdk_pixbuf_new_from_file(img, NULL);
        gtk_status_icon_set_from_pixbuf(GTK_STATUS_ICON(tray), pb);
        g_object_unref(pb);
        return FALSE;
    }
    qq_tray_blinking(QQ_TRAY(tray), g_queue_peek_tail(priv->blinking_queue));
    return FALSE;
}

//
// Custom the tooltip
//
static gboolean qq_tray_on_show_tooltip(GtkWidget* widget, int x, int y,
                                        gboolean keybord_mode, GtkTooltip* tip,
                                        gpointer data)
{
    GdkPixbuf *pb;
    gchar buf[500];

    if (!lwqq_client){
        // Not login. 
        g_snprintf(buf, 500, "%s/webqq_icon.png", lwqq_icons_dir);
        pb = gdk_pixbuf_new_from_file_at_size(buf, 35, 35, NULL);
        gtk_tooltip_set_markup(tip, "<b>GtkQQ</b>"); 
        gtk_tooltip_set_icon(tip, pb);
        g_object_unref(pb);
        return TRUE;
    }
	g_snprintf(buf, 500, "%s/%s", lwqq_user_dir, lwqq_client->username);

    /* FIXME */
    if (access(buf, F_OK)) {
        g_snprintf(buf, 500, "%s/webqq_icon.png", lwqq_icons_dir);
    }
    pb = gdk_pixbuf_new_from_file_at_size(buf, 35, 35, NULL);
    gtk_tooltip_set_icon(tip, pb);
    g_object_unref(pb);
    g_snprintf(buf, 500, "<b>%s</b><span color='blue'>(%s)</span>", "me",
               lwqq_client->username);
    gtk_tooltip_set_markup(tip, buf); 
    return TRUE;
}

/**
 * Status menu item signal handler
 */
#if 0
static void qq_tray_mute_menu_item_activate(GtkMenuItem *item, gpointer data)
{
	gint mute = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item));

	if (mute)
		g_print("Mute (%s, %d)\n", __FILE__, __LINE__);
	
	gqq_config_set_mute(cfg, mute);
}

/** 
 * Set mute item status, it usually called when user login.
 * 
 * @param tray 
 * @param mute 
 */
void qq_tray_set_mute_item(QQTray *tray, gboolean mute)
{
	QQTrayPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(tray,
			qq_tray_get_type(), QQTrayPriv);
	if (!priv)
		return ;
	gtk_check_menu_item_set_active(
			GTK_CHECK_MENU_ITEM(priv->mute_item), mute);
}

#endif

static void qq_tray_status_menu_item_activate(GtkMenuItem *item, gpointer data)
{
    const gchar *status = data;
    g_debug("Change status to %s (%s, %d)", status, __FILE__, __LINE__);
}

static void qq_tray_personal_setting_menu_item_activate(GtkMenuItem *item,
								gpointer data)
{
    g_debug("Tray personal setting (%s, %d)", __FILE__, __LINE__);
}
static void qq_tray_system_setting_menu_item_activate(GtkMenuItem *item,
								gpointer data)
{
    g_debug("Tray system setting (%s, %d)", __FILE__, __LINE__);

}

/** 
 * Show about window
 * 
 * @param item 
 * @param data 
 */
static void qq_tray_about_menu_item_activate(GtkMenuItem *item, gpointer data)
{
	gchar *copyright = "Copyright © 2012 lwqq";
	gchar *comment = _("A QQ client based on web qq protocol");
	gchar *licence = "GPL v3";
	GdkPixbuf *logo = NULL;
	gchar *authors[] = {
		"mathslinux <riegamaths@gmail.com>",
		NULL
	};

	g_debug("Tray about(%s, %d)", __FILE__, __LINE__);
	
	/* Our logo */
    gchar img_file[256];
    g_snprintf(img_file, sizeof(img_file), "%s/webqq_icon.png", lwqq_icons_dir);
	logo = gdk_pixbuf_new_from_file(img_file, NULL);

	GtkWidget *dialog = gtk_about_dialog_new();

	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), PACKAGE_NAME);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), PACKAGE_VERSION);
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(dialog), licence);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), copyright);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), (const gchar **)authors);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), comment);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), PACKAGE_URL);
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), logo);

	g_object_unref(logo);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

static void qq_tray_quit_menu_item_activate(GtkMenuItem *item, gpointer data)
{
    g_debug("Tray quit(%s, %d)", __FILE__, __LINE__);
    gtk_main_quit();
}

static void qq_tray_init(QQTray *tray)
{
    gchar img_file[256];
    gtk_status_icon_set_tooltip_markup(GTK_STATUS_ICON(tray), "<b>GtkQQ</b>");

    QQTrayPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(tray, qq_tray_get_type(),
                                                   QQTrayPriv);

    priv->blinking_queue = g_queue_new();
    priv->tmp_queue = g_queue_new();
    priv->popupmenu = gtk_menu_new();

    GtkWidget *menuitem;
#if 0
    menuitem = gtk_check_menu_item_new_with_label("Mute");
	priv->mute_item = menuitem;
	g_signal_connect(G_OBJECT(menuitem), "activate"
					 , G_CALLBACK(qq_tray_mute_menu_item_activate)
					 , NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(priv->popupmenu), menuitem);
#endif

    menuitem = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(priv->popupmenu), menuitem);

    GtkWidget *img;
    GdkPixbuf *pb;
#define STATUS_ITEM(x,y) {                                              \
    menuitem = gtk_image_menu_item_new();                               \
    gtk_menu_shell_append(GTK_MENU_SHELL(priv->popupmenu), menuitem);   \
    g_snprintf(img_file, sizeof(img_file), "%s/status/%s.png", lwqq_icons_dir, x); \
    pb = gdk_pixbuf_new_from_file_at_size(img_file, 12, 12, NULL);      \
    img = gtk_image_new_from_pixbuf(pb);                                \
    g_object_unref(pb);                                                 \
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), img);  \
    gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem), y);                \
    g_signal_connect(G_OBJECT(menuitem), "activate",                    \
                     G_CALLBACK(qq_tray_status_menu_item_activate), x); \
    }

    STATUS_ITEM("online", "Online");
    STATUS_ITEM("hidden", "Hidden");
    STATUS_ITEM("away", "Away");
    STATUS_ITEM("busy", "Busy");
    STATUS_ITEM("callme", "Call Me");
    STATUS_ITEM("silent", "Silent");
#undef STATUS_ITEM

    menuitem = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(priv->popupmenu), menuitem);

    menuitem = gtk_menu_item_new_with_label("Personal Setting");
    g_signal_connect(G_OBJECT(menuitem), "activate",
                     G_CALLBACK(qq_tray_personal_setting_menu_item_activate), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(priv->popupmenu), menuitem);

    menuitem = gtk_menu_item_new_with_label("System Setting");
    g_signal_connect(G_OBJECT(menuitem), "activate",
                     G_CALLBACK(qq_tray_system_setting_menu_item_activate), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(priv->popupmenu), menuitem);

    menuitem = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(priv->popupmenu), menuitem);

    menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
    g_signal_connect(G_OBJECT(menuitem), "activate",
                     G_CALLBACK(qq_tray_about_menu_item_activate), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(priv->popupmenu), menuitem);
    gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem), "About GtkQQ");

    menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
    g_signal_connect(G_OBJECT(menuitem), "activate",
                     G_CALLBACK(qq_tray_quit_menu_item_activate), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(priv->popupmenu), menuitem);
    gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem), "Quit");

    gtk_widget_show_all(priv->popupmenu);

    g_signal_connect(G_OBJECT(tray), "popup-menu",
                     G_CALLBACK(qq_tray_popup_menu), tray);
    g_signal_connect(G_OBJECT(tray), "button-press-event",
                     G_CALLBACK(qq_tray_button_press), tray);
    g_signal_connect(G_OBJECT(tray), "query-tooltip",
                     G_CALLBACK(qq_tray_on_show_tooltip), tray);
}

static void qq_trayclass_init(QQTrayClass *tc, gpointer data)
{
    g_type_class_add_private(tc, sizeof(QQTrayPriv));
}

void qq_tray_blinking_for(QQTray *tray, const gchar *uin)
{
    if (tray == NULL || uin == NULL) {
        return;
    }

    QQTrayPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(tray, qq_tray_get_type(),
                                                   QQTrayPriv);

    if(NULL != g_queue_find_custom(priv->blinking_queue, uin,
                                   (GCompareFunc)g_strcmp0)){
        // already blinking
        return;
    }
    g_queue_push_head(priv->blinking_queue, g_strdup(uin));
    qq_tray_blinking(tray, g_queue_peek_tail(priv->blinking_queue));
}

void qq_tray_stop_blinking_for(QQTray *tray, const gchar *uin)
{
    if (tray == NULL || uin == NULL) {
        return;
    }

    QQTrayPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(
        tray, qq_tray_get_type(),QQTrayPriv);

    gchar *tmpuin = NULL;
    g_queue_clear(priv->tmp_queue);
    while (!g_queue_is_empty(priv->blinking_queue)) {
        tmpuin = g_queue_pop_tail(priv->blinking_queue);
        if (g_strcmp0(tmpuin, uin) == 0) {
            //remove it
            g_free(tmpuin);
            break;
        }
        g_queue_push_head(priv->tmp_queue, tmpuin);
    }
    while (!g_queue_is_empty(priv->tmp_queue)) {
        g_queue_push_tail(priv->blinking_queue,
                          g_queue_pop_head(priv->tmp_queue));
    }

    GdkPixbuf *pb;
    if (g_queue_is_empty(priv->blinking_queue)) {
        gchar img[256];
        g_snprintf(img, sizeof(img), "%s/webqq_icon.png", lwqq_icons_dir);
        pb = gdk_pixbuf_new_from_file(img, NULL);
        gtk_status_icon_set_from_pixbuf(GTK_STATUS_ICON(tray), pb);
        g_object_unref(pb);
    } else {
        qq_tray_blinking(tray, g_queue_peek_tail(priv->blinking_queue));
    }
}
