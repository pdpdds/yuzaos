#pragma once
#include <stdio.h>

#ifdef WIN32STUB
typedef unsigned long _fsize_t;
#else
#include <minwindef.h>
#include <minwinbase.h>
#include <fileio.h>
#endif
//#include "dirent.h"

#ifdef __cplusplus
extern "C" {
#endif	

//프로세스 관련 인터페이스
typedef struct tag_SKY_PROCESS_INTERFACE
{
	unsigned int(*sky_kCreateThread)(unsigned int processId, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID param, DWORD flag, LPDWORD pThreadId);
	void(*sky_Sleep)(int ms);
	DWORD(*sky_GetTickCount)();

	void* (*sky_GetOrangeOSAPIByIndex)(int index);

	HMODULE(*sky_GetModuleHandle)(LPCTSTR lpModuleName);
	unsigned int(*sky_LoadLibrary)(LPCSTR lpLibFileName);
	unsigned int(*sky_GetProcAddress)(HMODULE hModule, LPCSTR lpProcName);
	bool (*sky_FreeLibrary)(HMODULE hLibModule);

	
	unsigned int(*sky_Time)(time_t* lpTime);
	
	unsigned int(*sky_GetLocalTime)(LPSYSTEMTIME lpSystemTime);

	HANDLE(*sky_kGetCurrentThread)();
	DWORD(*sky_kGetCurrentThreadId)();	 

	DWORD(*sky_TlsAlloc)();
	DWORD(*sky_TlsSetValue)(DWORD  dwTlsIndex, LPVOID lpTlsValue);
	LPVOID (*sky_TlsGetValue)(DWORD dwTlsIndex);
	DWORD(*sky_TlsFree)(DWORD dwTlsIndex);

	DWORD (*sky_WaitForSingleObject)(HANDLE hHandle, DWORD dwMilliseconds);
	

	void (*sky_InitializeCriticalSection)(LPCRITICAL_SECTION lpCriticalSection);
	void (*sky_DeleteCriticalSection)(LPCRITICAL_SECTION lpCriticalSection);
	BOOL (*sky_TryEnterCriticalSection)(LPCRITICAL_SECTION lpCriticalSection);
	void (*sky_EnterCriticalSection)(LPCRITICAL_SECTION lpCriticalSection);
	void (*sky_LeaveCriticalSection)(LPCRITICAL_SECTION lpCriticalSection);
	void (*sky_RaiseException)(DWORD dwExceptionCode, DWORD dwExceptionFlags, DWORD nNumberOfArguments, CONST ULONG_PTR* lpArguments);
	HANDLE(*sky_CreateFileMapping)(HANDLE hFile, DWORD fdwProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR pszName);
	PVOID(*sky_MapViewOfFile)(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, DWORD dwNumberOfBytesToMap);
	LPVOID(*sky_VirtualAlloc)(LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtect);

	HANDLE(*sky_CreateSemaphore)(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCSTR lpName);
	BOOL(*sky_ReleaseSemaphore)(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount);
	HANDLE(*sky_OpenSemaphore)(DWORD  dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName);
	DWORD(*sky_AquireSemaphore)(HANDLE handle, int timeout);

	DWORD(*sky_kSuspendThread)(HANDLE hThread);
	DWORD (*sky_kResumeThread)(HANDLE hThread);

	void (*sky_kExitThread)(DWORD dwExitCode);
	BOOL(*sky_VirtualFree)(void* lpAddress, unsigned int dwSize, unsigned int  dwFreeType);

	bool(*sky_VirtualProtect)(void* address, int size, int attribute, unsigned int* dwOld);
	DWORD(*sky_WaitForMultipleObjects)(DWORD nCount, const HANDLE* lpHandles, BOOL bWaitAll, DWORD dwMilliseconds);
	HANDLE(*sky_GetThreadRealHandle)();

} SKY_PROCESS_INTERFACE;

//데이터 입출력관련 인터페이스
typedef struct tag_SKY_PRINT_INTERFACE
{
	void(*sky_printf)(const char* str, ...);
	int(*sky_getchar)();
	void(*sky_cls)(char* szCls);
} SKY_PRINT_INTERFACE;


//DLL로 넘길 인터페이스  구조체
typedef struct tag_PlatformAPI
{
	SKY_PRINT_INTERFACE		_printInterface;
	SKY_PROCESS_INTERFACE   _processInterface;
}PlatformAPI;

typedef void(*pSetPlatformAPI)(PlatformAPI, bool);
typedef void(*pInitializeDll)();

 extern PlatformAPI g_platformAPI;
#ifdef __cplusplus
}
#endif

