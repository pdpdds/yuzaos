#pragma once
#include "Synchronization.h"
#include "Resource.h"

class Thread;

typedef struct TAG_SPINLOCK
{
	int teamId;
	int holder;
	int handleId;

}_SPINLOCK;

class SpinLock : public Resource
{
public:
	SpinLock();
	~SpinLock();	

	int Lock();
	void Unlock();

	Thread *fHolder;
	int teamId;

private:
	
	volatile int m_APICID;
	volatile DWORD m_dwLockCount;	
	volatile int m_lockFlag;		
};