/**
 * @file   buddytree.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Fri Jun 22 02:30:48 2012
 * 
 * @brief  This file is based on gtkqq written by HuangCongyu
 * 
 * 
 */


#include <string.h>
#include "buddytree.h"

enum {
    BDY_TYPE = 0,               /**< The client type */
    BDY_IMAGE,                  /** The face image of buddy */
    BDY_MARKNAME,               /**< Group name or buddy markname */
    BDY_NICK,                   /**< buddy nick */
    BDY_UIN,                    /**< buddy uin */
    BDY_QQNUMBER,               /**< buddy qqnumber */
    BDY_LONGNICK,               /**< buddy long nick */
    BDY_STATUS,                 /**< The status */

    BDY_TOOLTIPIMAGE,           /**< The face image of buddy */

    CATE_CNT,                   /**< Used by category. The online buddies's number */
    CATE_TOTAL,                 /**< Used by category. The total number of buddies. */
    CATE_INDEX,                 /**< Used by category. The index of buddies. */
    COLUMNS,                    /**< The number of columns */
};

/** 
 * Set the pixbuf columns' values
 * 
 * @param col 
 * @param renderer 
 * @param model 
 * @param iter 
 * @param data 
 */
static void qq_buddy_tree_pixbuf_cell_data_func(GtkTreeViewColumn *col,
                                                GtkCellRenderer *renderer,
                                                GtkTreeModel *model,
                                                GtkTreeIter *iter,
                                                gpointer data)
{
    /* FIXME */
#if 0
    GtkTreePath *path = gtk_tree_model_get_path(model, iter);
    if (gtk_tree_path_get_depth(path) > 1) {
        /* Buddy */
        gchar *status;
        g_object_set(renderer, "visible", TRUE, NULL);
        gtk_tree_model_get(model, iter, BDY_STATUS, &status, -1);
        if (g_strcmp0("online", status) == 0 ||
            g_strcmp0("away", status) == 0 ||
            g_strcmp0("busy", status) == 0 ||
            g_strcmp0("silent", status) == 0 ||
            g_strcmp0("callme", status) == 0) {
            g_object_set(renderer, "sensitive", TRUE, NULL);
        } else {
            g_object_set(renderer, "sensitive", FALSE, NULL);
        }
        g_free(status);
    } else {
        /* Category */
        g_object_set(renderer, "visible", FALSE, NULL);
    }
    gtk_tree_path_free(path);
#endif
}

/** 
 * Set the text columns' values
 * 
 * @param col 
 * @param renderer 
 * @param model 
 * @param iter 
 * @param data 
 */
static void qq_buddy_tree_text_cell_data_func(GtkTreeViewColumn *col,
                                              GtkCellRenderer *renderer,
                                              GtkTreeModel *model,
                                              GtkTreeIter *iter,
                                              gpointer data)
{
#if 0
    /* FIXME */
    GtkTreePath *path = gtk_tree_model_get_path(model, iter);
    gchar text[500];
    if (gtk_tree_path_get_depth(path) > 1) {
        /* Buddy */
        gchar *markname, *nick, *long_nick;
        gtk_tree_model_get(model, iter, BDY_MARKNAME, &markname, BDY_NICK,
                           &nick, BDY_LONGNICK, &long_nick, -1);
        GString *fmt = g_string_new(NULL);
        if (strlen(markname) <= 0) {
            g_string_append(fmt, "<b>%s%s</b>");
        } else {
            g_string_append(fmt, "<b>%s</b>(%s)");
        }
        /* long nick */
        g_string_append(fmt, "<span color='grey'>%s</span>");
        g_snprintf(text, 500, fmt -> str, markname, nick, long_nick);
        g_string_free(fmt, TRUE);
        g_object_set(renderer, "markup", text, NULL);
        g_free(markname);
        g_free(nick);
        g_free(long_nick);
    } else {
        /* Category */
        gchar *name;
        gint count, total;
        gtk_tree_model_get(model, iter, BDY_MARKNAME, &name, CATE_CNT,
                           &count, CATE_TOTAL, &total, -1);
        g_snprintf(text, 500, "%s [%d/%d]", name, count, total);
        g_object_set(renderer, "text", text, NULL);
        g_free(name);
    }
    gtk_tree_path_free(path);
#endif
}

