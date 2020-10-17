#include "cpu_asm.h"
#include "interrupt.h"
#include "Debugger.h"
#include <stringdef.h>
#include "Timer.h"
#include "Scheduler.h"
#include "intrinsic.h"

extern _Scheduler gScheduler;

extern void HardwareTimerBootstrap(InterruptStatus (*callback)());
extern void SetHardwareTimer(bigtime_t relativeTimeout);

_Queue Timer::fTimerQueue;

Timer::Timer()
	:	fPending(false)
{
}

Timer::~Timer()
{
	if (fPending)
		kPanic("Attempt to delete a pending timer2\n");
}

void Timer::SetTimeout(bigtime_t time, TimerMode mode)
{
	int st = DisableInterrupts();

	ASSERT(mode == kPeriodicTimer || mode == kOneShotTimer);
	if (fPending)
		kPanic("Attempt to Set() a pending timer1\n");
	
	fMode = mode;
	fPending = true;
	if (fMode == kPeriodicTimer) 
	{
		fInterval = time;
		if (fInterval <= 0)
			kPanic("Attempt to set periodic timer for zero or negative interval");
	
		fWhen = SystemTime() + fInterval;
	} else
		fWhen = time;

	const Timer *oldHead = static_cast<const Timer*>(fTimerQueue.GetHead());
	Enqueue(this);

	// If a different timer is now the head of the timer queue,
	// reset the hardware timer.
	if (static_cast<Timer*>(fTimerQueue.GetHead()) != oldHead)
		ReprogramHardwareTimer();
		
	RestoreInterrupts(st);
}

bool Timer::CancelTimeout()
{
	int st = DisableInterrupts();
	bool wasHead = (fTimerQueue.GetHead() == this);
	if (fPending)
		fTimerQueue.Remove(this);
	
	bool wasPending = fPending;
	fPending = false;
	if (wasHead)
		ReprogramHardwareTimer();
		
	RestoreInterrupts(st);
	return wasPending;
}

InterruptStatus Timer::HandleTimeout()
{
	return InterruptStatus::kUnhandledInterrupt;
}

void Timer::Bootstrap()
{
	Debugger::GetInstance()->AddCommand("timers", "list pending timers", PrintTimerQueue);

	HardwareTimerBootstrap(HardwareTimerInterrupt);	
}
UINT32 lastTickCount = 0;
unsigned int g_tickCount = 0;

InterruptStatus Timer::HardwareTimerInterrupt()
{		
	g_tickCount++;

	bigtime_t now = SystemTime();
	
	bool reschedule = false;
	Timer* expiredTimer = static_cast<Timer*>(fTimerQueue.GetHead());
	while (expiredTimer && now >= expiredTimer->fWhen)
	{
		expiredTimer = static_cast<Timer*>(fTimerQueue.Dequeue());

		if (expiredTimer->fMode == kPeriodicTimer) 
		{
			expiredTimer->fWhen += expiredTimer->fInterval;
			Enqueue(expiredTimer);
		} 
		else
		{
			expiredTimer->fPending = false;
		}

		InterruptStatus ret = expiredTimer->HandleTimeout();
		if (ret == InterruptStatus::kReschedule)
			reschedule = true;

		expiredTimer = static_cast<Timer*>(fTimerQueue.GetHead());
	}

	UINT32 currentTickCount = SystemTime();

	if (currentTickCount - lastTickCount >= 1000)
	{

		//kprintf("Timer::HardwareTimerInterrupt %x\n", g_tickCount);
		lastTickCount = currentTickCount;

		bigtime_t now = SystemTime();
		//kprintf("%d : %Ld\n", g_tickCount, now);

	}

	ReprogramHardwareTimer();

	if(reschedule)
		gScheduler.Reschedule();

	return reschedule ? InterruptStatus::kReschedule : InterruptStatus::kHandledInterrupt;
}

InterruptStatus Timer::HardwareTimerInterrupt2()
{
	g_tickCount++;

	bigtime_t now = SystemTime();

	bool reschedule = false;
	Timer* expiredTimer = static_cast<Timer*>(fTimerQueue.GetHead());
	while (expiredTimer && now >= expiredTimer->fWhen)
	{
		expiredTimer = static_cast<Timer*>(fTimerQueue.Dequeue());

		if (expiredTimer->fMode == kPeriodicTimer)
		{
			expiredTimer->fWhen += expiredTimer->fInterval;
			Enqueue(expiredTimer);
		}
		else
		{
			expiredTimer->fPending = false;
		}

		InterruptStatus ret = expiredTimer->HandleTimeout();
		if (ret == InterruptStatus::kReschedule)
			reschedule = true;

		expiredTimer = static_cast<Timer*>(fTimerQueue.GetHead());
	}

	UINT32 currentTickCount = SystemTime();

	if (currentTickCount - lastTickCount >= 1000)
	{

		//kprintf("Timer::HardwareTimerInterrupt %x\n", g_tickCount);
		lastTickCount = currentTickCount;

		bigtime_t now = SystemTime();
		//kprintf("%d : %Ld\n", g_tickCount, now);

	}

	ReprogramHardwareTimer();

	return reschedule ? InterruptStatus::kReschedule : InterruptStatus::kHandledInterrupt;
}

void KernelSoftwareInterrupt()
{
	int st = DisableInterrupts();
	InterruptStatus result = Timer::HardwareTimerInterrupt2();
	RestoreInterrupts(st); 

	if(result == InterruptStatus::kReschedule)
		gScheduler.Reschedule();
}

void Timer::Enqueue(Timer *newTimer)
{
	int st = DisableInterrupts();

	if (fTimerQueue.GetTail() == 0)
		fTimerQueue.Enqueue(newTimer); // Empty list.  Add this as the only element.
	else if (static_cast<Timer*>(fTimerQueue.GetHead())->fWhen >= newTimer->fWhen)
		fTimerQueue.AddBefore(fTimerQueue.GetHead(), newTimer); // First Element.
	else {
		// Search newTimer list from end to beginning and insert this in order.
		for (Timer *timer = static_cast<Timer*>(fTimerQueue.GetTail()); timer;
			timer = static_cast<Timer*>(fTimerQueue.GetPrevious(timer))) 
		{
			if (timer->fWhen < newTimer->fWhen) 
			{
				fTimerQueue.AddAfter(timer, newTimer);
				break;
			}
		}
	}

	RestoreInterrupts(st);
}

void Timer::ReprogramHardwareTimer()
{
	//20190303 Critical
	//Timer *head = static_cast<Timer*>(fTimerQueue.GetHead());
	//if (head)
		//SetHardwareTimer(head->fWhen - SystemTime());
}

void Timer::PrintTimerQueue(int, const char**)
{
	int st = DisableInterrupts();
	bigtime_t now = SystemTime();
	kprintf("\nCurrent time %Ld\n", now);
	kprintf("  %8s %16s %16s\n", "Timer", "Wake time", "relative time");
	for (const Timer *timer = static_cast<const Timer*>(fTimerQueue.GetHead()); timer;
		timer = static_cast<const Timer*>(fTimerQueue.GetNext(timer))) {
		kprintf("  %p %16Li %16Li\n", timer, timer->fWhen, timer->fWhen - now);
	}

	RestoreInterrupts(st);
}
