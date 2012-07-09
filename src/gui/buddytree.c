#include <stdlib.h>
#include <buddytree.h>
#include <string.h>
#include <logger.h>
#if 0
#include <chatwindow.h>
#endif

extern LwqqClient *lwqq_client;
extern LwqqClient *lwqq_client;
extern char *lwqq_install_dir;
extern char *lwqq_icons_dir;
extern char *lwqq_buddy_status_dir;
extern GtkWidget *main_win;
extern char *lwqq_user_dir;
extern GHashTable *lwqq_chat_window;

enum{
    BDY_TYPE= 0,        //The client type
    BDY_IMAGE,          //The face image of buddy
    BDY_MARKNAME,       //Group name or buddy markname
    BDY_NICK,           //buddy nick
    BDY_UIN,            //buddy uin
    BDY_QQNUMBER,       //buddy qqnumber
    BDY_LONGNICK,       //buddy long nick
    BDY_STATUS,         //The status

    BDY_TOOLTIPIMAGE,   //The face image of buddy

    CATE_CNT,           //Used by category. The online buddies's number
    CATE_TOTAL,         //Used by category. The total number of buddies.
    CATE_INDEX,         //Used by category. The index of buddies.
    COLUMNS             //The number of columns
};

//
// We have only instance around the program.
// So, I think this is fine...
//
// The map of the tree path and the qq uin
static GHashTable *tree_map = NULL;

//
// Set the text columns' values
//
static void qq_buddy_tree_text_cell_data_func(GtkTreeViewColumn *col
                                            , GtkCellRenderer *renderer
                                            , GtkTreeModel *model
                                            , GtkTreeIter *iter
                                            , gpointer data)
{
    GtkTreePath *path = gtk_tree_model_get_path(model, iter);
    gchar text[500];
    if(gtk_tree_path_get_depth(path) > 1){
        //
        // Buddy
        //
        gchar *markname, *nick, *long_nick;
        gtk_tree_model_get(model, iter
                            , BDY_MARKNAME, &markname
                            , BDY_NICK, &nick
                            , BDY_LONGNICK, &long_nick, -1);
        GString *fmt = g_string_new(NULL);
        if(strlen(markname) <= 0){
            g_string_append(fmt, "<b>%s%s</b>");
        }else{
            g_string_append(fmt, "<b>%s</b>(%s)");
        }
        //long nick
        g_string_append(fmt, "<span color='grey'>%s</span>");
        g_snprintf(text, 500, fmt -> str, markname, nick, long_nick);
        g_string_free(fmt, TRUE);
        g_object_set(renderer, "markup", text, NULL);
        g_free(markname);
        g_free(nick);
        g_free(long_nick);
    }else{
        //
        // Category
        //
        gchar *name;
        gint count, total;
        gtk_tree_model_get(model, iter
                            , BDY_MARKNAME, &name
                            , CATE_CNT, &count
                            , CATE_TOTAL, &total, -1);
        g_snprintf(text, 500, "%s [%d/%d]", name, count, total);
        g_object_set(renderer, "text", text, NULL);
        g_free(name);
    }
    gtk_tree_path_free(path);
}

//
// Set the pixbuf columns' values
//
static void qq_buddy_tree_pixbuf_cell_data_func(GtkTreeViewColumn *col
                                            , GtkCellRenderer *renderer
                                            , GtkTreeModel *model
                                            , GtkTreeIter *iter
                                            , gpointer data)
{
    GtkTreePath *path = gtk_tree_model_get_path(model, iter);
    if(gtk_tree_path_get_depth(path) > 1){
        //
        // Buddy
        //
        g_object_set(renderer, "visible", TRUE, NULL);
        gchar *status;
        gtk_tree_model_get(model, iter, BDY_STATUS, &status, -1);
        if(g_strcmp0("online", status) == 0 
                        || g_strcmp0("away", status) == 0
                        || g_strcmp0("busy", status) == 0
                        || g_strcmp0("silent", status) == 0
                        || g_strcmp0("callme", status) == 0){
            g_object_set(renderer, "sensitive", TRUE, NULL);
        }else{
            g_object_set(renderer, "sensitive", FALSE, NULL);
        }
        g_free(status);
    }else{
        //
        // Category
        //
        g_object_set(renderer, "visible", FALSE, NULL);
    }
    gtk_tree_path_free(path);
}

