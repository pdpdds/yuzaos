#include <windef.h>
#include <stdio.h>
#include <crtdefs.h>
#include <systemcall_impl.h>
#include <IDT.h>
#include <SystemAPI.h>
#include <intrinsic.h>
#include <atomic.h>
#include <kmalloc.h>
#include <Debugger.h>

void kSysCallTest(char* command)
{
	kprintf("System Call Test\n");
	Debugger::GetInstance()->ExecuteCommand(command);
}

void kHaltSystem(const char* errMsg);

#define MAX_SYSCALL 256
__EXTERN "C" SCTYPE _syscall(SCTYPE Function, SCTYPE Arg0, SCTYPE Arg1, SCTYPE Arg2, SCTYPE Arg3, SCTYPE Arg4);

int  NoOperation(void)
{
	kprintf("invalid syscall\n");
	return 0;
}

#define DefineSyscall(_Sys) ((uintptr_t)&_Sys)

//정적 시스템 콜 함수 테이블 초기화
uintptr_t   GlbSyscallTable[MAX_SYSCALL] = 
{
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),

	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),

	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),

	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),

	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),

	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),

	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),

	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),

	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),

	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
	DefineSyscall(NoOperation),
};

#if SKY_EMULATOR
#include "../win32stub/OrangeOSAPI.h"
#define AddSyscall(num, _Sys) (RegisterOrangeOSAPI(num, #_Sys, (void *)_Sys))
#else
#define AddSyscall(num, _Sys) (GlbSyscallTable[num] = (uintptr_t)&_Sys)
#endif

_declspec(naked) void SysCallDispatcher()
{
	//서비스 API 번호를 IDX에 저장하고 데이터 셀렉터를 변경한다(0X10)
	static uint32_t idx = 0;
	_asm 
	{
		PUSH EAX
		MOV EAX, 0x10
		MOV DS, AX
		POP EAX
		MOV[idx], EAX
		PUSHAD
	}

	// 요청한 서비스 API의 인덱스가 최대값보다 크면 아무런 처리를 하지 않는다.
	if (idx >= MAX_SYSCALL) 
	{
		_asm 
		{
			//레지스터 복원 및 리턴
			POPAD
			IRETD
		}
	}

	//서비스 인덱스 번호에 해당하는 시스템 함수를 얻어낸다.
	static uintptr_t fnct = 0;
	fnct = GlbSyscallTable[idx];

	//시스템 함수를 실행한다.
	_asm 
	{
		//레지스터들을 복원한 후 함수의 파라메터로 집어넣는다.
		POPAD
		PUSH EDI
		PUSH ESI
		PUSH EDX
		PUSH ECX
		PUSH EBX
		//시스템 콜
		CALL fnct
		//스택 정리 책임은 콜러에게 있다. 파라메터로 5개를 스택에 집어넣었으므로
		//스택 포인터의 값을 20바이트 증가시킨다.
		ADD ESP, 20
		//커널 데이터 셀렉터값을 유저 데이터 셀렉터값으로 변경한다.	
		PUSH EAX
		MOV EAX, 0x23
		MOV DS, AX
		POP EAX
		IRETD
	}
}

void Printf(char* buf)
{
	kprintf(buf);
}


void dslock(_In_ SafeMemoryLock_t* MemoryLock)
{
	bool locked = true;

#ifdef LIBC_KERNEL
	//MemoryLock->Flags = DisableInterrupts();
#endif

	while (1)
	{
		bool val = AtomicInterlockedExchange((LPLONG) & (MemoryLock->SyncObject), locked);
		if (val == false)
		{
			break;
		}
	}

}

