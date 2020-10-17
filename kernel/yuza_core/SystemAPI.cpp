#include "SystemAPI.h"
#include "PEImage.h"
#include <stringdef.h>
#include "SkyConsole.h"
#include "PlatformAPI.h"
#include <BootParams.h>
#include <SkyGUISystem.h>
#include <kmalloc.h>
#include "Team.h"
#include <tls.h>
#include <SystemCall_impl.h>
#include <TeamManager.h>
#include <SpinLock.h>
#include <Timer.h>
#include <TaskTimer.h>
#include <PageCache.h>
#include <AddressSpace.h>
#include <Area.h>
#include <intrinsic.h>
#include <Scheduler.h>
#include <ModuleManager.h>
#include <StackTracer.h>
#include <SerialPort.h>
#include <InterruptManager.h>
#include <IDT.h>
#include "physicalmap.h"
#include <./KeyBoard/KeyboardController.h>
#include <Thread.h>

extern BootParams g_bootParams;
DWORD g_dwSysLastError = 0;
extern _Scheduler gScheduler;

//32비트 PE파일 이미지 유효성 검사
BOOL ValidatePEImage(void* image)
{
	IMAGE_DOS_HEADER* dosHeader = 0;
	IMAGE_NT_HEADERS* ntHeaders = 0;

	dosHeader = (IMAGE_DOS_HEADER*)image;
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	if (dosHeader->e_lfanew == 0)
		return false;

	//NT Header 체크
	ntHeaders = (IMAGE_NT_HEADERS*)(dosHeader->e_lfanew + (uint32_t)image);
	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
		return false;

	/* only supporting for i386 archs */
	if (ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
		return false;

	/* only support 32 bit executable images */
	if (!(ntHeaders->FileHeader.Characteristics & (IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_32BIT_MACHINE)))
		return false;

	/*
	Note: 1st 4 MB remains idenitity mapped as kernel pages as it contains
	kernel stack and page directory. If you want to support loading below 1MB,
	make sure to move these into kernel land
	*/

	//로드되는 프로세스의 베이스 주소는 0x00400000다. 
	//비쥬얼 스튜디오에서 속성=> 링커 => 고급의 기준주소 항목에서 확인 가능하다
	if ((ntHeaders->OptionalHeader.ImageBase < 0x400000) || (ntHeaders->OptionalHeader.ImageBase > 0x80000000))
		return false;

	/* only support 32 bit optional header format */
	if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		return false;

	//유효한 32비트 PE 파일이다.
	return true;
}

extern "C" void kprintf(const char *fmt, ...)
{
	char buf[4096] = { 0, };

	va_list arglist;
	va_start(arglist, fmt);

	vsnprintf(buf, 4096, fmt, arglist);

	/*if (g_bootParams.bSystemInit)
	{
		//SkyGUISystem::GetInstance()->Print(buf);

		FILE* fp = fopen("sky32_log.txt", "ab");

		if (fp)
		{
			unsigned int pa = 0;
			char* buffer = (char*)kmalloc_ap(1024, &pa);
			memcpy(buffer, buf, 1024);
			fwrite(buffer, 1024, 1, fp);
			fclose(fp);
		}
	}*/

#if SKY_EMULATOR
	g_platformAPI._printInterface.sky_printf(buf);
#else

	if (SkyGUISystem::GetInstance()->GUIEnable())
	{
		QWORD taskId = Thread::GetRunningThread()->GetTeam()->GetTaskId();

		//if (taskId > 0)
			SkyGUISystem::GetInstance()->Print(taskId, buf);
	}
	else
	{
		SkyConsole::Print(buf);
	}

#endif
	
}


extern BOOL kGetCurrentConsoleWindowId(QWORD* qwWindowID);
extern "C" void kprintMsg(const char* str)
{

#if SKY_CONSOLE_MODE
	g_platformAPI._printInterface.sky_printf(str);
#else

	if (SkyGUISystem::GetInstance()->GUIEnable())
	{

#if SKY_EMULATOR
		QWORD taskId = 0;
		kGetCurrentConsoleWindowId(&taskId);
		SkyGUISystem::GetInstance()->Print(taskId, (char*)str);

#else
		QWORD taskId = Thread::GetRunningThread()->GetTeam()->GetTaskId();
		//if (taskId > 0)
			SkyGUISystem::GetInstance()->Print(taskId, (char*)str);

#endif
	}
	else
	{
		SkyConsole::Print(str);
	}

#endif
}

///////////////////////////////////////////////////////////////////////////////////
//외부로 노출안되는 메소드
///////////////////////////////////////////////////////////////////////////////////
inline Resource* GetResource(int handle, int type)
{
	return Thread::GetRunningThread()->GetTeam()->GetHandleTable()->GetResource(handle, type);
}

inline int OpenHandle(Resource* obj)
{
	return Thread::GetRunningThread()->GetTeam()->GetHandleTable()->Open(obj);
}

#if SKY_EMULATOR


DWORD RunSkyThread(void* data)
{
	ThreadParam* pThreadParms = (ThreadParam*)data;
	THREAD_START_ENTRY threadEntry = pThreadParms->entryPoint;
	void* args = pThreadParms->param;
	
	delete pThreadParms;
 	threadEntry(args);

	kExitThread(0);
	
	return (0);
}

extern void FreeArgument(main_args* args);
DWORD RunMainSkyThread(void* data)
{
	ThreadParam* pThreadParms = (ThreadParam*)data;
	THREAD_START_ENTRY threadEntry = pThreadParms->entryPoint;
	void* args = pThreadParms->param;

	delete pThreadParms;
	threadEntry(args); 

	FreeArgument((main_args *)args);
	kExitThread(0);

	return (0);
}




///////////////////////////////////////////////////////////////////////////////////
//SystemAPI Implementation
///////////////////////////////////////////////////////////////////////////////////

#endif

int UserThreadEntry(void* parameter)
{
	ThreadParam* pParam = (ThreadParam*)parameter;
	int res = 0;

	thread_storage_t    Tls;
	tls_create(&Tls);

	if (pParam->entryPoint == 0)
	{
		kDebugPrint("Main User Thread Start %s %x %x\n", (char*)pParam->name, pParam->entryPoint, parameter);
		Team* pTeam = Thread::GetRunningThread()->GetTeam();
		res = pTeam->StartMainThread(pParam);

		tls_cleanup(thrd_current(), NULL, res);
		tls_destroy(tls_current());

		Thread::GetRunningThread()->Exit();
	}
	else
	{
		kDebugPrint("User Thread Start. thread : 0x%x Name : %s %x %x\n", Thread::GetRunningThread(), (char*)pParam->name, pParam->entryPoint, parameter);
		res = pParam->entryPoint(pParam->param);

		kExitThread(0);
	}

	//Not Reached!!

	return 0;
}

main_args* MakeArgument(const char* path, void* param)
{
	main_args* args = new main_args;
	if (param != nullptr)
	{
		args->argv = (char**)kcalloc(10, sizeof(char*));
		args->argv[0] = (char*)kcalloc(256, 1);
		strcpy(args->argv[0], path);

		char* pCurrentToken = strtok((char*)param, " ");
		int index = 1;
		while (true)
		{
			if (pCurrentToken != nullptr)
			{
				args->argv[index] = (char*)kcalloc(256, 1);
				strcpy(args->argv[index], (char*)pCurrentToken);
				index++;
				pCurrentToken = strtok(NULL, " ");
			}
			else
				break;
		}

		args->argc = index;

		//for (int i = 0; i < args->argc; i++)
			//kprintf("argument %d, %s\n", i, args->argv[i]);

	}
	else
	{
		args->argc = 1;
		args->argv = (char**)kcalloc(2, sizeof(char*));
		args->argv[0] = (char*)kcalloc(256, 1);
		strcpy(args->argv[0], path);
	}

	return args;
}

static int ExecuteFile(const char* path, char* arg)
{
	FILE* fp = fopen(path, "rb");
	
	if (fp == nullptr)
	{
		kDebugPrint("ExecuteFile Fail : %s\n", path);
		return 0;
	}

	fclose(fp);

	ThreadParam* param = new ThreadParam;
	memset(param, 0, sizeof(ThreadParam));

	strcpy(param->name, path);
	
	main_args* args = MakeArgument(path, arg);

	param->param = args;

	param->entryPoint = 0;

	Team* newTeam = TeamManager::GetInstance()->CreateTeam(path);
	if (newTeam == 0)
		return E_NO_MEMORY;

	char cwd[MAX_PATH];
	kGetCurrentDirectory(MAX_PATH, cwd);
	strcpy(newTeam->m_szCWD, cwd);

	const char* filename = path + strlen(path);
	while (filename > path &&* filename != '/')
		filename--;

	char appName[OS_NAME_LENGTH];
	snprintf(appName, OS_NAME_LENGTH, "%.14s thread", filename);

	Thread* userthread = new Thread(appName, newTeam, UserThreadEntry, param);
	if (userthread == 0)
	{
		newTeam->ReleaseRef();
		return E_NO_MEMORY;
	}

	return (int)newTeam->GetTeamId();
}


HANDLE kCreateThread(THREAD_START_ENTRY entry, const char* name, void* data, int priority, DWORD flag)
{
	Team* pTeam = Thread::GetRunningThread()->GetTeam();

#if SKY_EMULATOR
	ThreadParam* param = new ThreadParam;
	memset(param, 0, sizeof(ThreadParam));
	strcpy(param->name, name);
	param->param = data;
	param->entryPoint = entry;

	Thread* thread = new Thread(name, pTeam, (THREAD_START_ENTRY)RunSkyThread, param, priority, flag);
#else
	Thread* thread = 0;
	if (pTeam->GetAddressSpace() == AddressSpace::GetKernelAddressSpace())
	{
		thread = new Thread(name, pTeam, entry, data, priority, flag);
	}
	else
	{
		ThreadParam* param = new ThreadParam;
		memset(param, 0, sizeof(ThreadParam));
		strcpy(param->name, name);
		param->param = data;
		param->entryPoint = entry;

		thread = new Thread(name, pTeam, UserThreadEntry, param, priority, flag);
	}

#endif

	kDebugPrint("kCreateThread. Team : %x, Name : %s, Thread : %x\n", pTeam, name, thread);

	return thread->m_resourceHandle;
}

BOOL kExitThread(int errorCode)
{
	int res = 0;
	tls_cleanup(thrd_current(), NULL, res);
	tls_destroy(tls_current());

	kCloseHandle(Thread::GetRunningThread()->m_resourceHandle);
	Thread::GetRunningThread()->Exit();

	return TRUE;
}

BOOL kTerminateThread(HANDLE handle, DWORD* lpRetCode)
{
#if SKY_EMULATOR
	g_platformAPI._processInterface.sky_kExitThread(0);
	return TRUE;
#endif
	lpRetCode = 0;
	ASSERT(Thread::GetRunningThread() != handle);
	Thread* target = (Thread *)Thread::GetRunningThread()->GetTeam()->GetHandleTable()->GetResource((int)handle, OBJ_THREAD);
	if (target == nullptr)
		return false;

	target->ForceExit();
	target->ReleaseRef();

	return true;
}

char* GetFileNameFromPath(char* path)
{
	char* ssc;
	int l = 0;
	ssc = strstr(path, "\\");

	if (ssc == 0)
		return path;

	do {
		l = strlen(ssc) + 1;
		path = &path[strlen(path) - l + 2];
		ssc = strstr(path, "\\");
	} while (ssc);

	return path;
}

HANDLE kCreateProcess(const char* execPath, void* param, int priority)
{
	char filepath[MAXPATH] = { 0, };
	char* path = 0;
	if (strlen(execPath) == 0)
		return 0;
 
	strcpy(filepath, execPath);
	path = filepath;

#if SKY_EMULATOR_DLL
	path = GetFileNameFromPath(filepath);
#endif


#if SKY_EMULATOR
	HANDLE moduleHandle = nullptr;

	typedef int(*PMain)(void*);
	//#if 1

	FILE* fp = fopen(path, "rb");
	if (fp == 0)
		return 0;
	fclose(fp);

	moduleHandle = (void*)ModuleManager::GetInstance()->LoadPE(path);

	if (moduleHandle == nullptr)
		return 0;

	PMain mainFunc = (PMain)ModuleManager::GetInstance()->GetModuleFunction(moduleHandle, "MainCRTStartup");

	SKY_ASSERT(mainFunc != nullptr, "main entry null!!");

	main_args* args = MakeArgument(path, param);

	kDebugPrint("CreateProcess %x %s %d\n", mainFunc, path, priority);

	Team* newTeam = TeamManager::GetInstance()->CreateTeam(path);
	newTeam->m_moduleHandle = moduleHandle;

	ThreadParam* pStartParam = new ThreadParam;
	pStartParam->entryPoint = mainFunc;
	strcpy(pStartParam->name, path);
	pStartParam->param = args;

	Thread* thread = new Thread(path, newTeam, (THREAD_START_ENTRY)RunMainSkyThread, pStartParam, priority);
	HANDLE handle = (HANDLE)thread->m_win32Handle;
	
#else
	HANDLE handle = (HANDLE)ExecuteFile(path, (char*)param);
#endif


	return handle;
}
 
DWORD kSuspendThread(HANDLE hThread)
{
	int fl = DisableInterrupts();
	Thread* target = reinterpret_cast<Thread*>(Thread::GetRunningThread()->GetTeam()->GetHandleTable()->GetResource((int)hThread, OBJ_THREAD));
	
	if (target == nullptr)
	{
		RestoreInterrupts(fl);
		return 0;
	}
	
	kDebugPrint("kSuspendThread Team : %s, Thread : 0x%0x\n", target->GetTeam()->GetName(), target);

	target->SetState(kThreadSuspended);
	target->IncreaseSuspendCount();

	RestoreInterrupts(fl);

	target->ReleaseRef();

	return target->GetSuspendCount();
}

DWORD kResumeThread(HANDLE hThread)
{
	Thread* thread = reinterpret_cast<Thread*>(Thread::GetRunningThread()->GetTeam()->GetHandleTable()->GetResource((int)hThread, OBJ_THREAD));
	
	if (thread == 0)
		return 0;

	ASSERT(hThread != Thread::GetRunningThread()->m_resourceHandle);
	ASSERT(thread->GetState() == kThreadSuspended);
	thread->SetState(kThreadReady);
	kprintf("found resume target %x\n", thread);

	gScheduler.EnqueueReadyThread(thread); 
	gScheduler.Reschedule();

	thread->ReleaseRef();
	return 0;
}

/*스레드 우선순위
THREAD_PRIORITY_TIME_CRITICAL   가장 높은 우선 순위보다 더 높은 우선 순위
THREAD_PRIORITY_HIGHEST         가장 높은 우선 순위
THREAD_PRIORITY_ABOVE_NORMAL    보통 보다 조금 높은 우선 순위
THREAD_PRIORITY_NORMAL          보통
THREAD_PRIORITY_BELOW_NORMAL    보통 보다 조금 낮은 우선 순위
THREAD_PRIORITY_LOWEST          가장 낮은 우선 순위
THREAD_PRIORITY_IDLE            가장 낮은 우선 순위보다 더 낮은 우선 순위*/

BOOL kSetThreadPriority(HANDLE hThread, int nPriority)
{
	return true;
}

int kGetThreadPriority(HANDLE hThread)
{
	return 0;
}

DWORD_PTR kSetThreadAffinityMask(HANDLE hThread, DWORD_PTR dwThreadAffinityMask)
{
	return 0;
}

BOOL kSetThreadPriorityBoost(HANDLE hThread, bool DisablePriorityBoost)
{
	return false;
}

int kGetCurrentThreadId(void)
{
#if SKY_EMULATOR
	return g_platformAPI._processInterface.sky_kGetCurrentThreadId();
#else
	return Thread::GetRunningThread()->GetCurrentThreadId();
#endif
}

int kGetCurrentThread()
{
//#if SKY_EMULATOR
	//return (int)g_platformAPI._processInterface.sky_kGetCurrentThread();
//#else
	return (int)Thread::GetRunningThread()->m_resourceHandle;
//#endif
}

////////////////////////////////////////////////////////////////////////
//세마포어
int kAquireSemaphore(HANDLE handle, int timeout)
{
#if SKY_EMULATOR
	return g_platformAPI._processInterface.sky_AquireSemaphore(handle, timeout);
#endif

	bigtime_t timeOut = timeout;

	if (timeout == -1)
		timeOut = INFINITE_TIMEOUT;

	Semaphore* sem = static_cast<Semaphore*>(GetResource((int)handle, OBJ_SEMAPHORE));
	if (sem == 0)
		return E_BAD_HANDLE;

	int ret = sem->Wait(timeOut);
	sem->ReleaseRef();
	return ret;
}

HANDLE kCreateSemaphore(const char* name, int count)
{
#if SKY_EMULATOR
	return g_platformAPI._processInterface.sky_CreateSemaphore(NULL, count, -1, name);
#endif

	Semaphore* sem = new Semaphore(name, count);

	if (sem == 0)
		return 0;

	return (HANDLE)OpenHandle(sem);
}

int kReleaseSemaphore(HANDLE handle, int count)
{
#if SKY_EMULATOR
	LONG previousCount;
	return g_platformAPI._processInterface.sky_ReleaseSemaphore(handle, count, &previousCount);
#endif

	Semaphore* sem = static_cast<Semaphore*>(GetResource((int)handle, OBJ_SEMAPHORE));
	if (sem == 0)
		return E_BAD_HANDLE;

	sem->Release(count);
	sem->ReleaseRef();
	return E_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////
//스핀락
int kCreateSpinLock(_SPINLOCK* handle)
{
	SpinLock* spinlock = new SpinLock();
	if (spinlock == 0)
		return E_NO_MEMORY;

	spinlock->teamId = Thread::GetRunningThread()->GetTeam()->GetTeamId();
	spinlock->fHolder = Thread::GetRunningThread();

	handle->teamId = spinlock->teamId;
	handle->holder = (int)spinlock->fHolder;
	handle->handleId = OpenHandle(spinlock);

	return E_NO_ERROR;
}

int kLockSpinLock(_SPINLOCK* handle)
{
	Team* team = TeamManager::GetInstance()->FindTeam(handle->teamId);

	if (team == nullptr)
		return false;

	SpinLock* spinlock = static_cast<SpinLock*>(team->GetHandleTable()->GetResource(handle->handleId, OBJ_SPINLOCK));

	if (spinlock == 0)
	{
		//team->ReleaseRef();
		return E_BAD_HANDLE;
	}

	int ret = spinlock->Lock();
	spinlock->ReleaseRef();

	//team->ReleaseRef();

	return ret;
}

int kUnlockSpinLock(_SPINLOCK* handle)
{
	Team* team = TeamManager::GetInstance()->FindTeam(handle->teamId);

	if (team == nullptr)
		return false;

	SpinLock* spinlock = static_cast<SpinLock*>(team->GetHandleTable()->GetResource(handle->handleId, OBJ_SPINLOCK));

	if (spinlock == 0)
	{
		//team->ReleaseRef();
		return E_BAD_HANDLE;
	}

	spinlock->Unlock();
	spinlock->ReleaseRef();

	//team->ReleaseRef();

	return E_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////
//뮤텍스
HANDLE kCreateMutex(const char* name)
{
	RecursiveLock* mutex = new RecursiveLock(name);
	if (mutex == 0)
		return 0;

	return (HANDLE)OpenHandle(mutex);
}

int kLockMutex(HANDLE handle)
{
	RecursiveLock* mutex = static_cast<RecursiveLock*>(GetResource((int)handle, OBJ_MUTEX));
	if (mutex == 0)
		return E_BAD_HANDLE;

	int ret = mutex->Lock();
	mutex->ReleaseRef();

	return E_NO_ERROR;
}

int kUnlockMutex(HANDLE handle)
{
	RecursiveLock* mutex = static_cast<RecursiveLock*>(GetResource((int)handle, OBJ_MUTEX));
	if (mutex == 0)
		return E_BAD_HANDLE;

	mutex->Unlock();
	mutex->ReleaseRef();

	return E_NO_ERROR;
}

int kCloseHandle(HANDLE handle)
{
	Resource* resource = static_cast<Resource*>(GetResource((int)handle, OBJ_ANY));
	
	if (resource == 0)
	{
		ASSERT("kCloseHandle");
		return E_BAD_HANDLE;
	}

	Thread::GetRunningThread()->GetTeam()->GetHandleTable()->Close((int)handle);

	resource->ReleaseRef();

	return 0; 
}

////////////////////////////////////////////////////////////////////////
//타이머
static int UserTimerThread(void* parameter)
{
	TimerObject* timer = (TimerObject*)parameter;

	while (1)
	{
		DWORD dwResult = kWaitForSingleObject(timer->eventHandle, -1);

		if (timer->m_disabled == true)
			break;

		if (dwResult == E_BAD_HANDLE)
		{
			kPanic("Timer Handle is BAD\n");
		}

		timer->lpfnTimer(0, timer->idEvent, 0, 0);
	}

	Syscall_ExitThread(0);

	return 0;
}

HANDLE kSetTimer(HWND hWnd, DWORD nIDEvent, UINT nElapse, void (CALLBACK* lpfnTimer)(HWND, UINT, DWORD*, DWORD))
{
	TimerObject* timer = new TimerObject(nIDEvent);
	if (timer == 0)
		return 0;

	HANDLE handle = (HANDLE)OpenHandle(timer);

	timer->Start(nElapse, lpfnTimer);
	HANDLE threadHandle = kCreateThread(UserTimerThread, "TimerThread", timer, 15, 0);
	//kCloseHandle(threadHandle);

	return (HANDLE)OpenHandle(timer);
}

BOOL kKillTimer(HWND hWnd, DWORD* nIDEvent)
{
	TimerObject* timer = static_cast<TimerObject*>(GetResource((int)hWnd, OBJ_TIMER));
	if (timer == 0)
		return false;

	timer->Stop();

	kCloseHandle(hWnd);
	timer->ReleaseRef();

	return true;
}

////////////////////////////////////////////////////////////////////////
//이벤트
HANDLE kCreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, bool bManualReset, bool bInitialState, LPCTSTR lpName)
{
	Semaphore* sem = new Semaphore("event", 0);
	if (sem == 0)
		return 0;

	return (HANDLE)OpenHandle(sem);
}

BOOL kSetEvent(HANDLE hEvent)
{
	Semaphore* sem = static_cast<Semaphore*>(GetResource((int)hEvent, OBJ_SEMAPHORE));
	if (sem == 0)
	{
		kPanic("sem is null %x\n", (int)hEvent);
		return false;
	}
	sem->Release();
	sem->ReleaseRef();

	return false;
}

BOOL kResetEvent(HANDLE hEvent)
{
	Semaphore* sem = static_cast<Semaphore*>(GetResource((int)hEvent, OBJ_SEMAPHORE));
	if (sem == 0)
		return false;

	sem->Reset();
	sem->ReleaseRef();

	return true;
}

DWORD kWaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds)
{
	bigtime_t timeOut = dwMilliseconds;

	if (dwMilliseconds == 0xffffffff)
		timeOut = INFINITE_TIMEOUT;

	Resource* res = static_cast<Thread*>(GetResource((int)hHandle, OBJ_ANY));
	if (res == 0)
		return WAIT_FAILED;

	int ret = res->Wait(timeOut);
	res->ReleaseRef();

	if (ret == E_NO_ERROR)
		return WAIT_OBJECT_0;
	else if (ret == E_TIMED_OUT)
		return WAIT_TIMEOUT;
	else if (ret == E_BAD_HANDLE)
		return WAIT_ABANDONED;

	return WAIT_FAILED;
}

int kWaitForMultipleObjects(int handleCount, const HANDLE* lpHandles, BOOL bWaitAll, DWORD dwMilliseconds)
{
/*#if SKY_EMULATOR
	DWORD res = g_platformAPI._processInterface.sky_WaitForMultipleObjects(handleCount, lpHandles, bWaitAll, dwMilliseconds);
	return res;
#endif*/
	int fl = DisableInterrupts();
	int result = E_NO_ERROR;

	bigtime_t timeOut = dwMilliseconds;

	if (dwMilliseconds == 0xffffffff)
		timeOut = INFINITE_TIMEOUT;

	Resource** resources = new Resource * [handleCount];
	for (int handleIndex = 0; handleIndex < handleCount; handleIndex++)
		resources[handleIndex] = 0;

	HandleTable* pTable = Thread::GetRunningThread()->GetTeam()->GetHandleTable();
	for (int handleIndex = 0; handleIndex < handleCount; handleIndex++)
	{
		resources[handleIndex] = pTable->GetResource((int)lpHandles[handleIndex]);
		
		if (resources[handleIndex] == 0) {
			result = E_BAD_HANDLE;
			break;
		}

		//if (resources[handleIndex]->GetType() == OBJ_THREAD)
			//kDebugPrint("kWaitForMultipleObjects 0. Thread Ref : %x %x\n", resources[handleIndex], resources[handleIndex]->GetRef());
	}

	RestoreInterrupts(fl);

	if (result == E_NO_ERROR)
	{
		result = Synchronization::WaitForMultipleSyncObject(handleCount, reinterpret_cast<Synchronization**>(resources), (WaitFlags)bWaitAll, dwMilliseconds);

	}
	
	for (int handleIndex = 0; handleIndex < handleCount; handleIndex++)
	{
		if (resources[handleIndex])
		{
			
			resources[handleIndex]->ReleaseRef();
		
			//if (resources[handleIndex]->GetType() == OBJ_THREAD)
				//kDebugPrint("kWaitForMultipleObjects 1. Thread Ref : %x %x\n", resources[handleIndex], resources[handleIndex]->GetRef());
		}
	}

	delete[] resources;
	
	return result;
}

int kWaitForChildProcess(int handle)
{
	return 0;
	return kWaitForSingleObject((HANDLE)handle, INFINITE);
}

HANDLE kCreateArea(const char* name, unsigned int* requestAddr, int flags, unsigned int size, PageProtection protection)
{
	unsigned int va = INVALID_PAGE;
	if (flags & EXACT_ADDRESS)
		va = *requestAddr;

	PageCache* cache = new PageCache;
	if (cache == 0)
		return 0;

	// It is important that CreateArea not incur a fault!		
	char nameCopy[OS_NAME_LENGTH] = { 0, };
	if (!CopyUser(nameCopy, name, strlen(name)))
		return 0;

	Area* newArea = AddressSpace::GetCurrentAddressSpace()->CreateArea(nameCopy, size, AREA_NOT_WIRED,
		protection | USER_READ | SYSTEM_READ | ((protection & USER_WRITE)
			? SYSTEM_WRITE : 0), cache, 0, va, flags);

	if (newArea == 0)
	{
		delete cache;
		return 0;
	}

	*requestAddr = newArea->GetBaseAddress();

	return (HANDLE)OpenHandle(newArea);
}

int kDeleteArea(HANDLE handle)
{
	Area* area = static_cast<Area*>(GetResource((int)handle, OBJ_AREA));
	if (area == 0)
		return E_BAD_HANDLE;

	Thread::GetRunningThread()->GetTeam()->GetHandleTable()->Close((int)handle);
	AddressSpace::GetCurrentAddressSpace()->DeleteArea(area);
	area->ReleaseRef();
	return E_NO_ERROR;

}

HANDLE kLoadLibrary(char* dllName)
{
	if(strcmp(dllName, "Win32Stub.dll") == 0)
		return (HANDLE)g_platformAPI._processInterface.sky_LoadLibrary(dllName);

#if SKY_EMULATOR_DLL
	return (HANDLE)g_platformAPI._processInterface.sky_LoadLibrary(dllName);
#endif
	return (HANDLE)ModuleManager::GetInstance()->LoadPE(dllName);
}

HANDLE kGetProcAddress(HMODULE handle, const char* name)
{
	HANDLE func = (HANDLE)ModuleManager::GetInstance()->GetModuleFunction(handle, name);
	return func;
}

BOOL kFreeLibrary(HMODULE handle)
{
#if SKY_EMULATOR
	return g_platformAPI._processInterface.sky_FreeLibrary(handle);
#else
	return ModuleManager::GetInstance()->UnloadPE((LOAD_DLL_INFO*)handle);
#endif
}

BOOL kVirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD  dwFreeType)
{
	ASSERT(lpAddress != NULL);
	
#if SKY_EMULATOR
	return g_platformAPI._processInterface.sky_VirtualFree(lpAddress, dwSize, dwFreeType);
#endif
	//ASSERT(dwSize == 0);
	//ASSERT(dwFreeType == MEM_RELEASE);
	//return Thread::GetRunningThread()->GetTeam()->DeallocateMemory(lpAddress);
	
	kfree(lpAddress);
	return true;
}