//
// Get category iter by index
//
static gboolean get_category_iter_by_index(GtkTreeModel *model, gint index
                                            , GtkTreeIter *iter)
{
    if(!gtk_tree_model_get_iter_first(model, iter)){
        return FALSE;
    }
    gint cate_idx;

    while(TRUE){
        gtk_tree_model_get(model, iter, CATE_INDEX, &cate_idx, -1);
        if(cate_idx == index){
            return TRUE;
        }
        if(!gtk_tree_model_iter_next(model, iter)){
            return FALSE;
        }
    }
    return TRUE;
}
//
// Get the face image of num with width and height.
//
static GdkPixbuf* create_face_image(const gchar *num, gint width, gint height)
{
    /* FIXME */
    if (num) 
        num = "12345678";
    gchar buf[500];
    GError *err = NULL;
	g_snprintf(buf, 500, "%s/%s", lwqq_user_dir, num);
    GdkPixbuf *pb = gdk_pixbuf_new_from_file_at_size(buf, width, height, &err);
    if(pb == NULL){
        g_error_free(err);
        err = NULL;
        gchar img[256];
        g_snprintf(img, sizeof(img), "%s/avatar.gif", lwqq_icons_dir);
        pb = gdk_pixbuf_new_from_file_at_size(img, width, height, &err);
        if(pb == NULL){
            g_warning("Load default face image error. %s (%s, %d)"
                                    , err -> message, __FILE__, __LINE__);
            g_error_free(err);
        }
    }
    return pb;
}

static gboolean buddy_tree_on_show_tooltip(GtkWidget* widget
                                            , int x
                                            , int y
                                            , gboolean keybord_mode
                                            , GtkTooltip* tip
                                            , gpointer data)
{
	GtkTreeView *tree = GTK_TREE_VIEW(widget);
    GtkTreeModel *model = gtk_tree_view_get_model(tree); 
    GtkTreePath *path;
    GtkTreeIter iter;

	if(!gtk_tree_view_get_tooltip_context(tree , &x , &y , keybord_mode
						, &model , &path , &iter)){
		return FALSE;
    }
    if(gtk_tree_path_get_depth(path) < 2){
        return FALSE;
    }

    gchar *nick, *markname, *lnick, *qqnum;
    GdkPixbuf *pb;
    gtk_tree_model_get(model, &iter
                        , BDY_NICK, &nick
                        , BDY_TOOLTIPIMAGE, &pb
                        , BDY_MARKNAME, &markname
                        , BDY_LONGNICK, &lnick
                        , BDY_QQNUMBER, &qqnum, -1);
    gchar buf[500];
    // ☾ ☼ ☆
    g_snprintf(buf, 500, "<span color='#808080'>Nick Name:</span><b>%s</b>\n"
                         "<span color='#808080'>Mark Name:</span><b>%s</b>\n"
                         "<span color='#808080'>QQ Number:</span><span "
                         "color='blue'><b>%s</b></span>\n"
                         "<span color='#808080'>Long Nick:</span><span "
                         "color='grey'>%s</span><span color='blue'><b>☾ ☼ ☆ </b></span>"
                            , nick, markname, qqnum, lnick);
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
}

static void buddy_tree_on_double_click(GtkTreeView *tree
                                    , GtkTreePath *path 
                                    , GtkTreeViewColumn  *col 
                                    , gpointer data)
{
#if 0
    gchar *uin;
    GtkTreeModel *model = gtk_tree_view_get_model(tree); 
    if(gtk_tree_path_get_depth(path) < 2){
        return;
    }

    GtkTreeIter iter;
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, BDY_UIN, &uin, -1);

    GtkWidget *cw = gqq_config_lookup_ht(cfg, "chat_window_map", uin); 
    if(cw != NULL){
        // We have open a window for this uin
        g_object_set(cw, "uin", uin, NULL);
        gtk_widget_show(cw);
        g_free(uin);
        return;
    }
    
    g_debug("Create chat window for %s(%s, %d)", uin, __FILE__, __LINE__);
    cw = qq_chatwindow_new(uin);
    gtk_widget_show(cw);
    gqq_config_insert_ht(cfg, "chat_window_map", uin, cw);
    g_free(uin);
#endif
}

static gboolean buddy_tree_on_rightbutton_click(GtkWidget* tree
		                                        , GdkEventButton* event
                                                , gpointer data)
{
    return FALSE;
}

