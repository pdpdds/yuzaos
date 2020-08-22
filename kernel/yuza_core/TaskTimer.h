#pragma once
#include <Timer.h>
#include <Resource.h>

class  TimerObject;
class Team;

class TaskTimer : public Timer
{
public:
	TaskTimer(TimerObject* pOwner);
	virtual ~TaskTimer();

	virtual InterruptStatus HandleTimeout() override;

private:
	TimerObject* m_pOwner;
};

class TimerObject : public Resource
{
public:
	TimerObject(UINT nIDEvent);
	virtual ~TimerObject();

	bool Start(UINT nElapse, void (CALLBACK* lpTimer)(HWND, UINT, DWORD*, DWORD));
	bool Stop();
	void HandleTimer();

	void (CALLBACK* lpfnTimer)(HWND, UINT, DWORD*, DWORD);
	HANDLE eventHandle;
	Team* team;
	UINT idEvent;
	bool m_disabled;
private:
	TaskTimer m_timer;
	UINT nElapse;
	
};