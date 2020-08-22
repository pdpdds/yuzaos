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
	WINBASEAPI  WINAPI HANDLE CreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, /*LPSECURITY_ATTRIBUTES*/void* lpSecurityAttributes,
		DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
	WINBASEAPI WINAPI BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
	WINBASEAPI WINAPI BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
	WINBASEAPI WINAPI BOOL CloseHandle(HANDLE hObject);
	WINBASEAPI BOOL WINAPI CloseFile(HANDLE hObject);

	WINBASEAPI WINAPI DWORD GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);

	WINBASEAPI WINAPI HANDLE FindFirstFile(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData);
	WINBASEAPI WINAPI BOOL FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData);
	WINBASEAPI WINAPI BOOL FindClose(HANDLE hFindFile);
	
	WINBASEAPI WINAPI int MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
	WINBASEAPI WINAPI BOOL GetSystemTime(SYSTEMTIME* systime);

	WINBASEAPI DWORD WINAPI SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
	WINBASEAPI int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
	WINBASEAPI int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte,
		LPCSTR lpDefaultChar, BOOL* lpUsedDefaultChar);	

	WINBASEAPI WINAPI BOOL SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime, LPFILETIME lpFileTime);
	WINBASEAPI WINAPI BOOL LocalFileTimeToFileTime(const FILETIME* lpLocalFileTime, LPFILETIME lpFileTime);

	WINBASEAPI WINAPI HANDLE CreateThread(LPSECURITY_ATTRIBUTES   lpThreadAttributes, SIZE_T  dwStackSize, LPTHREAD_START_ROUTINE  lpStartAddress,
						LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
	WINBASEAPI WINAPI FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
	WINBASEAPI WINAPI HMODULE  LoadLibrary(LPCSTR lpLibFileName);

	WINBASEAPI HANDLE WINAPI GetCurrentThread(VOID);
	WINBASEAPI DWORD _stdcall GetCurrentThreadId(VOID);
	WINBASEAPI DWORD WINAPI WaitForSingleObject(HANDLE hHandle, DWORD  dwMilliseconds);

	WINBASEAPI DWORD WINAPI SuspendThread(HANDLE hThread);
	WINBASEAPI DWORD WINAPI ResumeThread(HANDLE hThread);

	WINBASEAPI BOOL WINAPI TerminateThread(HANDLE hThread, DWORD dwExitCode);
	
	WINBASEAPI void WINAPI Sleep(DWORD dwMilliseconds);
	WINBASEAPI void WINAPI SetLastError(DWORD dwErrCode);
	WINBASEAPI DWORD WINAPI GetLastError(void);

	WINBASEAPI bool WINAPI FreeLibrary(HMODULE handle);

	WINBASEAPI HANDLE WINAPI CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName);
	WINBASEAPI bool WINAPI SetEvent(HANDLE hEvent);
	WINBASEAPI bool WINAPI ResetEvent(HANDLE hEvent);
	WINBASEAPI DWORD WINAPI WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
	WINBASEAPI DWORD WINAPI  WaitForMultipleObjects(DWORD nCount, const HANDLE* lpHandles, BOOL bWaitAll, DWORD dwMilliseconds);

	WINBASEAPI HANDLE WINAPI CreateSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCSTR lpName);
	WINBASEAPI BOOL WINAPI ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount);

	WINBASEAPI HANDLE WINAPI OpenSemaphore(DWORD  dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName);
	WINBASEAPI DWORD WINAPI AquireSemaphore(HANDLE handle, int timeout);

	WINBASEAPI bool WINAPI SetThreadPriority(HANDLE hThread, int nPriority);
	WINBASEAPI int WINAPI GetThreadPriority(HANDLE hThread);
	WINBASEAPI DWORD_PTR WINAPI SetThreadAffinityMask(HANDLE hThread, DWORD_PTR dwThreadAffinityMask);

	WINBASEAPI BOOL WINAPI TlsSetValue(DWORD  dwTlsIndex, LPVOID lpTlsValue);
	WINBASEAPI LPVOID WINAPI TlsGetValue(DWORD dwTlsIndex);
	WINBASEAPI DWORD WINAPI TlsAlloc();
	WINBASEAPI  WINAPI BOOL TlsFree(DWORD dwTlsIndex);

	WINBASEAPI DWORD WINAPI GetCurrentProcessId();
	WINBASEAPI UINT WINAPI GetSystemDirectory(LPSTR lpBuffer, UINT  uSize);

	WINBASEAPI BOOL WINAPI ReleaseSemaphore(HANDLE hSemaphore, LONG   lReleaseCount, LPLONG lpPreviousCount);

	WINBASEAPI UINT WINAPI GetSystemDirectory(LPSTR lpBuffer, UINT  uSize);

	WINBASEAPI BOOL WINAPI GetProcessAffinityMask(HANDLE     hProcess, PDWORD_PTR lpProcessAffinityMask, PDWORD_PTR lpSystemAffinityMask);
	WINBASEAPI HANDLE WINAPI GetCurrentProcess();
	WINBASEAPI BOOL WINAPI DuplicateHandle(HANDLE   hSourceProcessHandle, HANDLE   hSourceHandle, HANDLE   hTargetProcessHandle,
			LPHANDLE lpTargetHandle, DWORD    dwDesiredAccess, BOOL     bInheritHandle, DWORD    dwOptions);

	WINBASEAPI HANDLE WINAPI OpenProcess(DWORD dwDesiredAccess, BOOL  bInheritHandle, DWORD dwProcessId);
	WINBASEAPI BOOL WINAPI apiCreateDirectory(const char* path, void* p);

	WINBASEAPI LONG WINAPI InterlockedIncrement(LONG volatile* Addend);
	WINBASEAPI LONG WINAPI InterlockedDecrement(LONG volatile* Addend);
	WINBASEAPI LONG WINAPI InterlockedExchange(LONG volatile* Target, LONG Value);
	WINBASEAPI PVOID WINAPI InterlockedExchangePointer(PVOID volatile* Target, PVOID Value);
	WINBASEAPI LONG WINAPI InterlockedExchangeAdd(LONG volatile* Addend, LONG Value);

	WINBASEAPI LONG WINAPI InterlockedCompareExchange(LONG volatile* Destination, LONG ExChange, LONG Comperand);
	WINBASEAPI PVOID WINAPI InterlockedCompareExchangePointer(PVOID volatile* Destination, PVOID Exchange, PVOID Comperand);

	WINBASEAPI LONG WINAPI InterlockedAdd(LONG volatile* Addend, LONG Value);
	WINBASEAPI LONG WINAPI InterlockedAnd(LONG volatile* Destination, LONG Value);
	WINBASEAPI LONG WINAPI InterlockedOr(LONG volatile* Destination, LONG Value);
	WINBASEAPI LONG WINAPI InterlockedXor(LONG volatile* Destination, LONG Value);

	WINBASEAPI void WINAPI InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
	WINBASEAPI void WINAPI DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
	WINBASEAPI BOOL WINAPI TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
	WINBASEAPI void WINAPI EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
	WINBASEAPI void WINAPI LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

	WINBASEAPI bool WINAPI VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD  dwFreeType);
	WINBASEAPI LPVOID WINAPI VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtect);
	WINBASEAPI bool WINAPI VirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD  flNewProtect, PDWORD lpflOldProtect);

	WINBASEAPI HANDLE WINAPI SetTimer(HWND hWnd, DWORD nIDEvent, UINT nElapse, void (CALLBACK* lpfnTimer)(HWND, UINT, DWORD*, DWORD));
	WINBASEAPI bool WINAPI KillTimer(HWND hWnd, DWORD* nIDEvent);

	WINBASEAPI uintptr_t _beginthreadex(void* security, unsigned stack_size, unsigned (__stdcall* start_address)(void*), void* arglist, unsigned initflag, unsigned* thrdaddr);
	WINBASEAPI void _endthreadex(unsigned retval);

	WINBASEAPI HANDLE WINAPI CreateMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName);
	WINBASEAPI BOOL WINAPI ReleaseMutex(HANDLE hMutex);

	WINBASEAPI VOID WINAPI RaiseException(DWORD dwExceptionCode, DWORD dwExceptionFlags, DWORD nNumberOfArguments, CONST ULONG_PTR* lpArguments);

	WINBASEAPI DWORD WINAPI GetTickCount();

	WINBASEAPI bool WINAPI CreateWindow(RECT* rect, const char* title, DWORD flags, QWORD* windowId);
	WINBASEAPI bool WINAPI DrawWindow(QWORD* windowId, char* buffer, RECT* rect);
	WINBASEAPI bool WINAPI DeleteWindow(QWORD* windowId);
	WINBASEAPI bool WINAPI ReceiveEventFromWindowQueue(QWORD* windowId, EVENT* pstEvent);

	WINBASEAPI HANDLE WINAPI CreateFileMapping(HANDLE hFile, PSECURITY_ATTRIBUTES psa, DWORD fdwProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR pszName);
	WINBASEAPI PVOID WINAPI MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, DWORD dwNumberOfBytesToMap);
	
	//no winapi
	WINBASEAPI bool WINAPI ExitThread(int errorcode);

	WINBASEAPI bool WINAPI GetLocalTime(LPSYSTEMTIME lpSystemTime);
	
	WINBASEAPI DWORD WINAPI GetFileAttributes(LPCSTR lpFileName);
	

	DWORD GetEnvironmentVariable(LPCTSTR lpName, LPTSTR  lpBuffer, DWORD  nSize);
	BOOL SetEnvironmentVariable(LPCTSTR lpName, LPCTSTR lpValue);

	///////////////////////////////////////////////////////////////////////////
	//not implemented
	/*LONG64 __cdecl InterlockedCompare64Exchange128(LONG64 volatile* Destination, LONG64 ExchangeHigh, LONG64 ExchangeLow, LONG64 Comparand);
	LONG64 InterlockedDecrement64(LONG64 volatile* Addend);
	LONGLONG __cdecl InterlockedCompareExchangeAcquire64(LONGLONG volatile* Destination, LONGLONG Exchange, LONGLONG Comparand);
	LONG __cdecl InterlockedDecrementAcquire(LONG volatile* Addend);
	LONG __cdecl InterlockedDecrementRelease(LONG volatile* Addend);*/

#ifdef __cplusplus
}
#endif