LPVOID kVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtect)
{
#if SKY_EMULATOR
	return g_platformAPI._processInterface.sky_VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
#endif

	return kmalloc(dwSize);
	//if (lpAddress)
		//return lpAddress;

	//return Thread::GetRunningThread()->GetTeam()->AllocateMemory(dwSize);
}

BOOL kVirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD  flNewProtect, PDWORD lpflOldProtect)
{
#if SKY_EMULATOR
	return g_platformAPI._processInterface.sky_VirtualProtect(lpAddress, dwSize, flNewProtect, (unsigned int*)lpflOldProtect);
#endif

	return true;
}

////////////////////////////////////////////////////////////////////////
//윈도우 생성과 삭제
BOOL kCreateWindow(RECT* rect, const char* title, DWORD flags, QWORD* windowId)
{
	bool result =  SkyGUISystem::GetInstance()->CreateWindow(rect, title, flags, windowId);

	if (result)
	{
		Thread::GetRunningThread()->GetTeam()->SetTaskId(*windowId);
	}

	return result;
}

BOOL kDrawWindow(QWORD* windowId, char* buffer, RECT* rect)
{
	return SkyGUISystem::GetInstance()->DrawWindow(windowId, buffer, rect);
}

BOOL kDeleteWindow(QWORD* windowId)
{
	return SkyGUISystem::GetInstance()->DeleteWindow(windowId);
}

