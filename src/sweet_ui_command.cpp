// play_ui_command.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "sweet_ui.h"
#include "sweet_ui_command.h"

#include "sweet_c_parser.h"

/* multi-thread synchronization */
unsigned int        numberOfCores   = 1;        // multi-thread
CRITICAL_SECTION    cs              = { 0 };
HANDLE              hSemEmpty       = NULL;
HANDLE              hSemFull        = NULL;

ANA_THREAD_INFO     tAnaInfo        = { 0 };
int                 exitThread      = 0;

S_TREE_ROOT *sTreeRoot;         // as root (PTR) (invisible to other proj?)

FILE_ICON_TABLE fileTypeTable[] = {
    {_T("."     ) , SWEET_FILE_FOLDER       },
    {_T(".c"    ) , SWEET_FILE_C            },
    {_T(".cpp"  ) , SWEET_FILE_CPP          },
    {_T(".h"    ) , SWEET_FILE_H            },
    {_T(".txt"  ) , SWEET_FILE_TXT          },
    {_T(".htm"  ) , SWEET_FILE_HTM          },
    {_T(".cmd"  ) , SWEET_FILE_CMD          },
    {_T(".inf"  ) , SWEET_FILE_INF          },
    {_T(".dll"  ) , SWEET_FILE_DLL          },
    {_T(".exe"  ) , SWEET_FILE_EXE          },
    {_T(".rar"  ) , SWEET_FILE_RAR          },
    {_T(".docx" ) , SWEET_FILE_DOCX         },
    {_T(".xlsx" ) , SWEET_FILE_XLSX         },

    {_T(".html" ) , SWEET_FILE_HTM          },
    {_T(".bat"  ) , SWEET_FILE_CMD          },
    {_T(".ini"  ) , SWEET_FILE_INF          },
    {_T(".zip"  ) , SWEET_FILE_RAR          },
    {_T(".cab"  ) , SWEET_FILE_RAR          },
    {_T(".doc"  ) , SWEET_FILE_DOCX         },
    {_T(".xls"  ) , SWEET_FILE_XLSX         },
};

// forward declarations of functions included in this code module:
int cmd_open_file           (TCHAR *);
int cmd_save_file           (TCHAR *);
int cmd_open_folder         (TCHAR *);

int create_proc_thread  (PROC_THREAD_INFO *);
DWORD WINAPI proc_thread(PVOID );

int create_analysis_thread  (unsigned int );
DWORD WINAPI analysis_thread(PVOID );

int cmd_analysis_file       (TCHAR *);
int cmd_analysis_folder     (TCHAR *);

void cmd_copy               ();
void cmd_export             (TCHAR *);
void cmd_about              ();

int open_save_box           (TCHAR *, BOOL );

int detect_cpu              (unsigned int *);

int traverse_folder         (TCHAR *, S_TREE_NODE *, S_TREE_ROOT *);

int draw_tree               ();
int draw_tree_cb            (S_TREE_NODE_INFO *);

void refresh_tools()
{
	return ;
}

// these are public functions:
int sweet_play_file()
{
    TCHAR szFile[MAX_PATH] = { 0 };
    if (cmd_open_file(szFile))
    {
        SORRY(hWnd);
        return -1;      // bye-bye
        cmd_analysis_file(szFile);
    }
    return 1;
}

int sweet_play_folder()
{
    TCHAR szFolder[MAX_PATH] = { 0 };
    if (cmd_open_folder(szFolder))
    {
        cmd_analysis_folder(szFolder);
    }
    return 1;
}

int sweet_play_export()
{
    TCHAR szFile[MAX_PATH] = { 0 };
    if (cmd_save_file(szFile))
    {
        cmd_export(szFile);
    }
    return 1;
}

int sweet_play_about()
{
    cmd_about();
    return 1;
}

// these are private functions:
int cmd_open_file(TCHAR *szFile)
{
    sweet_print(_T("COMMAND_OPEN_FILE"));

    if (open_save_box(szFile, FALSE))
    {
        sweet_print(szFile);
    }
    else
    {
        return 0;
    }

	return 1;
}

