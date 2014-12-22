enum
{
    RESULT_TYPE = 0,
    RESULT_NAME,
    RESULT_PATH,
    RESULT_SHORTCUT,
    COL_NUM
};

typedef enum {APP, FOLDER, DOC, WEB, SYS, CAL, SET} SUPPORT_TYPE;

struct RESULT
{
    SUPPORT_TYPE type;
    gchar path[100];
    gchar name[100];
};

struct PREFERENCE
{
    int get_app;
    int get_folder;
    int get_document;
    int get_web;
    int get_sys;
    int get_cal;

    int file_txt;
    int file_doc;
    int file_c;
    int file_h;

    gchar find_path[100];

    int hotkey_space;
    int hotkey_shift;
    int hotkey_control;
    int hotkey_option;

    int auto_launch;
}settings;

gboolean g_has_alfred;

int search_engine_simulation(gchar *keywords, struct RESULT **resultList);

void open_setting_window();


void on_filechooserbuttonPath_file_set(GtkFileChooserButton *widget, gpointer user_data);
void on_checkbutton_clicked (GtkButton *button, gpointer user_data);
void on_checkbutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void quit_alfred(GtkWidget *object, gpointer user_data);
gboolean on_listResult_key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
void on_textInput_changed(GtkEditable *textInput, gpointer listResult);