static void tree_map_destroy_value(gpointer value)
{
    if(value == NULL){
        return;
    }
    GtkTreeRowReference *ref = value;
    gtk_tree_row_reference_free(ref);
}

GtkWidget* qq_buddy_tree_new()
{
    GtkWidget *view= gtk_tree_view_new();
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    // The client type column
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", BDY_TYPE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    gtk_tree_view_column_set_cell_data_func(column, renderer
                                        , qq_buddy_tree_pixbuf_cell_data_func
                                        , NULL, NULL);

    // The image column
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", BDY_IMAGE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    gtk_tree_view_column_set_cell_data_func(column, renderer
                                        , qq_buddy_tree_pixbuf_cell_data_func
                                        , NULL, NULL);

    // The text column
    column = gtk_tree_view_column_new();
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    gtk_tree_view_column_set_cell_data_func(column, renderer
                                        , qq_buddy_tree_text_cell_data_func
                                        , NULL, NULL);


    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
    gtk_tree_view_set_level_indentation(GTK_TREE_VIEW(view), -35);
    gtk_tree_view_set_hover_selection(GTK_TREE_VIEW(view), TRUE);

    gtk_widget_set_has_tooltip(view, TRUE);
	g_signal_connect(view, "query-tooltip"
                        , G_CALLBACK(buddy_tree_on_show_tooltip) , NULL);

	g_signal_connect(view
				   , "button_press_event"
				   , G_CALLBACK(buddy_tree_on_rightbutton_click)
				   , NULL);
	g_signal_connect(view
				   , "row-activated"
				   , G_CALLBACK(buddy_tree_on_double_click)
				   , NULL);

    tree_map = g_hash_table_new_full(g_str_hash, g_str_equal, g_free
                                        , tree_map_destroy_value);

    //create the chat window map
#if 0
    gqq_config_create_str_hash_table(cfg, "chat_window_map");
#endif
    return view;
}

//
// Create the QQTreeMap hash table.
//
static gboolean clear_treemap_destroy(gpointer key, gpointer value, gpointer data)
{
    return TRUE;
}

static void tree_store_set_buddy_info(GtkTreeStore *store, LwqqBuddy *bdy,
                                      GtkTreeIter *iter)
{
    /* FIXME, lwqq_chat_window APIs need to be wrapped */
    GtkWidget *cw = g_hash_table_lookup(lwqq_chat_window, bdy->uin);

    /* set markname and nick */
    gtk_tree_store_set(store, iter,
                       BDY_MARKNAME, bdy->markname ?: "",
                       BDY_NICK, bdy->nick ?: "", -1);
    if (cw) {
        g_object_set(cw, "name", bdy -> markname ?: "", NULL);
    }

    gtk_tree_store_set(store, iter,
                       BDY_LONGNICK, bdy->qqnumber ?: "",
                       BDY_STATUS, bdy->status ?: "offline",
                       BDY_QQNUMBER, bdy->qqnumber ?: "",
                       BDY_UIN, bdy->uin, -1);
    if (cw) {
        g_object_set(cw, "uin", bdy->qqnumber ?: "", NULL);
    }

    gchar buf[500];
    GdkPixbuf *pb;
    GError *err = NULL;
    int ct;
    if (bdy->client_type) {
        ct = atoi(bdy->client_type);
    } else {
        ct = 1;
    }
    switch(ct)
    {
    case 1:
		gtk_tree_store_set(store, iter, BDY_TYPE, NULL, -1);
        break;
    case 21:
        g_snprintf(buf, 500, "%s/phone.png", lwqq_icons_dir);
        pb = gdk_pixbuf_new_from_file_at_size(buf, 10, 15, &err);
        if(err != NULL){
            g_error_free(err);
            err = NULL;
        }
        gtk_tree_store_set(store, iter, BDY_TYPE, pb, -1);
        g_object_unref(pb);
        break;
    case 41:
        g_snprintf(buf, 500, "%s/webqq.png", lwqq_icons_dir);
        pb = gdk_pixbuf_new_from_file_at_size(buf, 15, 15, &err);
        if(err != NULL){
            g_error_free(err);
            err = NULL;
        }
        gtk_tree_store_set(store, iter, BDY_TYPE, pb, -1);
        g_object_unref(pb);
        break;
    default:
        break;
    }

    // set face image
    pb = create_face_image(bdy->qqnumber, 20, 20);
    gtk_tree_store_set(store, iter, BDY_IMAGE, pb, -1);
    g_object_unref(pb);

    // set the tool tip image
    pb = create_face_image(bdy->qqnumber, 80, 80);
    gtk_tree_store_set(store, iter, BDY_TOOLTIPIMAGE, pb, -1);
    g_object_unref(pb);
}