int cmd_save_file(TCHAR *szFile)
{
    sweet_print(_T("COMMAND_SAVE_FILE"));

    if (open_save_box(szFile, TRUE))
    {
        sweet_print(szFile);
        SORRY(hWnd);
    }
    else
    {
        return 0;
    }

    return 1;
}

int cmd_open_folder(TCHAR *szFolder)
{
    sweet_print(_T("COMMAND_OPEN_FOLDER"));

    BROWSEINFO      bi   = { 0 };
    LPITEMIDLIST    lpDList;

    TCHAR szTitle[MAX_LOADSTRING];
    LoadString(hInst, IDS_PROMPT_BROWSE_FOLDER, szTitle, MAX_LOADSTRING);

#ifdef _DEBUG
    //bi.pidlRoot     = ILCreateFromPath(_T("F:\\241\\S2W_MINE\\userapps\\s2w\\src"));
    //bi.pidlRoot     = ILCreateFromPath(_T("F:\\241\\S2W_MINE"));
#endif

    bi.hwndOwner    = hWnd;
    bi.lpszTitle    = szTitle;
    bi.ulFlags      = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;

    lpDList = SHBrowseForFolder(&bi);

    if (lpDList != NULL)
    {
        SHGetPathFromIDList(lpDList, szFolder);
        CoTaskMemFree(lpDList);
    }
    else
    { // didn't select the folder
        return 0;
    }

    return 1;
}

int create_proc_thread(PROC_THREAD_INFO *tInfo)
{
    DWORD   dwThreadId;
    HANDLE  hThread   ;

        hThread = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size  
            proc_thread,            // thread function name
            tInfo,                  // argument to thread function 
            0,                      // use default creation flags 
            &dwThreadId);           // returns the thread identifier 

    return 1;
}

int create_analysis_thread(unsigned int threadCount)
{
    // prepare critical section:
    InitializeCriticalSection(&cs);

    // prepare semaphore:

    hSemEmpty = CreateSemaphore( 
        NULL,           // default security attributes
        threadCount,    // initial count        ( 0 available, we need to call release first to increase )
        threadCount,    // maximum count
        NULL);          // unnamed semaphore
    hSemFull = CreateSemaphore( 
        NULL,           // default security attributes
        0,              // initial count        ( 0 available, we need to call release first to increase )
        threadCount,    // maximum count
        NULL);          // unnamed semaphore

    if (!hSemEmpty || !hSemFull)
    {
        sweet_print(_T("CreateSemaphore error: %d"), GetLastError());
        return -1;
    }

    DWORD   dwThreadId  [MAX_THREAD];
    HANDLE  hThread     [MAX_THREAD];

    for (unsigned int iThread = 0; iThread < threadCount; iThread ++)
    {
        hThread[iThread] = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            analysis_thread,        // thread function name
            NULL,                   // argument to thread function 
            0,                      // use default creation flags 
            &dwThreadId[iThread]);           // returns the thread identifier 
    }

    return 1;
}

DWORD WINAPI proc_thread(PVOID lpParam)
{
    PROC_THREAD_INFO *tInfo = (PROC_THREAD_INFO *)lpParam;
    
    sweet_print(_T("analysis thread created"));

    if (tInfo->isFolder)
    {
        /* folder: */
        // initialize Bi-Tree for children:
        tree_init(&sTreeRoot);

        // copy name:
        if (_tcslen(tInfo->szPath) > 3) // exclude root path
        {
            TCHAR *szFolderName = _tcsrchr(tInfo->szPath, _T('\\'));
            _tcscpy(sTreeRoot->sTree->data.FileName, szFolderName + 1);
        }
        else
        {
            _tcscpy(sTreeRoot->sTree->data.FileName, tInfo->szPath);
        }
        // copy the root folder path:
        _tcscpy(sTreeRoot->sTree->data.FilePath, tInfo->szPath);

        traverse_folder(tInfo->szPath, sTreeRoot->sTree, sTreeRoot);

        // everything is prepared, let's tell UI:
        init_sweet_progressbar(sTreeRoot->sProjInfo.FileCount);

#ifdef ENABLE_MULTI_THREAD
        // let's begin analysis:
        detect_cpu(&numberOfCores);
#else
        numberOfCores = 1;
#endif

        exitThread = 0;

        create_analysis_thread(numberOfCores);

        draw_tree();
        exitThread = 1;     // tell threads to exit
    }
    else
    {
        /* file: */
        //sweet_c_parse(tInfo->szPath, &cfInfo);
    }

    expand_sweet_tree(&sTreeRoot->sTree->data);

    // too fast?
    //EndDialog(tInfo->hDlg, IDCANCEL);
    // don't use end dialog, we should send a WM_QUIT message to politely quit the model window
    SendMessage(hWndProcess, WM_CLOSE, 0, 0);

    return 0;
}

