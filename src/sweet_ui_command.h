#pragma once

#include "sweet_data_struct.h"

// switch
#define ENABLE_MULTI_THREAD


#define MAX_THREAD                  8
//#define CS_SPIN_COUNT               0x80000400          // which spin count should define?!
#define SEM_WAIT_TIMEOUT            INFINITE// 1000                 // 1000 ms

#define PERCENT(a, b)               (a * 100 / (b + 1)) + 1         // temp use only
#define FILE_KINDS                  20                  // how many file types can we recognize
#define SORRY(h)                    MessageBox(h, _T("you can't do that"), _T("sorry"), MB_ICONINFORMATION);

enum _SWEET_FILE_TYPE
{
    SWEET_FILE_TYPE_BASE,
    SWEET_FILE_FOLDER = SWEET_FILE_TYPE_BASE,
    SWEET_FILE_FOLDER_OPEN,
    SWEET_FILE_C,
    SWEET_FILE_CPP,
    SWEET_FILE_H,
    SWEET_FILE_TXT,
    SWEET_FILE_HTM,
    SWEET_FILE_CMD,
    SWEET_FILE_INF,
    SWEET_FILE_DLL,
    SWEET_FILE_EXE,
    SWEET_FILE_RAR,
    SWEET_FILE_DOCX,
    SWEET_FILE_XLSX,
    SWEET_FILE_UNKNOWN,
    SWEET_FILE_TYPE_TOP = SWEET_FILE_UNKNOWN,
};

enum _SWEET_LIST_TYPE
{
    SWEET_LIST_TYPE_BASE = SWEET_FILE_TYPE_TOP,
    SWEET_LIST_PROPERTY,
    SWEET_LIST_RETURN,
    SWEET_LIST_COMMENT,
    SWEET_LIST_FUNCTION,
    SWEET_LIST_EMPTY,
    SWEET_LIST_TYPE_TOP = SWEET_LIST_EMPTY,
};

typedef struct _FILE_ICON_TABLE
{
    TCHAR *FileType;
    unsigned int FileTypeIndex;                 // icon index
} FILE_ICON_TABLE;

typedef struct _PROC_THREAD_INFO
{
    unsigned int isFolder;
    TCHAR *szPath;
    HWND hDlg;
} PROC_THREAD_INFO;

typedef struct _ANA_THREAD_INFO
{
    char Path[MAX_PATH];
    S_TREE_NODE_INFO *Data;
} ANA_THREAD_INFO;

// global functions:
extern void refresh_tools   ();

extern int sweet_play_file  ();
extern int sweet_play_folder();
extern int sweet_play_export();
extern int sweet_play_about ();

extern int draw_list        (SWEET_C_FILE_INFO *);