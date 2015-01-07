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
void *g_database;
int *g_index_done;

extern int idx_query(void *db, char* queryString, gboolean onlyInName, int maxResults, struct RESULT **results,const struct PREFERENCES *pref);
extern int* idx_create_database_task(void* files, const struct PREFERENCES *pref, void **db);
extern int idx_add_documents(const char *directory, int recursive, void **collection, int boost);
extern int idx_set_stopwords(const char *dictfile);
extern void idx_set_verbose(int enabled);
extern void idx_delete_database(void *db);
extern int idx_save_database(const char *dbfile, void *db);
extern int idx_load_database(const char *dbfile, void **db);
extern int idx_init_dict();




