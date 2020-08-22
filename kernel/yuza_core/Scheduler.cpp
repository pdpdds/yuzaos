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

const int kQuantum = 8000;

_Scheduler gScheduler;

_Scheduler::_Scheduler()
	:	fHighestReadyThread(0)
{
}

#if SKY_EMULATOR
bool _Scheduler::EnableReadyThread()
{
	fHighestReadyThread = 16;
	while (fHighestReadyThread >= 0)
	{
		while (fReadyQueue[fHighestReadyThread].GetHead() != 0)
		{
			Thread* nextThread = static_cast<Thread*>(fReadyQueue[fHighestReadyThread].Dequeue());
			DWORD result = g_platformAPI._processInterface.sky_kResumeThread(nextThread->m_handle);
			//result = result + 1;
		}

		fHighestReadyThread--;
	}

	if (fHighestReadyThread < 0)
		fHighestReadyThread = 0;

	return true;
}

void _Scheduler::Reschedule()
{
	EnableReadyThread();

	CancelTimeout();
	SetTimeout(SystemTime() + kQuantum, kOneShotTimer);
}
#else
void _Scheduler::Reschedule()
{
	int st = DisableInterrupts();
	Thread *thread = Thread::GetRunningThread();
	if (thread->GetState() == kThreadRunning) 
	{
		if (fHighestReadyThread <= thread->GetCurrentPriority() && thread->GetQuantumUsed() < kQuantum) 
		{
			// This thread hasn't used its entire timeslice, and there
			// isn't a higher priority thread ready to run.  Don't reschedule.
			// Instead, continue running this thread.  Try to let the thread
			// use its entire timeslice whenever possible for better performance.
			RestoreInterrupts(st);
			return;
		}

		EnqueueReadyThread(thread);
	}

	CancelTimeout();		
	SetTimeout(SystemTime() + kQuantum, kOneShotTimer);
	
	Thread* nextThread = PickNextThread();

	if(nextThread)
		nextThread->SwitchTo(thread);	

	RestoreInterrupts(st);
}
#endif

void _Scheduler::EnqueueSuspendThread(Thread *thread)
{
	fSuspendQueue[thread->GetCurrentPriority()].Enqueue(thread);
}

void _Scheduler::EnqueueReadyThread(Thread *thread)
{
	if (thread->GetState() == kThreadSuspended || thread->GetState() == kThreadDead)
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
			
	thread->SetState(kThreadReady);
	fReadyQueue[thread->GetCurrentPriority()].Enqueue(thread);
	
	RestoreInterrupts(st);
}

void _Scheduler::RepickHighestReadyThread(Thread* thread)
{	
	int priority = thread->GetCurrentPriority();
	Thread* node = (Thread*)fReadyQueue[priority].Remove(thread);
	ASSERT(node == thread);

	if (node == nullptr)
		return;

	
	//EnqueueSuspendThread(thread);
	//kprintf("Suspend Thread %x\n", node);	
}

Thread *_Scheduler::PickNextThread() 
{		
	Thread *nextThread = static_cast<Thread*>(fReadyQueue[fHighestReadyThread].Dequeue());
	while (fReadyQueue[fHighestReadyThread].GetHead() == 0)
		fHighestReadyThread--;

	//ASSERT(nextThread);			
	return nextThread;
}

InterruptStatus _Scheduler::HandleTimeout()
{
	return InterruptStatus::kReschedule;
}
