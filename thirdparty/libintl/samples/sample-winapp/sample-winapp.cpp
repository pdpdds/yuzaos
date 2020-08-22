//
//

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <process.h>
#include <shlwapi.h>
#ifdef _MSC_VER
#pragma comment(lib, "shlwapi.lib")
#endif
#include <commdlg.h>
#include <commctrl.h>
#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#endif

#include "sample-winapp.h"
#include "../gettext.h"
#include "../get_lcid.h"

#define PACKAGE_NAME _T("hello-c")

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#define MAX_LOADSTRING 100

// global variables
HINSTANCE hInst;								// My Instance
TCHAR szTitle[MAX_LOADSTRING];					// Text of title bar
TCHAR szWindowClass[MAX_LOADSTRING];			// main window class name

HWND hCtrls[16];
int iCtrlsCnt = 0;
int iCtrlFontName = 9;
int iCtrlFontSize = 9;
int iCtrlLang = 9;
int iCtrlLangName = 9;
int iCtrlBtnFont = 9;
int iCtrlBtnChange = 9;
int margin = 8;
int padding = 4;

LOGFONT lf;
HFONT hFont = NULL;
_TCHAR font_name[_MAX_PATH];
double font_size = 0.0;
char   lang_name[_MAX_PATH];
bool   lang_ok = false;
_TCHAR locale_dir[_MAX_PATH];

// prototype definition
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

static bool set_locale(const char *locale);
static bool initialize_locale();
static int update_window(HWND hWnd);

// main
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	InitCommonControls();

	// initialize global string
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SAMPLEWINAPP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// setting locale
	lang_ok = initialize_locale();

	// create window
	if (!InitInstance (hInstance, nCmdShow))
	{
		return 0;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SAMPLEWINAPP));

	// message loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// free all resources in libintl library
	libintl_tfreeres();

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return (int) msg.wParam;
}


//  Regist window class for my app
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SAMPLEWINAPP));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SAMPLEWINAPP);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SAMPLEWINAPP));

	return RegisterClassEx(&wcex);
}

//  Create window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 640, 200, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


/// set logical font
static void set_font(HWND hWnd, const _TCHAR *font_name, double point, LPLOGFONT plf)
{
	HDC hdc = GetDC(hWnd);

	int logdpiy = GetDeviceCaps(hdc, LOGPIXELSY);
	LONG nHeight = (LONG)(point * (double)logdpiy / -72.0);

	memset(plf, 0, sizeof(LOGFONT));
    plf->lfHeight = nHeight;
    plf->lfWidth = 0;
    plf->lfEscapement = 0;
    plf->lfOrientation = plf->lfEscapement;
    plf->lfWeight = FW_DONTCARE;
    plf->lfItalic = 0;
    plf-> lfUnderline = 0;
    plf->lfStrikeOut = 0;
    plf->lfCharSet = DEFAULT_CHARSET;
    plf->lfOutPrecision = OUT_DEFAULT_PRECIS;
    plf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    plf->lfQuality = DEFAULT_QUALITY;
    plf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	// font name
	_tcscpy(plf->lfFaceName, font_name);

	ReleaseDC(hWnd, hdc);
}

/// get logical font on non client area
static void get_logfont_from_screen(LPLOGFONT plf)
{
	// get OS version
	OSVERSIONINFO vi;
	memset(&vi, 0, sizeof(vi));
	vi.dwOSVersionInfoSize = sizeof(vi);
	GetVersionEx(&vi);

	// get logfont on non client area
	NONCLIENTMETRICS ncm;
	// diffrent structure size
#if (WINVER >= 0x0600)
	ncm.cbSize = (vi.dwMajorVersion >= 6 ? sizeof(ncm) : sizeof(ncm) - 4);
#else
	ncm.cbSize = sizeof(ncm);
#endif
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	*plf = ncm.lfMessageFont;
}

// calcrate point size from height of font
static double height_to_point(HWND hWnd, LONG h)
{
	HDC hdc = GetDC(hWnd);
	int logdpiy = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(hWnd, hdc);
	if (logdpiy == 0) logdpiy = 96;
	return (double)h * -72.0 / logdpiy;
}