BOOL kReceiveEventFromWindowQueue(QWORD* windowId, EVENT* pstEvent)
{
	return SkyGUISystem::GetInstance()->ReceiveEventFromWindowQueue(windowId, pstEvent);
}

BOOL kGetScreenArea(RECT* pScreenArea)
{
	return SkyGUISystem::GetInstance()->GetScreenArea(pScreenArea);
}

BOOL kGetWindowArea(QWORD* windowId, RECT* pWindowArea)
{
	return SkyGUISystem::GetInstance()->GetWindowArea(windowId, pWindowArea);
}

BOOL kUpdateScreenByWindowArea(QWORD* qwWindowID, const RECT* pstArea)
{
	return SkyGUISystem::GetInstance()->UpdateScreenByWindowArea(qwWindowID, pstArea);
}

BOOL kShowWindow(QWORD* windowId, bool show)
{
	return SkyGUISystem::GetInstance()->ShowWindow(windowId, show);
}

BOOL kDrawRect(QWORD* qwWindowID, RECT* rect, COLOR color, bool fill)
{
	return SkyGUISystem::GetInstance()->DrawRect(qwWindowID, rect, color, fill);
}

BOOL kDrawLine(int left, int top, int right, int bottom, COLOR color)
{
	return SkyGUISystem::GetInstance()->DrawLine(left, top, right, bottom, color);
}

