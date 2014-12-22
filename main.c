#include <stdlib.h>
#include <stdio.h>
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
        case SET: return "Settings";
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
    else if(0 == strcmp("Settings", type))
        return SET;
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
        response[i].type = SET;
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

void write_settings()
{
    FILE *fp;
    char buffer[100];
    fp = fopen("settings.txt", "w");
    if(NULL != fp)
    {
        sprintf(buffer, "%d", settings.auto_launch);
        fputs(buffer, fp);
        sprintf(buffer, "%d%d%d%d%d%d", settings.get_app, settings.get_folder, settings.get_document, settings.get_web, settings.get_sys, settings.get_cal);
        fputs(buffer, fp);
        sprintf(buffer, "%d%d%d%d", settings.file_txt, settings.file_doc, settings.file_c, settings.file_h);
        fputs(buffer, fp);
        sprintf(buffer, "%d%d%d%d\n", settings.hotkey_space, settings.hotkey_shift, settings.hotkey_control, settings.hotkey_option);
        fputs(buffer, fp);
        sprintf(buffer, "%s\n", settings.find_path);
        fputs(buffer, fp);
        fclose(fp);
    }
}

void init_settings()
{
    FILE *fp;
    fp = fopen("settings.txt", "r");
    char buffer[100];
    if(NULL == fp)
    {
        printf("null fp\n");
        settings.auto_launch = FALSE;
        settings.get_app = TRUE;
        settings.get_folder = TRUE;
        settings.get_document = TRUE;
        settings.get_web = TRUE;
        settings.get_sys = TRUE;
        settings.get_cal = TRUE;
        settings.file_txt = TRUE;
        settings.file_doc = FALSE;
        settings.file_c = FALSE;
        settings.file_h = FALSE;
        settings.hotkey_space = TRUE;
        settings.hotkey_shift = FALSE;
        settings.hotkey_control = TRUE;
        settings.hotkey_option = FALSE;
        sprintf(settings.find_path, "/home/momo/Document");
        return;
    }
    fgets(buffer, 100, fp);
    settings.auto_launch = buffer[0] - '0';
    settings.get_app = buffer[1] - '0';
    settings.get_folder = buffer[2] - '0';
    settings.get_document = buffer[3] - '0';
    settings.get_web = buffer[4] - '0';
    settings.get_sys = buffer[5] - '0';
    settings.get_cal = buffer[6] - '0';
    settings.file_txt = buffer[7] - '0';
    settings.file_doc = buffer[8] - '0';
    settings.file_c = buffer[9] - '0';
    settings.file_h = buffer[10] - '0';
    settings.hotkey_space = buffer[11] - '0';
    settings.hotkey_shift = buffer[12] - '0';
    settings.hotkey_control = buffer[13] - '0';
    settings.hotkey_option = buffer[14] - '0';
    fgets(buffer, 100, fp);
    sprintf(settings.find_path, "%s", buffer);
    settings.find_path[strlen(settings.find_path)-1]=0;
    fclose(fp);
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
        case SET:
            open_setting_window();
            break;
    }
}

void open_setting_window()
{
    char buffer[100];

    GtkWidget *preferences = NULL;
    GtkWidget *checkButtonSpace = NULL;
    GtkWidget *checkbuttonOption = NULL;
    GtkWidget *checkbuttonShift = NULL;
    GtkWidget *checkbuttonControl = NULL;
    GtkWidget *checkbuttonAutoLaunch = NULL;
    GtkWidget *buttonQuit = NULL;
    GtkWidget *checkbuttonApp = NULL;
    GtkWidget *checkbuttonFolder = NULL;
    GtkWidget *checkbuttonDoc = NULL;
    GtkWidget *checkbuttonWeb = NULL;
    GtkWidget *checkbuttonSys = NULL;
    GtkWidget *checkbuttonCal = NULL;
    GtkWidget *checkbuttonFileTxt = NULL;
    GtkWidget *checkbuttonFileDoc = NULL;
    GtkWidget *checkbuttonFileC = NULL;
    GtkWidget *checkbuttonFileH = NULL;
    GtkWidget *entrySearchPath = NULL;
    GtkWidget *filechooserbuttonPath = NULL;

    GtkBuilder *builder = NULL;

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "interface.glade", NULL);

    preferences = GTK_WIDGET(gtk_builder_get_object(builder, "preferences"));
    checkButtonSpace = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonSpace"));
    checkbuttonOption = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonOption"));
    checkbuttonShift = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonShift"));
    checkbuttonControl = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonControl"));
    checkbuttonAutoLaunch = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonAutoLaunch"));
    buttonQuit = GTK_WIDGET(gtk_builder_get_object(builder, "buttonQuit"));
    checkbuttonApp = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonApp"));
    checkbuttonFolder = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonFolder"));
    checkbuttonDoc = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonDoc"));
    checkbuttonWeb = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonWeb"));
    checkbuttonSys = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonSys"));
    checkbuttonCal = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonCal"));
    checkbuttonFileTxt = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonFileTxt"));
    checkbuttonFileDoc = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonFileDoc"));
    checkbuttonFileC = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonFileC"));
    checkbuttonFileH = GTK_WIDGET(gtk_builder_get_object(builder, "checkbuttonFileH"));
    entrySearchPath = GTK_WIDGET(gtk_builder_get_object(builder, "entrySearchPath"));
    filechooserbuttonPath = GTK_WIDGET(gtk_builder_get_object(builder, "filechooserbuttonPath"));

    if(settings.get_app) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonApp), TRUE);
    if(settings.get_folder) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonFolder), TRUE);
    if(settings.get_document) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonDoc), TRUE);
    if(settings.get_web) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonWeb), TRUE);
    if(settings.get_sys) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonSys), TRUE);
    if(settings.get_cal) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonCal), TRUE);
    if(settings.hotkey_space) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkButtonSpace), TRUE);
    if(settings.hotkey_shift) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonShift), TRUE);
    if(settings.hotkey_control) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonControl), TRUE);
    if(settings.hotkey_option) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonOption), TRUE);
    if(settings.file_txt) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonFileTxt), TRUE);
    if(settings.file_doc) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonFileDoc), TRUE);
    if(settings.file_c) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonFileC), TRUE);
    if(settings.file_h) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbuttonFileH), TRUE);

    gtk_entry_set_text(entrySearchPath, settings.find_path);
    printf("find path: %s\n", settings.find_path);

    gtk_widget_show(preferences);

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

void quit_alfred(GtkWidget *object, gpointer user_data)
{
    printf("quit alfred in\n");
    write_settings();
    gtk_main_quit();
}

void on_checkbutton_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    printf("check button toggled\n");
}

void
on_checkbutton_clicked (GtkButton *button, gpointer user_data)
{
    printf("check button clicked\n");
}

on_filechooserbuttonPath_file_set

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
    init_settings();

    //g_signal_connect(G_OBJECT(alfred), "destroy", G_CALLBACK(quit_alfred), NULL);

    gtk_widget_show(alfred);


    /* Enter the main loop */
    //gtk_widget_show_all (alfred);
    gtk_main ();
    return 0;
}


