#include "errno.h"
#include "FileSysAdaptor.h"
#include "stdarg.h"

#include <Thread.h>
#include <Team.h>
#include <string>

void __cdecl operator delete(void* p) { kfree(p); }
void operator delete(void* p, size_t size) { kfree(p); }
void operator delete[](void* p) {kfree(p); }
void operator delete[](void* p, size_t size) {kfree(p); }
void* operator new(size_t size){return kmalloc(size);}
void* operator new[](size_t size){return kmalloc(size);}
void* operator new(size_t, void* p) { return p; }
void* operator new[](size_t, void* p) { return p; }

int __cdecl _purecall()
{
	assert(0);
	// kprintf("Pure Function Called!!\n");
	return 0;
}

extern "C" int kGetCurrentDriveId()
{
	char driveLetter = Thread::GetRunningThread()->GetTeam()->m_currentDrive;
	return driveLetter -'A';
}

////////////////////////////////////////////////////////////////////////
//파일, 디렉토리
extern "C" DWORD kGetCurrentDirectory(DWORD nBufferLength, char* lpBuffer)
{
/*#if SKY_EMULATOR_DLL
	return g_platformAPI._processInterface.sky_GetCurrentDirectory(nBufferLength, lpBuffer);
#endif*/

	
	Thread* pThread = Thread::GetRunningThread();
	Team* pTeam = pThread->GetTeam();

	int len = strlen(pTeam->m_szCWD);

	int pathMax = 1024;

	if (len == 0 || nBufferLength > (DWORD)pathMax)
		return 0;

	strcpy(lpBuffer, pTeam->m_szCWD);

	return len;
}

extern "C" BOOL kSetCurrentDriveId(char drive)
{
	Thread::GetRunningThread()->GetTeam()->m_currentDrive = drive;
	
	return true;
}

extern "C" BOOL kSetCurrentDirectory(const char* lpPathName)
{
	if (strlen(lpPathName) >= MAXPATH || strlen(lpPathName) == 0)
		return false;

	Team* pTeam = Thread::GetRunningThread()->GetTeam();
	strcpy(pTeam->m_szCWD, lpPathName);
	return true;
}

