#ifndef _SYSTEM_CALL_IMPL_H
#define _SYSTEM_CALL_IMPL_H

#include <crtdefs.h>

#define SCTYPE  int

#define SCPARAM(Arg)                ((SCTYPE)Arg)
_CODE_BEGIN
CRTDECL(SCTYPE, syscall0(SCTYPE Function));
CRTDECL(SCTYPE, syscall1(SCTYPE Function, SCTYPE Arg0));
CRTDECL(SCTYPE, syscall2(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1));
CRTDECL(SCTYPE, syscall3(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1, SCTYPE Arg2));
CRTDECL(SCTYPE, syscall4(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1, SCTYPE Arg2, SCTYPE Arg3));
CRTDECL(SCTYPE, syscall5(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1, SCTYPE Arg2, SCTYPE Arg3, SCTYPE Arg4));
//CRTDECL(SCTYPE, syscall6(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1, SCTYPE Arg2, SCTYPE Arg3, SCTYPE Arg4, SCTYPE Arg5));
_CODE_END

enum
{
	eSysCallTest = 0,
	eSleep = 1,
	eGetSystemTime = 2,
	eGetTickCount = 3,
	eGetTime =		4,
	ePanic = 5,

	eExitProcess = 19,
	eExitThread = 20,
	eCreateThread = 21,
	eTerminateThread = 22,
	eGetCurrentThreadId = 23,
	eSetThreadPriority = 24,
	eGetThreadPriority = 25,
	eSetThreadAffinityMask = 26,
	eSetThreadPriorityBoost = 27,
	eSuspendThread = 28,
	eResumeThread = 29,
	eGetCurrentThread = 30,

	eCreateSemaphore = 31,
	eAquireSemaphore = 32,
	eReleaseSemaphore = 33,
	
	eCloseHandle =		41,
	eCreateEvent = 42,
	eSetEvent = 43,
	eResetEvent = 44,
	eWaitForSingleObject = 45,

	eCreateMutex = 46,
	eLockMutex = 47,
	eUnlockMutex = 48,
	
	eWaitForMultipleObjects = 49,

	eCreateArea = 51,
	eDeleteArea = 52, 

	ePipeOpen = 61,
	ePipeClose = 62,
	ePipeRead = 63,
	ePipeSend = 64,
	ePipeReceive = 65,
	eRemoteCall = 67,
	eRpcGetResponse = 68,
	eRemoteCallWait = 69,
	eRemoteCallRespond = 70,	

	eCreateProcess = 71,
	eGetCurrentProcessId = 72,

	eCreateSpinlock = 81,
	eLockSpinlock = 82,
	eUnlockSpinlock = 83,
	eSetTimer = 84,
	eKillTimer = 85,
	
	eDSLock = 90,
	eDSUnlock = 91,

	eMapPhysicalMemory = 93,
	eGetPAFromVM = 94,
	eTraceCallStack = 96,

	eCreateWindow = 99,
	eDrawWindow = 100,
	eDeleteWindow = 101,
	eReceiveEventFromWindowQueue = 102,	

	eLoadLibrary = 103,
	eGetProcAddress = 104,
	eFreeLibrary = 105,

	eQueryPCIInfo = 106,
	eGetCommandFromKeyboard = 107,
	eGetChar = 108,
	ePrintf = 109,
	eMalloc = 110,
	eFree = 111,
	eRealloc = 112,
	eCalloc = 113,

	eGetCurrentDirectory,
	eSetCurrentDirectory,
	eOutPortByte,
	eOutPortWord,
	eOutPortDWord,
	eInPortByte,
	eInPortWord,
	eInPortDWord,	

	eSetLastError,
	eGetLastError,
	eSetDriverInterruptVector,

	eObserveInterrupt,
	eIgnoreInterrupt,

	eWaitForChildProcess,
	eMalloc_Aligned,
	eSetCurrentDriveId,
	eGetCurrentDriveId,
	eSendSerialLog,
	