/// font dialog
static void choose_font(HWND hWnd)
{
	CHOOSEFONT cf;
	memset(&cf, 0, sizeof(cf));
	cf.lStructSize = sizeof(cf);
	cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_LIMITSIZE | CF_NOVERTFONTS;
	cf.hwndOwner = hWnd;
	cf.hDC = NULL;
	cf.lpLogFont = &lf;
	cf.nSizeMin = 6;
	cf.nSizeMax = 36;

	// display font dialog
	BOOL rc = ChooseFont(&cf);
	if (rc == TRUE) {
		_tcscpy(font_name, lf.lfFaceName);		
		font_size = height_to_point(hWnd, lf.lfHeight);

		SetWindowText(hCtrls[iCtrlFontName], font_name);
		_TCHAR msg[_MAX_PATH];
		_stprintf(msg, _T("%.0lf"), font_size);
		SetWindowText(hCtrls[iCtrlFontSize], msg);

		update_window(hWnd);
	}
}

// translate messages on sub menu
// this function is recursive.
static void translate_submenu(HMENU hMenu)
{
	_TCHAR buf[_MAX_PATH];
	MENUITEMINFO mii;

	int nums = GetMenuItemCount(hMenu);
	for(int i=0; i<nums; i++) {
		memset(&mii, 0, sizeof(mii));
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_SUBMENU | MIIM_STRING;
		mii.dwTypeData = buf;
		mii.cch = _MAX_PATH - 1;
		GetMenuItemInfo(hMenu, i, TRUE, &mii);
		if (mii.cch > 0) {
			mii.fMask = MIIM_STRING;
			const _TCHAR *p = _tgettext(mii.dwTypeData);
			if (p != buf) {
				memset(buf, 0, sizeof(buf));
				_tcsncpy(buf, p, _MAX_PATH-1);
			}
			mii.dwTypeData = buf;
			mii.cch = (UINT)_tcslen(buf);
			SetMenuItemInfo(hMenu, i, TRUE, &mii);
		}
		if (mii.hSubMenu != NULL) {
			translate_submenu(mii.hSubMenu);
		}
	}
}

// translate all messages on menu
static void translate_menu(HWND hWnd, HMENU hMenu)
{
	translate_submenu(hMenu);
}

// get local sub-dir name
static bool add_locale_name(const _TCHAR *top_dir)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	_TCHAR dir[_MAX_PATH];

	_tcscpy(dir, top_dir);
	if (PathAppend(dir, _T("\\*")) == FALSE) {
		return false;
	}

	hFind = FindFirstFile(dir, &FindFileData);
	while (hFind != INVALID_HANDLE_VALUE) {
		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && FindFileData.cFileName[0] != _T('.')) {
			ComboBox_AddString(hCtrls[iCtrlLang], FindFileData.cFileName);
		}

		if (!FindNextFile(hFind, &FindFileData)) {
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
			break;
		}
	}
	ComboBox_SetCurSel(hCtrls[iCtrlLang], 0);
	return true;
}

