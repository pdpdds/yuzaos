#include "Log.h"
#include "LogManager.h"
#include "gzip.h"
#include "ThreadManager.h"
#include <string.h>
#include <minwinconst.h>
#include <stat_def.h>

CLogThread::CLogThread() : m_pLogHandle(NULL), m_bCompress(TRUE),
m_hFlush(CreateEvent(0, TRUE, FALSE, 0)),
m_hFile(INVALID_HANDLE_VALUE), m_dwTotalWritten(0),
m_dwMaxFileSize(MAX_FILE_SIZE)
{
	InterlockedExchange(&m_bEnd, FALSE);
}

CLogThread::~CLogThread()
{
	if(INVALID_HANDLE_VALUE != m_hFile)
	{ 
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}

	if(0 != m_hFlush) 
	{
		CloseHandle(m_hFlush); 
		m_hFlush = 0;
	}	
}

void CLogThread::Initialize(CLogManager* pLog, BOOL bCompress)
{
	m_bCompress = bCompress;
	m_pLogHandle = pLog;
}

unsigned int CLogThread::Run()
{
	// 여기서 m_bEnd가 TRUE가 아니면 스레드가 작동하고 있다.
	for(;TRUE != InterlockedCompareExchange(&m_bEnd, TRUE, TRUE);)
	{
		// 현재 이벤트가 사용중이면 1초간 대기후 다시 확인.
		if(WAIT_TIMEOUT == WaitForSingleObject(m_hFlush, 1000))
		{
			continue;
		}

		WriteLog();
	};

	WriteLog();
	return 0;
}

BOOL CLogThread::End()
{
	InterlockedExchange(&m_bEnd, TRUE);
	SetEvent(m_hFlush);

	return TRUE;
}