//
// Create the model of the contact tree
//
static GtkTreeModel *create_model(LwqqClient *lc)
{
    LwqqFriendCategory *cate;
    //Clear
    g_hash_table_foreach_remove(tree_map, clear_treemap_destroy, NULL);

    GtkTreeIter iter;
    GtkTreeStore *store = gtk_tree_store_new(COLUMNS,
                                             GDK_TYPE_PIXBUF,
                                             GDK_TYPE_PIXBUF,
                                             G_TYPE_STRING,
                                             G_TYPE_STRING,
                                             G_TYPE_STRING,
                                             G_TYPE_STRING,
                                             G_TYPE_STRING,
                                             G_TYPE_STRING,
                                             GDK_TYPE_PIXBUF,
                                             G_TYPE_INT,
                                             G_TYPE_INT,
                                             G_TYPE_INT);
    LIST_FOREACH(cate, &lc->categories, entries) {
        GtkTreeIter child;
        LwqqBuddy *buddy;
        int j = 0;
        gtk_tree_store_append(store, &iter, NULL);
        gtk_tree_store_set(store, &iter, BDY_MARKNAME, cate->name,
                           CATE_CNT, 0,
                           CATE_TOTAL, cate->count,
                           CATE_INDEX, cate->index, -1);

        /* add the buddies in this category. */
        LIST_FOREACH(buddy, &lc->friends, entries) {
            GtkTreePath *path;
            GtkTreeRowReference *ref;
            if (atoi(buddy->cate_index) != cate->index) {
                /* Find the buddy belong to this category */
                continue;
            }

            if (++j > cate->count) {
                lwqq_log(LOG_ERROR, "BUG!!!!!\n");
                break;
            }
            
            gtk_tree_store_append(store, &child, &iter);
            tree_store_set_buddy_info(store, buddy, &child);
            //get the tree row reference
            path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &child);
            ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(store), path);
            gtk_tree_path_free(path);

            //create and add a tree map
            g_hash_table_insert(tree_map, g_strdup(buddy->uin), ref);
            
        }
#if 0
        for (j = 0; j < cate->count; ++j) {
            gtk_tree_store_append(store, &child, &iter);
            bdy = (QQBuddy*) cate -> members -> pdata[j];
            
            tree_store_set_buddy_info(store, bdy, &child);

            //get the tree row reference
            path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &child);
            ref = gtk_tree_row_reference_new(GTK_TREE_MODEL(store), path);
            gtk_tree_path_free(path);

            //create and add a tree map
            g_hash_table_insert(tree_map, g_strdup(bdy -> uin -> str), ref);
        }
#endif
    }

    return GTK_TREE_MODEL(store);
}

//
// Update the face image of uin
//
#if 0
static void update_face_image(GtkWidget *tree, LwqqClient *lc, const gchar *uin)
{
#if 0
    QQBuddy *bdy = qq_info_lookup_buddy_by_uin(info , uin);
    if(bdy == NULL){
        return;
    }

    GtkTreeModel *model;
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
    GtkTreeRowReference *ref = g_hash_table_lookup(tree_map, uin);
    if(ref == NULL){
        g_warning("No TreeMap for %s(%s, %d)", uin, __FILE__, __LINE__);
        return;
    }

    GtkTreePath *path;
    GtkTreeIter iter;
    path = gtk_tree_row_reference_get_path(ref);
    gtk_tree_model_get_iter(model, &iter, path);

    GdkPixbuf *pb = create_face_image(bdy -> qqnumber -> str, 20, 20);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, BDY_IMAGE, pb, -1);
    g_object_unref(pb);
    pb = create_face_image(bdy -> qqnumber -> str, 80, 80);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, BDY_TOOLTIPIMAGE, pb, -1);
    g_object_unref(pb);

    gtk_tree_store_set(GTK_TREE_STORE(model), &iter
                            , BDY_QQNUMBER, bdy -> qqnumber -> str, -1);
    gtk_tree_path_free(path);
    
    // update the chat window
    GtkWidget *cw = gqq_config_lookup_ht(cfg, "chat_window_map", uin); 
    if(cw != NULL){
        g_object_set(cw, "qqnumber", bdy -> qqnumber -> str, NULL);
    }