	eGetScreenArea,
	eShowWindow,
	eDrawRect,
	eDrawCircle,
	eDrawLine,
	eDrawText,
	eGetWindowArea,
	eSendEventToWindow,
	eSendEventToWindowManager,
	eFindWindowByTitle,
	eDrawButton,
	eUpdateScreenByID,
	eGetCursorPosition,
	eGetTopWindowID,
	eUpdateScreenByWindowArea,
	eBitBlt,
	eMoveWindowToTop,
	eMoveWindow,

	eInitializeCriticalSection,
	eDeleteCriticalSection,
	eTryEnterCriticalSection,
	eEnterCriticalSection,
	eLeaveCriticalSection,
	eIsGraphicMode,
	eVirtualAlloc,
	eVirtualProtect,
	eVirtualFree,	
	eCreateHeap,
	eRaiseException,
	eIsEmulationMode,

	eCreateFileMapping,
	eMapViewOfFile,
	eGetLocalTime,

	eGetCurrentConsoleWindowId,
	eRegisterWindowId,

	eGetEnvironmentVariable,
	eSetEnvironmentVariable,
};

/* Communication system calls
* - Communication related system call definitions */
#define Syscall_PipeOpen(Port, Flags) (int)syscall2(ePipeOpen, SCPARAM(Port), SCPARAM(Flags))
#define Syscall_PipeClose(Port) (int)syscall1(ePipeClose, SCPARAM(Port))
#define Syscall_PipeRead(Port, Buffer, Length) (int)syscall3(ePipeRead, SCPARAM(Port), SCPARAM(Buffer), SCPARAM(Length))
#define Syscall_PipeSend(ProcessId, Port, Buffer, Length) (int)syscall4(ePipeSend, SCPARAM(ProcessId), SCPARAM(Port), SCPARAM(Buffer), SCPARAM(Length))
#define Syscall_PipeReceive(ProcessId, Port, Buffer, Length) (int)syscall4(ePipeReceive, SCPARAM(ProcessId), SCPARAM(Port), SCPARAM(Buffer), SCPARAM(Length))
#define Syscall_RemoteCall(RemoteCall, Asynchronous) (int)syscall2(eRemoteCall, SCPARAM(RemoteCall), SCPARAM(Asynchronous))
#define Syscall_RpcGetResponse(RemoteCall) (int)syscall1(eRpcGetResponse, SCPARAM(RemoteCall))
#define Syscall_RemoteCallWait(Port, RemoteCall, ArgumentBuffer) (int)syscall3(eRemoteCallWait, SCPARAM(Port), SCPARAM(RemoteCall), SCPARAM(ArgumentBuffer))
#define Syscall_RemoteCallRespond(RemoteCall, Buffer, Length) (int)syscall3(eRemoteCallRespond, SCPARAM(RemoteCall), SCPARAM(Buffer), SCPARAM(Length))

#define Syscall_SysCallTest(command) (int)syscall1(eSysCallTest, SCPARAM(command))
#define Syscall_Panic(msg) syscall1(ePanic, SCPARAM(msg))

#define Syscall_Sleep(timeout) syscall1(eSleep, SCPARAM(timeout))
#define Syscall_GetSystemTime(tm) (int)syscall1(eGetSystemTime, SCPARAM(tm))
#define Syscall_GetTickCount() (int)syscall0(eGetTickCount)
#define Syscall_GetTime(time) (int)syscall1(eGetTime, SCPARAM(time))

#define Syscall_CreateThread(entry, name, data, priority, flag) (int)syscall5(eCreateThread, SCPARAM(entry), SCPARAM(name), SCPARAM(data), SCPARAM(priority), flag)
#define Syscall_TerminateThread(hThread, dwExitCode) (bool)syscall2(eTerminateThread, SCPARAM(hThread), SCPARAM(dwExitCode))
#define Syscall_ExitThread(dwExitCode) (int)syscall1(eExitThread, SCPARAM(dwExitCode))
#define Syscall_GetCurrentThreadId() (int)syscall0(eGetCurrentThreadId)

#define Syscall_SuspendThread(handle) (bool)syscall1(eSuspendThread, SCPARAM(handle))
#define Syscall_ResumeThread(handle) (bool)syscall1(eResumeThread, SCPARAM(handle))

