#pragma once
#include <minwindef.h>
#include <string>
#include <time.h>
#include <osdefs.h>
#include <crtdefs.h>
#include <stringdef.h>
#include "SysError.h"
#include <kassert.h>
#include <SpinLock.h>
#include <skyoswindow.h>
#include <DeviceDriverManager.h>
#include <BuildOption.h>

#ifdef __cplusplus
extern "C" {
	class InterruptHandler;
	class Team;
	class Thread;
#endif
	HANDLE kCreateProcess(const char* execPath, void* param, int priority);
	HANDLE kCreateThread(THREAD_START_ENTRY entry, const char* name, void* data, int priority, DWORD flag);
	DWORD kSuspendThread(HANDLE hThread);
	DWORD kResumeThread(HANDLE hThread);
	BOOL kTerminateThread(HANDLE handle, DWORD* dwRetCode);
	BOOL kExitThread(int errorCode);
	BOOL kExitProcess(int errorCode);

	BOOL kSetThreadPriority(HANDLE hThread, int nPriority);
	int kGetThreadPriority(HANDLE hThread);
	DWORD_PTR kSetThreadAffinityMask(HANDLE hThread, DWORD_PTR dwThreadAffinityMask);
	BOOL kSetThreadPriorityBoost(HANDLE hThread, bool DisablePriorityBoost);

	int kGetCurrentThreadId(void);
	int kGetCurrentThread();

	int kGetCurrentProcessId(void);

	int kAquireSemaphore(HANDLE handle, int timeout);
	HANDLE kCreateSemaphore(const char* name, int count);
	int kReleaseSemaphore(HANDLE handle, int count);

	HANDLE kCreateMutex(const char* name);
	int kLockMutex(HANDLE handle);
	int kUnlockMutex(HANDLE handle);

	int kCreateSpinLock(_SPINLOCK* handle);
	int kLockSpinLock(_SPINLOCK* handle);
	int kUnlockSpinLock(_SPINLOCK* handle);

	int kCloseHandle(HANDLE handle);

	void kInitializeCriticalSection(void* lpCriticalSection);
	void kDeleteCriticalSection(void* lpCriticalSection);
	BOOL kTryEnterCriticalSection(void* lpCriticalSection);
	void kEnterCriticalSection(void* lpCriticalSection);
	void kLeaveCriticalSection(void* lpCriticalSection);

	BOOL kIsGraphicMode();
	BOOL kIsEmulationMode();

	BOOL kCreateHeap();
	

	void kRaiseException(DWORD dwExceptionCode, DWORD dwExceptionFlags, DWORD nNumberOfArguments, CONST ULONG_PTR* lpArguments);
	HANDLE kCreateFileMapping(HANDLE hFile, DWORD fdwProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR pszName);
	PVOID kMapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, DWORD dwNumberOfBytesToMap);

	HANDLE kSetTimer(HWND hWnd, DWORD nIDEvent, UINT nElapse, void (CALLBACK* lpfnTimer)(HWND, UINT, DWORD*, DWORD));
	BOOL kKillTimer(HWND hWnd, DWORD* nIDEvent);

	BOOL kSetEvent(HANDLE hEvent);
	HANDLE kCreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, bool bManualReset, bool bInitialState, LPCTSTR lpName);
	BOOL kResetEvent(HANDLE hEvent);
	BOOL kPulseEvent(HANDLE hEvent);
	DWORD kWaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
	int kWaitForMultipleObjects(int dispatcherCount, const HANDLE* lpHandles, BOOL bWaitAll, DWORD dwMilliseconds);
	int kWaitForChildProcess(int handle);

	HANDLE kCreateArea(const char* name, unsigned int* requestAddr, int flags, unsigned int size, PageProtection protection);
	int kDeleteArea(HANDLE handle);

	HANDLE kLoadLibrary(char* file);
	HANDLE kGetProcAddress(HMODULE handle, const char* name);
	BOOL kFreeLibrary(HMODULE handle);

	void kSleep(DWORD dwMilliseconds);
	DWORD kGetTickCount();
	int kGetSystemTime(tm* lpTime);
	int kGetTime(time_t* lptime);
	BOOL kGetLocalTime(LPSYSTEMTIME lpSystemTime);
	BYTE kSetLocalTime(LPSYSTEMTIME lpSystemTime);

	BOOL kCreateWindow(RECT* rect, const char* title, DWORD flags, QWORD* windowId);
	BOOL kDrawWindow(QWORD* windowId, char* buffer, RECT* rect);
	BOOL kDeleteWindow(QWORD* windowId);
	BOOL kReceiveEventFromWindowQueue(QWORD* windowId, EVENT* pstEvent);

	BOOL kGetScreenArea(RECT* pScreenArea);
	BOOL kShowWindow(QWORD* windowId, bool show);
	BOOL kDrawRect(QWORD* qwWindowID, RECT* rect, COLOR color, bool fill);
	BOOL kDrawLine(int left, int top, int right, int bottom, COLOR color);
	BOOL kDrawCircle(int iX, int iY, int iRadius, COLOR color, bool fill);
	BOOL kDrawText(QWORD* windowId, POINT* point, TEXTCOLOR* textColor, const char* text, int length);
	BOOL kGetWindowArea(QWORD* windowId, RECT* pWindowArea);
	BOOL kSendEventToWindow(QWORD* qwWindowID, const EVENT* pstEvent);
	BOOL kSendEventToWindowManager(const EVENT* pstEvent);
	BOOL kFindWindowByTitle(const char* pcTitle, QWORD* qwWindowId);
	BOOL kDrawButton(QWORD* windowId, RECT* pstButtonArea, COLOR stBackgroundColor, const char* pcText, COLOR stTextColor);
	BOOL kUpdateScreenByID(QWORD* qwWindowID);
	BOOL kUpdateScreenByWindowArea(QWORD* qwWindowID, const RECT* pstArea);
	BOOL kBitBlt(QWORD* qwWindowID, RECT* rect, COLOR* pstBuffer, int width, int height);
	BOOL kMoveWindowToTop(QWORD* qwWindowID);
	BOOL kMoveWindow(QWORD* qwWindowID, int x, int y);

	void kGetCursorPosition(int* piX, int* piY);
	BOOL kGetTopWindowID(QWORD* windowId);

	void kPanic(const char* fmt, ...);
	int kTraceCallStack();
	void kSendSerialLog(char* buffer, int size);
	void kDebugPrint(const char* fmt, ...);
	
	DWORD kGetLastError();
	DWORD kSetLastError(DWORD dwErrorCode);
	
	void kObserveInterrupt(int _vector, InterruptHandler* pHandler);
	void kIgnoreInterrupt(InterruptHandler* pHandler);
	void kSetDriverInterruptVector(int intno, void(&vect) ());

	BOOL kSetCurrentDriveId(char drive);
	BOOL kSetCurrentDirectory(const char* lpPathName);
	DWORD kGetCurrentDirectory(DWORD  nBufferLength, char* lpBuffer);
	int kGetCurrentDriveId();

	int kQueryPCIInfo(unsigned int venderId, unsigned int deviceId, PCIDeviceDetails* pDeviceDetails);
	unsigned int kMapPhysicalMemory(const char* name, unsigned int pa, unsigned int size, PageProtection protection, unsigned int fixed_va);
	unsigned int kGetPAFromVM(unsigned int va);

	BOOL kVirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD  dwFreeType);
	LPVOID kVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtect);
	BOOL kVirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD  flNewProtect, PDWORD lpflOldProtect);

	int kGetCommandFromKeyboard(char* commandBuffer, int bufSize);
	char kGetChar();

	void dslock(_In_ SafeMemoryLock_t* MemoryLock);
	void dsunlock(_In_ SafeMemoryLock_t* MemoryLock);
	void PrintCurrentTime();

	void kprintf(const char* fmt, ...);
	
	int CreateFileArea(const char name[], const char path[], unsigned int va, off_t offset,
		size_t size, int flags, PageProtection prot, Team& team);

	BOOL CopyUser(void* dest, const void* src, int size);

	BOOL kGetCurrentConsoleWindowId(QWORD* qwWindowID);
	BOOL kRegisterWindowId(QWORD* qwWindowID);

	DWORD kGetEnvironmentVariable(LPCTSTR lpName, LPTSTR  lpBuffer, DWORD  nSize);
	BOOL kSetEnvironmentVariable(LPCTSTR lpName, LPCTSTR lpValue);

	//-----------------------------------------------------------------------------------
	HMODULE kGetModuleHandle(LPCSTR lpModuleName);
	BOOL kTlsSetValue(DWORD  dwTlsIndex, LPVOID lpTlsValue);
	LPVOID kTlsGetValue(DWORD dwTlsIndex);
	DWORD kTlsAlloc();
	BOOL kTlsFree(DWORD dwTlsIndex);

#ifdef __cplusplus
}
#endif

