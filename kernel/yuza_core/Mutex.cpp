#include "Mutex.h"
#include "Thread.h"
#include "cpu_asm.h"
#include <SystemAPI.h>

const int kMaxReaders = 0x50000000;

Mutex::Mutex(int)
{
#if SKY_EMULATOR
	kInitializeCriticalSection(&m_cs);
	return;
#endif

	Signal(false);
}

Mutex::~Mutex()
{
#if SKY_EMULATOR
	kDeleteCriticalSection(&m_cs);
	return;
#endif
}

int Mutex::Lock()
{
#if SKY_EMULATOR
	kEnterCriticalSection(&m_cs);
	return E_NO_ERROR;
#endif

	return Wait();
}

void Mutex::Unlock()
{
#if SKY_EMULATOR
	kLeaveCriticalSection(&m_cs);
	return;
#endif

	Signal(false);
}

void Mutex::ThreadWoken()
{
#if SKY_EMULATOR
	return;
#endif

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
	return 0;
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
#if SKY_EMULATOR
	kInitializeCriticalSection(&m_cs);
#endif
}

RecursiveLock::~RecursiveLock()
{
#if SKY_EMULATOR
	kDeleteCriticalSection(&m_cs);
#endif
}

int RecursiveLock::Lock()
{
#if SKY_EMULATOR
	kEnterCriticalSection(&m_cs);
	fHolder = Thread::GetRunningThread();
	return E_NO_ERROR;
#else
	if (fHolder != Thread::GetRunningThread())
	{
		while (fMutex.Lock() == E_INTERRUPTED)
			;
		fHolder = Thread::GetRunningThread();
	}

	fRecursion++;
	return E_NO_ERROR;
#endif

}

void RecursiveLock::Unlock()
{
#if SKY_EMULATOR
	fHolder = 0;
	kLeaveCriticalSection(&m_cs);
#else
	ASSERT(fHolder == Thread::GetRunningThread());
	if (--fRecursion == 0) {
		fHolder = 0;
		fMutex.Unlock();
	}
#endif	
}

bool RecursiveLock::IsLocked() const
{
	return fHolder == Thread::GetRunningThread();
}


