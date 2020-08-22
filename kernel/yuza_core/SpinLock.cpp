#include "SpinLock.h"
#include "cpu_asm.h"
#include "Thread.h"
#include "intrinsic.h"

#define LOCK_FALSE 0
#define LOCK_TRUE 1

//멀티코어로 확장할때 이동시킬 것
DWORD GetAPICID()
{
	return 0;
}

SpinLock::SpinLock() : Resource(OBJ_SPINLOCK, "")
{
	fHolder = nullptr;
	teamId = 0;

	m_dwLockCount = 0;
	m_APICID = 0xff;

	m_lockFlag = LOCK_FALSE;
}

SpinLock::~SpinLock()
{
}

int SpinLock::Lock()
{
	int fl = DisableInterrupts();
	
	//임계영역 진입
	if (cmpxchg32(&(m_lockFlag), 0, 1) == false)
	{
		
		//
		if (m_APICID == GetAPICID() && fHolder == Thread::GetRunningThread())
		{
			RestoreInterrupts(fl);
			m_dwLockCount++;
			return E_NO_ERROR;
		}

		//
		while (cmpxchg32(&(m_lockFlag), 0, 1) == false)
		{			
			while (m_lockFlag == LOCK_TRUE)
			{
				__asm
				{
					pause
				}
			}
		}
	}
	
	//임계영역 진입. 현재 스레드가 이후 Unlock을 하지 않는한 
	//다른 스레드는 스핀락을 소유할 수 없으며 위에서 루프를 돌며 대기한다.	
	m_dwLockCount = 1;
	m_APICID = GetAPICID();
	fHolder = Thread::GetRunningThread();

	RestoreInterrupts(fl);

	return E_NO_ERROR;
}

void SpinLock::Unlock()
{
	int fl = DisableInterrupts();

	//스핀락을 잠근 태스크가 아니면 실패
	if ((m_lockFlag == false) || (m_APICID != GetAPICID()) || fHolder != Thread::GetRunningThread())
	{
		RestoreInterrupts(fl);
		return;
	}
	
	if (m_dwLockCount > 1)
	{
		m_dwLockCount--;
		RestoreInterrupts(fl);
		return;
	}

	m_APICID = 0xFF;
	m_dwLockCount = 0;
	m_lockFlag = LOCK_FALSE;
	fHolder = 0;

	RestoreInterrupts(fl);
}