BOOL kDrawCircle(int iX, int iY, int iRadius, COLOR color, bool fill)
{
	return SkyGUISystem::GetInstance()->DrawCircle(iX, iY, iRadius, color, fill);
}

BOOL kDrawText(QWORD* windowId, POINT* point, TEXTCOLOR* textColor, const char* text, int length)
{
	return SkyGUISystem::GetInstance()->DrawText(windowId, point, textColor, text, length);
}

BOOL kSendEventToWindow(QWORD* qwWindowID, const EVENT* pstEvent)
{
	return SkyGUISystem::GetInstance()->SendEventToWindow(qwWindowID, pstEvent);
}
BOOL kSendEventToWindowManager(const EVENT* pstEvent)
{
	return SkyGUISystem::GetInstance()->SendEventToWindowManager(pstEvent);
}

BOOL kFindWindowByTitle(const char* pcTitle, QWORD* qwWindowId)
{
	return SkyGUISystem::GetInstance()->FindWindowByTitle(pcTitle, qwWindowId);
}

BOOL kDrawButton(QWORD* windowId, RECT* pstButtonArea, COLOR stBackgroundColor, const char* pcText, COLOR stTextColor)
{
	return SkyGUISystem::GetInstance()->DrawButton(windowId, pstButtonArea, stBackgroundColor, pcText, stTextColor);
}

