// C_SWEET_UI.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "sweet_ui.h"
#include "sweet_ui_command.h"

#define WINDOW_MIN_WIDTH	720
#define WINDOW_MIN_HEIGHT	480

#define WINDOW_TREEVIEW_WEIGHT      0.3     // (in Percentage)  how much space the treeview occupies

#define LISTVIEW_COLUMN1_WIDTH      360     // (in PX)          the width that 1st column occupies
#define LISTVIEW_COLUMN2_WIDTH      380

#define LISTVIEW_SEP_WIDTH          5       // (in PX)          separator width

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle		[MAX_LOADSTRING];			// The title bar text
TCHAR szWindowClass	[MAX_LOADSTRING];			// the main window class name

HANDLE hAniIcon     [ANI_FRAME_TOTAL];          // 16x16 32bpp  animation

HWND hWnd;
HWND hWndRebar;
HWND hWndToolbar;
HWND hWndStatusbar;
HWND hWndTreeView;
HWND hWndListView;
HWND hWndSeparator;
HIMAGELIST hImagelistList;
//HIMAGELIST hImagelistAni;

// process dlg:
HWND hWndProcess;
HWND hWndAni;
HWND hWndProgressbar;
HWND hWndStatusTitle;

HANDLE hWndConOut;		// console output screen buffer

SWEET_LISTVIEW_INFO **szListViewItems   = NULL;     // listview subitem buffer
unsigned int listViewItemCount          = 0;        // count of items

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);