// create window
static int create_window(HWND hWnd)
{
	// translate menu messages
	translate_menu(hWnd, GetMenu(hWnd));

	// create child controls
	_TCHAR msg[_MAX_PATH];


	// set logical font
	get_logfont_from_screen(&lf);
	HFONT hNewFont = CreateFontIndirect(&lf);
	if (hNewFont != NULL) hFont = hNewFont;

	_tcscpy(font_name, lf.lfFaceName);
	font_size = height_to_point(hWnd, lf.lfHeight);

	RECT re;

	DWORD dwStyle = 0;
	int w = abs(lf.lfHeight);
	int h = abs(lf.lfHeight);
	padding = h / 3;
	margin = padding * 2;

	memset(hCtrls, 0, sizeof(hCtrls));

	// edit box
	re.left = margin;
	re.right = w * 35 + re.left;
	re.top = margin;
	re.bottom = re.top + padding + h + padding;
	dwStyle = ES_AUTOHSCROLL | ES_LEFT | WS_CHILD | WS_VISIBLE | WS_BORDER;
	hCtrls[iCtrlsCnt] = CreateWindow(_T("EDIT"), _T("")
	, dwStyle
	,re.left ,re.top, re.right-re.left, re.bottom-re.top, hWnd, NULL, hInst, NULL);
	if (hCtrls[iCtrlsCnt]) {
		// set text
		_stprintf(msg, _("This program is running as process number %d."), _getpid());
		SetWindowText(hCtrls[iCtrlsCnt], msg);
	}
	iCtrlsCnt++;

	// font name
	re.left = margin;
	re.right = w * 5 + re.left;
	re.top = re.bottom + margin;
	re.bottom = re.top + padding + h + padding;
	dwStyle = SS_CENTER | WS_CHILD | WS_VISIBLE;
	hCtrls[iCtrlsCnt] = CreateWindow(_T("STATIC"), _T("font name")
	, dwStyle
	,re.left ,re.top, re.right-re.left, re.bottom-re.top, hWnd, NULL, hInst, NULL);
	iCtrlsCnt++;

	re.left = re.right + padding;
	re.right = w * 15 + re.left;
	dwStyle = ES_AUTOHSCROLL | ES_LEFT | WS_CHILD | WS_VISIBLE | WS_BORDER;
	hCtrls[iCtrlsCnt] = CreateWindow(_T("EDIT"), font_name
	, dwStyle
	,re.left ,re.top, re.right-re.left, re.bottom-re.top, hWnd, NULL, hInst, NULL);
	iCtrlFontName = iCtrlsCnt;
	iCtrlsCnt++;

	// font size
	re.left = re.right + padding;
	re.right = w * 5 + re.left;
	dwStyle = SS_CENTER | WS_CHILD | WS_VISIBLE;
	hCtrls[iCtrlsCnt] = CreateWindow(_T("STATIC"), _T("font size")
	, dwStyle
	,re.left ,re.top, re.right-re.left, re.bottom-re.top, hWnd, NULL, hInst, NULL);
	iCtrlsCnt++;

	re.left = re.right + padding;
	re.right = w * 5 + re.left;
	dwStyle = ES_AUTOHSCROLL | ES_LEFT | WS_CHILD | WS_VISIBLE | WS_BORDER;
	hCtrls[iCtrlsCnt] = CreateWindow(_T("EDIT"), _T("")
	, dwStyle
	,re.left ,re.top, re.right-re.left, re.bottom-re.top, hWnd, NULL, hInst, NULL);
	iCtrlFontSize = iCtrlsCnt;
	if (hCtrls[iCtrlsCnt]) {
		// set text
		_stprintf(msg, _("%.0lf"), font_size);
		SetWindowText(hCtrls[iCtrlsCnt], msg);
	}
	iCtrlsCnt++;

	// button
	re.left = re.right + padding;
	re.right = w * 5 + re.left;
	dwStyle = BS_DEFPUSHBUTTON | BS_TEXT | BS_CENTER | WS_CHILD | WS_VISIBLE;
	hCtrls[iCtrlsCnt] = CreateWindow(_T("BUTTON"), _T("font...")
	, dwStyle
	,re.left ,re.top, re.right-re.left, re.bottom-re.top, hWnd, NULL, hInst, NULL);
	iCtrlBtnFont = iCtrlsCnt;
	iCtrlsCnt++;

	// language
	re.left = margin;
	re.right = w * 4 + re.left;
	re.top = re.bottom + margin;
	re.bottom = re.top + padding + h + padding;
	dwStyle = SS_CENTER | WS_CHILD | WS_VISIBLE;
	hCtrls[iCtrlsCnt] = CreateWindow(_T("STATIC"), _T("lang")
	, dwStyle
	,re.left ,re.top, re.right-re.left, re.bottom-re.top, hWnd, NULL, hInst, NULL);
	iCtrlsCnt++;

	re.left = re.right + padding;
	re.right = w * 6 + re.left;
	dwStyle = CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER;
	hCtrls[iCtrlsCnt] = CreateWindow(_T("ComboBox"), _T("")
	, dwStyle
	,re.left ,re.top, re.right-re.left, 1000, hWnd, NULL, hInst, NULL);
	iCtrlLang = iCtrlsCnt;
	iCtrlsCnt++;

	re.left = re.right + padding;
	re.right = w * 20 + re.left;
	dwStyle = SS_LEFT | WS_CHILD | WS_VISIBLE;
	hCtrls[iCtrlsCnt] = CreateWindow(_T("STATIC"), _T("")
	, dwStyle
	,re.left ,re.top, re.right-re.left, (re.bottom-re.top) * 2, hWnd, NULL, hInst, NULL);
	iCtrlLangName = iCtrlsCnt;
	if (hCtrls[iCtrlsCnt]) {
		// set text
		SetWindowTextA(hCtrls[iCtrlsCnt], lang_ok ? lang_name : "(Not supported)");
	}
	iCtrlsCnt++;

	// button
	re.left = re.right + padding;
	re.right = w * 5 + re.left;
	dwStyle = BS_DEFPUSHBUTTON | BS_TEXT | BS_CENTER | WS_CHILD | WS_VISIBLE;
	hCtrls[iCtrlsCnt] = CreateWindow(_T("BUTTON"), _T("change")
	, dwStyle
	,re.left ,re.top, re.right-re.left, re.bottom-re.top, hWnd, NULL, hInst, NULL);
	iCtrlBtnChange = iCtrlsCnt;
	iCtrlsCnt++;

	hCtrls[iCtrlsCnt] = NULL;

	for(int i=0; i<iCtrlsCnt; i++) {
		// set font
		if (hFont != NULL) SendMessage(hCtrls[i], WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	}

	add_locale_name(locale_dir);

	return 0;
}

// change controls in window
static int update_window(HWND hWnd)
{
	_TCHAR msg[_MAX_PATH];

	// set languate
	char new_locale[_MAX_PATH];
	GetWindowTextA(hCtrls[iCtrlLang], new_locale, _MAX_PATH-1);
	lang_ok = set_locale(new_locale);

	//
	// Menu messages on window are already translated. So you need reattach original messages and translate them again.
	//
	// load menu from resource
	HMENU hNewMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDC_SAMPLEWINAPP));
	// destroy current menu on window
	DestroyMenu(GetMenu(hWnd));
	// reattach menu to window
	SetMenu(hWnd, hNewMenu);
	// translate menu messages
	translate_menu(hWnd, GetMenu(hWnd));

	// get font name and size from window
	GetWindowText(hCtrls[iCtrlFontName], font_name, _MAX_PATH-1);
	GetWindowText(hCtrls[iCtrlFontSize], msg, _MAX_PATH-1);
	_TCHAR *endp;
	font_size = _tcstod(msg, &endp);
	if (font_size < 6.0) font_size = 6.0;
	if (font_size > 36.0) font_size = 36.0;

	// set logical font
	set_font(hWnd, font_name, font_size, &lf);
	HFONT hNewFont = CreateFontIndirect(&lf);
	if (hNewFont != NULL) hFont = hNewFont;

	RECT re;

	int cnt = 0;
	int w = abs(lf.lfHeight);
	int h = abs(lf.lfHeight);
	padding = h / 3;
	margin = padding * 2;

	// edit box
	re.left = margin;
	re.right = w * 35 + re.left;
	re.top = margin;
	re.bottom = re.top + padding + h + padding;
	if (hCtrls[cnt]) {
		SetWindowPos(hCtrls[cnt], NULL, re.left, re.top, re.right - re.left, re.bottom - re.top, 0);

		// set text
		_stprintf(msg, _("This program is running as process number %d."), _getpid());
		SetWindowText(hCtrls[cnt], msg);
	}
	cnt++;

	// font name
	re.left = margin;
	re.right = w * 5 + re.left;
	re.top = re.bottom + margin;
	re.bottom = re.top + padding + h + padding;
	if (hCtrls[cnt]) {
		SetWindowPos(hCtrls[cnt], NULL, re.left, re.top, re.right - re.left, re.bottom - re.top, 0);
	}
	cnt++;

	re.left = re.right + padding;
	re.right = w * 15 + re.left;
	if (hCtrls[cnt]) {
		SetWindowPos(hCtrls[cnt], NULL, re.left, re.top, re.right - re.left, re.bottom - re.top, 0);

		// set text
		SetWindowText(hCtrls[iCtrlsCnt], font_name);
	}
	cnt++;

	// font size
	re.left = re.right + padding;
	re.right = w * 5 + re.left;
	if (hCtrls[cnt]) {
		SetWindowPos(hCtrls[cnt], NULL, re.left, re.top, re.right - re.left, re.bottom - re.top, 0);
	}
	cnt++;

	re.left = re.right + padding;
	re.right = w * 5 + re.left;
	if (hCtrls[cnt]) {
		SetWindowPos(hCtrls[cnt], NULL, re.left, re.top, re.right - re.left, re.bottom - re.top, 0);

		// set text
		_TCHAR msg[100];
		_stprintf(msg, _("%.1f"), font_size);

		SetWindowText(hCtrls[iCtrlsCnt], msg);
	}
	cnt++;

	// button
	re.left = re.right + padding;
	re.right = w * 5 + re.left;
	if (hCtrls[cnt]) {
		SetWindowPos(hCtrls[cnt], NULL, re.left, re.top, re.right - re.left, re.bottom - re.top, 0);
	}
	cnt++;

	// lang
	re.left = margin;
	re.right = w * 4 + re.left;
	re.top = re.bottom + margin;
	re.bottom = re.top + padding + h + padding;
	if (hCtrls[cnt]) {
		SetWindowPos(hCtrls[cnt], NULL, re.left, re.top, re.right - re.left, re.bottom - re.top, 0);
	}
	cnt++;

	re.left = re.right + padding;
	re.right = w * 6 + re.left;
	if (hCtrls[cnt]) {
		SetWindowPos(hCtrls[cnt], NULL, re.left, re.top, re.right - re.left, 1000, 0);
	}
	cnt++;

	re.left = re.right + padding;
	re.right = w * 20 + re.left;
	if (hCtrls[cnt]) {
		SetWindowPos(hCtrls[cnt], NULL, re.left, re.top, re.right - re.left, (re.bottom-re.top) * 2, 0);

		// set text
		SetWindowTextA(hCtrls[cnt], lang_ok ? lang_name : "(Not supported)");
	}
	cnt++;

	// button
	re.left = re.right + padding;
	re.right = w * 5 + re.left;
	if (hCtrls[cnt]) {
		SetWindowPos(hCtrls[cnt], NULL, re.left, re.top, re.right - re.left, re.bottom - re.top, 0);
	}
	cnt++;

	for(int i=0; i<iCtrlsCnt; i++) {
		// set font
		if (hFont != NULL) SendMessage(hCtrls[i], WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	}

	return 0;
}