DWORD WINAPI analysis_thread(PVOID lpParam)
{
    // save a local copy:
    char filePath[MAX_PATH];
    S_TREE_NODE_INFO *nodeData;

    while (!exitThread)
    {
        // fetch parameter (requires lock)
            if (WaitForSingleObject(hSemFull, SEM_WAIT_TIMEOUT) == WAIT_OBJECT_0)   // Full --
            {
        EnterCriticalSection(&cs);
                strcpy(filePath, tAnaInfo.Path);
                nodeData = tAnaInfo.Data;
        LeaveCriticalSection(&cs);
                ReleaseSemaphore(hSemEmpty, 1, NULL);   // Empty ++
                // call analysis
                sweet_c_parse(filePath, nodeData);
            }
        //LeaveCriticalSection(&cs);      // why not call twice?!
    }
    
    return 0;
}

int cmd_analysis_file(TCHAR *szFile)
{
    sweet_print(_T("COMMAND_ANALYSIS_FILE"));

    PROC_THREAD_INFO tProcInfo = { 1, szFile, NULL };

    // do the wide->multibyte convertion:
    char filePath[MAX_PATH];
    WideCharToMultiByte(CP_OEMCP, 0, szFile, -1, filePath, MAX_PATH, NULL, NULL);
        
    SWEET_C_FILE_INFO cfInfo = { 0 };

    create_proc_thread(&tProcInfo);

    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PROCESS), hWnd, dlgProcess, (LPARAM)&tProcInfo.hDlg);

    sweet_print(_T("Visible Chars: %d\r\nLines: %d\r\nSpaces: %d\r\nTabs: %d\r\n"),
        cfInfo.VisibleCharCount, cfInfo.LineCount, cfInfo.SpaceCount, cfInfo.TabCount);
    
    return 1;
}

int cmd_analysis_folder(TCHAR *szFolder)
{
    sweet_print(_T("COMMAND_ANALYSIS_FOLDER"));

    PROC_THREAD_INFO tInfo = { 1, szFolder, NULL };
    create_proc_thread(&tInfo);

    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PROCESS), hWnd, dlgProcess, (LPARAM)&tInfo.hDlg);
    return 1;
}

void cmd_copy()
{
    sweet_print(_T("COMMAND_COPY"));

    //SendMessage(hWndRichedit, WM_COPY, 0, 0);
    return ;
}


void cmd_export(TCHAR *szFile)
{
    sweet_print(_T("COMMAND_EXPORT"));
    return ;
}