LRESULT CALLBACK	WndProc		(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK	dlgProcess	(HWND, UINT, WPARAM, LPARAM);

HWND create_sweet_rebar		(HWND , HWND );
HWND create_sweet_toolbar	(HWND );            // toolbar itself contains a imagelist
HWND create_sweet_statusbar	(HWND );
HWND create_sweet_treeview  (HWND );
HWND create_sweet_listview  (HWND );
HWND create_sweet_separator (HWND );
HIMAGELIST create_sweet_imagelist ();           // the image list for treeview & listview

BOOL init_sweet_controls        (HWND );        // init main
BOOL init_sweet_treeview        (HWND );
BOOL init_sweet_listview        (HWND );
BOOL init_sweet_statusbar       (HWND );
VOID update_sweet_ani           ();

#ifdef _DEBUG
void test();

void sweet_console_print(const TCHAR * szFormat, ...)
{
	TCHAR szBuffer[MAX_GENERAL_LENGTH];

	va_list pArgs;
	va_start(pArgs, szFormat);
	_vstprintf(szBuffer, szFormat, pArgs);
	va_end(pArgs);

    _tcscat(szBuffer, _T("\r\n"));

	WriteConsole(hWndConOut, szBuffer, _tcslen(szBuffer), NULL, NULL);
	return ;
}

void create_console()
{
	if (AllocConsole())
	{	// success
		hWndConOut = GetStdHandle(STD_OUTPUT_HANDLE);
	}
}
#endif

LONG sweet_crash(LPEXCEPTION_POINTERS p)
{
    MessageBox(NULL, _T("不好！！！检测到爆了！"), NULL, 0);
    return EXCEPTION_EXECUTE_HANDLER;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

    // Initialize crash handler:
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&sweet_crash);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
#ifdef ENABLE_MULTI_THREAD
    _tcscat(szTitle, _T(" - 多线程版本 （新版）"));
#endif

	LoadString(hInstance, IDC_C_SWEET_UI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_C_SWEET_UI));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_C_SWEET_UI));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= NULL;//MAKEINTRESOURCE(IDC_C_SWEET_UI);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindowEx(WS_EX_APPWINDOW, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	// Initialize common controls:
	INITCOMMONCONTROLSEX icex;
	icex.dwSize	= sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC	= ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_PROGRESS_CLASS | ICC_TREEVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
    case WM_SHOWWINDOW:
        break;
	case WM_CREATE:
		{
            init_sweet_controls(hWnd);
			break;
		}
	case WM_COMMAND:
		{
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			
			{	// Parse the menu selections:
				switch (wmId)
				{
				case IDM_OPEN_FILE:
                    sweet_play_file();
					break;
				case IDM_OPEN_FOLDER:
                    sweet_play_folder();
					break;
                case IDM_EXPORT:
                    sweet_play_export();
                    break;
				case IDM_COPY:
					break;
                case IDM_BACK:
                case IDM_FORWARD:
                    SORRY(hWnd)
                    break;
				case IDM_ABOUT:
                    sweet_play_about();
					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
				break;
			}
		}
	case WM_GETMINMAXINFO:
		{
 			((LPMINMAXINFO)lParam)->ptMinTrackSize.x = WINDOW_MIN_WIDTH;
 			((LPMINMAXINFO)lParam)->ptMinTrackSize.y = WINDOW_MIN_HEIGHT;
			LPMINMAXINFO pMinMaxNfo = (LPMINMAXINFO)lParam;
			pMinMaxNfo->ptMinTrackSize.x = WINDOW_MIN_WIDTH;
			pMinMaxNfo->ptMinTrackSize.y = WINDOW_MIN_HEIGHT;
			break;
		}
	case WM_NOTIFY:
		{ 
			TCHAR outer[MAX_PATH];
			_stprintf(outer, _T("%d"), wParam);
			OutputDebugString(outer);

			switch (((LPNMHDR)lParam)->code)
			{
            // Treeview:
            case TVN_SELCHANGED:
                {
                    HTREEITEM htI = TreeView_GetSelection(hWndTreeView);
                    TVITEM tvI = { 0 };

                    tvI.hItem   = htI;
                    tvI.mask    = TVIF_PARAM;
                    TreeView_GetItem(hWndTreeView, &tvI);

                    draw_list((SWEET_C_FILE_INFO *)tvI.lParam);
                }
                break;
            case LVN_GETDISPINFO:
                {
                    NMLVDISPINFO *plvdi = (NMLVDISPINFO*)lParam;
                    switch (plvdi->item.iSubItem)
                    {
                    case 0:
                        plvdi->item.pszText = szListViewItems[plvdi->item.iItem]->Column1;
                        break;
                    case 1:
                        plvdi->item.pszText = szListViewItems[plvdi->item.iItem]->Column2;
                        break;
                    default:
                        break;
                    }
                }
                break;
            // Tooltip:
			case TTN_GETDISPINFO:
				{ 
					LPTOOLTIPTEXT lpTooltip = (LPTOOLTIPTEXT)lParam; 

					// Set the instance of the module that contains the resource:
					lpTooltip->hinst = hInst; 

					UINT_PTR idButton = lpTooltip->hdr.idFrom; 
					switch (idButton)
					{
					case IDM_OPEN_FILE:
						lpTooltip->lpszText = MAKEINTRESOURCE(IDS_TIP_OPEN_FILE);
						break;
					case IDM_OPEN_FOLDER:
						lpTooltip->lpszText = MAKEINTRESOURCE(IDS_TIP_OPEN_FOLDER);
                        break;
                    case IDM_BACK:
                        lpTooltip->lpszText = MAKEINTRESOURCE(IDS_TIP_NAV_BACK);
                        break;
                    case IDM_FORWARD:
                        lpTooltip->lpszText = MAKEINTRESOURCE(IDS_TIP_NAV_FORWARD);
                        break;
					case IDM_EXPORT:
						lpTooltip->lpszText = MAKEINTRESOURCE(IDS_TIP_EXPORT);
						break;
					case IDM_ABOUT:
						lpTooltip->lpszText = MAKEINTRESOURCE(IDS_TIP_ABOUT);
						break;
					}
					break;
				}
				break;
			}
		}
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_SIZE:
		{
#define PARTS_1_WIDTH		0xFF

			SendMessage(hWndRebar		, WM_SIZE, 0, 0);
			SendMessage(hWndStatusbar	, WM_SIZE, 0, 0);

			// Calculate the size of TREEVIEW:

 			RECT rWindow, rRebar, rStatus, rTreeview, rListview, rSeparator;
 			GetClientRect(hWnd			, &rWindow);	// Retrieve client area*/
			GetClientRect(hWndRebar		, &rRebar);
			GetClientRect(hWndStatusbar	, &rStatus);

            // Treeview:
			rTreeview.left		= 0;
			rTreeview.top		= rRebar.bottom;
			rTreeview.right		= rWindow.right * WINDOW_TREEVIEW_WEIGHT;
			rTreeview.bottom	= rWindow.bottom - rStatus.bottom;
            MoveWindow(hWndTreeView, 
                (int)rTreeview.left,
                (int)rTreeview.top,
                (int)(rTreeview.right - rTreeview.left),
                (int)(rTreeview.bottom - rTreeview.top),
                FALSE);

            // Separator:
            rSeparator.left     = rTreeview.right;
            rSeparator.top      = rRebar.bottom;
            rSeparator.right    = rTreeview.right + LISTVIEW_SEP_WIDTH;
            rSeparator.bottom   = rWindow.bottom - rStatus.bottom;
            MoveWindow(hWndSeparator, 
                (int)rSeparator.left,
                (int)rSeparator.top,
                (int)(rSeparator.right - rSeparator.left),
                (int)(rSeparator.bottom - rSeparator.top),
                FALSE);

            // Listview:
            rListview.left		= rSeparator.right;
            rListview.top		= rRebar.bottom;
            rListview.right		= rWindow.right;
            rListview.bottom	= rWindow.bottom - rStatus.bottom;
            MoveWindow(hWndListView, 
                (int)rListview.left,
                (int)rListview.top,
                (int)(rListview.right - rListview.left),
                (int)(rListview.bottom - rListview.top),
                FALSE);

            break;
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for process box:
INT_PTR CALLBACK dlgProcess(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
        *((HWND *)lParam) = hDlg;       // Save this handle for later use (WM_CLOSE)
        hWndProcess = hDlg;
        init_sweet_processdlg(hDlg);
		return (INT_PTR)TRUE;
    case WM_CLOSE:
        // TO DO: clean up
        EndDialog(hDlg, LOWORD(wParam));
        break;
// 	case WM_COMMAND:
// 		if (LOWORD(wParam) == IDC_ABORT || LOWORD(wParam) == IDCANCEL)
// 		{
// 			EndDialog(hDlg, LOWORD(wParam));
// 			return (INT_PTR)TRUE;
// 		}
// 		break;
// 	case WM_CTLCOLORBTN:
// 	case WM_CTLCOLORDLG:
// 	case WM_CTLCOLORSTATIC:
// 		// paint white:
//         return (INT_PTR)GetStockObject(WHITE_BRUSH);
// 		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
        case NM_CLICK:
		case NM_RETURN:
			ShellExecute(NULL, _T("open"), _T("http://win2000.howeb.cn"), NULL, NULL, SW_SHOW);
		}
		break;
    case WM_TIMER:
        switch (wParam)
        {
        case TIMER_ANI:
            // process the 10-second timer:
            update_sweet_ani();
            //return 0; 
        }
        break;
	}
	return (INT_PTR)FALSE;
}

