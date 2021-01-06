#pragma once
#include <minwindef.h>
#include "minwinbase.h"
#include <stdint.h>
#include <skyoswindow.h>

#ifdef DLL_WIN32API_EXPORT
#define WINBASEAPI __declspec(dllexport) 
#else
#define WINBASEAPI __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif
	WINBASEAPI bool WINAPI CreateWindow(RECT* rect, const char* title, DWORD flags, QWORD* windowId);
	WINBASEAPI bool WINAPI DrawWindow(QWORD* windowId, char* buffer, RECT* rect);
	WINBASEAPI bool WINAPI DeleteWindow(QWORD* windowId);
	WINBASEAPI bool WINAPI ReceiveEventFromWindowQueue(QWORD* windowId, EVENT* pstEvent);

#ifdef __cplusplus
}
#endif
