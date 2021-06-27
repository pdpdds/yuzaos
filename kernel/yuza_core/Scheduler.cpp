// 
// Copyright 1998-2012 Jeff Bush
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

#include "cpu_asm.h"
#include "Scheduler.h"
#include "Semaphore.h"
#include <stringdef.h>
#include "Thread.h"
#include "Debugger.h"
#include "math.h"
#include <stringdef.h>
#include "intrinsic.h"
#include "systemcall.h"
#include "PlatformAPI.h"

const int kQuantum = 8;

_Scheduler gScheduler;

_Scheduler::_Scheduler()
: fHighestReadyThread(0)
, fReadyThreadCount(0)
{
}

void _Scheduler::Reschedule()
{
	int st = DisableInterrupts();
	Thread* pThread = Thread::GetRunningThread();
	

#if SKY_EMULATOR
	if (pThread == 0)
	{
		CancelTimeout();
		//if (fReadyThreadCount > 1)
		SetTimeout(SystemTime() + kQuantum, kOneShotTimer);

		//스레드 선택과 동시에 스레드는 큐에서 제거됨
		Thread* nextThread = PickNextThread();

		if (nextThread)
		{

			bigtime_t now = SystemTime(); 
			nextThread->fLastEvent = now;
			nextThread->SetState(kThreadRunning);
			DWORD result = g_platformAPI._processInterface.sky_kResumeThread(nextThread->m_win32Handle);
			ASSERT(result != -1);
		}
		
		RestoreInterrupts(st);
		return;
	}
#endif

	int enqueue = 0;
	if (pThread->GetState() == kThreadRunning)
	{
		if (fHighestReadyThread <= pThread->GetCurrentPriority() && pThread->GetQuantumUsed() < kQuantum)
		{
			// This thread hasn't used its entire timeslice, and there
			// isn't a higher priority thread ready to run.  Don't reschedule.
			// Instead, continue running this thread.  Try to let the thread
			// use its entire timeslice whenever possible for better performance.
			RestoreInterrupts(st);
			return;
		}

		EnqueueReadyThread(pThread);
		enqueue = 1;
	}

	CancelTimeout();		

	//if (fReadyThreadCount > 1)
		SetTimeout(SystemTime() + kQuantum, kOneShotTimer);
	
	//다음 스레드 선택과 동시에 해당 스레드는 큐에서 제거됨
	Thread* nextThread = PickNextThread();

#if SKY_EMULATOR
	if (pThread != nextThread)
	{
		if (nextThread)
		{
			bigtime_t now = SystemTime();
			pThread->fLastEvent = now;
			nextThread->fLastEvent = now;
			nextThread->SetState(kThreadRunning);
			DWORD result = g_platformAPI._processInterface.sky_kResumeThread(nextThread->m_win32Handle);
			ASSERT(result != -1);
		}
			
		if (pThread->GetState() == kThreadRunning)
		{
			if(enqueue == 0)
			EnqueueReadyThread(pThread);
			//ASSERT(enqueue == 1);
		}

		RestoreInterrupts(st);
			
		g_platformAPI._processInterface.sky_kSuspendThread(pThread->m_win32Handle);

		return;
	}

#else
	nextThread->SwitchTo(pThread);
#endif

	RestoreInterrupts(st);
} 

void _Scheduler::EnqueueReadyThread(Thread *thread)
{
	if (thread->GetState() == kThreadDead)
		return;

	int st = DisableInterrupts();
	if (thread->GetSleepTime() > kQuantum * 4) {
		// Boost this thread's priority if it has been blocked for a while
		if (thread->GetCurrentPriority() < MIN(kMaxPriority, thread->GetBasePriority() + 3))
			thread->SetCurrentPriority(thread->GetCurrentPriority() + 1);
	} else {
		// This thread has run for a while.  If it's priority was boosted,
		// lower it back down.
		if (thread->GetCurrentPriority() > thread->GetBasePriority())
			thread->SetCurrentPriority(thread->GetCurrentPriority() - 1);
	}
	
	if (thread->GetCurrentPriority() > fHighestReadyThread)
		fHighestReadyThread = thread->GetCurrentPriority();
			
	fReadyThreadCount++;

	fReadyQueue[thread->GetCurrentPriority()].Enqueue(thread);
	
	RestoreInterrupts(st);
}

Thread *_Scheduler::PickNextThread() 
{		
	if (fHighestReadyThread < 0)
	{
		return 0;
	}

	ASSERT(fHighestReadyThread >= 0 && fReadyQueue[fHighestReadyThread].GetHead() != 0);

	Thread *nextThread = static_cast<Thread*>(fReadyQueue[fHighestReadyThread].Dequeue());
	fReadyThreadCount--;

	while (fHighestReadyThread >= 0 && fReadyQueue[fHighestReadyThread].GetHead() == 0)
		fHighestReadyThread--;

	if (fHighestReadyThread < 0)
	{
		return nextThread;
	} 
	 
	ASSERT(nextThread);
	return nextThread;
}

InterruptStatus _Scheduler::HandleTimeout()
{
	return InterruptStatus::kReschedule;
}