BOOL kUpdateScreenByID(QWORD* qwWindowID)
{
	return SkyGUISystem::GetInstance()->UpdateScreenByID(qwWindowID);
}

void kGetCursorPosition(int* piX, int* piY)
{
	return SkyGUISystem::GetInstance()->GetCursorPosition(piX, piY);
}

BOOL kGetTopWindowID(QWORD* windowId)
{
	return SkyGUISystem::GetInstance()->GetTopWindowID(windowId);
}

BOOL kBitBlt(QWORD* qwWindowID, RECT* rect, COLOR* pstBuffer, int width, int height)
{
	return SkyGUISystem::GetInstance()->BitBlt(qwWindowID, rect, pstBuffer, width, height);
}

BOOL kMoveWindowToTop(QWORD* qwWindowID)
{
	return SkyGUISystem::GetInstance()->MoveWindowToTop(qwWindowID);
}

BOOL kMoveWindow(QWORD* qwWindowID, int x, int y)
{
	return SkyGUISystem::GetInstance()->MoveWindow(qwWindowID, x, y);
}

#include "LoadDLL.h"

char* remove_ext(char* myStr, char extSep, char pathSep) {
	char* retStr, * lastExt, * lastPath;

	// Error checks and allocate string.

	if (myStr == NULL) return NULL;
	if ((retStr = (char*)malloc(strlen(myStr) + 1)) == NULL) return NULL;

	// Make a copy and find the relevant characters.

	strcpy(retStr, myStr);
	lastExt = (char*)strrchr(retStr, extSep);
	lastPath = (pathSep == 0) ? NULL : (char*)strrchr(retStr, pathSep);

	// If it has an extension separator.

	if (lastExt != NULL) {
		// and it's to the right of the path separator.

		if (lastPath != NULL) {
			if (lastPath < lastExt) {
				// then remove it.

				*lastExt = '\0';
			}
		}
		else {
			// Has extension separator with no path separator.

			*lastExt = '\0';
		}
	}

	// Return the modified string.

	return retStr;
}