#define Syscall_CreateProcess(name, param, priority) (int)syscall3(eCreateProcess, SCPARAM(name), SCPARAM(param), SCPARAM(priority))
#define Syscall_GetCurrentProcessId() (int)syscall0(eGetCurrentProcessId)

#define Syscall_CreateSemaphore(name, count) (int)syscall2(eCreateSemaphore, SCPARAM(name), SCPARAM(count))
#define Syscall_AquireSemaphore(handle,timeout) (int)syscall2(eAquireSemaphore, SCPARAM(handle), SCPARAM(timeout))
#define Syscall_ReleaseSemaphore(handle,count) (int)syscall2(eReleaseSemaphore, SCPARAM(handle), SCPARAM(count))

#define Syscall_CloseHandle(handle) (int)syscall1(eCloseHandle, SCPARAM(handle))
#define Syscall_CreateEvent(attr, manualreset, initstate, name) (HANDLE)syscall4(eCreateEvent, SCPARAM(attr), SCPARAM(manualreset), SCPARAM(initstate), SCPARAM(name))
#define Syscall_SetEvent(handle) (bool)syscall1(eSetEvent, SCPARAM(handle))
#define Syscall_ResetEvent(handle) (bool)syscall1(eResetEvent, SCPARAM(handle))
#define Syscall_WaitForSingleObject(handle, timeout) (DWORD)syscall2(eWaitForSingleObject, SCPARAM(handle), SCPARAM(timeout))
#define Syscall_WaitForMultipleObjects(dispatcherCount, syncObjects, flags, dwMilliseconds) (int)syscall4(eWaitForMultipleObjects, SCPARAM(dispatcherCount), SCPARAM(syncObjects), SCPARAM(flags), SCPARAM(dwMilliseconds))

#define Syscall_CreateMutex(name) (DWORD)syscall1(eCreateMutex, SCPARAM(name))
#define Syscall_LockMutex(handle) (int)syscall1(eLockMutex, SCPARAM(handle))
#define Syscall_UnlockMutex(handle) (int)syscall1(eUnlockMutex, SCPARAM(handle))

#define Syscall_SetThreadPriority(hThread, nPriority) (bool)syscall2(eSetThreadPriority, SCPARAM(hThread), SCPARAM(nPriority))
#define Syscall_GetThreadPriority(hThread) (int)syscall1(eGetThreadPriority, SCPARAM(hThread))
#define Syscall_SetThreadAffinityMask(hThread, dwThreadAffinityMask) (DWORD_PTR)syscall2(eSetThreadAffinityMask, SCPARAM(hThread), SCPARAM(dwThreadAffinityMask))
#define Syscall_SetThreadPriorityBoost(hThread, DisablePriorityBoost)  (bool)syscall2(eSetThreadPriority, SCPARAM(hThread), SCPARAM(DisablePriorityBoost))

#define Syscall_GetCurrentThread()  (int)syscall0(eGetCurrentThread)
#define Syscall_exit(reason)  (int)syscall1(eExitProcess, reason)
#define Syscall_SetTimer(hWnd, nIDEvent, nElapse, lpfnTimer) (HANDLE)syscall4(eSetTimer, SCPARAM(hWnd), SCPARAM(nIDEvent), SCPARAM(nElapse), SCPARAM(lpfnTimer))
#define Syscall_KillTimer(hWnd, nIDEvent) (bool)syscall2(eKillTimer, SCPARAM(hWnd), SCPARAM(nIDEvent))

#define Syscall_CreateArea(name, requestAddr, flags, size, protection) (int)syscall5(eCreateArea, SCPARAM(name), SCPARAM(requestAddr), SCPARAM(flags), SCPARAM(size), SCPARAM(protection))
#define Syscall_DeleteArea(handle) (int)syscall1(eDeleteArea, SCPARAM(handle))

#define Syscall_CreateSpinLock(handle) (int)syscall1(eCreateSpinlock, SCPARAM(handle))
#define Syscall_LockSpinLock(handle) (int)syscall1(eLockSpinlock, SCPARAM(handle))
#define Syscall_UnlockSpinLock(handle) (int)syscall1(eUnlockSpinlock, SCPARAM(handle))