//  Process messages
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// selected menu item
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			if ((HWND)lParam == hCtrls[iCtrlBtnChange]) {
				// change button
				update_window(hWnd);
			} else if ((HWND)lParam == hCtrls[iCtrlBtnFont]) {
				// choose font
				choose_font(hWnd);
			} else {
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
	case WM_CREATE:
		create_window(hWnd);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// translate messages on about box
static BOOL CALLBACK enum_about_box_proc(HWND hWnd, LPARAM lParam)
{
	_TCHAR class_name[_MAX_PATH];
	GetClassName(hWnd, class_name, _MAX_PATH-1);
	if (_tcscmp(class_name, _T("Static")) == 0) {
		// translate text on static control
		_TCHAR str[_MAX_PATH];
		memset(str, 0, sizeof(str));
		GetWindowText(hWnd, str, _MAX_PATH-1);
		SetWindowText(hWnd, _tgettext(str));
	}
	return TRUE;
}

// create about box
static INT_PTR create_about_box(HWND hDlg)
{
	_TCHAR str[_MAX_PATH];
	memset(str, 0, sizeof(str));

	// translate title bar
	GetWindowText(hDlg, str, _MAX_PATH-1);
	SetWindowText(hDlg, _tgettext(str));

	// translate messages on child controls
	EnumChildWindows(hDlg, enum_about_box_proc, NULL);

	return (INT_PTR)TRUE;
}

// about dialog box
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return create_about_box(hDlg);

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// set locale
static bool set_locale(const char *locale)
{
	memset(lang_name, 0, sizeof(lang_name));

	const char *new_locale = setlocale(LC_ALL, locale);
	if (new_locale == NULL) {
		return false;
	}
	LCID lcid = get_lcid(new_locale);
	if (SetThreadLocale(lcid) != TRUE) {
		return false;
	}

	strncpy(lang_name, new_locale, _MAX_PATH-1);

	return true;
}

// initialize locale
static bool initialize_locale()
{
	if (!set_locale("")) {
		return false;
	}

	DWORD result = GetModuleFileName(GetModuleHandle(NULL), 
									locale_dir, sizeof(locale_dir)/sizeof(_TCHAR) - 1);

	if (result == 0 || result > 2950) {
		return false;
	}

	if (PathRemoveFileSpec(locale_dir) == FALSE) {
		return false;
	}
	TCHAR *p = _tcsstr(locale_dir, _T("sample-"));
	if (p != NULL) {
		*p = 0;
	}

	const TCHAR dir[] = _T("locale");
	if (PathAppend(locale_dir, dir) == FALSE) {
		return false;
	}

	if (_tbindtextdomain(PACKAGE_NAME, locale_dir) == NULL) {
		return false;
	}
#if defined(UNICODE) || defined(_UNICODE)
	if (wbind_textdomain_codeset(PACKAGE_NAME, _T("UTF-16LE")) == NULL) {
		return false;
	}
#endif
	if (_ttextdomain(PACKAGE_NAME) == NULL) {
		return false;
	}

	return true;
}
