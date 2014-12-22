#include <stdlib.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <vector>

typedef enum
{
    RESULT_ICON = 0,
    RESULT_DETAIL,
    RESULT_SHORTCUT,
    COL_NUM
};


/*
struct
{
}
*/

void on_textInput_changed(GtkEditable *textInput, gpointer listResult)
{
    printf("changed\n");

    GtkTreeIter iter;
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(listResult)));

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "mamama", -1);

}


int main (int argc, char *argv[])
{
    printf("main begin\n");
    GtkWidget *alfred = NULL;
    GtkWidget *textInput = NULL;
    GtkWidget *listResult = NULL;
    GtkListStore *store = NULL;
    GtkTreeViewColumn *column = NULL;
    GtkTreeIter iter;
    GtkCellRenderer *renderer = NULL;

    GtkBuilder *builder = NULL;

    gtk_init(&argc, &argv);
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "interface.glade", NULL);

    alfred = GTK_WIDGET(gtk_builder_get_object(builder, "alfred"));
    textInput = GTK_WIDGET(gtk_builder_get_object(builder, "textInput"));
    listResult = GTK_WIDGET(gtk_builder_get_object(builder, "listResult"));

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));


    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(listResult), -1, "Result", renderer, "text", 0, NULL);

    store = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "hahaha", -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "lalala", -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "wawawa", -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "mamama", -1);

    gtk_tree_view_set_model(GTK_TREE_VIEW(listResult), GTK_TREE_MODEL(store));



    printf("show widget\n");
    gtk_widget_show(alfred);
    printf("finish show widget\n");

    std::vector<int> v;
    v.push_back(1);

    /* Enter the main loop */
    //gtk_widget_show_all (alfred);
    gtk_main ();
    return 0;
}

