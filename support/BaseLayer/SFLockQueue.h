#pragma once
#include "SFLock.h"
#include <queue>

template <typename T>
class SFLockQueue
{
	typedef std::queue<T*> LockQueue;

public:
	SFLockQueue(void)
	{
		Initialize();
	}
	virtual ~SFLockQueue(void)
	{
		Finally();
	}

	bool Push(T* pMsg) 
	{
		SFLockHelper LockHelper(&m_Lock);
		m_Queue.push(pMsg);

		return true;
	}

	T* Pop(int wait)
	{
		SFLockHelper LockHelper(&m_Lock);
		if(!m_Queue.empty())
		{
			T* pMsg = m_Queue.front();
			m_Queue.pop();
			return pMsg;
		}

		return nullptr;
	}

	int Size(){return m_Queue.size();}

protected:
	virtual bool Initialize(){return true;}
	virtual bool Finally()
	{
		while (m_Queue.empty() == false)
		{
			T* pMsg = m_Queue.front();
			m_Queue.pop();
			delete pMsg;
		}

		return true;
	}


private:
	SFLock m_Lock;
	LockQueue m_Queue;
};
