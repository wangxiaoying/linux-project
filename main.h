enum
{
    RESULT_TYPE = 0,
    RESULT_NAME,
    RESULT_PATH,
    RESULT_SHORTCUT,
    COL_NUM
};

typedef enum {APP, FOLDER, DOC, WEB, SYS, CAL} SUPPORT_TYPE;

struct RESULT
{
    SUPPORT_TYPE type;
    char path[100];
    char name[100];
};

struct PREFERENCE
{
    bool get_app;
    bool get_folder;
    bool get_document;
    bool get_web;
    bool get_sys;
    bool get_cal;

    bool file_txt;
    bool file_doc;
    bool file_c;
    bool file_h;

    char find_path[100];

    bool hotkey_space;
    bool hotkey_shift;
    bool hotkey_control;
    bool hotkey_option;

    bool auto_launch;
};

int search_engine_simulation(char *keywords, struct RESULT **resultList);


