#include "AddressSpace.h"
#include "Area.h"
#include "cpu_asm.h"
#include "Debugger.h"
#include "PageCache.h"
#include "Scheduler.h"
#include <stringdef.h>
#include "Team.h"
#include "Thread.h"
#include "intrinsic.h"
#include "PlatformAPI.h"
#include <memory_layout.h>

Thread* Thread::fRunningThread = 0;
std::map<DWORD, Thread*>* Thread::fMapThread = 0;

_Queue Thread::fReapQueue;
Semaphore Thread::fThreadsToReap("threads to reap", 0);

Thread::Thread(const char name[], Team *team, THREAD_START_ENTRY startAddress, ThreadParam* param, int priority, DWORD flag)
	:	Resource(OBJ_THREAD, name),
		fThreadContext(team->GetAddressSpace()->GetPhysicalMap()),
		fBasePriority(priority),
		fCurrentPriority(priority),
		fFaultHandler(0),
		fLastEvent(SystemTime()),
		fState(kThreadCreated),
		fTeam(team),
		fKernelStack(0),
		fUserStack(0),
		fSuspendCount(0),
		m_handle(0)
{
	char stackName[OS_NAME_LENGTH];
	snprintf(stackName, OS_NAME_LENGTH, "%.12s stack", name);
	//kDebugPrint("thread name : %s\n", name);

	fKernelStack = AddressSpace::GetKernelAddressSpace()->CreateArea(stackName,
		kKernelStackSize, AREA_WIRED, SYSTEM_READ | SYSTEM_WRITE, new PageCache, 0,
		INVALID_PAGE, SEARCH_FROM_TOP);

	if (fKernelStack == 0) 
	{
		printf("team = %p\n", fTeam);
		kPanic("Can't create kernel stack for thread: out of virtual space\n");
	}
	
	unsigned int kernelStack = fKernelStack->GetBaseAddress() + kKernelStackSize - 4;
	unsigned int userStack = 0;
	if (fTeam->GetAddressSpace() != AddressSpace::GetKernelAddressSpace()) 
	{
		// Create the user stack
		fUserStack = fTeam->GetAddressSpace()->CreateArea(stackName, kUserStackSize,
			AREA_WIRED, USER_READ | USER_WRITE | SYSTEM_READ | SYSTEM_WRITE,
			new PageCache, 0, INVALID_PAGE, SEARCH_FROM_TOP);
		
		if (fUserStack == 0) 
		{
			kprintf("team = %p\n", fTeam);
			kPanic("Can't create user stack for thread: out of virtual space\n");
		}

		userStack = fUserStack->GetBaseAddress() + kUserStackSize - 4;

	}
	
	fThreadContext.SetupThread(startAddress, param, userStack, kernelStack);

	// Inherit the current directory from the thread that created this.
	//fCurrentDir = GetRunningThread()->fCurrentDir;
	//if (fCurrentDir)
	//	fCurrentDir->AcquireRef();

	AcquireRef();	// This reference is effectively owned by the actual thread
					// of execution.  It will be released by the Grim Reaper
	team->ThreadCreated(this);

	if (flag & CREATE_SUSPENDED)
		SetState(kThreadSuspended);

#if SKY_EMULATOR
	int thread = g_platformAPI._processInterface.sky_kCreateThread(Thread::GetRunningThread()->GetTeam()->GetTaskId(), (LPTHREAD_START_ROUTINE)startAddress, param, flag);
	(*Thread::fMapThread)[thread] = this;
	
#else
	gScheduler.EnqueueReadyThread(this);
	gScheduler.Reschedule();
#endif	
}

void Thread::Exit()
{
	//kDebugPrint("ExitThread %s 0x%x 0x%x\n", GetName(), this, fRefCount);

	ASSERT(GetRunningThread() == this);

#if SKY_EMULATOR
	return;
#endif

	int fl = DisableInterrupts();
	SetState(kThreadDead);
	fReapQueue.Enqueue(this);
	fThreadsToReap.Release(1, false);
	RestoreInterrupts(fl);

	gScheduler.Reschedule();

#if SKY_EMULATOR
#else
	kPanic("terminated thread got scheduled");
#endif
}

int Thread::ForceExit()
{
	ASSERT(GetRunningThread() != this);

	int fl = DisableInterrupts();
	SetState(kThreadDead);
	fReapQueue.Enqueue(this);
	fThreadsToReap.Release(1, false);	
	RestoreInterrupts(fl);
	
	gScheduler.Reschedule();

	return 0;
}