//#define Syscall_QueryPCIInfo(venderId, deviceId, pDeviceDetails) (int)syscall3(eQueryPCIInfo, SCPARAM(venderId), SCPARAM(deviceId), SCPARAM(pDeviceDetails))
#define Syscall_MapPhysicalMemory(name, pa, size, protection, fixed_va) (unsigned int)syscall5(eMapPhysicalMemory, SCPARAM(name), SCPARAM(pa), SCPARAM(size), SCPARAM(protection), SCPARAM(fixed_va))
#define Syscall_GetPAFromVM(va) (unsigned int)syscall1(eGetPAFromVM, SCPARAM(va))

#define Syscall_TraceCallStack() (int)syscall0(eTraceCallStack)

#define Syscall_CreateWindow(rect, title, flags, windowId) (int)syscall4(eCreateWindow, SCPARAM(rect), SCPARAM(title), SCPARAM(flags), SCPARAM(windowId))
#define Syscall_DrawWindow(windowId, buffer, rect) (bool)syscall3(eDrawWindow, SCPARAM(windowId), SCPARAM(buffer), SCPARAM(rect))

#define Syscall_DeleteWindow(windowId) (bool)syscall1(eDeleteWindow, SCPARAM(windowId))
#define Syscall_ReceiveEventFromWindowQueue(qwWindowID, pstEvent) (bool)syscall2(eReceiveEventFromWindowQueue, SCPARAM(qwWindowID),SCPARAM(pstEvent))

#define Syscall_LoadLibrary(name) (DWORD)syscall1(eLoadLibrary, SCPARAM(name))
#define Syscall_GetProcAddress(handle, name) (void*)syscall2(eGetProcAddress, SCPARAM(handle), SCPARAM(name))
#define Syscall_FreeLibrary(name) (bool)syscall1(eFreeLibrary, SCPARAM(name))

#define Syscall_GetCommandFromKeyboard(buffer, size) (int)syscall2(eGetCommandFromKeyboard, SCPARAM(buffer), SCPARAM(size))
#define Syscall_GetChar() (char)syscall0(eGetChar)
#define Syscall_Printf(buf, arglist) (void)syscall2(ePrintf, SCPARAM(buf), SCPARAM(arglist))

#define Syscall_Malloc(size) (char*)syscall1(eMalloc, SCPARAM(size))
#define Syscall_Malloc_Aligned(size, alignment) (void*)syscall2(eMalloc_Aligned, SCPARAM(size), SCPARAM(alignment))
#define Syscall_SetCurrentDriveId(drive) (bool)syscall1(eSetCurrentDriveId, SCPARAM(drive))
#define Syscall_GetCurrentDriveId() (int)syscall0(eGetCurrentDriveId)

#define Syscall_Realloc(ptr, size) (u32int)syscall2(eRealloc, SCPARAM(ptr), SCPARAM(size))
#define Syscall_Calloc(count, size) (u32int)syscall2(eCalloc, SCPARAM(count), SCPARAM(size))

#define Syscall_Free(ptr) (void)syscall1(eFree, SCPARAM(ptr))

#define Syscall_GetCurrentDirectory(nBufferLength, lpBuffer) (int)syscall2(eGetCurrentDirectory, SCPARAM(nBufferLength), SCPARAM(lpBuffer))
#define Syscall_SetCurrentDirectory(path) (bool)syscall1(eSetCurrentDirectory, SCPARAM(path))
#define Syscall_OutPortByte(port, value) (void)syscall2(eOutPortByte, SCPARAM(port), SCPARAM(value))
#define Syscall_OutPortWord(port, value) (void)syscall2(eOutPortWord, SCPARAM(port), SCPARAM(value))
#define Syscall_OutPortDWord(port, value) (void)syscall2(eOutPortDWord, SCPARAM(port), SCPARAM(value))
#define Syscall_InPortByte(port) (uchar)syscall1(eInPortByte, SCPARAM(port))
#define Syscall_InPortWord(port) (ushort)syscall1(eInPortWord, SCPARAM(port))
#define Syscall_InPortDWord(port) (long)syscall1(eInPortDWord, SCPARAM(port))