HWND create_sweet_rebar(HWND hWndParent, HWND hwndToolbar)
{
#define NUMBUTTONS 9

	// Check for Toolbar:
	if ((hwndToolbar == NULL))
	{
		return NULL;
	}

	// Create the rebar:
	HWND hwndRebar = CreateWindowEx(0, REBARCLASSNAME, NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		RBS_BANDBORDERS | RBS_VARHEIGHT | CCS_NODIVIDER | CCS_TOP,
		0, 0, 0, 0,	hWndParent,	NULL, hInst, NULL);

	if(!hwndRebar)
	{
		return NULL;
	}

	// Initialize band info used by both bands:
	REBARBANDINFO rbBand = { sizeof(REBARBANDINFO) };
	rbBand.cbSize	= sizeof(REBARBANDINFO);
	rbBand.fMask	= RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE;
	rbBand.fStyle	= RBBS_GRIPPERALWAYS | RBBS_USECHEVRON | RBBS_CHILDEDGE;  

	// Get the height of the toolbar:
	DWORD dwBtnSize = (DWORD)SendMessage(hwndToolbar, TB_GETBUTTONSIZE, 0, 0);

	// Set values unique to the band with the toolbar:
	rbBand.lpText		= TEXT("TOOLBRA");
	rbBand.hwndChild	= hwndToolbar;
	rbBand.cx			= 0;
	rbBand.cyChild		= HIWORD(dwBtnSize);
	rbBand.cxMinChild	= NUMBUTTONS * LOWORD(dwBtnSize);
	rbBand.cyMinChild	= HIWORD(dwBtnSize);

	rbBand.cxIdeal		= NUMBUTTONS * LOWORD(dwBtnSize);

	// Add the band that has the toolbar:
	if( !SendMessage(hwndRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand))
	{
		return NULL;
	}

	return hwndRebar;
}