void Thread::SwitchTo(Thread* prevThread)
{
	int cs = DisableInterrupts();

	fState = kThreadRunning;
	if (fRunningThread != this) 
	{				
		bigtime_t now = SystemTime();
		fRunningThread->fLastEvent = now;
		fLastEvent = now;
		fRunningThread = this;

#if SKY_EMULATOR
		DWORD result = g_platformAPI._processInterface.sky_kResumeThread(nextThread->m_handle);
		_platformAPI._processInterface.sky_kSuspendThread(prevThread->m_handle)l
#else
		fThreadContext.SwitchTo();
#endif
	}

	RestoreInterrupts(cs);
}

bool Thread::CopyUser(void *dest, const void *src, int size)
{
	ASSERT(GetRunningThread() == this);

	return CopyUserInternal(dest, src, size, &fFaultHandler);
}

bigtime_t Thread::GetQuantumUsed() const
{
	if (fState == kThreadRunning)
		return SystemTime() - fLastEvent;

	return 0;
}

bigtime_t Thread::GetSleepTime() const
{
	if (fState == kThreadWaiting)
		return SystemTime() - fLastEvent;

	return 0;
}

APC* Thread::DequeueAPC()
{
	int fl = DisableInterrupts();
	APC *apc = static_cast<APC*>(fApcQueue.Dequeue());
	RestoreInterrupts(fl);

	return apc;
}

void Thread::EnqueueAPC(APC *apc)
{
	int fl = DisableInterrupts();
	fApcQueue.Enqueue(apc);
#if 0
	if (GetState() == kThreadWaiting)
		Wake(E_INTERRUPTED);
#endif

	RestoreInterrupts(fl);
}

void Thread::Bootstrap()
{
#if SKY_EMULATOR
	fMapThread = new std::map<DWORD, Thread*>();
#endif
	fRunningThread = new Thread("Init Thread");

#if SKY_EMULATOR
	fRunningThread->m_handle = (HANDLE)kGetCurrentThreadObject();
	g_platformAPI._processInterface.sky_TlsSetValue(100, fRunningThread->m_handle);
	(*Thread::fMapThread)[(DWORD)fRunningThread->m_handle] = fRunningThread;
#endif
	Debugger::GetInstance()->AddCommand("st", "Stack trace of current thread", StackTrace);
}

void Thread::SetKernelStack(Area *area)
{
	fKernelStack = area;
}
HANDLE kCreateThreadWithTeam(THREAD_START_ENTRY entry, const char* name, void* data, int priority, Team* team, DWORD flag);
void Thread::SetTeam(Team *team)
{
	fTeam = team;
	team->ThreadCreated(this);

	// Threading is ready to go... start the Grim Reaper.
	// This is kind of a weird side effect, the function should be more explicit.

	new Thread("Grim Reaper", team, GrimReaper, 0, 30);
}

// This is the constructor for bootstrap thread.  There is less state to
// setup since it is already running.
Thread::Thread(const char name[])
	:	Resource(OBJ_THREAD, name),
		fBasePriority(16),
		fCurrentPriority(16),
		fFaultHandler(0),
		fLastEvent(SystemTime()),
//		fCurrentDir(0),
		fState(kThreadRunning),
		fKernelStack(0),
		fUserStack(0),
		fSuspendCount(0),
		fTeam(0),
		m_handle(0)
{
	
}

Thread::~Thread()
{	
	if (fKernelStack)	
		AddressSpace::GetKernelAddressSpace()->DeleteArea(fKernelStack);
			
	if (fUserStack)
		fTeam->GetAddressSpace()->DeleteArea(fUserStack);

	kDebugPrint("Thread Terminated. thread : 0x%x, name : %s\n", this, GetName());
	fTeam->ThreadTerminated(this);
}

void Thread::StackTrace(int, const char**)
{
	GetRunningThread()->fThreadContext.PrintStackTrace();
}

// The Grim Reaper thread reclaims resources for threads and teams that
// have exited.
int Thread::GrimReaper(void*)
{
	for (;;) {
		
		fThreadsToReap.Wait();
		
		int fl = DisableInterrupts();
		Thread *victim = static_cast<Thread*>(fReapQueue.Dequeue());
		RestoreInterrupts(fl);

		// The thread may not actually get deleted here if someone else has
		// a handle to it.

		if (victim)
		{
			victim->Signal(false);

			//kDebugPrint("Grim Reaper. Victim : %x %s\n", victim, victim->GetName());
			victim->ReleaseRef();
		} 
		
	}
}

Thread* Thread::GetRunningThread()
{
#if SKY_EMULATOR

	DWORD thread = (DWORD)g_platformAPI._processInterface.sky_TlsGetValue(100);
	//DWORD threadId = (DWORD)g_platformAPI._processInterface.sky_kGetCurrentThread();
	Thread* pObject = (*fMapThread)[thread];

	if(pObject == 0)
		return (*fMapThread)[0];

	return pObject;

#endif // SKY_EMULATOR

	return fRunningThread;
}