int kTraceCallStack()
{
	auto iter = Thread::GetRunningThread()->GetTeam()->m_loadedDllList.begin();

	for (; iter != Thread::GetRunningThread()->GetTeam()->m_loadedDllList.end(); iter++)
	{
		LOAD_DLL_INFO* pInfo = (LOAD_DLL_INFO *)(*iter);

		if (ModuleManager::GetInstance()->GetSystemPE(pInfo->moduleName))
			continue;

		char buf[256];
		strcpy(buf, pInfo->moduleName);
		char* retStr = remove_ext(buf, '.', '/');
		std::string name = retStr;
		name += ".map";

		StackTracer::GetInstance()->AddSymbol(name.c_str(), pInfo->image_base);
	}

	StackTracer::GetInstance()->TraceStackWithSymbol(30, 0);

	return 0;
}

void kSendSerialLog(char* buffer, int size)
{
#if SKY_EMULATOR
	return;
#endif

	SendSerialData((BYTE*)buffer, size);
}

extern bool g_serialPortInit;
void kDebugPrint(const char* fmt, ...)
{
#ifndef DEBUG_KERNEL
	return;
#endif

	char buf[1024] = { 0, };

	va_list arglist;
	va_start(arglist, fmt);
	vsnprintf(buf, 1024, fmt, arglist);
	va_end(arglist);

#if SKY_EMULATOR
	kprintf(buf);
	return;
#endif

	if(g_bootParams.bGraphicMode && g_serialPortInit == true)
		SendSerialData((BYTE*)buf, strlen(buf) + 1);
	else
		kprintf(buf);
}

