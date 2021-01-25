/** 
*  @file		Thread.h
*  @brief		Auto Thread 생성 클래스
*  @remarks	
*  @author		강동명(edith2580@gmail.com)
*  @date		2009-04-02
*/

#pragma once
#include <minwindef.h>
#include <minwinbase.h>
/** 
*  @class        NFThread
*  @brief        스레드 객체 클래스
*  @remarks      
*                
*  @par          
*  @author  Edith
*  @date    2009-04-05
*/
class CThread
{
public:
	CThread() : m_hThreadHandle(INVALID_HANDLE_VALUE) 
	{ 
	}

	~CThread() 
	{ 
	}

	typedef unsigned int(__stdcall *LPThreadFunc)(void*);
	/// 스레드 함수포인터
	static inline unsigned int __stdcall ThreadFunc(void* pArg);

	/// 스레드 핸들을 리턴한다.
	inline HANDLE GetHandle() { return m_hThreadHandle; }
	/// 스레드 핸들을 설정한다.
	inline void SetHandle(HANDLE hHandle) { m_hThreadHandle = hHandle; }

	/// 실제 실행 되는 루프를 넣는다.
	virtual unsigned int Run() = 0;		
	/// 루프가 끝날 수 있는 루틴을 넣는다.
	virtual BOOL End() = 0;				

private:
	/// 스레드핸들
	HANDLE	m_hThreadHandle;

	//		friend class NFThreadManager;
};

/// 스레드 호출함수
inline unsigned int __stdcall CThread::ThreadFunc(void* pArg)
{
	return static_cast<CThread*>(pArg)->Run();
}
