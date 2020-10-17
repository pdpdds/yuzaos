#include "Mutex.h"
#include "Thread.h"
#include "cpu_asm.h"
#include <SystemAPI.h>

const int kMaxReaders = 0x50000000;

Mutex::Mutex(int)
{
	Signal(false);
}

Mutex::~Mutex()
{
}

int Mutex::Lock()
{
	return Wait();
}

void Mutex::Unlock()
{
	Signal(false);
}

void Mutex::ThreadWoken()
{
	Unsignal();
}

RWLock::RWLock()
	: fCount(0),
	fWriteSem("rwlock:writer_wait", 0),
	fReadSem("rwlock:reader_wait", 0)
{
}

int RWLock::LockRead()
{
	int error = E_NO_ERROR;
	if (AtomicAdd(&fCount, 1) < 0)
		error = fReadSem.Wait();

	return error;
}

void RWLock::UnlockRead()
{
	if (AtomicAdd(&fCount, -1) < 0)
		fWriteSem.Release();
}

int RWLock::LockWrite()
{
	int error = fWriteLock.Lock();
	if (error != E_NO_ERROR)
		return error;

	int readerCount = AtomicAdd(&fCount, -kMaxReaders);
	while (readerCount-- > 0) {
		error = fWriteSem.Wait();
		if (error != E_NO_ERROR)
			break;
	}

	return error;
}

void RWLock::UnlockWrite()
{
	int waitingReaders = AtomicAdd(&fCount, kMaxReaders) + kMaxReaders;
	if (waitingReaders > 0)
		fReadSem.Release(waitingReaders);

	fWriteLock.Unlock();
}

RecursiveLock::RecursiveLock(const char name[])
	: Resource(OBJ_MUTEX, name),
	fHolder(0),
	fRecursion(0)
{
}

RecursiveLock::~RecursiveLock()
{
}

int RecursiveLock::Lock()
{
	if (fHolder != Thread::GetRunningThread())
	{
		while (fMutex.Lock() == E_INTERRUPTED)
			;
		fHolder = Thread::GetRunningThread();
	}

	fRecursion++;
	return E_NO_ERROR;

}

void RecursiveLock::Unlock()
{
	ASSERT(fHolder == Thread::GetRunningThread());
	if (--fRecursion == 0) {
		fHolder = 0;
		fMutex.Unlock();
	}
}

bool RecursiveLock::IsLocked() const
{
	return fHolder == Thread::GetRunningThread();
}