DWORD kGetLastError()
{
	return g_dwSysLastError;
}

DWORD kSetLastError(DWORD dwErrorCode)
{
	return g_dwSysLastError = dwErrorCode;
}


////////////////////////////////////////////////////////////////////////
//인터럽트 및 인터럽트 핸들러

extern void InterrputDefaultHandler();

void kObserveInterrupt(int _vector, InterruptHandler* pHandler)
{
	InterruptManager::ObserveInterrupt(_vector, pHandler);
}

void kIgnoreInterrupt(InterruptHandler* pHandler)
{
	InterruptManager::IgnoreInterrupts(pHandler);
}

void kSetDriverInterruptVector(int intno, void(&vect) ())
{

	if ((int)vect == 0)
		SetInterruptVector(intno, InterrputDefaultHandler, 0);
	else
		SetInterruptVector(intno, vect, 0);
}

int kQueryPCIInfo(unsigned int venderId, unsigned int deviceId, PCIDeviceDetails* pDeviceDetails)
{
	PCIDeviceDetails* info = DeviceDriverManager::GetInstance()->FindPCIDevice(venderId, deviceId);

	if (info == nullptr)
		return -1;

	memcpy(pDeviceDetails, info, sizeof(PCIDeviceDetails));

	return 0;
}

unsigned int kMapPhysicalMemory(const char* name, unsigned int pa, unsigned int size, PageProtection protection, unsigned int fixed_va)
{
	if(fixed_va == 0)
		return AddressSpace::GetKernelAddressSpace()->MapPhysicalMemory(name, pa, size, protection)->GetBaseAddress();
	
	return  AddressSpace::GetKernelAddressSpace()->MapPhysicalMemory(name, pa, size, protection, fixed_va)->GetBaseAddress();
}

unsigned int kGetPAFromVM(unsigned int va)
{
	AddressSpace* pAddress = AddressSpace::GetCurrentAddressSpace();
	PhysicalMap* pMap = (PhysicalMap*)pAddress->GetPhysicalMap();

	int physicalAddress = pMap->GetPhysicalAddress(va);

	if(INVALID_PAGE == physicalAddress)
		kPanic("kGetPAFromVM Fail!!\n");

	return physicalAddress;
}

int kGetCommandFromKeyboard(char* commandBuffer, int bufSize)
{
	char c = 0;
	bool	BufChar;
	bool newline = false;

	//버퍼 크기만큼 문자열을 얻어낸다.
	int i = 0;
	while (i < bufSize)
	{

		//! buffer the next char
		BufChar = true;

#if SKY_EMULATOR
		if (kIsGraphicMode())
		{
			c = SkyGUISystem::GetInstance()->GetCh();
			
			if(c == 0)
				continue;
		}
		else
		{
			c = g_platformAPI._printInterface.sky_getchar();
		}
#else

		c = KeyboardController::GetInput();

#endif

		//return
		if (c == 0x0d || c == '\n')
		{
			kprintf("\n");
			newline = true;
			break;
		}

		//backspace
		if (c == 0x08) {

			//! dont buffer this char
			BufChar = false;

			if (i > 0) {

#if SKY_EMULATOR
				g_platformAPI._printInterface.sky_printf("%c", c);
				g_platformAPI._printInterface.sky_printf("%c", ' ');
				g_platformAPI._printInterface.sky_printf("%c", c);

#else
				//! go back one char
				uint y, x;
				SkyConsole::GetCursorPos(x, y);

				if (x > 0)
					SkyConsole::MoveCursor(x, y);
				else {
					//! x is already 0, so go back one line
					y--;
					x = 80;
				}

				//! erase the character from display
				SkyConsole::WriteChar(' ');
				SkyConsole::MoveCursor(x, y);

				//! go back one char in cmd buf

#endif 
				i--;
			}
		}

		//! only add the char if it is to be buffered
		if (BufChar) {

			//! convert key to an ascii char and put it in buffer
			//char c = KeyBoard::ConvertKeyToAscii(key);
			//if (c != 0 && KEY_SPACE != c) { //insure its an ascii char
			if (c != 0) { //insure its an ascii char

#if SKY_EMULATOR
				g_platformAPI._printInterface.sky_printf("%c", c);
#else
				SkyConsole::WriteChar(c);
#endif
				commandBuffer[i++] = c;
			}
		}

		//! wait for next key. You may need to adjust this to suite your needs
		//msleep(10);
	}

	//! null terminate the string
	commandBuffer[i] = 0;
	return i;
}

char kGetChar()
{
	char c = 0;
	bool	BufChar;
	bool newline = false;

	//버퍼 크기만큼 문자열을 얻어낸다.
	int i = 0;

	//! buffer the next char
	BufChar = true;

#if SKY_EMULATOR
	if (kIsGraphicMode())
	{
		c = SkyGUISystem::GetInstance()->GetCh();

	}
	else
	{
		c = g_platformAPI._printInterface.sky_getchar();
	}
#else
	c = KeyboardController::GetInput();

#endif

	//return
	if (c == 0x0d || c == '\n')
	{
		kprintf("\n");
		newline = true;
		return 0;
	}

	//backspace
	if (c == 0x08) 
	{

		//! dont buffer this char
		BufChar = false;

		if (i > 0) 
		{

#if SKY_EMULATOR
			g_platformAPI._printInterface.sky_printf("%c", c);
			g_platformAPI._printInterface.sky_printf("%c", ' ');
			g_platformAPI._printInterface.sky_printf("%c", c);

#else
			//! go back one char
			uint y, x;
			SkyConsole::GetCursorPos(x, y);

			if (x > 0)
				SkyConsole::MoveCursor(x, y);
			else {
				//! x is already 0, so go back one line
				y--;
				x = 80;
			}

			//! erase the character from display
			SkyConsole::WriteChar(' ');
			SkyConsole::MoveCursor(x, y);

			//! go back one char in cmd buf

#endif 
			i--;
		}

		return 0;
	}

	//! only add the char if it is to be buffered
	if (BufChar) 
	{

		//! convert key to an ascii char and put it in buffer
		//char c = KeyBoard::ConvertKeyToAscii(key);
		//if (c != 0 && KEY_SPACE != c) { //insure its an ascii char
		if (c != 0) { //insure its an ascii char

#if SKY_EMULATOR
			g_platformAPI._printInterface.sky_printf("%c", c);
#else
			SkyConsole::WriteChar(c);
#endif

		}
	}

	//! wait for next key. You may need to adjust this to suite your needs
	//msleep(10);

	return c;
}

