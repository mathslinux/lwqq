/**
 * @file   loginpanel.h
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Mon May 28 22:27:48 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef LWQQ_LOGINPANEL_H
#define LWQQ_LOGINPANEL_H

#include <gtk/gtk.h>

#define QQ_LOGINPANEL(obj)                                              \
    G_TYPE_CHECK_INSTANCE_CAST(obj, qq_loginpanel_get_type(), QQLoginPanel)
#define QQ_LOGINPANEL_CLASS(c)                                          \
    G_TYPE_CHECK_CLASS_CAST(c, qq_loginpanel_get_type(), QQLoginPanelClass)
#define QQ_IS_LOGINPANEL(obj)                                   \
    G_TYPE_CHECK_INSTANCE_TYPE(obj, qq_loginpanel_get_type())


typedef struct _QQLoginPanel {
    GtkVBox parent;

    /*< private >*/
    GtkWidget *uin_label, *uin_entry;
    GtkWidget *passwd_label, *passwd_entry;
    GtkWidget *rempwcb;             /* remember password check button */
    GtkWidget *err_label;           /* show error infomation. */
    GtkWidget *login_btn, *status_comb;

    const gchar *uin, *passwd, *status;
    gint rempw;

    GtkWidget *container;

#if 0
    //used to mark the login state.
    QQLoginPanelLoginState login_state;
#endif
} QQLoginPanel;

typedef struct _QQLoginPanelClass {
    GtkVBoxClass parent;
} QQLoginPanelClass;

/**
 * Create a new instance of QQLoginPanel.
 *
 * @param container : the container which contains this instance. Can be
 *     set to NULL.
 */
GtkWidget* qq_loginpanel_new(GtkWidget *container);
GType qq_loginpanel_get_type();

/**
 * Get the inputs
 */
const gchar* qq_loginpanel_get_uin(QQLoginPanel *loginpanel);
const gchar* qq_loginpanel_get_passwd(QQLoginPanel  *loginpanel);
const gchar* qq_loginpanel_get_status(QQLoginPanel *loginpanel);
gint qq_loginpanel_get_rempw(QQLoginPanel *loginpanel);

#endif  /* LWQQ_LOGINPANEL_H */