void cmd_about()
{
    TCHAR szAboutTitle[MAX_LOADSTRING], szAboutDescription[MAX_PATH];
    TCHAR szTmp[MAX_LOADSTRING];

    LoadString(hInst, IDS_APP_TITLE, szAboutTitle, MAX_LOADSTRING);
    
    LoadString(hInst, IDS_APP_TITLE, szTmp, MAX_LOADSTRING);
    _tcscpy(szAboutDescription, szTmp);
    _tcscat(szAboutDescription, _T("\r\n"));

    LoadString(hInst, IDS_ABOUT_VERSION, szTmp, MAX_LOADSTRING);
    _tcscat(szAboutDescription, szTmp);
    _tcscat(szAboutDescription, _T(" "));
    _tcscat(szAboutDescription, _T(CS_VERSION));
    _tcscat(szAboutDescription, _T("\r\n"));
    _tcscat(szAboutDescription, _T("\r\n"));

    LoadString(hInst, IDS_ABOUT_COPYRIGHT, szTmp, MAX_LOADSTRING);
    _tcscat(szAboutDescription, szTmp);

    _tcscat(szAboutDescription, _T("\r\n"));

    LoadString(hInst, IDS_ABOUT_WEB, szTmp, MAX_LOADSTRING);
    _tcscat(szAboutDescription, szTmp);

    MessageBox(hWnd, szAboutDescription, szAboutTitle, MB_ICONEXCLAMATION | MB_OK);
    return ;
}

int open_save_box(TCHAR *path, BOOL isSave)
{
	OPENFILENAME	ofn					= { 0 };	// common dialog box structure
	TCHAR			szFile[MAX_PATH]	= { 0 };	// buffer for file name

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize		= sizeof(ofn);
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFile		= szFile;

	ofn.nMaxFile		= MAX_PATH;
	ofn.nFilterIndex	= 1;
	ofn.lpstrFileTitle	= NULL;
	ofn.nMaxFileTitle	= 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags			= OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	// Display the Open dialog box.

	if (isSave)
	{
        ofn.lpstrFilter = SAVE_FILTER;
		if (GetSaveFileName(&ofn))
		{
			_tcscpy(path, ofn.lpstrFile);
		}
        else
        {
            return 0;
        }
	}
	else
	{
        ofn.lpstrFilter = OPEN_FILTER;
		if (GetOpenFileName(&ofn))
		{
			_tcscpy(path, ofn.lpstrFile);
		}
        else
        {
            return 0;
        }
	}
	return 1;
}

int detect_cpu(unsigned int *numberOfCores)
{
    SYSTEM_INFO si;

    GetSystemInfo(&si);

    *numberOfCores = si.dwNumberOfProcessors;

#if defined _DEBUG && defined ENABLE_MULTI_THREAD
    TCHAR szMsg[MAX_LOADSTRING];
    _stprintf(szMsg, _T("你有%d个核"), si.dwNumberOfProcessors);
    if (MessageBox(hWndProcess, _T("“要用多线程加速吗？？？？”"), szMsg, MB_YESNO | MB_ICONHAND) == IDNO)
        *numberOfCores = 1;  // single thread
#endif

    return 1;
}

