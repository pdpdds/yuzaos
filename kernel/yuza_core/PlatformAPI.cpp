#include "PlatformAPI.h"
#include "intrinsic.h"
#include <kmalloc.h>
#include <stringdef.h>
#include "SkyConsole.h"
#include "SystemAPI.h"


unsigned int sky_CreateThread(unsigned int processId, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID param, DWORD flag, LPDWORD pThreadId)
{
	HANDLE hThread = (HANDLE)kCreateThread((THREAD_START_ENTRY)lpStartAddress, "memoryThread", (void*)param, 1, flag);
	return (unsigned int)hThread;
}

void sky_Sleep(int ms)
{
	kSleep(ms);
}

extern DWORD kGetTickCount();
DWORD sky_GetTickCount()
{
	return kGetTickCount();
}

//프로세스 생성 및 삭제
SKY_PROCESS_INTERFACE _processInterface =
{
	sky_CreateThread,
	sky_Sleep,
	sky_GetTickCount,
};

