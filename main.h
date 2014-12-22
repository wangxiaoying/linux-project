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
    char path[100];
    char name[100];
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

    char find_path[100];

    int hotkey_space;
    int hotkey_shift;
    int hotkey_control;
    int hotkey_option;

    int auto_launch;
}settings;

int search_engine_simulation(char *keywords, struct RESULT **resultList);