int traverse_folder(TCHAR *szFolder, S_TREE_NODE *sTreeParent, S_TREE_ROOT *sTreeRoot)
{
    WIN32_FIND_DATA wfd;

    TCHAR szDir[MAX_PATH]; //*******ABC
    TCHAR szSubFolder[MAX_PATH];

    _tcscpy(szDir, szFolder);
    _tcscat(szDir, _T("\\*"));          // wildcard

    HANDLE hFind = INVALID_HANDLE_VALUE;
    hFind = FindFirstFile(szDir, &wfd);

    if (hFind == INVALID_HANDLE_VALUE) 
    {
        return -1;
    }

    int makeSiblingCount = 0;           // a counter that to assure make child first

    do
    {
        if (_tcscmp(wfd.cFileName, _T(".")) == 0 || _tcscmp(wfd.cFileName, _T("..")) == 0)  // escape "." and ".."
            continue;   // escape the loop to next

        S_TREE_NODE *sTreeNode = NULL;
        tree_create_child(&sTreeNode);

        sTreeRoot->sProjInfo.FileCount ++;

        _tcscpy(sTreeNode->data.FilePath, szFolder);
        _tcscpy(sTreeNode->data.FileName, wfd.cFileName);
        
        sTreeNode->data.FileSize = wfd.nFileSizeLow;
        sTreeNode->data.FileTime = wfd.ftLastWriteTime;

        sTreeNode->data.FileType = SWEET_FILE_UNKNOWN;

        TCHAR *suffixName = _tcsrchr(wfd.cFileName, _T('.'));

        if (suffixName)
        {
            for (int iFileType = 0; iFileType < FILE_KINDS; iFileType ++)
            {
                if (!_tcsicmp(fileTypeTable[iFileType].FileType, suffixName)) // same
                {
                    sTreeNode->data.FileType = fileTypeTable[iFileType].FileTypeIndex;
                    break;
                }
            }
        }

        //@sTreeNode->data.FileType =;
        sTreeNode->data.Depth = sTreeParent->data.Depth;

        if (makeSiblingCount == 0)
        {
            sTreeNode->data.Depth ++;   // record the depth for tree (not Bi-Tree)
        }

        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            sweet_print(_T("DIR:\t%s"), wfd.cFileName);
            // Forge the subdirectory for recursive:
            _tcscpy(szSubFolder, szFolder);
            _tcscat(szSubFolder, _T("\\"));
            _tcscat(szSubFolder, wfd.cFileName);

            sTreeNode->data.FileType = SWEET_FILE_FOLDER;
            
            tree_attach_child(sTreeParent, makeSiblingCount, sTreeNode); // Sib
            makeSiblingCount ++;

            traverse_folder(szSubFolder, sTreeNode, sTreeRoot);
        }
        else
        {
            sweet_print(_T("FILE:\t%s"), wfd.cFileName);
            // Bi-Tree: L=Child; R=Sibling
            tree_attach_child(sTreeParent, makeSiblingCount, sTreeNode); // Sib
            makeSiblingCount ++;
        }
        
        sTreeParent = sTreeNode;

    }
     while (FindNextFile(hFind, &wfd));

    return 1;
}

int draw_tree() // draw the tree view
{
    tree_traverse_node(sTreeRoot->sTree, draw_tree_cb);
    return 1;
}

int draw_tree_cb(S_TREE_NODE_INFO *data)
{
    sweet_print(_T("Traversed:%s"), data->FileName);

    static TCHAR szProcParse[MAX_LOADSTRING], szProcSkip[MAX_LOADSTRING];
    static int szProcParseLen = 0;

    TCHAR szTmp[MAX_PATH];

    if (!szProcParseLen)
    {
        szProcParseLen = LoadString(hInst, IDS_PROC_PARSE, szProcParse, MAX_LOADSTRING);
        LoadString(hInst, IDS_PROC_SKIP, szProcSkip, MAX_LOADSTRING);
    }

    //Sleep(100);

    static int skipLabelDisplayed = 0;

    if (data->FileType >= SWEET_FILE_C && data->FileType <= SWEET_FILE_H)
    { // parse c files only:
        TCHAR szFilePath    [MAX_PATH];
        //char filePath       [MAX_PATH];

        _tcscpy(szFilePath, data->FilePath);

        if (_tcslen(szFilePath) > 3)
        {
            _tcscat(szFilePath, _T("\\"));
        }
        
        _tcscat(szFilePath, data->FileName);

        //sweet_print(_T("parsing..."));

        _tcscpy(szTmp, szProcParse);
        _tcscat(szTmp, data->FileName);
        _tcscat(szTmp, _T(".."));
        update_sweet_status_description(szTmp);

        WaitForSingleObject(hSemEmpty, INFINITE);   // Empty --

        // store into global bridge
        EnterCriticalSection(&cs);
            WideCharToMultiByte(CP_OEMCP, 0, szFilePath, -1, tAnaInfo.Path, MAX_PATH, NULL, NULL);
            //strcpy(tAnaInfo.Path, filePath);
            tAnaInfo.Data = data;
        LeaveCriticalSection(&cs);

        // ++
        ReleaseSemaphore(hSemFull, 1, NULL);    // when full, auto block (Full ++)
        //sweet_c_parse(filePath, data);

        skipLabelDisplayed = 0;
    }
    else
    {
        if (!skipLabelDisplayed)
        {
            _tcscpy(szTmp, szProcSkip);
            update_sweet_status_description(szTmp);
            skipLabelDisplayed = 1;
        }
    }

    add_sweet_treeview_item(data);
    update_sweet_progressbar();
    
    return 1;
}

