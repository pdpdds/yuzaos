#include "TaskTimer.h"
#include <stdio.h>
#include <SystemAPI.h>
#include <Semaphore.h>
#include <Team.h>
#include <SystemAPI.h>
#include <Thread.h>

TaskTimer::TaskTimer(TimerObject* pOwner)
	: m_pOwner(pOwner)
{
}


TaskTimer::~TaskTimer()
{
}

InterruptStatus TaskTimer::HandleTimeout()
{
	if (m_pOwner && m_pOwner->lpfnTimer)
	{	
		m_pOwner->HandleTimer();
		return InterruptStatus::kHandledInterrupt;
	}
	return InterruptStatus::kHandledInterrupt;
}

////////////////////////////////////////////
////////////////////////////////////////////
TimerObject::TimerObject(UINT nIDEvent)
	: Resource(OBJ_TIMER, "timer")
	, m_timer(this)
	, eventHandle(0)
	, team(nullptr)
	, idEvent(nIDEvent)
	, m_disabled(false)
	, lpfnTimer(nullptr)
{
	eventHandle = kCreateEvent(0, false, false, "Wait");
	team = Thread::GetRunningThread()->GetTeam();
}


TimerObject::~TimerObject()
{
	if (eventHandle)
		kCloseHandle(eventHandle);
}

bool TimerObject::Stop()
{
	Semaphore* sem = static_cast<Semaphore*>(team->GetHandleTable()->GetResource((int)eventHandle, OBJ_SEMAPHORE));
	m_timer.CancelTimeout();
	m_disabled = true;
	sem->Release(1);
	sem->ReleaseRef();
	return true;
}

void TimerObject::HandleTimer()
{
	Semaphore* sem = static_cast<Semaphore*>(team->GetHandleTable()->GetResource((int)eventHandle, OBJ_SEMAPHORE));
	if (sem == 0)
	{
		kPanic("sem is null %x\n", (int)eventHandle);
	}
	sem->Release(1);
	sem->ReleaseRef();
}

bool TimerObject::Start(UINT nElapse, void (CALLBACK* lpTimer)(HWND, UINT, DWORD*, DWORD))
{
	this->nElapse = nElapse;
	lpfnTimer = lpTimer;
	m_timer.SetTimeout(nElapse, kPeriodicTimer);
	return true;
}
