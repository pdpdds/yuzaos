/** 
*  @file		Sync.h
*  @brief		Sync 클래스
*  @remarks	
*  @author		강동명(edith2580@gmail.com)
*  @date		2009-04-02
*/

#pragma once
#include <winapi.h>
/** 
*  @class        CSync
*  @brief        Sync 클래스
*  @remarks      
*                CSync Sync;				\r\n
*                CSyncLock CL(&Sync);
*                
*  @par          
*  @author  Edith
*  @date    2009-04-05
*/
class CSync
{
public:
	CSync(void)
	{
		InitializeCriticalSection(&m_Sync);
	}

	~CSync(void)
	{
		DeleteCriticalSection(&m_Sync);
	}

	inline void Enter(void)
	{
		EnterCriticalSection(&m_Sync);
	}

	inline void Leave(void)
	{
		LeaveCriticalSection(&m_Sync);
	}

private:
	CRITICAL_SECTION	m_Sync;
};

/** 
*  @class        CSyncLock
*  @brief        싱글스레드용 싱크
*  @remarks      
*                
*  @par          
*  @author  Edith
*  @date    2009-04-05
*/
class CSyncLock
{
public:
	CSyncLock(LPVOID lpVoid)
	{
		m_pThis = (CSync*)lpVoid;
		m_pThis->Enter();
	}

	~CSyncLock(void)
	{
		if(m_pThis)
			m_pThis->Leave();
	}

private:
	CSync *m_pThis;
};