BOOL CLogThread::WriteLog()
{
	// 로그에서 버퍼를 읽어온다.
	if(!m_pLogHandle)
		return FALSE;

	m_pLogHandle->SpliceInWriteBuffer(m_WriteBufferList);

	if(m_WriteBufferList.empty())
		return TRUE;

	if(INVALID_HANDLE_VALUE == m_hFile)
	{
		SetLogFileName();
		m_hFile = CreateFile(m_szLogFileName, GENERIC_WRITE,
			FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

		if(INVALID_HANDLE_VALUE == m_hFile)
		{
			return FALSE;
		}
	}

	unsigned long dwWritten = 0;

	for(CLogBuffer::List::iterator itr = m_WriteBufferList.begin();
		itr != m_WriteBufferList.end(); ++itr)
	{
		CLogBuffer* pLogBuffer = *itr;
		if(FALSE == WriteFile(m_hFile, pLogBuffer->m_Buffer, pLogBuffer->m_dwUsage, &dwWritten, 0))
		{
			LOG_ERROR(("로그 파일 기록에 실패했습니다. ErrorNum : %d, FileHandle:0x%p, 버퍼 크기:%d",
				GetLastError(), m_hFile, pLogBuffer->m_dwUsage));
		}

		m_dwTotalWritten += dwWritten;
	}

	m_pLogHandle->SpliceInFreeBuffer(m_WriteBufferList);

	// 파일은 계속 열어놓는다.
	if(m_dwTotalWritten > m_dwMaxFileSize)
	{
		if(INVALID_HANDLE_VALUE != m_hFile) { CloseHandle(m_hFile); m_hFile = INVALID_HANDLE_VALUE; }	// 파일 닫고...

		if(m_bCompress)
			Compress();

		m_dwTotalWritten = 0;
	}

	return TRUE;
}

BOOL CLogThread::Compress()
{
	HANDLE hFile = CreateFile(m_szLogFileName, GENERIC_READ, 
		FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if(INVALID_HANDLE_VALUE == hFile)
	{
		LOG_ERROR(("게임 로그를 압축할 수 없습니다."));
		return FALSE;
	}

	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	unsigned long dwSpinCount = 0;
	char szCompressedLogFileName[MAX_PATH*2];

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	splitpath(m_szLogFileName, drive, dir, fname, ext);

	while (TRUE) 
	{		
		snprintf(szCompressedLogFileName, MAX_PATH*2, "%s\\%s-%04d%02d%02d-%02d%02d%02d-%04d.log.gz",
			dir, fname, sysTime.wYear, sysTime.wMonth, sysTime.wDay, 
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond, dwSpinCount);

		struct stat info;

		int result = stat(szCompressedLogFileName, &info);
		if (result != 0)
		{
			break;
		}

		++dwSpinCount;
	}

	// 압축 알고리즘 추가.
	SDK::CGZip gzip;

	if (!gzip.Open(szCompressedLogFileName, SDK::CGZip::ArchiveModeWrite))
	{
		LOG_ERROR(("압축 파일의 생성에 실패했습니다."));
		CloseHandle(hFile);
		return FALSE;
	}

	unsigned long	dwRead = 0;
	char	szBuffer[4096];
	int nError = 0;
	while(FALSE != ReadFile(hFile, szBuffer, 4096, &dwRead, 0))
	{
		if(0 == dwRead)
		{
			break;
		}

		if(!gzip.WriteBuffer(szBuffer, dwRead))
		{
			LOG_ERROR(("에러로 인해 압축 파일에 기록할 수 없습니다."));
			nError = -1;
			break;
		}
	}

	gzip.Close();

	CloseFile(hFile);

	//int nError = GetLastError();
	if(0 != nError)
	{
		LOG_ERROR(("%d 에러로 인해 로그 파일 압축이 실패했습니다.", nError));
		return FALSE;
	}

	// 로그파일 삭제
	if(0 != rmdir(m_szLogFileName))
	{
		LOG_ERROR(("%d 에러로 인해 압축하고 난 로그를 삭제하지 못했습니다.", nError));
		return FALSE;
	}	

	return TRUE;
}

BOOL CLogThread::SetLogFileName()
{
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	unsigned long dwSpinCount = 0;
	char szProgramName[MAX_PATH] = "temp";
	char szLogFilePrefix[MAX_PATH];

	// 프로그램이름과 동일한 하위폴더를 생성한다.
	//GetProgramName(szProgramName, MAX_PATH);

	if(m_pLogHandle)
		snprintf(szLogFilePrefix, MAX_PATH - 1, "%s", m_pLogHandle->GetLogFilePrefix());
	else
		strcpy(szLogFilePrefix, szProgramName);

	struct stat info;

	int result = stat(szProgramName, &info);
	if (result != 0)
	{
		if (0 != mkdir(szProgramName, 0))
		{			
			return FALSE;
		}
	}

	while (TRUE) 
	{
		int LogLen = snprintf(m_szLogFileName, MAX_PATH, 
			"%s\\%s-%04d%02d%02d-%02d%02d%02d-%04d.log", 
			szProgramName, szLogFilePrefix, sysTime.wYear, sysTime.wMonth, sysTime.wDay,
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond, dwSpinCount);

		if(LogLen <= 0)
		{
			LOG_ERROR(("상세 로그를 위한 파일 이름을 생성할 수 없습니다."));
			return FALSE;
		}
		struct stat info;

		int result = stat(szProgramName, &info);
		if (result != 0)
		{
			break;
		}

		++dwSpinCount;
	}

	return TRUE;
}

CLogManager::CLogManager(void) : m_lpDetailBuffer(0)
{
}

CLogManager::~CLogManager(void)
{
	Destroy();
}

BOOL CLogManager::Initialize(BOOL bCompress, const char* szLogFilePrefix)
{
	char strName[MAX_PATH] = "temp";
	//GetProgramName(strName, MAX_PATH);
	strcpy(m_szLogFilePrefix, strName);

	if(szLogFilePrefix != 0)
	{
		strcat(m_szLogFilePrefix, szLogFilePrefix);
	}

	// 처음에 프리버퍼 개수만큼 만든다.
	for(int nCount = 0; nCount < DEFAULT_FREE_LOG_BUFFER_NUM; ++nCount)
	{
		CLogBuffer* pDetailBuffer = new CLogBuffer;
		if(0 == pDetailBuffer)
		{
			LOG_ERROR(("Fail to Allocate Detail Log Buffer!!"));
			Destroy();
			return FALSE;
		}

		m_FreeList.push_back(pDetailBuffer);
	};

	// 버퍼가 없으니 우선 1개 가져옴.
	if(m_lpDetailBuffer == 0)
	{
		m_lpDetailBuffer = GetBuffer();
	}

	m_LogSaveThread.Initialize(this, bCompress);

	if(INVALID_HANDLE_VALUE == CThreadManager::Run(&m_LogSaveThread))
	{
		LOG_ERROR(("Flush스레드를 생성하는 데 실패했습니다"));
		return FALSE;
	}

	return TRUE;
}

BOOL CLogManager::Destroy()
{
	Flush();
	CThreadManager::Stop(&m_LogSaveThread, INFINITE);

	// 싱크건다.
	CSyncLock CL(&m_LogSync);

	CLogBuffer* pDetailBuffer;
	for(CLogBuffer::List::iterator itr = m_FreeList.begin(); itr != m_FreeList.end(); ++itr)
	{
		pDetailBuffer = (*itr);
		_DELETE(pDetailBuffer);
	}
	m_FreeList.clear();

	for(CLogBuffer::List::iterator itr = m_WriteList.begin(); itr != m_WriteList.end(); ++itr)
	{
		pDetailBuffer = (*itr);
		_DELETE(pDetailBuffer);
	}
	m_WriteList.clear();

	return TRUE;
}

void CLogManager::SetLogMaxSize(DWORD dwSize)
{
	m_LogSaveThread.SetLogMaxSize(dwSize);
}

CLogBuffer* CLogManager::GetBuffer()
{
	// 싱크건다.
	CSyncLock CL(&m_LogSync);

	CLogBuffer*	pLogBuffer = 0;
	if(m_FreeList.empty())
	{
		// 남는 버퍼가 없으면 실시간으로 생성해버린다. 어쩔수없다.
		pLogBuffer = new CLogBuffer;
	}
	else
	{
		pLogBuffer = m_FreeList.front();
		m_FreeList.pop_front();
	}

	if(0 == pLogBuffer)
	{
		LOG_ERROR(("상세 로그 버퍼를 할당할 수 없습니다."));
		return 0;
	}

	pLogBuffer->Initialize();
	return pLogBuffer;
};

BOOL CLogManager::Flush()
{
	// 버퍼를 넣는다.
	PushBuffer(&m_lpDetailBuffer);
	return m_LogSaveThread.FlushSignal();
}

char* CLogManager::ReserveBuffer(unsigned short usReserve)
{
	CSyncLock CL(&m_LogSync);

	if(0 == m_lpDetailBuffer)
	{
		m_lpDetailBuffer = GetBuffer();
	}

	if(CLogBuffer::MAX_LOG_BUFFER < m_lpDetailBuffer->m_dwUsage + usReserve)
	{
		Flush();	// 버퍼가 꽉찼으니 저장해라.
		m_lpDetailBuffer = GetBuffer();		
	}

	if(0 == m_lpDetailBuffer)
	{
		LOG_ERROR(("로그 버퍼가 0입니다."));
		return 0;
	}

	// 컴플리트를 먼저해서 해당영역을 확보한다.
	char* pPoint = &m_lpDetailBuffer->m_Buffer[m_lpDetailBuffer->m_dwUsage];
	Complete(usReserve);
	return pPoint;
}

void CLogManager::SpliceInWriteBuffer(CLogBuffer::List& logBufferList)
{
	CSyncLock CL(&m_LogSync);

	if(m_WriteList.empty())
		return;

	// WriteBuffer 를 가져온다.
	logBufferList.splice(logBufferList.end(), m_WriteList);
}

void CLogManager::SpliceInFreeBuffer(CLogBuffer::List& logBufferList)
{
	CSyncLock CL(&m_LogSync);

	if(logBufferList.empty())
		return;

	m_FreeList.splice(m_FreeList.end(), logBufferList);
}

void CLogManager::PushBuffer(CLogBuffer** ppDetailBuffer)
{
	if(0 == *ppDetailBuffer) { return; }

	CSyncLock CL(&m_LogSync);

	if(0 == (*ppDetailBuffer)->m_dwUsage)
	{
		// 사이즈가 0이니 다시 Free에 넣음.
		m_FreeList.push_back(*ppDetailBuffer);
	}
	else
	{
		m_WriteList.push_back(*ppDetailBuffer);
	}

	// 이부분 대문에 이중포인터로 변수를 받은것.
	*ppDetailBuffer = 0;
}
