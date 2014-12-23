#ifndef SEARCHING_H_INCLUDED
#define SEARCHING_H_INCLUDED

typedef enum {APP, FOLDER, DOC, WEB, SYS, CAL, SET} SUPPORT_TYPE;

struct RESULT
{
    SUPPORT_TYPE type;
    char path[300];
    char name[100];
};

struct PREFERENCES
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
    int multi_core;
    int pinyin_search;
};
#endif // SEARCHING_H_INCLUDED
