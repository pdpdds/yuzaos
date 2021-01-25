/** 
*  @file		NFThreadManager.h
*  @brief		Auto ThreadManager 클래스
*  @remarks	
*  @author		강동명(edith2580@gmail.com)
*  @date		2009-04-02
*/

#pragma once

#define WINDOWS_LEAN_AND_MEAN


#include "Sync.h"
#include "Thread.h"
#include "Singleton.h" 
/** 
*  @class        NFThreadManager
*  @brief        스레드 매니져
*  @remarks      
*                1) NFThread 를 상속받은 녀석				\r\n
*															\r\n
*                Nave::NFThreadManager::Run(this);			\r\n
*                Nave::NFThreadManager::Stop(this, 2000);	\r\n
*															\r\n
*                2) 스레드 매니져를 사용하려면				\r\n
*                Nave::NFThreadManager::GetInstance().Register( new CTest(this) );	\r\n
*															\r\n
*                // 소멸시 자동으로 UnRegister가 호출되어메모리에서 삭제됨..		\r\n
*                
*  @par          
*  @author  Edith
*  @date    2009-04-05
*/
class CThreadManager : public CSingleton<CThreadManager>
{
public:
	/// NFThreadManager 생성자
	CThreadManager();
	/// NFThreadManager 소멸자
	virtual ~CThreadManager();

	/// 스레드를 등록한다. 최대 32개까지 등록할 수 있다.
	BOOL Register(CThread* lpThread);	
	/// 모든 스레드를 종료시킨다.
	BOOL UnRegister();					

	/// 매니저를 사용하지 않고, 그냥 실행 시킨다.
	static HANDLE	Run(CThread* lpThread);
	/// 매니저를 사용하지 않고, 그냥 종료 시킨다.
	static BOOL		Stop(CThread* lpThread, unsigned long dwTimeout = INFINITE);

private:
	enum { 
		MAX_THREAD_NUM = 32 /// 최대 스레드 개수
	};

	/// 스레드 객체 포인터
	CThread*			m_lpThreads[MAX_THREAD_NUM];
	/// 스레드 핸들
	HANDLE				m_hThreads[MAX_THREAD_NUM];

	/// 스레드 Lock
	CSync		m_ThreadLock;

	/// 스레드 개수
	DWORD		m_nThreadNum;
	/// UnRegister의 진행여부
	DWORD		m_bUnRegStarted;
};