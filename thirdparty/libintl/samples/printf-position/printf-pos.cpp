#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#pragma warning( disable : 4819 )
#pragma warning( disable : 4995 )
#pragma warning( disable : 4996 )

#include "libintl_printf.h"
#if defined(UNICODE) || defined(_UNICODE)
#ifndef _DEBUG
#pragma comment(lib, "libintlu.lib")
#else
#pragma comment(lib, "libintlud.lib")
#endif
#else
#ifndef _DEBUG
#pragma comment(lib, "libintl.lib")
#else
#pragma comment(lib, "libintld.lib")
#endif
#endif

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

int _tmain() {
	char nformat[1024];
	char nmsg[1024];
	wchar_t wformat[1024];
	wchar_t wmsg[1024];

	strcpy(nformat, "char: [1]%d [2]%.2f [3]%16s [4]%c [5]%x\n");
	sprintf(nmsg, nformat, 123, 456.789, "abcdあいう", 'e', 0x7fff);
	printf(nformat, 123, 456.789, "abcdあいう", 'e', 0x7fff);
	MessageBoxA(NULL, nmsg, "sprintf default", MB_OK);

	wcscpy(wformat, L"wchar_t: [1]%d [2]%.2f [3]%16s [4]%c [5]%x\n");
	swprintf(wmsg, 1024, wformat, 123, 456.789, L"abcdあいう", L'e', 0x7fff);
	wprintf(wformat, 123, 456.789, L"abcdあいう", L'e', 0x7fff);
	MessageBoxW(NULL, wmsg, L"wsprintf default", MB_OK);

	strcpy(nformat, "char: [3]%3$16s [1]%1$d [5]%5$x [4]%4$c [2]%2$.2f\n");
	sprintf(nmsg, nformat, 123, 456.789, "abcdあいう", 'e', 0x7fff);
	printf(nformat, 123, 456.789, "abcdあいう", 'e', 0x7fff);
	MessageBoxA(NULL, nmsg, "sprintf position", MB_OK);

	wcscpy(wformat, L"wchar_t: [3]%3$16s [1]%1$d [5]%5$x [4]%4$c [2]%2$.2f\n");
	swprintf(wmsg, 1024, wformat, 123, 456.789, L"abcdあいう", L'e', 0x7fff);
	wprintf(wformat, 123, 456.789, L"abcdあいう", L'e', 0x7fff);
	MessageBoxW(NULL, wmsg, L"wsprintf position", MB_OK);

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