/* dsunlock
* Releases the lock given and restores any previous flags. */
void dsunlock(_In_ SafeMemoryLock_t* MemoryLock)
{
	AtomicInterlockedExchange((LPLONG)&MemoryLock->SyncObject, false);
#ifdef LIBC_KERNEL
	//RestoreInterrupts(MemoryLock->Flags);
#endif
}
extern "C" void uprintf(const char* format, va_list arglist);
void RegisterSysCall()
{
	//Develop
	AddSyscall(eSysCallTest, kSysCallTest);
	SetInterruptVector(0x80, (void(__cdecl&)(void))SysCallDispatcher, I86_IDT_DESC_RING3);

	AddSyscall(ePanic, kHaltSystem);
	AddSyscall(eTraceCallStack, kTraceCallStack);
	AddSyscall(eSendSerialLog, kSendSerialLog);
	AddSyscall(ePrintf, uprintf);
	AddSyscall(eSetLastError, kSetLastError);
	AddSyscall(eGetLastError, kGetLastError);

	AddSyscall(eMalloc, kmalloc);
	AddSyscall(eFree, kfree);
	AddSyscall(eRealloc, krealloc);
	AddSyscall(eCalloc, kcalloc);
	AddSyscall(eMalloc_Aligned, kmalloc_aligned);

	AddSyscall(eCreateProcess, kCreateProcess);
	AddSyscall(eGetCurrentProcessId, kGetCurrentProcessId);

	AddSyscall(eCreateThread, kCreateThread);
	AddSyscall(eSuspendThread, kSuspendThread);
	AddSyscall(eResumeThread, kResumeThread);
	AddSyscall(eTerminateThread, kTerminateThread);
	AddSyscall(eExitThread, kExitThread);
	AddSyscall(eExitProcess, kExitProcess);

	AddSyscall(eGetCurrentThreadId, kGetCurrentThreadId);
	AddSyscall(eGetCurrentThread, kGetCurrentThread);
	AddSyscall(eSetThreadPriority, kSetThreadPriority);
	AddSyscall(eGetThreadPriority, kGetThreadPriority);
	AddSyscall(eSetThreadAffinityMask, kSetThreadAffinityMask);
	AddSyscall(eSetThreadPriorityBoost, kSetThreadPriorityBoost);
	
	AddSyscall(eCreateSemaphore, kCreateSemaphore);
	AddSyscall(eAquireSemaphore, kAquireSemaphore);
	AddSyscall(eReleaseSemaphore, kReleaseSemaphore);

	AddSyscall(eCreateMutex, kCreateMutex);
	AddSyscall(eLockMutex, kLockMutex);
	AddSyscall(eUnlockMutex, kUnlockMutex);

	AddSyscall(eCreateSpinlock, kCreateSpinLock);
	AddSyscall(eLockSpinlock, kLockSpinLock);
	AddSyscall(eUnlockSpinlock, kUnlockSpinLock);

	AddSyscall(eCloseHandle, kCloseHandle);

	AddSyscall(eSetTimer, kSetTimer);
	AddSyscall(eKillTimer, kKillTimer);

	AddSyscall(eCreateEvent, kCreateEvent);
	AddSyscall(eSetEvent, kSetEvent);
	AddSyscall(eResetEvent, kResetEvent);
	AddSyscall(eWaitForSingleObject, kWaitForSingleObject);
	AddSyscall(eWaitForMultipleObjects, kWaitForMultipleObjects);
	AddSyscall(eWaitForChildProcess, kWaitForChildProcess);

	AddSyscall(eCreateArea, kCreateArea);
	AddSyscall(eDeleteArea, kDeleteArea);

	AddSyscall(eLoadLibrary, kLoadLibrary);
	AddSyscall(eGetProcAddress, kGetProcAddress);
	AddSyscall(eFreeLibrary, kFreeLibrary);

	AddSyscall(eOutPortByte, OutPortByte);
	AddSyscall(eOutPortWord, OutPortWord);
	AddSyscall(eOutPortDWord, OutPortDWord);

	AddSyscall(eInPortByte, InPortByte);
	AddSyscall(eInPortWord, InPortWord);
	AddSyscall(eInPortDWord, InPortDWord);
	
	AddSyscall(eSleep, kSleep);
	AddSyscall(eGetTickCount, kGetTickCount);
	AddSyscall(eGetSystemTime, kGetSystemTime);
	AddSyscall(eGetTime, kGetTime);

	AddSyscall(eCreateWindow, kCreateWindow);
	AddSyscall(eDrawWindow, kDrawWindow);
	AddSyscall(eDeleteWindow, kDeleteWindow);
	AddSyscall(eReceiveEventFromWindowQueue, kReceiveEventFromWindowQueue);
	//GDI API
	AddSyscall(eGetScreenArea, kGetScreenArea);
	AddSyscall(eShowWindow, kShowWindow);
	AddSyscall(eDrawRect, kDrawRect);
	AddSyscall(eDrawLine, kDrawLine);
	AddSyscall(eDrawCircle, kDrawCircle);
	AddSyscall(eDrawText, kDrawText);
	AddSyscall(eGetWindowArea, kGetWindowArea);
	AddSyscall(eUpdateScreenByWindowArea, kUpdateScreenByWindowArea);
	AddSyscall(eBitBlt, kBitBlt);


	AddSyscall(eSendEventToWindow, kSendEventToWindow);
	AddSyscall(eSendEventToWindowManager, kSendEventToWindowManager);
	AddSyscall(eFindWindowByTitle, kFindWindowByTitle);
	AddSyscall(eDrawButton, kDrawButton);
	AddSyscall(eUpdateScreenByID, kUpdateScreenByID);
	AddSyscall(eGetCursorPosition, kGetCursorPosition);
	AddSyscall(eGetTopWindowID, kGetTopWindowID);
	AddSyscall(eMoveWindowToTop, kMoveWindowToTop);
	AddSyscall(eMoveWindow, kMoveWindow);

	AddSyscall(eObserveInterrupt, kObserveInterrupt);
	AddSyscall(eIgnoreInterrupt, kIgnoreInterrupt);
	AddSyscall(eSetDriverInterruptVector, kSetDriverInterruptVector);

	AddSyscall(eGetCurrentDirectory, kGetCurrentDirectory);
	AddSyscall(eSetCurrentDirectory, kSetCurrentDirectory);
	AddSyscall(eSetCurrentDriveId, kSetCurrentDriveId);
	AddSyscall(eGetCurrentDriveId, kGetCurrentDriveId);

	AddSyscall(eQueryPCIInfo, kQueryPCIInfo);
	AddSyscall(eMapPhysicalMemory, kMapPhysicalMemory);
	AddSyscall(eGetPAFromVM, kGetPAFromVM);

	AddSyscall(eGetCommandFromKeyboard, kGetCommandFromKeyboard);
	AddSyscall(eGetChar, kGetChar);

	AddSyscall(eDSLock, dslock);
	AddSyscall(eDSUnlock, dsunlock);

	AddSyscall(eInitializeCriticalSection, kInitializeCriticalSection);
	AddSyscall(eDeleteCriticalSection, kDeleteCriticalSection);
	AddSyscall(eTryEnterCriticalSection, kTryEnterCriticalSection);
	AddSyscall(eEnterCriticalSection, kEnterCriticalSection);
	AddSyscall(eLeaveCriticalSection, kLeaveCriticalSection);

	AddSyscall(eIsGraphicMode, kIsGraphicMode);

	AddSyscall(eVirtualAlloc, kVirtualAlloc);
	AddSyscall(eVirtualProtect, kVirtualProtect);
	AddSyscall(eVirtualFree, kVirtualFree);

	AddSyscall(eCreateHeap, kCreateHeap);
	AddSyscall(eRaiseException, kRaiseException);
	AddSyscall(eIsEmulationMode, kIsEmulationMode);

	AddSyscall(eCreateFileMapping, kCreateFileMapping);
	AddSyscall(eMapViewOfFile, kMapViewOfFile);
	AddSyscall(eGetLocalTime, kGetLocalTime);

	AddSyscall(eGetCurrentConsoleWindowId, kGetCurrentConsoleWindowId);
	AddSyscall(eRegisterWindowId, kRegisterWindowId);

	AddSyscall(eGetEnvironmentVariable, kGetEnvironmentVariable);
	AddSyscall(eSetEnvironmentVariable, kSetEnvironmentVariable);

	AddSyscall(eGetModuleHandle, kGetModuleHandle);
	AddSyscall(eTlsSetValue, kTlsSetValue);
	AddSyscall(eTlsGetValue, kTlsGetValue);
	AddSyscall(eTlsAlloc, kTlsAlloc);
	AddSyscall(eTlsFree, kTlsFree);

	AddSyscall(eMallocSize, kmalloc_size);

}