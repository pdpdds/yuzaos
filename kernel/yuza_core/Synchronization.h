#pragma once
#include "windef.h"
#include <ktypes.h>
#include <ds/queue.h>

class Synchronization
{
public:
	Synchronization();
	~Synchronization();

	int Wait(bigtime_t timeout = INFINITE_TIMEOUT);
	static int WaitForMultipleSyncObject(int dispatcherCount, Synchronization *syncObjects[],
		WaitFlags flags, bigtime_t timeout = INFINITE_TIMEOUT);

protected:	
	void Signal(bool reschedule);
	void Unsignal();
	virtual void ThreadWoken();

private:
	static int WaitInternal(int dispatcherCount, Synchronization *syncObjects[],
		WaitFlags flags, bigtime_t timeout, class WaitTag[]);
	bool m_bSignalled;
	_Queue m_waitTagList;
};

