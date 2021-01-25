#include <wchar.h>
#include <minwindef.h>
#include <string>
#include <list>
#include <winapi.h>
#include "StringGenericConversion.h"

#include "Log.h"

HWND			CLog::s_hLogHandle = NULL;
BOOL			CLog::s_bEnableLogPrint = TRUE;
BOOL			CLog::s_bSaveLogFile = FALSE;
int				CLog::s_iLogLimit = CLog::Info;
BOOL			CLog::s_bLogDetail = FALSE;
CLogManager*	CLog::s_pLogManager = NULL;


// Message를 출력할 윈도우 핸들을 셋팅한다.
void CLog::SetLogHandle(HWND hListWnd)
{
	s_hLogHandle = hListWnd;
}

void CLog::SetLogPrint(BOOL enable)
{
	s_bEnableLogPrint = enable;
}

void CLog::SetLogOutput(BOOL enable, DWORD dwSize)
{
	s_bSaveLogFile = enable;
	SetLogPrint(enable);

	_DELETE(s_pLogManager);
	if(s_bSaveLogFile)
	{
		s_pLogManager = new CLogManager();
		s_pLogManager->Initialize(FALSE);
		s_pLogManager->SetLogMaxSize(dwSize);
	}
}

void CLog::CloseLog()
{
	if(s_pLogManager)
	{
		s_pLogManager->Flush();
		_DELETE(s_pLogManager);
	}
}

void CLog::SetLogLimit(int limit) 
{
	s_iLogLimit=limit;
}

void CLog::SetLogDetail(BOOL detail)
{
	s_bLogDetail = detail;
}

void CLog::AddLogMsg( HWND hWnd, char* String, int Len )
{
	// 삭제
	/*int iCount = (int)SendMessageW( hWnd, LB_GETCOUNT, 0, 0L )-128;
	for(int i = 0; i < iCount; ++i)
		SendMessageW( hWnd, LB_DELETESTRING, 0, 0L );

	// 추가
	WCHAR* p;
	int k;
	p = String;

	int iAdd = 0;

	k = 0;
	while( k++ < Len ) 
	{
		switch( *String ) 
		{
		case L'\n':
			*String = 0;
			++iAdd;
			SendMessageW( hWnd, LB_ADDSTRING, 0, (LPARAM)(LPWSTR)p );
			p = ++String;
			break;
		default :
			++String;
		}
	}

	if( *p )
	{
		++iAdd;
		SendMessageW( hWnd, LB_ADDSTRING, 0, (LPARAM)(LPWSTR)p );
	}

	int Top = (int)SendMessageW( hWnd, LB_GETTOPINDEX, 0, 0L );
	SendMessageW( hWnd, LB_SETTOPINDEX, Top+iAdd, 0L );*/
}

void CLog::LogPrintf(char* msg, ... )
{
	va_list v;
	char buf[1024];
	int len;

	va_start( v, msg );
	len = vsprintf( buf, msg, v );

	va_end( v );

	LogPrintf( Info, buf);
}

void CLog::OutputLog(const char* log)
{
	static CHAR stDot[4] = "\r\n";
	static CHAR stTime[32];

	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	sprintf(stTime, "[%04d.%02d.%02d-%02d:%02d:%02d]", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	std::string str = stTime;
	str += ToASCII(log);
	str += stDot;

	int iSize = str.size();
	char* pPoint = s_pLogManager->ReserveBuffer(iSize);
	memcpy(pPoint, str.c_str(), iSize);
}

void CLog::LogPrintf( int group, char* msg )
{
	if (group < s_iLogLimit) 
		return;

	if(s_bEnableLogPrint)
	{
		if(s_hLogHandle == NULL)
		{
			// setlocale 함수를 지정해야 한글이 출력됨
			printf(msg);
			printf("\n");
		}
		else
			AddLogMsg( s_hLogHandle, msg, (int)strlen(msg) );
	}

#ifdef _DEBUG
	OutputDebugStringW( msg );
	OutputDebugStringW( L"\r\n" );
#endif
	if(s_bSaveLogFile)
	{
		std::string tmp = msg;
		std::string logmsg;
		switch (group) {
			case Info:
				logmsg="[INF] " + tmp;
				break;
			case Warning:
				logmsg="[WAR] " + tmp;
				break;
			case Error:
				logmsg="[ERR] " + tmp;
				break;
			case Exception:
				logmsg="[EXP] " + tmp;
				break;
			case Important:
				logmsg="[DET] " + tmp;
				break;
		};

		OutputLog(logmsg.c_str());
	}
}

void CLog::LogPrintf( int group, const char* pFile, int pLinenum, const char* pFunc, char* msg )
{
	if (group < s_iLogLimit) 
		return;

	if(s_bEnableLogPrint)
	{
		if(s_hLogHandle == NULL)
		{
			printf(msg);
			printf("\n");
		}
		else
			AddLogMsg( s_hLogHandle, msg, (int)strlen(msg) );
	}

#ifdef _DEBUG
	OutputDebugStringW( msg );
	OutputDebugStringW( L"\r\n" );
#endif

	if(s_bSaveLogFile)
	{
		std::string tmp = msg;
		std::string logmsg;
		switch (group) {
			case Info:
				logmsg="[INF]" + tmp;
				break;
			case Warning:
				logmsg="[WAR]" + tmp;
				break;
			case Error:
				logmsg="[ERR]" + tmp;
				break;
			case Exception:
				logmsg="[EXP]" + tmp;
				break;
			case Important:
				logmsg="[DET]" + tmp;
				break;
		};

		OutputLog(logmsg.c_str());

		static char deta[1024];
		sprintf(deta, "                 -> [%s,%d,%s]", pFile, pLinenum, pFunc);
		OutputLog(deta);
	}
}

CLog::Proxy::Proxy(int pGroup, const char* pFile, int pLinenum, const char* pFunc) : file(pFile),linenum(pLinenum), func(pFunc), group(pGroup) 
{
}

void CLog::Proxy::Log(const char* msg, ...)
{
	if (group < CLog::s_iLogLimit) 
		return;

	char txt[1024];
	va_list l;
	va_start(l,msg);
	vsnprintf( txt, 1024, msg, l );
	va_end(l); 
	if(!CLog::s_bLogDetail)
		LogPrintf(group,txt);
	else
		LogPrintf(group,file,linenum,func,txt);		// 이건 파일과 기타 정보를 남길때.. 
}
