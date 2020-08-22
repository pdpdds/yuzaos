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

/// @file Scheduler.h
#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "Timer.h"
#include "ktypes.h"

const int kMaxPriority = 31;

class Thread;

/// The scheduler chooses which thread should run next
class _Scheduler : public Timer {
public:
	_Scheduler();

	/// Pick the next thread that should be run and call SwitchTo on it.

	void Reschedule();

#if SKY_EMULATOR	
	bool EnableReadyThread();	
#endif

	void RepickHighestReadyThread(Thread* thread);

	/// Mark a thread ready to run and put it on the ready queue
	void EnqueueReadyThread(Thread*);
	void EnqueueSuspendThread(Thread *thread);

private:
	Thread *PickNextThread();
	InterruptStatus HandleTimeout();
	
	int fHighestReadyThread;
	_Queue fReadyQueue[kMaxPriority + 1];
	_Queue fSuspendQueue[kMaxPriority + 1];
};

extern _Scheduler gScheduler;

#endif
