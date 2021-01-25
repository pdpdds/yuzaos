/** 
*  @file		NFLogManager.h
*  @brief		LogManager 클래스
*  @remarks	
*  @author		강동명(edith2580@gmail.com)
*  @date		2009-04-02
*/

#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <time.h>
#include <list>

#include "Sync.h"
#include "Thread.h"
#include <winapi.h>

class CLogManager;

/** 
*  @class        NFLogBuffer
*  @brief        로그가 저장되는 로그 버퍼 이게 파일로 저장된다.
*  @remarks      
*                
*  @par          
*  @author  Edith
*  @date    2009-04-04
*/
class CLogBuffer
{
public:
	/// Write, Free 버퍼를 리스트로 관리하게 되는데 typedef를 여기서 했다.
	typedef std::list<CLogBuffer*>		List;	

	enum 
	{ 
		MAX_LOG_BUFFER = 65536	/// 로그버퍼의 크기
	};

	/// 사용된 버퍼의 사이즈
	unsigned long		m_dwUsage;						
	/// 실제 버퍼
	char				m_Buffer[MAX_LOG_BUFFER];		

	/// NFLogBuffer 생성자
	CLogBuffer() : m_dwUsage(0) 
	{ 
	}

	//초기화 함수
	void Initialize()
	{ 
		m_dwUsage = 0; 
	}
};

/** 
*  @class        NFLogThread
*  @brief        로그를 관리하는 로그 스레드이다. 로그매니져에서 사용됨.
*  @remarks      
*                
*  @par          
*  @author  Edith
*  @date    2009-04-04
*/
class CLogThread : public CThread
{
public:
	/// NFLogThread 생성자
	CLogThread();
	/// NFLogThread 소멸자
	~CLogThread();

	/**
	* @brief		로그를 초기화한다.
	* @param pLog		로그메니져 객체
	* @param bCompress 압축여부
	*/
	void		Initialize(CLogManager* pLog, BOOL bCompress);

	/**
	* @brief	로그파일의 크기를 조절한다. 이크기가 되면 다른 파일로 로그를남긴다.
	* @param dwSize 로그파일의 크기 byte 수로 남긴다 10 * 1024 * 1024 는 10메가
	*/
	inline void SetLogMaxSize(DWORD dwSize = MAX_FILE_SIZE)  { m_dwMaxFileSize = dwSize; }

	/// 로그를 저장한다.
	BOOL		FlushSignal() { return PulseEvent(m_hFlush); }

	/// 로그 스레드를 실행한다.
	virtual unsigned int Run();
	/// 로그 스레드를 종료합니다.
	virtual BOOL End();			

private:
	enum 
	{ 
		MAX_FILE_SIZE = 100 * 1024 * 1024		/// 기본 파일 사이즈 100메가
	};

	/**
	* @brief	로그파일의 이름을 설정합니다. 
	* @return  성공여부
	*/
	BOOL				SetLogFileName();
	/**
	* @brief	로그버퍼에 로그를 Write 합니다.
	* @return  성공여부
	*/
	BOOL				WriteLog();
	/**
	* @brief	생성된 로그파일을 압축합니다.
	* @return	성공여부
	*/
	BOOL				Compress();

private:
	/// 해당 스레드가 종료됬는지 보는로직
	volatile LONG		m_bEnd;					

	/// 현재 해당 스레드가 작동중인지 확인하는 이벤트
	HANDLE				m_hFlush;				
	/// 로그가 저장되는 파일핸들
	HANDLE				m_hFile;
	/// 사용된 파일크기
	DWORD				m_dwTotalWritten;		

	/// 로그 메니져
	CLogManager*		m_pLogHandle;
	/// 압축 유무
	BOOL				m_bCompress;

	/// 쓰기 로그 버퍼
	CLogBuffer::List	m_WriteBufferList;

	/// 로그의 파일이름
	char				m_szLogFileName[MAX_PATH];

	/// 로그파일의 최대 크기
	DWORD				m_dwMaxFileSize;
};

/** 
*  @class        NFLogManager
*  @brief        로그를 저장할때 사용하는 로그 매니져 클래스
*  @remarks      로그 매니져는 한 어플이 여러개의 로그를 종류별로 출력할 수도 있기 때문에 싱글톤으로 만들지 않는다.\r\n
*  			  디테일로그는 바이너리리를 넣기위해 존재하는 로그다.												\r\n
*  		 	  안에 들어가는 로그는 게임 별로 해당 로그 구조체등을 구현해야한다.									\r\n
*  			  디테일 로그의 경우 속도증가를 위해 처음엔 메모리상에 버퍼를 적고 버퍼가 꽉 찼으면 해당 버퍼를		\r\n
*  			  WriteList에 넣은후 해당 버퍼를 처리한다.															\r\n
*                
*  @par          
*  @author  Edith
*  @date    2009-04-04
*/
class CLogManager 
{
public:
	/// NFLogManager 생성자
	CLogManager(void);
	/// NFLogManager 소멸자
	~CLogManager(void);

	/**
	* @brief	로그 메니져를 초기화 합니다.
	* @param bCompress			압축유무
	* @param szLogFilePrefix	로그파일명
	* @return	성공여부
	*/
	BOOL			Initialize(BOOL bCompress, const char* szLogFilePrefix = 0);
	/// 로그매니져를 종료합니다.
	BOOL			Destroy();

	/// 로그버퍼의 사이즈를 조절한다.
	void			SetLogMaxSize(DWORD dwSize = 10 * 1024 * 1024);


	/**
	* @brief	로그파일의 이름
	* @return	로그파일이름
	*/
	const char*	GetLogFilePrefix() const { return m_szLogFilePrefix; }

	/**
	* @brief	로그를 저장합니다.
	* @return	성공여부
	*/
	BOOL			Flush();

	/**
	* @brief	현재 사용되는 로그 버퍼를 구합니다.
	* @return	로그버퍼
	*/
	CLogBuffer*	GetBuffer();

	/**
	* @brief	 로그버퍼를 할당한다.
	* @param usReserve 할당 사이즈
	* @return	로그버퍼의 위치포인터
	*/
	char*			ReserveBuffer(unsigned short usReserve);

	/**
	* @brief	할당된 로그버퍼를 적용시킵니다.
	* @param usRealUse 사이즈
	*/
	void		    Complete(unsigned short usRealUse) { m_lpDetailBuffer->m_dwUsage += usRealUse; }

	/**
	* @brief	출력상태로 로그버퍼를 할당합니다.
	* @param ppDetailBuffer 
	*/
	void			PushBuffer(CLogBuffer** ppDetailBuffer);

	void			SpliceInWriteBuffer(CLogBuffer::List& logBufferList);
	void			SpliceInFreeBuffer(CLogBuffer::List& logBufferList);

private:
	enum 
	{ 
		DEFAULT_FREE_LOG_BUFFER_NUM = 10		/// 최대 10개까지 프리버퍼를 생성한다.
	};

	/// Sync 객체
	CSync							m_LogSync;

	/// 로그 파일명
	char							m_szLogFilePrefix[MAX_PATH];

	/// 로그 저장용 스레드 변수
	CLogThread						m_LogSaveThread;

	/// 비어있는 버퍼
	CLogBuffer::List				m_FreeList;		
	/// 파일에 쓰여질 버퍼
	CLogBuffer::List				m_WriteList;	

	/// 현재 사용되는 버퍼
	CLogBuffer*					m_lpDetailBuffer;
};