#endif
}
#endif
void qq_buddy_tree_update_faceimg(GtkWidget *tree, LwqqClient *lc)
{
#if 0
    //update the face images
    gint i;
    QQBuddy *bdy;
    for(i = 0; i < info -> buddies -> len; ++i){
        bdy = (QQBuddy*)g_ptr_array_index(info -> buddies, i);
        if(bdy == NULL){
            continue;
        }
        update_face_image(tree, info, bdy -> uin -> str);
    }
#endif
}

void qq_buddy_tree_update_model(GtkWidget *tree, LwqqClient *lc)
{
    //update the contact tree
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree)
                            , create_model(lc));
}

void qq_buddy_tree_update_online_buddies(GtkWidget *tree, LwqqClient *lc)
{
    gint cate_cnt;
//    QQBuddy *bdy;
    GtkTreePath *path;
    GtkTreeIter iter, cate_iter;
    GtkTreeModel *model;
    GtkTreeRowReference *ref;
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));
    GtkWidget *cw;
    LwqqBuddy *bdy;

    // clear the online count
    LwqqFriendCategory *cate;
    LIST_FOREACH(cate, &lc->categories, entries) {
        if(!get_category_iter_by_index(model, cate->index, &cate_iter)){
            continue;
        }
        gtk_tree_store_set(GTK_TREE_STORE(model), &cate_iter, CATE_CNT, 0, -1);
    }

#define MOVE_TO_FIRST(x)                                                \
    LIST_FOREACH(bdy, &lc->friends, entries) {                          \
        if (g_strcmp0(bdy->status, x) == 0) {                           \
            ref = g_hash_table_lookup(tree_map, bdy->uin);              \
            if (ref == NULL) {                                          \
                g_warning("No TreeMap for %s. We may need add it..(%s, %d)" \
                          , bdy->uin, __FILE__, __LINE__);              \
                continue;                                               \
            }                                                           \
            path = gtk_tree_row_reference_get_path(ref);                \
            gtk_tree_model_get_iter(model, &iter, path);                \
            tree_store_set_buddy_info(GTK_TREE_STORE(model), bdy, &iter); \
            gtk_tree_path_free(path);                                   \
            gtk_tree_store_move_after(GTK_TREE_STORE(model), &iter, NULL); \
            if(!get_category_iter_by_index(model, atoi(bdy->cate_index), &cate_iter)){ \
                g_warning("No category's index is %s (%s, %d)", bdy -> cate_index \
                          , __FILE__, __LINE__);                        \
            }else{                                                      \
                gtk_tree_model_get(model, &cate_iter, CATE_CNT, &cate_cnt, -1); \
                ++cate_cnt;                                             \
                gtk_tree_store_set(GTK_TREE_STORE(model), &cate_iter,   \
                                   CATE_CNT, cate_cnt, -1);             \
            }                                                           \
            cw = g_hash_table_lookup(lwqq_chat_window, bdy->uin);       \
            if(cw){                                                     \
                g_object_set(cw, "status", bdy -> status ?: "offline", NULL); \
            }                                                           \
        }                                                               \
    }
    
    MOVE_TO_FIRST("busy");
    MOVE_TO_FIRST("away");
    MOVE_TO_FIRST("silent");
    MOVE_TO_FIRST("online");
    MOVE_TO_FIRST("callme");
#undef MOVE_TO_FIRST
}

void qq_buddy_tree_update_buddy_info(GtkWidget *tree, LwqqClient *lc)
{
    LwqqBuddy *bdy;
    GtkTreePath *path;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeRowReference *ref;
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree));

    LIST_FOREACH(bdy, &lc->friends, entries) {
        ref = g_hash_table_lookup(tree_map, bdy -> uin);
        if(ref == NULL){
            g_warning("No TreeMap for %s. We may need add it..(%s, %d)"
                      , bdy -> uin, __FILE__, __LINE__);
            continue;
        }
        path = gtk_tree_row_reference_get_path(ref);
        gtk_tree_model_get_iter(model, &iter, path);
        tree_store_set_buddy_info(GTK_TREE_STORE(model), bdy, &iter);
        gtk_tree_path_free(path);        
    }
}