HWND create_sweet_toolbar(HWND hWndParent)
{
// TOOLBAR:
//	NEW | OPEN | SAVE | SAVEAS | UNDO | RUN | BREAK | STOP | ABOUT

#define TOOLBAR_BUTTONS     8
#define TOOLBAR_IMAGES      5

	// Create the toolbar:
    HWND hToolbar = CreateWindowEx(0 , TOOLBARCLASSNAME, NULL, 
        WS_CHILD /*| TBSTYLE_LIST*/ | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | TBSTYLE_TRANSPARENT | WS_CLIPCHILDREN |
		WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NORESIZE | CCS_TOP | WS_VISIBLE,
		0, 0, 0, 0, hWndParent,
		NULL, hInst, NULL);	// (HMENU) ID_TOOLBAR
	if (hToolbar == NULL)
	{
		return NULL;
	}

    // Set the toolbar extend style:
    //SendMessage(hToolbar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_MIXEDBUTTONS);

	// Create the imagelist:
	HIMAGELIST hToolImageList = ImageList_Create(48, 48, ILC_COLOR32, TOOLBAR_BUTTONS, 0);

	// Set the image list.
	SendMessage(hToolbar, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)hToolImageList);	// (WPARAM)0 = Only one imagelist

	// Load the text from a resource:
	// In the string table, the text for all buttons is a single entry that 
	// appears as "~New~Open~Save~~". The separator character is arbitrary, 
	// but it must appear as the first character of the string. The message 
	// returns the index of the first item, and the items are numbered 
	// consecutively.
	int iToolbra = SendMessage(hToolbar, TB_ADDSTRING, (WPARAM)hInst, (LPARAM)IDS_TOOLBRA); 

	TBBUTTON tbButtons[TOOLBAR_BUTTONS] = {
        { 0				, IDM_OPEN_FOLDER   , TBSTATE_ENABLED		, BTNS_BUTTON | BTNS_SHOWTEXT   ,
            {0}, 0, iToolbra        },
		{ I_IMAGENONE	, 0				    , TBSTATE_ENABLED		, BTNS_SEP                      ,
            {0}, 0, (INT_PTR)_T("") },
        { 1				, IDM_BACK          , TBSTATE_ENABLED	    , BTNS_BUTTON | BTNS_SHOWTEXT   ,
            {0}, 0, iToolbra + 1    },
        { 2				, IDM_FORWARD       , TBSTATE_ENABLED       , BTNS_BUTTON/*| BTNS_SHOWTEXT*/,
            {0}, 0, iToolbra + 2    },
		{ I_IMAGENONE	, 0				    , TBSTATE_ENABLED		, BTNS_SEP                      ,
            {0}, 0, (INT_PTR)_T("") },
		{ 3				, IDM_EXPORT        , TBSTATE_ENABLED		, BTNS_BUTTON | BTNS_SHOWTEXT   ,
            {0}, 0, iToolbra + 3    },
		{ I_IMAGENONE	, 0				    , TBSTATE_ENABLED		, BTNS_SEP                      ,
            {0}, 0, (INT_PTR)_T("") },
        { 4				, IDM_ABOUT		    , TBSTATE_ENABLED		, BTNS_BUTTON | BTNS_SHOWTEXT   ,
            {0}, 0, iToolbra + 4    }
	};

	// Send the TB_BUTTONSTRUCTSIZE message, which is required for backward compatibility:
	SendMessage(hToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 

	// Load the icon resources, and add the icons into image list
#define IDI_TOOLBAR_FIRST       IDI_TOOL_OPEN_FOLDER

    HANDLE hIcoTmp;
    for (int iIco = 0; iIco < TOOLBAR_IMAGES; iIco ++)
    { // Since the shared icon is automatically destroyed, there's no need for us to destroy it.
        hIcoTmp = LoadImage(hInst, MAKEINTRESOURCE(iIco + IDI_TOOLBAR_FIRST), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
        ImageList_AddIcon(hToolImageList, (HICON)hIcoTmp);
    }

	// Add buttons
	SendMessage(hToolbar, TB_ADDBUTTONS, (WPARAM)TOOLBAR_BUTTONS, (LPARAM) (LPTBBUTTON) &tbButtons);

	// Tell the toolbar to resize itself
	SendMessage(hToolbar, TB_AUTOSIZE, 0, 0);
	return hToolbar; 
}


HWND create_sweet_statusbar(HWND hWndParent)
{
	HWND hStatusbar = CreateWindowEx(0, STATUSCLASSNAME, (LPCTSTR) NULL,
		SBARS_SIZEGRIP | WS_CHILD |	WS_VISIBLE, 0, 0, 0, 0, 
		hWndParent, NULL, hInst, NULL);
	
	return hStatusbar; 
}

HWND create_sweet_treeview(HWND hWndParent)
{
    HWND hTreeview;

    // Create a tree-view control in icon view.
    hTreeview = CreateWindowEx(WS_EX_CLIENTEDGE,
        WC_TREEVIEW, _T(""), WS_VISIBLE | WS_CHILD | TVS_DISABLEDRAGDROP | TVS_FULLROWSELECT |
        TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_TRACKSELECT, 0, 0, 0, 0,
        hWndParent, NULL , hInst, NULL);

    return hTreeview;
}

HWND create_sweet_listview(HWND hWndParent)
{
    HWND hListview;

    hListview = CreateWindowEx(0,
        WC_LISTVIEW, _T(""), WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER,
        0, 0, 0, 0,
        hWndParent, NULL, hInst, NULL);

    return hListview;
}

HWND create_sweet_separator(HWND hWndParent)
{
    HWND hSeparator;

    hSeparator = CreateWindowEx(NULL, 
        _T("STATIC"), _T(""), WS_VISIBLE | WS_CHILD | SS_ETCHEDHORZ,
        0, 0, 0, 0,
        hWndParent, NULL, hInst, NULL);

    return hSeparator;
}

/* invisible controls */
HIMAGELIST create_sweet_imagelist(WORD wInteger, unsigned int itemCount)
{
    // Create the imagelist:
    HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR32, TOOLBAR_BUTTONS, 0);

    HANDLE hIcoTmp;
    for (int iIco = 0; iIco < itemCount; iIco ++)
    { // Since the shared icon is automatically destroyed, there's no need for us to destroy it.
        hIcoTmp = LoadImage(hInst, MAKEINTRESOURCE(iIco + wInteger), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
        ImageList_AddIcon(hImageList, (HICON)hIcoTmp);
    }
    return hImageList;
}

BOOL init_sweet_controls(HWND hWnd)
{
/*** 1st: create the controls ***/
    // create TOOLBAR:
    hWndToolbar = create_sweet_toolbar(hWnd);

    // create REBAR:
    if (hWndToolbar)
    {
        hWndRebar = create_sweet_rebar(hWnd, hWndToolbar);
    }

    // create STATUSBAR:
    hWndStatusbar = create_sweet_statusbar(hWnd);

    // create TREEVIEW:
    hWndTreeView = create_sweet_treeview(hWnd);

    // create LISTVIEW:
    hWndListView = create_sweet_listview(hWnd);
        
    // create SEPARATOR:
    hWndSeparator = create_sweet_separator(hWnd);

    // create IMAGELIST for treeview & listview:
    #define LISTVIEW_IMAGES             19
    hImagelistList  = create_sweet_imagelist(IDI_LIST_FOLDER_CLOSED, LISTVIEW_IMAGES);

#ifdef _DEBUG
    create_console();
#endif

/*** 2nd: initialize the controls ***/
    // init the treeview:
    if (init_sweet_treeview(hWndTreeView) == FALSE)
        return FALSE;

    // init the listview:
    if (init_sweet_listview(hWndListView) == FALSE)
        return FALSE;

    // init the statusbar:
    if (init_sweet_statusbar(hWndStatusbar) == FALSE)
        return FALSE;

    refresh_tools();

    return TRUE;
}

BOOL init_sweet_treeview(HWND hWndTreeview)
{

    // Select the imagelist:
    TreeView_SetImageList(hWndTreeview, hImagelistList, TVSIL_NORMAL);

    return TRUE;
}

BOOL init_sweet_listview(HWND hWndListview)
{
    // set styles:
    ListView_SetExtendedListViewStyle(hWndListview, 
        LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);

    // add columns:
#define SWEET_COLUMNS   2

    TCHAR szStrColumn[MAX_LOADSTRING];
    LVCOLUMN lvc; 

    // Initialize the LVCOLUMN structure.
    // The mask specifies that the format, width, text, and subitem members
    // of the structure are valid. 
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

    // Add the columns:
    for (int iCol = 0; iCol < SWEET_COLUMNS; iCol++)
    { 
        lvc.iSubItem = iCol;
        lvc.pszText = szStrColumn;
        if (iCol == 0)
        {
            lvc.cx = LISTVIEW_COLUMN1_WIDTH;
        }
        else
        {
            lvc.cx = LISTVIEW_COLUMN2_WIDTH;
        }

        LoadString(hInst, IDS_LIST_COL_A + iCol, szStrColumn, MAX_LOADSTRING);

        if (ListView_InsertColumn(hWndListview, iCol, &lvc) == -1)
        {
            return FALSE;
        }
    }

    ListView_SetImageList(hWndListview, hImagelistList, LVSIL_SMALL);

    return TRUE;
}

BOOL init_sweet_statusbar(HWND hWndStatusbar)
{
    TCHAR szStat[MAX_LOADSTRING];
    TCHAR szStrTmp[MAX_LOADSTRING];

    LoadString(hInst, IDS_APP_TITLE, szStrTmp, MAX_LOADSTRING);
    _tcscpy(szStat, szStrTmp);
    LoadString(hInst, IDS_ABOUT_VERSION, szStrTmp, MAX_LOADSTRING);
    _tcscat(szStat, _T(" - "));
    _tcscat(szStat, szStrTmp);
    _tcscat(szStat, _T(" "));
    _tcscat(szStat, _T(CS_VERSION));

    SendMessage(hWndStatusbar, SB_SETTEXT, 0, (LPARAM)szStat);

    return TRUE;
}

// for progress dialog
BOOL init_sweet_processdlg(HWND hDlg)
{
    // save the handles 
    hWndAni             = GetDlgItem(hDlg, IDC_ANI              );
    hWndProgressbar     = GetDlgItem(hDlg, IDC_PROGRESS_BRA     );
    hWndStatusTitle     = GetDlgItem(hDlg, IDC_STATUS_TITLE     );

    // TO DO: destroy them
    // load ani icon into memory:
    for (int iIco = 0; iIco < ANI_FRAME_TOTAL; iIco++)
    {
        hAniIcon[iIco] = LoadImage(hInst, MAKEINTRESOURCE(IDI_ANI1 + iIco), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
    }

    TCHAR szTmp[MAX_LOADSTRING];

    LoadString(hInst, IDS_PROC_PREPARE, szTmp, MAX_LOADSTRING);
    update_sweet_status_description(szTmp);

    // create animation timer
    SetTimer(hDlg, TIMER_ANI, ANI_INTERVAL, (TIMERPROC)NULL);

    return TRUE;
}

BOOL init_sweet_progressbar(unsigned int maxRange)
{
    SendMessage(hWndProgressbar, PBM_SETRANGE, 0, MAKELPARAM(0, maxRange)); 
    SendMessage(hWndProgressbar, PBM_SETSTEP, (WPARAM) 1, 0); 
    return TRUE;
}

BOOL add_sweet_treeview_item(SWEET_C_FILE_INFO *pCFInfo)
{
    static HTREEITEM hPrev[MAX_TREE_DEPTH] = { TVI_ROOT };

    TVINSERTSTRUCT tvIns = { 0 };

    // Set the item text:
    tvIns.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

    tvIns.item.iImage           = pCFInfo->FileType;
    if (pCFInfo->FileType == SWEET_FILE_FOLDER)
    {
        tvIns.item.iSelectedImage = SWEET_FILE_FOLDER_OPEN;
    }
    else
    {
        tvIns.item.iSelectedImage = pCFInfo->FileType;
    }
    
    tvIns.item.pszText          = pCFInfo->FileName;
    tvIns.item.cchTextMax       = sizeof(tvIns.item.pszText) / sizeof(tvIns.item.pszText[0]);  // TO DO

    if (pCFInfo->Depth) // not the root node
    {
        tvIns.hParent = hPrev[pCFInfo->Depth - 1];
        tvIns.hInsertAfter = TVI_LAST;
    }
    else
    {
        tvIns.hInsertAfter = TVI_ROOT;
    }

    tvIns.item.lParam = (LPARAM)pCFInfo;

    pCFInfo->HTI = TreeView_InsertItem(hWndTreeView, &tvIns);

    hPrev[pCFInfo->Depth] = pCFInfo->HTI;

    //int e = GetLastError();

    return TRUE;
}

BOOL add_sweet_listview_item(SWEET_LISTVIEW_INFO *pLVInfo)
{
    if (listViewItemCount % LV_ITEM_INCREMENT == 0)
    {
        szListViewItems = (SWEET_LISTVIEW_INFO **)(realloc(szListViewItems, sizeof(SWEET_LISTVIEW_INFO *) * (listViewItemCount + LV_ITEM_INCREMENT)));
    }

    *(szListViewItems + listViewItemCount) = (SWEET_LISTVIEW_INFO *)malloc(sizeof(SWEET_LISTVIEW_INFO));

    //CopyMemory(*(szListViewItems + listViewItemCount), pLVInfo, sizeof(SWEET_LISTVIEW_INFO)); // copy sub items for later use
    _tcscpy((*(szListViewItems + listViewItemCount))->Column1, pLVInfo->Column1);
    _tcscpy((*(szListViewItems + listViewItemCount))->Column2, pLVInfo->Column2);
    (*(szListViewItems + listViewItemCount))->ImageIndex = pLVInfo->ImageIndex;

    LVITEM lvI = { 0 };

    lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_NORECOMPUTE;
    lvI.iItem    = listViewItemCount;
    lvI.iImage   = pLVInfo->ImageIndex;
    lvI.iSubItem = 0;
    //lvI.lParam = (LPARAM) &rgPetInfo[index];
    lvI.pszText = LPSTR_TEXTCALLBACK;       // sends an LVN_GETDISP message.

    if (ListView_InsertItem(hWndListView, &lvI) == -1)
        return FALSE;

    listViewItemCount ++;
    return TRUE;
}

VOID clear_sweet_treeview()
{
    // TO DO: clean up the subitem buffer first

    TreeView_DeleteAllItems(hWndTreeView);
}

VOID clear_sweet_listview()
{
    if (szListViewItems)
    {
        for (int iCount = 0; iCount < listViewItemCount; iCount ++)
        {
            free(*(szListViewItems + iCount));  // free children
        }
        free(szListViewItems);                  // free father
        szListViewItems = NULL; /* !!!IMPORTANT!!! */

        listViewItemCount = 0;
    }

    ListView_DeleteAllItems(hWndListView);
}

// update progress dialog
VOID update_sweet_ani()
{
/*
    the opacity factor of the animation cursor:

        100 16

    88          28

    76          40

        64  52

*/
    static unsigned int aniFrame = 0;
    (++aniFrame) %= ANI_FRAME_TOTAL;
    SendMessage(hWndAni, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hAniIcon[aniFrame]);
}

VOID update_sweet_status_description(TCHAR *szTitle)
{
    SetWindowText(hWndStatusTitle, szTitle);
}

VOID update_sweet_progressbar()
{
    SendMessage(hWndProgressbar, PBM_STEPIT, 0, 0);
}

VOID expand_sweet_tree(SWEET_C_FILE_INFO *pCFInfo)
{
    TreeView_Expand(hWndTreeView, pCFInfo->HTI, TVE_EXPAND);
    TreeView_SelectItem(hWndTreeView, pCFInfo->HTI);
}

void test()
{
}