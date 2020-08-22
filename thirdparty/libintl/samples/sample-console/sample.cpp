#include <windows.h>
//#include <vector>
#include <fstream>
#include <tchar.h>
#include <shlwapi.h>
#ifdef _MSC_VER
#pragma comment(lib, "shlwapi.lib")
#endif
#include <process.h>
#include "../gettext.h"
#include "../get_lcid.h"

#define PACKAGE_NAME _T("hello-c")

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

int _tmain() {
	const char *locale = setlocale(LC_ALL, "de");
	if (locale == NULL) {
		locale = setlocale(LC_ALL, "");
		return 1;
	}
	LCID lcid = get_lcid(locale);
	SetThreadLocale(lcid);

	TCHAR locale_dir[3000];
	DWORD result = GetModuleFileName(GetModuleHandle(NULL), 
									locale_dir, sizeof(locale_dir)/sizeof(TCHAR) - 1);

	if (result == 0 || result > 2950) {
		return 1;
	}

	if (PathRemoveFileSpec(&locale_dir[0]) == FALSE) {
		return 1;
	}
	TCHAR *p = _tcsstr(locale_dir, _T("sample-"));
	if (p != NULL) {
		*p = 0;
	}

	const TCHAR dir[] = _T("locale");
	if (PathAppend(&locale_dir[0], dir) == FALSE) {
		return 1;
	}

#if defined(UNICODE) || defined(_UNICODE)
	_putws(L"UNICODE");
	if (wbindtextdomain(PACKAGE_NAME, locale_dir) == NULL) {
		return 1;
	}
	if (wbind_textdomain_codeset(PACKAGE_NAME, _T("UTF-16LE")) == NULL) {
		return 1;
	}
#else
	puts("MBCS");
	if (bindtextdomain(PACKAGE_NAME, locale_dir) == NULL) {
		return 1;
	}
#endif

	if (_ttextdomain(PACKAGE_NAME) == NULL) {
		return 1;
	}

	_putts(_("Hello, world!"));
	MessageBox(NULL, _("Hello, world!"), locale_dir, MB_OK);

	_TCHAR msg[3000];
	_stprintf(msg, _("This program is running as process number %d."), _getpid());

	_putts(msg);
	MessageBox(NULL, msg, locale_dir, MB_OK);

	// free all resources in libintl library
	libintl_tfreeres();

end:
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

