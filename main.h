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
    gchar path[300];
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
    int multi_core;
    int pinyin_search;
}settings, settings_old;

gboolean g_has_alfred;
gboolean g_is_quiting;
gboolean g_change_index;

int search_engine_simulation(gchar *keywords, struct RESULT **resultList, struct PREFERENCE *settings, int onlyName, int maxLength);