#define Syscall_SetLastError(error) (void)syscall1(eSetLastError, SCPARAM(error))
#define Syscall_GetLastError() (DWORD)syscall0(eSetLastError)

#define Syscall_SetDriverInterruptVector(num, func) (void)syscall2(eSetDriverInterruptVector, SCPARAM(num), SCPARAM(func))
#define Syscall_DebugPrint(pMsg) (void)syscall1(eDebugPrint, SCPARAM(pMsg))

#define Syscall_kObserveInterrupt(_vector, pHandler) (void)syscall2(eObserveInterrupt, SCPARAM(_vector), SCPARAM(pHandler))
#define Syscall_kIgnoreInterrupt(pHandler) (void)syscall1(eIgnoreInterrupt, SCPARAM(pHandler))
#define Syscall_WaitForChildProcess(hwnd) (void)syscall1(eWaitForChildProcess, SCPARAM(hwnd))

#define Syscall_SendSerialLog(buf, size) (void)syscall2(eSendSerialLog, SCPARAM(buf), SCPARAM(size))

#define Syscall_DSLock(MemoryLock) (void)syscall1(eDSLock, SCPARAM(MemoryLock))
#define Syscall_DSUnlock(MemoryLock) (void)syscall1(eDSUnlock, SCPARAM(MemoryLock))

#define Syscall_GetScreenArea(rect) (bool)syscall1(eGetScreenArea, SCPARAM(rect))
#define Syscall_ShowWindow(windowId, show) (bool)syscall2(eShowWindow, SCPARAM(windowId), SCPARAM(show))
#define Syscall_DrawRect(windowId, rect, color, fill) (bool)syscall4(eDrawRect, SCPARAM(windowId), SCPARAM(rect), SCPARAM(color), SCPARAM(fill))
#define Syscall_DrawLine(left, top, right, bottom, color) (bool)syscall5(eDrawLine, SCPARAM(left), SCPARAM(top), SCPARAM(right), SCPARAM(bottom), SCPARAM(color))
#define Syscall_DrawCircle(x, y, radius, color, fill) (bool)syscall5(eDrawCircle, SCPARAM(x), SCPARAM(y), SCPARAM(radius), SCPARAM(color), SCPARAM(fill))
#define Syscall_DrawText(windowId, point, textColor, text, length) (bool)syscall5(eDrawText, SCPARAM(windowId), SCPARAM(point), SCPARAM(textColor), SCPARAM(text), SCPARAM(length))
#define Syscall_GetWindowArea(windowId, area) (bool)syscall2(eGetWindowArea, SCPARAM(windowId), SCPARAM(area))

#define Syscall_SendEventToWindow(qwWindowID, pstEvent) (bool)syscall2(eSendEventToWindow, SCPARAM(qwWindowID), SCPARAM(pstEvent))
#define Syscall_SendEventToWindowManager(pstEvent) (bool)syscall1(eSendEventToWindowManager, SCPARAM(pstEvent))
#define Syscall_FindWindowByTitle(pcTitle, qwWindowId) (bool)syscall2(eFindWindowByTitle, SCPARAM(pcTitle), SCPARAM(qwWindowId))
#define Syscall_DrawButton(windowId, pstButtonArea, stBackgroundColor, pcText, stTextColor) (bool)syscall5(eDrawButton, SCPARAM(windowId), SCPARAM(pstButtonArea), SCPARAM(stBackgroundColor), SCPARAM(pcText), SCPARAM(stTextColor))
#define Syscall_UpdateScreenByID(qwWindowID) (bool)syscall1(eUpdateScreenByID, SCPARAM(qwWindowID))
#define Syscall_GetCursorPosition(piX, piY) (bool)syscall2(eGetCursorPosition, SCPARAM(piX), SCPARAM(piY))
#define Syscall_GetTopWindowID(windowId) (bool)syscall1(eGetTopWindowID, SCPARAM(windowId))
#define Syscall_UpdateScreenByWindowArea(windowId, area) (bool)syscall2(eUpdateScreenByWindowArea, SCPARAM(windowId), SCPARAM(area))
#define Syscall_BitBlt(windowId, rect, buffer, width, height) (bool)syscall5(eBitBlt, SCPARAM(windowId), SCPARAM(rect), SCPARAM(buffer), SCPARAM(width), SCPARAM(height))
#define Syscall_MoveWindowToTop(windowId) (bool)syscall1(eMoveWindowToTop, SCPARAM(windowId))
#define Syscall_MoveWindow(windowId, x, y) (bool)syscall3(eMoveWindow, SCPARAM(windowId), SCPARAM(x), SCPARAM(y))

