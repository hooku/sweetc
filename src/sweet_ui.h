#pragma once

#include "resource.h"
#include "sweet_c_parser.h"

#define CS_VERSION          "2.0"

#define MAX_GENERAL_LENGTH	1024
#define MAX_TREE_DEPTH      16

#define MAX_LOADSTRING		100
#define OPEN_FILTER	_T("C++ Files (*.c; *.cpp; *.h; *.hpp)\0*.c;*.cpp;*.h;*.hpp\0All Files\0*.*\0")
#define SAVE_FILTER	_T("Webpage (*.html)\0*.html\0All Files\0*.*\0")

#define LV_ITEM_INCREMENT   100

#define ANI_INTERVAL        78          // ms
#define ANI_FRAME_TOTAL     8           // 8 pictures
#define TIMER_ANI           0x7788

#ifdef _DEBUG
#define sweet_print(...)    sweet_console_print(__VA_ARGS__)
#else // DEBUG_SUPPORT
#define sweet_print(...)
#endif // DEBUG_SUPPORT


typedef struct _SWEET_COLUMN_INFO
{
    int ImageIndex;
    TCHAR Column1[MAX_GENERAL_LENGTH];
    TCHAR Column2[MAX_GENERAL_LENGTH];
} SWEET_LISTVIEW_INFO;

extern TCHAR szTitle		[MAX_LOADSTRING];

extern HINSTANCE	hInst;
extern HWND			hWnd;
extern HWND			hWndRebar;
extern HWND			hWndToolbar; 
extern HWND			hWndStatusbar;
extern HWND         hWndProcess;

#ifdef _DEBUG
void sweet_console_print            (const TCHAR *, ...);
#endif
void sorry(HWND );

void statusbar_update		(UINT uID);

// global functions:
extern INT_PTR CALLBACK	dlgProcess  (HWND, UINT, WPARAM, LPARAM);

extern BOOL init_sweet_processdlg   (HWND );
extern BOOL init_sweet_progressbar  (unsigned int );

extern BOOL add_sweet_treeview_item (SWEET_C_FILE_INFO      *);
extern BOOL add_sweet_listview_item (SWEET_LISTVIEW_INFO    *);

extern VOID clear_sweet_treeview    ();
extern VOID clear_sweet_listview    ();

extern VOID update_sweet_status_description   (TCHAR *);
extern VOID update_sweet_progressbar();

extern VOID expand_sweet_tree       (SWEET_C_FILE_INFO      *);