#include "cpu_asm.h"
#include "Synchronization.h"
#include "Timer.h"
#include "Thread.h"
#include "Debugger.h"
#include "Scheduler.h"
#include <stringdef.h>
#include "intrinsic.h"
#include "PlatformAPI.h"
#include <BuildOption.h>
#include "InterruptDisabler.h"

class ThreadWaitEvent;

class WaitTag : public _QueueNode {
public:
	ThreadWaitEvent *fEvent;
	WaitTag *ftagNext;
	Synchronization *fSyncObject;
};

/// A ThreadWaitEvent is created when a thread has blocks waiting for
/// some state change, and records what the thread is waiting for.
class ThreadWaitEvent : public Timer {
public:
	ThreadWaitEvent(Thread *thread, WaitFlags flags);
	virtual InterruptStatus HandleTimeout();

	Thread *fThread;
	WaitTag *fTags;
	WaitFlags fFlags;
	int fWakeError;
};

ThreadWaitEvent::ThreadWaitEvent(Thread *thread, WaitFlags flags)
	: fThread(thread),
	fTags(0),
	fFlags(flags),
	fWakeError(E_NO_ERROR)
{
}

InterruptStatus ThreadWaitEvent::HandleTimeout()
{
	// Remove this event from the wait queues of all the dispatchers.
	for (WaitTag* threadBlock = fTags; threadBlock; threadBlock = threadBlock->ftagNext)
	{
		threadBlock->RemoveFromList();
	}

	fWakeError = E_TIMED_OUT;
	gScheduler.EnqueueReadyThread(fThread);

	return InterruptStatus::kReschedule;
}

Synchronization::Synchronization()
	: m_bSignalled(false)
{
}

Synchronization::~Synchronization()
{
}

int Synchronization::Wait(bigtime_t timeout)
{
	int fl = DisableInterrupts();

	int result = E_NO_ERROR;
	if (m_bSignalled)
	{
		ThreadWoken();

		RestoreInterrupts(fl);
	}
	else
	{
		WaitTag tag;
		Synchronization *list = this;

		RestoreInterrupts(fl);
		
		result = WaitInternal(1, &list, WAIT_FOR_ONE, timeout, &tag);
	}

	

	return result;
}

int Synchronization::WaitForMultipleSyncObject(int dispatcherCount, Synchronization *syncObjects[], WaitFlags flags, bigtime_t timeout)
{
	int result = E_NO_ERROR;

	int fl = DisableInterrupts();
	bool satisfied;
	if (flags & WAIT_FOR_ALL) 
	{
		satisfied = true;
		for (int dispatcherIndex = 0; dispatcherIndex < dispatcherCount; dispatcherIndex++)
			if (!syncObjects[dispatcherIndex]->m_bSignalled) 
			{
				satisfied = false;
				break;
			}
	}
	else 
	{
		satisfied = false;
		for (int dispatcherIndex = 0; dispatcherIndex < dispatcherCount; dispatcherIndex++) 
		{
			if (syncObjects[dispatcherIndex]->m_bSignalled) 
			{
				satisfied = true;
				break;
			}
		}
	}

	if (satisfied) 
	{
		for (int dispatcherIndex = 0; dispatcherIndex < dispatcherCount; dispatcherIndex++)
			syncObjects[dispatcherIndex]->ThreadWoken();

		RestoreInterrupts(fl);
		return E_NO_ERROR;
	}

	RestoreInterrupts(fl);

	const int kMaxStackAlloc = 5;
	if (dispatcherCount <= kMaxStackAlloc) 
	{
		WaitTag tags[kMaxStackAlloc];

		result = WaitInternal(dispatcherCount, syncObjects, flags, timeout, tags);
	}
	else 
	{ 
		WaitTag *tags = new WaitTag[dispatcherCount];
		result = WaitInternal(dispatcherCount, syncObjects, flags, timeout, tags);
		delete[] tags;
	}

	
	return result;
}

void Synchronization::Signal(bool reschedule)
{
	int fl = DisableInterrupts();
	ASSERT(!_get_interrupt_state());

	m_bSignalled = true;
	bool threadsWoken = false;
	for (WaitTag *nextWaitTag = static_cast<WaitTag*>(m_waitTagList.GetHead()); nextWaitTag && m_bSignalled;)
	{
		WaitTag *waitTag = nextWaitTag;
		nextWaitTag = static_cast<WaitTag*>(m_waitTagList.GetNext(nextWaitTag));

		// ������ ����ȭ ��ü�� �ñ׳��� ��ٸ��� ������
		// �����尡 ����ϴ� ����ȭ ��ü�� ���켭 ��� �ñ׳� ���¿� �ִ��� Ȯ��
		// ��� �ñ׳� ���¿� �ִٸ� wake = true
		bool wake = true;
		if (waitTag->fEvent->fFlags & WAIT_FOR_ALL)
		{
			for (WaitTag *threadBlock = waitTag->fEvent->fTags; threadBlock; threadBlock = threadBlock->ftagNext) 
			{
				if (!threadBlock->fSyncObject->m_bSignalled) 
				{
					wake = false;
					break;
				}
			}
		}

		if (wake) 
		{
			for (WaitTag *threadBlock = waitTag->fEvent->fTags; threadBlock; threadBlock = threadBlock->ftagNext)
			{
				threadBlock->RemoveFromList();

				if (waitTag->fEvent->fFlags & WAIT_FOR_ALL)
					threadBlock->fSyncObject->ThreadWoken();
			}

			if ((waitTag->fEvent->fFlags & WAIT_FOR_ALL) == 0)
				waitTag->fSyncObject->ThreadWoken();

			waitTag->fEvent->CancelTimeout();
			gScheduler.EnqueueReadyThread(waitTag->fEvent->fThread);
			threadsWoken = true;
		}
	}

	RestoreInterrupts(fl);

	if (reschedule && threadsWoken)
		gScheduler.Reschedule();
}

void Synchronization::Unsignal()
{
	m_bSignalled = false;
}

void Synchronization::ThreadWoken()
{
}

int Synchronization::WaitInternal(int syncObjectCount,
									   Synchronization *syncObjects[], 
									   WaitFlags flags,
	                                   bigtime_t timeout, 
	                                   WaitTag waitTags[])
{
	int fl = DisableInterrupts();

	int result = E_NO_ERROR;
	ThreadWaitEvent waitEvent(Thread::GetRunningThread(), flags);

	//ASSERT(waitEvent.fThread->m_resourceHandle != 0);

	for (int index = 0; index < syncObjectCount; index++)
	{
		waitTags[index].fEvent = &waitEvent;
		syncObjects[index]->m_waitTagList.Enqueue(&waitTags[index]);
		waitTags[index].ftagNext = waitEvent.fTags;
		waitTags[index].fSyncObject = syncObjects[index];
		waitEvent.fTags = &waitTags[index];
	}
	
	if (timeout != INFINITE_TIMEOUT)
		waitEvent.SetTimeout(SystemTime() + timeout, kOneShotTimer);

	if(waitEvent.fThread)
		waitEvent.fThread->SetState(kThreadWaiting);

	RestoreInterrupts(fl);
	
	gScheduler.Reschedule();

	return waitEvent.fWakeError;	
	
}