void kInitializeCriticalSection(void* lpCriticalSection)
{
	CRITICAL_SECTION* cs = (CRITICAL_SECTION*)lpCriticalSection;
	if (cs == 0)
		return;

#if SKY_EMULATOR
	g_platformAPI._processInterface.sky_InitializeCriticalSection((LPCRITICAL_SECTION)lpCriticalSection);
#else

	memset(cs, 0, sizeof(CRITICAL_SECTION));
	HANDLE handle = kCreateMutex("CS");

	if (handle == 0)
	{
		kPanic("InitializeCriticalSection handle allocation fail");
	}

	cs->LockSemaphore = handle;
#endif
}

void kDeleteCriticalSection(void* lpCriticalSection)
{
	CRITICAL_SECTION* cs = (CRITICAL_SECTION*)lpCriticalSection;
	if (lpCriticalSection == 0 || cs->LockSemaphore == 0)
		return;

#if SKY_EMULATOR
	g_platformAPI._processInterface.sky_DeleteCriticalSection((LPCRITICAL_SECTION)lpCriticalSection);
#else
	kCloseHandle(cs->LockSemaphore);
#endif
}

BOOL kTryEnterCriticalSection(void* lpCriticalSection)
{
	CRITICAL_SECTION* cs = (CRITICAL_SECTION*)lpCriticalSection;
	if (lpCriticalSection == 0 || cs->LockSemaphore == 0)
		return FALSE;

#if SKY_EMULATOR
	return g_platformAPI._processInterface.sky_TryEnterCriticalSection((LPCRITICAL_SECTION)lpCriticalSection);
#else
	return FALSE;
#endif
}

void kEnterCriticalSection(void* lpCriticalSection)
{
	//CRITICAL_SECTION* cs = (CRITICAL_SECTION*)lpCriticalSection;
	//if (lpCriticalSection == 0 || cs->LockSemaphore == 0)
	//	return;

#if SKY_EMULATOR
	g_platformAPI._processInterface.sky_EnterCriticalSection((LPCRITICAL_SECTION)lpCriticalSection);
#else
	kLockMutex(cs->LockSemaphore);
#endif
}

void kLeaveCriticalSection(void* lpCriticalSection)
{
//	CRITICAL_SECTION* cs = (CRITICAL_SECTION*)lpCriticalSection;
	//if (lpCriticalSection == 0 || cs->LockSemaphore == 0)
		//return;

#if SKY_EMULATOR
	g_platformAPI._processInterface.sky_LeaveCriticalSection((LPCRITICAL_SECTION)lpCriticalSection);
#else
	kUnlockMutex(cs->LockSemaphore);
#endif
}

BOOL kIsGraphicMode()
{
	return SkyGUISystem::GetInstance()->GUIEnable();
}

BOOL kCreateHeap()
{
	return Thread::GetRunningThread()->GetTeam()->CreateHeap();
}

BOOL kIsEmulationMode()
{
#if SKY_EMULATOR
	return true;
#endif
	return false;
}

void kRaiseException(DWORD dwExceptionCode, DWORD dwExceptionFlags, DWORD nNumberOfArguments, CONST ULONG_PTR* lpArguments)
{
#if SKY_EMULATOR
	g_platformAPI._processInterface.sky_RaiseException(dwExceptionCode, dwExceptionFlags, nNumberOfArguments, lpArguments);
#else
#endif
}

HANDLE kCreateFileMapping(HANDLE hFile,DWORD fdwProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR pszName)
{
#if SKY_EMULATOR
	return g_platformAPI._processInterface.sky_CreateFileMapping(hFile, fdwProtect, dwMaximumSizeHigh, dwMaximumSizeLow, pszName);
#else
#endif
	return 0;
}

PVOID kMapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, DWORD dwNumberOfBytesToMap)
{
#if SKY_EMULATOR
	return g_platformAPI._processInterface.sky_MapViewOfFile(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
#else
#endif
	return 0;
}

std::map<DWORD, QWORD>* g_mapWindowId = 0;

BOOL kGetCurrentConsoleWindowId(QWORD* qwWindowID)
{
	if (!g_mapWindowId)
		return FALSE;

	std::map<DWORD, QWORD>::iterator iter = g_mapWindowId->find(kGetCurrentThreadId());

	if (iter != g_mapWindowId->end())
	{
		*qwWindowID = iter->second;
		return TRUE;
	}

	return FALSE;
}

inline BOOL CopyUser(void* dest, const void* src, int size)
{
	return Thread::GetRunningThread()->CopyUser(dest, src, size);
}

BOOL kRegisterWindowId(QWORD* qwWindowID)
{
	if (!g_mapWindowId)
		g_mapWindowId = new std::map<DWORD, QWORD>();
	(*g_mapWindowId)[kGetCurrentThreadId()] = *qwWindowID;
	return TRUE;
}

#include <getenv.h>
DWORD kGetEnvironmentVariable(LPCTSTR lpName, LPTSTR  lpBuffer, DWORD  nSize)
{
	char* value = getenv(lpName);

	if (value == 0 || strlen(value) > nSize)
		return 0;

	strcpy(lpBuffer, value);

	return strlen(value);
}

BOOL kSetEnvironmentVariable(LPCTSTR lpName, LPCTSTR lpValue)
{
	int result = setenv(lpName, lpValue, 1);

	return result == 0;
}


#if SKY_EMULATOR
extern CRITICAL_SECTION g_interrupt_cs;

extern "C" void kLock()
{
	kEnterCriticalSection(&g_interrupt_cs);
}

extern "C" void kUnlock()
{
	kLeaveCriticalSection(&g_interrupt_cs);
}
#endif