#define Syscall_InitializeCriticalSection(cs) (void)syscall1(eInitializeCriticalSection, SCPARAM(cs))
#define Syscall_DeleteCriticalSection(cs) (void)syscall1(eDeleteCriticalSection, SCPARAM(cs))
#define Syscall_TryEnterCriticalSection(cs) (bool)syscall1(eTryEnterCriticalSection, SCPARAM(cs))
#define Syscall_EnterCriticalSection(cs) (void)syscall1(eEnterCriticalSection, SCPARAM(cs))
#define Syscall_LeaveCriticalSection(cs) (void)syscall1(eLeaveCriticalSection, SCPARAM(cs))
#define Syscall_IsGraphicMode() (bool)syscall0(eIsGraphicMode)
#define Syscall_VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect) (LPVOID)syscall4(eVirtualAlloc, SCPARAM(lpAddress), SCPARAM(dwSize), SCPARAM(flAllocationType), SCPARAM(flProtect))
#define Syscall_VirtualProtect(lpAddress, dwSize, flNewProtect, lpflOldProtect) (LPVOID)syscall4(eVirtualProtect, SCPARAM(lpAddress), SCPARAM(dwSize), SCPARAM(flNewProtect), SCPARAM(lpflOldProtect))
#define Syscall_VirtualFree(lpAddress, dwSize, dwFreeType) (LPVOID)syscall3(eVirtualFree, SCPARAM(lpAddress), SCPARAM(dwSize), SCPARAM(dwFreeType))
#define Syscall_RaiseException(dwExceptionCode, dwExceptionFlags, nNumberOfArguments, lpArguments) syscall4(eRaiseException, SCPARAM(dwExceptionCode), SCPARAM(dwExceptionFlags), SCPARAM(nNumberOfArguments), SCPARAM(lpArguments))
#define Syscall_CreateHeap() (bool)syscall0(eCreateHeap)
#define Syscall_IsEmulationMode() (bool)syscall0(eIsEmulationMode)

#define Syscall_CreateFileMapping(hFile, fdwProtect, dwMaximumSizeHigh, dwMaximumSizeLow, pszName) (HANDLE)syscall5(eCreateFileMapping, SCPARAM(hFile), SCPARAM(fdwProtect), SCPARAM(dwMaximumSizeHigh), SCPARAM(dwMaximumSizeLow), SCPARAM(pszName))
#define Syscall_MapViewOfFile(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap) (PVOID)syscall5(eMapViewOfFile, SCPARAM(hFileMappingObject), SCPARAM(dwDesiredAccess), SCPARAM(dwFileOffsetHigh), SCPARAM(dwFileOffsetLow), SCPARAM(dwNumberOfBytesToMap))
#define Syscall_GetLocalTime(lpSystemTime) (bool)syscall1(eGetLocalTime, SCPARAM(lpSystemTime))

#define Syscall_GetCurrentConsoleWindowId(qwWindowId) (BOOL)syscall1(eGetCurrentConsoleWindowId, SCPARAM(qwWindowId))
#define Syscall_RegisterWindowId(qwWindowId) (BOOL)syscall1(eRegisterWindowId, SCPARAM(qwWindowId))

#define Syscall_GetEnvironmentVariable(lpName, lpBuffer, nSize) (DWORD)syscall3(eGetEnvironmentVariable, SCPARAM(lpName), SCPARAM(lpBuffer), SCPARAM(nSize))
#define Syscall_SetEnvironmentVariable(lpName, lpValue) (DWORD)syscall2(eSetEnvironmentVariable, SCPARAM(lpName), SCPARAM(lpValue))
#endif

