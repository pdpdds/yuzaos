#pragma once
#include "Synchronization.h"
#include "Semaphore.h"
#include <BuildOption.h>

class Thread;

class Mutex : public Synchronization {
public:
	Mutex(int = 0);
	~Mutex();
	int Lock();
	void Unlock();
private:
	virtual void ThreadWoken();

#if SKY_EMULATOR
	CRITICAL_SECTION m_cs;
#endif
};

/// Reader/Writer lock
class RWLock {
public:
	RWLock();
	int LockRead();
	void UnlockRead();
	int LockWrite();
	void UnlockWrite();

private:
	volatile int fCount;
	Semaphore fWriteSem;
	Semaphore fReadSem;
	Mutex fWriteLock;
};


class RecursiveLock : public Resource
{
public:
	RecursiveLock(const char name[]);
	~RecursiveLock();
	int Lock();
	void Unlock();
	bool IsLocked() const;
private:
	Mutex fMutex;

	Thread *fHolder;
	int fRecursion;


};
