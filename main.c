#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade.h>
#include <string.h>
#include "main.h"

int i_result = 0;

char *get_type_title(SUPPORT_TYPE type)
{
    switch(type)
    {
        case APP: return "Application";
        case FOLDER: return "Folder";
        case DOC: return "Document";
        case WEB: return "WebPages";
        case SYS: return "System";
        case CAL: return "Calculator";
    }
}

SUPPORT_TYPE get_type_code(char *type)
{
    if(0 == strcmp("Application",type))
        return APP;
    else if(0 == strcmp("Folder",type))
        return FOLDER;
    else if(0 == strcmp("Document",type))
        return DOC;
    else if(0 == strcmp("WebPages",type))
        return WEB;
    else if(0 == strcmp("System",type))
        return SYS;
    else if(0 == strcmp("Calculator",type))
        return CAL;
    else
        return -1;
}

int search_engine_simulation(char *keywords, struct RESULT **resultList)
{
    int i;
    struct RESULT *response = malloc(5 * sizeof(struct RESULT));

    for(i = 0; i < 5; ++i)
    {
        sprintf(response[i].name, "test text name %d", i_result);
        //sprintf(response[i].path, "/home/momo/Downloads/SublimeText2/sublime_text", i_result++);
        sprintf(response[i].path, "/home/momo/Document/course/linux/jmp.c", i_result++);
        response[i].type = DOC;
    }
    *resultList = response;

    return 5;
}

void init_result_list(GtkWidget *listResult)
{
    GtkCellRenderer *renderer = NULL;
    GtkListStore *store = NULL;

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(listResult), -1, "Type", renderer, "text", RESULT_TYPE, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(listResult), -1, "Name", renderer, "text", RESULT_NAME, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(listResult), -1, "Path", renderer, "text", RESULT_PATH, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(listResult), -1, "ShortCut", renderer, "text", RESULT_SHORTCUT, NULL);

    store = gtk_list_store_new(COL_NUM, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    gtk_tree_view_set_model(GTK_TREE_VIEW(listResult), GTK_TREE_MODEL(store));
}

void init_settings()
{
    FILE *fp;
    fp = fopen("settings.txt", "rt+");
    if(NULL == fp)
    {

    }
}

void add_result_list(GtkWidget *listResult, struct RESULT *response, int size)
{
    //GtkTreeIter iter;
    char shortCut[100];
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(listResult)));

    gtk_list_store_clear(store);

    int i;
    for(i = 0; i < size; ++i)
    {
        GtkTreeIter *iter = malloc(sizeof(GtkTreeIter));
        gtk_list_store_append(store, iter);
        sprintf(shortCut, "Ctrl+%d", i);

        gtk_list_store_set(store, iter,
            RESULT_TYPE, get_type_title(response[i].type),
            RESULT_NAME, response[i].name,
            RESULT_PATH, response[i].path,
            RESULT_SHORTCUT, shortCut, -1);

    }
}

void open_file_or_application(char *type, char *name, char *path)
{
    printf("into open file function\n");
    printf("path: %s\n", path);
    printf("type: %s\t code: %d\n", type, get_type_code(type));
    char buffer[150];
    switch(get_type_code(type))
    {
        case APP:
            system(path);
            break;
        case FOLDER:
            break;
        case DOC:
            sprintf(buffer, "gedit --new-window %s", path);
            printf("buffer: %s\n", buffer);
            system(buffer);
            break;
        case WEB:
            break;
        case SYS:
            system(path);
            break;
        case CAL:
            break;
    }
}


/*************************** SIGNALS ***************************/

void on_textInput_changed(GtkEditable *textInput, gpointer listResult)
{
    int size;
    const gchar *keywords;
    printf("changed\n");

    keywords = gtk_entry_get_text(GTK_ENTRY(textInput));

    struct RESULT *resultList;
    size = search_engine_simulation(keywords, &resultList);

    add_result_list(listResult, resultList, size);
}



gboolean on_listResult_key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    printf("into key release event\n");

    GtkTreeSelection *selection = NULL;
    GtkTreeModel *store = NULL;
    GtkTreeIter iter;
    char *type, *name, *path;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
    store = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
    gtk_tree_selection_get_selected(selection, &store, &iter);


    switch(event->keyval)
    {
        case GDK_KEY_Return:
        {
            printf("press ENTER\n");
            gtk_tree_model_get(store, &iter,
                  RESULT_TYPE, &type,
                  RESULT_NAME, &name,
                  RESULT_PATH, &path, -1);
            open_file_or_application(type, name, path);

            free(type);
            free(name);
            free(path);

            break;
        }

    }




    return FALSE;
}

/*************************** SIGNALS ***************************/


int main (int argc, char *argv[])
{
    printf("main begin\n");
    GtkWidget *alfred = NULL;
    GtkWidget *textInput = NULL;
    GtkWidget *listResult = NULL;
    GtkListStore *store = NULL;

    GtkBuilder *builder = NULL;

    gtk_init(&argc, &argv);
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "interface.glade", NULL);

    alfred = GTK_WIDGET(gtk_builder_get_object(builder, "alfred"));
    textInput = GTK_WIDGET(gtk_builder_get_object(builder, "textInput"));
    listResult = GTK_WIDGET(gtk_builder_get_object(builder, "listResult"));

    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));

    init_result_list(listResult);

    g_signal_connect(G_OBJECT(alfred), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show(alfred);


    /* Enter the main loop */
    //gtk_widget_show_all (alfred);
    gtk_main ();
    return 0;
}


