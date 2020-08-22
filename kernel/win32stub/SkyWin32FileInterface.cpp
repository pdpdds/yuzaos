#include <Windows.h>
#include "../../runtime/include/platformapi.h"
#include <conio.h>
#include <string>
#include <stdlib.h>
#include <time.h>

extern "C" __declspec(dllexport) size_t sky_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fread(ptr, size, nmemb, stream);
}

extern "C" __declspec(dllexport) FILE* sky_fopen(const char *filename, const char *mode)
{
	std::string szFileName;
	szFileName += filename;
	return fopen(szFileName.c_str(), mode);
}

extern bool PrintWin32GUI(char* str);
extern "C" __declspec(dllexport) size_t sky_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	
	if (stdout == stream && PrintWin32GUI((char*)ptr))
		return size * nmemb;

	return fwrite(ptr, size, nmemb, stream);
}

extern "C" __declspec(dllexport) int sky_fclose(FILE *stream)
{
	return fclose(stream);
}

extern "C" __declspec(dllexport) int sky_fseek(FILE* stream, long int offset, int whence)
{
	return fseek(stream, offset, whence);
}

void sky_printf(const char* str, ...)
{
	char* p = new char[1024];
	va_list ap;

	va_start(ap, str);
	(void)vsnprintf(p, 1024, str, ap);
	va_end(ap);

	printf(p);
	delete p;
}

int sky_getchar()
{
	return _getch();
}

/* Standard error macro for reporting API errors */
#define PERR(bSuccess, api){if(!(bSuccess)) printf("%s:Error %d from %s on line %d\n", __FILE__, GetLastError(), api, __LINE__);}

void cls(HANDLE hConsole)
{
	COORD coordScreen = { 0, 0 };    /* here's where we'll home the
									 cursor */
	BOOL bSuccess;
	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */
	DWORD dwConSize;                 /* number of character cells in
									 the current buffer */

									 /* get the number of character cells in the current buffer */

	bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);
	PERR(bSuccess, "GetConsoleScreenBufferInfo");
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

	/* fill the entire screen with blanks */

	bSuccess = FillConsoleOutputCharacter(hConsole, (TCHAR) ' ',
		dwConSize, coordScreen, &cCharsWritten);
	PERR(bSuccess, "FillConsoleOutputCharacter");

	/* get the current text attribute */

	bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);
	PERR(bSuccess, "ConsoleScreenBufferInfo");

	/* now set the buffer's attributes accordingly */

	bSuccess = FillConsoleOutputAttribute(hConsole, csbi.wAttributes,
		dwConSize, coordScreen, &cCharsWritten);
	PERR(bSuccess, "FillConsoleOutputAttribute");

	/* put the cursor at (0, 0) */

	bSuccess = SetConsoleCursorPosition(hConsole, coordScreen);
	PERR(bSuccess, "SetConsoleCursorPosition");
	return;
}

void sky_cls(char* szCls)
{
	system(szCls);
}

SKY_PRINT_INTERFACE g_printInterface =
{
	sky_printf,	
	sky_getchar,
	sky_cls,
};


LPVOID sky_VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtect)
{
	return VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
}

unsigned int sky_CreateThread(unsigned int processId, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID param)
{
	DWORD dwThreadId = 0;
	HANDLE hThread = CreateThread(NULL, 0, lpStartAddress, param, 0, &dwThreadId);

	return (unsigned int)hThread;
}

void sky_Sleep(int ms)
{
	Sleep(ms);
}

unsigned int sky_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	return (unsigned int)GetProcAddress(hModule, lpProcName);
}

unsigned int sky_LoadLibrary(LPCSTR lpLibFileName)
{
	return (unsigned int)LoadLibraryA(lpLibFileName);
}

DWORD sky_GetTickCount()
{
	return GetTickCount();
}

unsigned int sky_GetLocalTime(LPSYSTEMTIME lpSystemTime)
{
	GetLocalTime(lpSystemTime);
	return 0;
}

unsigned int sky_Time(time_t* lpTime)
{
	time(lpTime);
	return 0;
}

DWORD sky_kSuspendThread(HANDLE hThread)
{
	return SuspendThread(hThread);
}

DWORD sky_kResumeThread(HANDLE hThread)
{
	return ResumeThread(hThread);
}

HANDLE sky_kGetCurrentThread()
{
	HANDLE hPProcess = GetCurrentProcess();
	HANDLE hPThread = GetCurrentThread();

	HANDLE hRProcess;
	HANDLE hRThread;
	DuplicateHandle(hPProcess, hPProcess, hPProcess, &hRProcess,NULL, FALSE, DUPLICATE_SAME_ACCESS);
	DuplicateHandle(hPProcess, hPThread, hPProcess, &hRThread, NULL, FALSE, DUPLICATE_SAME_ACCESS);

	return hRThread;
}

