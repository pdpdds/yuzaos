#pragma once
#ifdef _WIN32
#include <Windows.h>
#else
#include <mutex>
#endif

class SFLock
{
public:
	SFLock(void)
	{
#ifdef _WIN32
		InitializeCriticalSection(&m_CS);
#endif
	}

	virtual ~SFLock(void)
	{
#ifdef _WIN32
		DeleteCriticalSection(&m_CS);
#endif
	}

	void Lock() {
#ifdef _WIN32
		EnterCriticalSection(&m_CS);
#else
		mtx_lock.lock();
#endif
	}
	void Unlock() 
	{
#ifdef _WIN32
		LeaveCriticalSection(&m_CS);
#else
		mtx_lock.unlock();
#endif
		
	}

protected:

private:
#ifdef _WIN32
	CRITICAL_SECTION m_CS;
#else	
	std::mutex mtx_lock;
#endif
};

class SFLockHelper
{
public:
	SFLockHelper(SFLock* pCS)
	{
		pCS->Lock();
		m_pCS = pCS;
	}

	virtual ~SFLockHelper(void)
	{
		m_pCS->Unlock();
	}

protected:

private:
	SFLock* m_pCS;
};