static gboolean buddy_tree_on_show_tooltip(GtkWidget* widget, int x, int y,
                                           gboolean keybord_mode,
                                           GtkTooltip* tip, gpointer data)
{
    return TRUE;
    /* FIXME */
#if 0
	GtkTreeView *tree = GTK_TREE_VIEW(widget);
    GtkTreeModel *model = gtk_tree_view_get_model(tree); 
    GtkTreePath *path;
    GtkTreeIter iter;

	if (!gtk_tree_view_get_tooltip_context(tree, &x, &y, keybord_mode,
                                           &model, &path, &iter)) {
		return FALSE;
    }
    if (gtk_tree_path_get_depth(path) < 2) {
        return FALSE;
    }

    gchar *nick, *markname, *lnick, *qqnum;
    GdkPixbuf *pb;
    gtk_tree_model_get(model, &iter, BDY_NICK, &nick, BDY_TOOLTIPIMAGE,
                       &pb, BDY_MARKNAME, &markname, BDY_LONGNICK, &lnick,
                       BDY_QQNUMBER, &qqnum, -1);
    gchar buf[500];
    /* ☾ ☼ ☆ */
    g_snprintf(buf, 500, "<span color='#808080'>Nick Name:</span><b>%s</b>\n"
               "<span color='#808080'>Mark Name:</span><b>%s</b>\n"
               "<span color='#808080'>QQ Number:</span><span "
               "color='blue'><b>%s</b></span>\n"
               "<span color='#808080'>Long Nick:</span><span "
               "color='grey'>%s</span><span color='blue'><b>☾ ☼ ☆ </b></span>",
               nick, markname, qqnum, lnick);
    gtk_tooltip_set_markup(tip, buf);
    gtk_tooltip_set_icon(tip, pb);
    gtk_tree_view_set_tooltip_row(tree, tip, path);

    gtk_tree_path_free(path);
    g_object_unref(pb);
    g_free(nick);
    g_free(markname);
    g_free(lnick);
    g_free(qqnum);
    return TRUE;
#endif
}

static gboolean buddy_tree_on_rightbutton_click(GtkWidget* tree,
                                                GdkEventButton* event,
                                                gpointer data)
{
    return FALSE;
}

static void buddy_tree_on_double_click(GtkTreeView *tree
                                    , GtkTreePath *path 
                                    , GtkTreeViewColumn  *col 
                                    , gpointer data)
{
    /* FIXME */
    return ;
}

GtkWidget* qq_buddy_tree_new()
{
    GtkWidget *view = gtk_tree_view_new();
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    /* The client type column */
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", BDY_TYPE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
                                            qq_buddy_tree_pixbuf_cell_data_func,
                                            NULL, NULL);

    /* The image column */
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", BDY_IMAGE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
                                            qq_buddy_tree_pixbuf_cell_data_func,
                                            NULL, NULL);

    /* The text column */
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    gtk_tree_view_column_set_cell_data_func(column, renderer,
                                            qq_buddy_tree_text_cell_data_func,
                                            NULL, NULL);

    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
    gtk_tree_view_set_level_indentation(GTK_TREE_VIEW(view), -35);
    gtk_tree_view_set_hover_selection(GTK_TREE_VIEW(view), TRUE);

    gtk_widget_set_has_tooltip(view, TRUE);
	g_signal_connect(view, "query-tooltip",
                     G_CALLBACK(buddy_tree_on_show_tooltip) , NULL);

	g_signal_connect(view, "button_press_event",
                     G_CALLBACK(buddy_tree_on_rightbutton_click),
                     NULL);
	g_signal_connect(view, "row-activated",
                     G_CALLBACK(buddy_tree_on_double_click),
                     NULL);

#if 0
    tree_map = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                     tree_map_destroy_value);

    /* Create the chat window map */
    gqq_config_create_str_hash_table(cfg, "chat_window_map");
#endif

    return view;
}