DWORD sky_kGetCurrentThreadId()
{
	return GetCurrentThreadId();
}

DWORD sky_TlsAlloc()
{
	return TlsAlloc();
}

DWORD sky_TlsSetValue(DWORD  dwTlsIndex, LPVOID lpTlsValue)
{	
	return (DWORD)TlsSetValue(dwTlsIndex, lpTlsValue);
}

LPVOID sky_TlsGetValue(DWORD dwTlsIndex)
{
	return TlsGetValue(dwTlsIndex);
}

DWORD sky_TlsFree(DWORD dwTlsIndex)
{
	return (DWORD)TlsFree(dwTlsIndex);
}

HMODULE sky_GetModuleHandle(LPCTSTR lpModuleName)
{
	return GetModuleHandle(lpModuleName);
}

DWORD sky_WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds)
{
	return WaitForSingleObject(hHandle, dwMilliseconds);
}

bool sky_FreeLibrary(HMODULE hLibModule)
{
	return FreeLibrary(hLibModule);
}

void sky_InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	InitializeCriticalSection(lpCriticalSection);
}
void sky_DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	DeleteCriticalSection(lpCriticalSection);
}

BOOL sky_TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	return TryEnterCriticalSection(lpCriticalSection);
}

void sky_EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	EnterCriticalSection(lpCriticalSection);
}

void sky_LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	LeaveCriticalSection(lpCriticalSection);
}

void sky_RaiseException(DWORD dwExceptionCode, DWORD dwExceptionFlags, DWORD nNumberOfArguments, CONST ULONG_PTR* lpArguments)
{
	RaiseException(dwExceptionCode, dwExceptionFlags, nNumberOfArguments, lpArguments);
}

HANDLE sky_CreateFileMapping(HANDLE hFile, DWORD fdwProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR pszName)
{
	return CreateFileMapping(hFile, NULL, fdwProtect, dwMaximumSizeHigh, dwMaximumSizeLow, pszName);
}

PVOID sky_MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, DWORD dwNumberOfBytesToMap)
{
	return MapViewOfFile(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
}

HANDLE sky_CreateSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCSTR lpName)
{
	HANDLE result = CreateSemaphoreA(lpSemaphoreAttributes, lInitialCount, 100, lpName);
	DWORD error = GetLastError();
	if (error == ERROR_INVALID_HANDLE)
	{
		int b = 0;
	}

	return result;
}

BOOL sky_ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount)
{
	return ReleaseSemaphore(hSemaphore, lReleaseCount, lpPreviousCount);
}

HANDLE sky_OpenSemaphore(DWORD  dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName)
{
	return 0;
}
DWORD sky_AquireSemaphore(HANDLE handle, int timeout)
{
	DWORD result = WaitForSingleObject(handle, timeout);

	if (result != WAIT_OBJECT_0)
	{
		int j = 0;
	}

	return result;
}

void sky_kExitThread(DWORD dwExitCode)
{
	ExitThread(dwExitCode);
}

extern "C" bool sky_VirtualProtect(void* address, int size, int attribute, unsigned int* dwOld)
{
	return VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, (DWORD*)dwOld);
}

extern "C" BOOL sky_VirtualFree(void* lpAddress, unsigned int dwSize, unsigned int  dwFreeType)
{
	return (BOOL)VirtualFree(lpAddress, dwSize, dwFreeType);
}

extern "C" void* GetOrangeOSAPIByIndex(int index);
//프로세스 생성 및 삭제
SKY_PROCESS_INTERFACE g_processInterface =
{
	sky_CreateThread,
	sky_Sleep,
	sky_GetTickCount,
	GetOrangeOSAPIByIndex,

	sky_GetModuleHandle,
	sky_LoadLibrary,
	sky_GetProcAddress,
	sky_FreeLibrary,

	sky_Time,

	sky_GetLocalTime,
	sky_kGetCurrentThread,
	sky_kGetCurrentThreadId,

	sky_TlsAlloc,
	sky_TlsSetValue,
	sky_TlsGetValue,
	sky_TlsFree,

	sky_WaitForSingleObject,

	sky_InitializeCriticalSection,
	sky_DeleteCriticalSection,
	sky_TryEnterCriticalSection,
	sky_EnterCriticalSection,
	sky_LeaveCriticalSection,
	sky_RaiseException,
	sky_CreateFileMapping,
	sky_MapViewOfFile,
	sky_VirtualAlloc,

	sky_CreateSemaphore,
	sky_ReleaseSemaphore,
	sky_OpenSemaphore,
	sky_AquireSemaphore,

	sky_kSuspendThread,
	sky_kResumeThread,

	sky_kExitThread,
	sky_VirtualFree,
	sky_VirtualProtect,
};