int draw_list(SWEET_C_FILE_INFO *pCFInfo) // draw the listview
{
    clear_sweet_listview();

    SWEET_LISTVIEW_INFO lvInfo = { 0 };

    //TCHAR szTmp[MAX_LOADSTRING];

    /* file name */
    lvInfo.ImageIndex = pCFInfo->FileType;
    LoadString(hInst, IDS_LIST_ITEM_FILENAME, lvInfo.Column1, MAX_LOADSTRING);
    _tcscpy(lvInfo.Column2, pCFInfo->FileName);
    add_sweet_listview_item(&lvInfo);

    /* file size */
    lvInfo.ImageIndex = SWEET_LIST_PROPERTY;
    LoadString(hInst, IDS_LIST_ITEM_FILESIZE, lvInfo.Column1, MAX_LOADSTRING);
    if (pCFInfo->FileSize)
    {
        _stprintf(lvInfo.Column2, _T("%d Bytes"), pCFInfo->FileSize);
    }
    else
    {
        _tcscpy(lvInfo.Column2, _T("0"));
    }
    
    add_sweet_listview_item(&lvInfo);

    /* file date */
    lvInfo.ImageIndex = SWEET_LIST_PROPERTY;
    LoadString(hInst, IDS_LIST_ITEM_FILEDATE, lvInfo.Column1, MAX_LOADSTRING);

    SYSTEMTIME stUTC, stLocal;
    FileTimeToSystemTime(&pCFInfo->FileTime, &stUTC);           // to UTC
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);    // to LOCAL
    _stprintf(lvInfo.Column2, _T("%02d-%02d-%02d  %02d:%02d"),
        stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour, stLocal.wMinute);
    add_sweet_listview_item(&lvInfo);

    // source file analysis

    /* line count */
    if (pCFInfo->FileType>= SWEET_FILE_C && pCFInfo->FileType <= SWEET_FILE_H)
    {
    // CR count
        lvInfo.ImageIndex = SWEET_LIST_RETURN;
        LoadString(hInst, IDS_LIST_ITEM_CR, lvInfo.Column1, MAX_LOADSTRING);
        _stprintf(lvInfo.Column2, _T("%d"), pCFInfo->CRCount);
        add_sweet_listview_item(&lvInfo);

    // LF count
        lvInfo.ImageIndex = SWEET_LIST_RETURN;
        LoadString(hInst, IDS_LIST_ITEM_LF, lvInfo.Column1, MAX_LOADSTRING);
        _stprintf(lvInfo.Column2, _T("%d"), pCFInfo->LFCount);
        add_sweet_listview_item(&lvInfo);

    // CRLF count
        lvInfo.ImageIndex = SWEET_LIST_RETURN;
        LoadString(hInst, IDS_LIST_ITEM_CRLF, lvInfo.Column1, MAX_LOADSTRING);
        _stprintf(lvInfo.Column2, _T("%d"), pCFInfo->CRLFCount);
        add_sweet_listview_item(&lvInfo);

    // Actual line count
        lvInfo.ImageIndex = SWEET_LIST_RETURN;
        LoadString(hInst, IDS_LIST_ITEM_LINE, lvInfo.Column1, MAX_LOADSTRING);
        _stprintf(lvInfo.Column2, _T("%d"), pCFInfo->LineCount);
        add_sweet_listview_item(&lvInfo);

    // Space count
        lvInfo.ImageIndex = SWEET_LIST_PROPERTY;
        LoadString(hInst, IDS_LIST_ITEM_SPACE, lvInfo.Column1, MAX_LOADSTRING);
        _stprintf(lvInfo.Column2, _T("%d"), pCFInfo->SpaceCount);
        add_sweet_listview_item(&lvInfo);

    // Tab count
        lvInfo.ImageIndex = SWEET_LIST_PROPERTY;
        LoadString(hInst, IDS_LIST_ITEM_TAB, lvInfo.Column1, MAX_LOADSTRING);
        _stprintf(lvInfo.Column2, _T("%d"), pCFInfo->TabCount);
        add_sweet_listview_item(&lvInfo);

    // Comment count
        lvInfo.ImageIndex = SWEET_LIST_COMMENT;
        LoadString(hInst, IDS_LIST_ITEM_COMMENT, lvInfo.Column1, MAX_LOADSTRING);
        _stprintf(lvInfo.Column2, _T("%d"), pCFInfo->CommentLineCount);
        add_sweet_listview_item(&lvInfo);

    // Code count
        lvInfo.ImageIndex = SWEET_LIST_COMMENT;
        LoadString(hInst, IDS_LIST_ITEM_CODE, lvInfo.Column1, MAX_LOADSTRING);
        _stprintf(lvInfo.Column2, _T("%d"), pCFInfo->CodeLineCount);
        add_sweet_listview_item(&lvInfo);

    // Empty count
        lvInfo.ImageIndex = SWEET_LIST_COMMENT;
        LoadString(hInst, IDS_LIST_ITEM_EMPTY, lvInfo.Column1, MAX_LOADSTRING);
        _stprintf(lvInfo.Column2, _T("%d"), pCFInfo->EmptyLineCount);
        add_sweet_listview_item(&lvInfo);

    // Char count
        lvInfo.ImageIndex = SWEET_LIST_PROPERTY;
        LoadString(hInst, IDS_LIST_ITEM_VISIBLE_CHAR, lvInfo.Column1, MAX_LOADSTRING);
        _stprintf(lvInfo.Column2, _T("%d"), pCFInfo->VisibleCharCount);
        add_sweet_listview_item(&lvInfo);
    
    // Code percentage
        lvInfo.ImageIndex = SWEET_LIST_PROPERTY;
        LoadString(hInst, IDS_LIST_ITEM_CODE_PERCENT, lvInfo.Column1, MAX_LOADSTRING);
        _stprintf(lvInfo.Column2, _T("%d %%"), PERCENT(pCFInfo->CodeLineCount, (pCFInfo->LineCount + 1))); // TO DO: can not divide zero
        add_sweet_listview_item(&lvInfo);

	// Active percentage
	lvInfo.ImageIndex = SWEET_LIST_PROPERTY;
	LoadString(hInst, IDS_LIST_ITEM_EFFECT_PERCENT, lvInfo.Column1, MAX_LOADSTRING);
	_stprintf(lvInfo.Column2, _T("%d %%"), PERCENT(pCFInfo->Parse2ActiveCharCount, (pCFInfo->Parse2ActiveCharCount + pCFInfo->Parse2InactiveCharCount))); // TO DO: can not divide zero
	add_sweet_listview_item(&lvInfo);

    // Function count
        lvInfo.ImageIndex = SWEET_LIST_PROPERTY;
        LoadString(hInst, IDS_LIST_ITEM_FUNCTION_COUNT, lvInfo.Column1, MAX_LOADSTRING);
        _stprintf(lvInfo.Column2, _T("%d"), pCFInfo->FunctionCount);
        add_sweet_listview_item(&lvInfo);

        /* function list */
        lvInfo.ImageIndex = SWEET_LIST_FUNCTION;
        for (int iFunc = 0; iFunc < pCFInfo->FunctionCount; iFunc ++)
        {
            lvInfo.ImageIndex = SWEET_LIST_FUNCTION;
            MultiByteToWideChar(CP_OEMCP, 0, pCFInfo->FunctionList[iFunc], -1, lvInfo.Column1, MAX_GENERAL_LENGTH);
            //_stprintf(lvInfo.Column1, _T("%s"), pCFInfo->FunctionList[iFunc]);
            _stprintf(lvInfo.Column2, _T(""));
            add_sweet_listview_item(&lvInfo);
        }
    }

    return 1;
}

int clean_up()  // once failed or user request to terminate the analysis process
{
    // Clear UI
    clear_sweet_treeview();
    clear_sweet_listview();

    // Clear MEM

    return 1;
}