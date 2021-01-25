/** 
*  @file		NFLog.h
*  @brief		Log 출력 클래스
*  @remarks	
*  @author		강동명(edith2580@gmail.com)
*  @date		2009-04-02
*/

#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string>

#define _DELETE(x) if(x){ delete x; x = 0;}  

#include "LogManager.h"

class CLog
{
public:
	/// 각 로그의 Index 타입
	enum Groups {
		Info = 1 << 0,			
		Warning = 1 << 1,		
		Error = 1 << 2,
		Exception = 1 << 3,
		Important = 1 << 4,
	};

	/// 로그 출력용 구조체
	struct Proxy 
	{
		/// 로그가 호출된 함수
		const char* func;
		/// 로그가 호출된 파일
		const char* file;
		/// 로그가 호출된 라인번호
		int			linenum;
		/// 로그의 그룹아이디
		int			group;

		Proxy(int pGroup, const char* pFile, int pLinenum, const char* pFunc);
		void Log(const char* msg, ...);
	};

	/// Message를 출력할 윈도우 핸들을 셋팅한다.
	static void SetLogHandle(HWND hListWnd);
	/// 화면에 로고를 찍을꺼냐
	static void SetLogPrint(BOOL enable = TRUE);
	/// 파일로 로그를 찍을꺼냐
	static void SetLogOutput(BOOL enable = TRUE, DWORD dwSize = 100 * 1024 * 1024);
	/// 로그의 사용 리미터를 정의합니다. NFLog::Info ~ NFLog::Important
	static void SetLogLimit(int limit);
	/// 상세 로그를 남깁니다.
	static void SetLogDetail(BOOL detail);

	/// 로그를 종료합니다. (임시변수에 저장된 로그를 파일로 저장합니다.)
	static void CloseLog();

private:
	/**
	* @brief	hWnd에 String을 출력한다.
	* @param hWnd		윈도우 핸들
	* @param String	메시지
	* @param Len		메시지길이
	*/
	static void AddLogMsg( HWND hWnd, char* String, int Len );

	/**
	* @brief		메시지를 출력한다. 콘솔일때 printf로 윈도우모드일때 Msg윈도우로 출력
	* @param *msg	출력 메시지
	* @param ...	인자
	*/
	static void LogPrintf(char* msg, ... );

	/**
	* @brief		메시지를 출력한다. 콘솔일때 printf로 윈도우모드일때 Msg윈도우로 출력
	* @param group	그룹인덱스
	* @param *msg	메시지
	*/
	static void LogPrintf( int group, char* msg );

	/**
	* @brief		메시지를 출력한다. 콘솔일때 printf로 윈도우모드일때 Msg윈도우로 출력
	* @param group		그룹인덱스
	* @param pFile		호출파일명
	* @param pLinenum	호출라인
	* @param pFunc		호출함수
	* @param *msg		메시지
	*/
	static void LogPrintf( int group, const char* pFile, int pLinenum, const char* pFunc, char* msg );

	/// 로그 문자열을 추가합니다. 
	static void OutputLog(const char* log);

private:
	/// Message를 출력할 윈도우 핸들
	static HWND		s_hLogHandle;
	/// 로그출력을 활성화 합니다.
	static BOOL		s_bEnableLogPrint;
	/// 로그를 파일로 저장합니다.
	static BOOL		s_bSaveLogFile;
	/// 로그 Limit
	static int		s_iLogLimit;
	/// 로그 Detail 
	static BOOL		s_bLogDetail;

	/// 로그를 파일을 관리하는 매니져
	static CLogManager*	s_pLogManager;
};

/// 문자열 앞에 L을 연결하여 char 타입으로 변환함
#define WIDEN2(x) L ## x
/// CHAR의 값을 char로 변환하기 위한 정의
#define WIDEN(x) WIDEN2(x)
/// __FILE__ 의 char형 정의
#define __WFILE__ WIDEN(__FILE__)
/// __FUNCSIG__ 의 char형 정의
#define __WFUNCSIG__ WIDEN(__FUNCSIG__)

/// Info Log
#define LOG_INFO(LOGMESSAGE) {CLog::Proxy(CLog::Info, __FILE__, __LINE__,__FUNCSIG__).Log LOGMESSAGE;} 
/// Warning Log
#define LOG_WARNING(LOGMESSAGE) {CLog::Proxy(CLog::Warning,__WFILE__, __LINE__,__FUNCSIG__).Log LOGMESSAGE;} 
/// Error Log
#define LOG_ERROR(LOGMESSAGE) {CLog::Proxy(CLog::Error,__FILE__, __LINE__,__FUNCSIG__).Log LOGMESSAGE;} 
/// Exeption Log
#define LOG_EXCEPTION(LOGMESSAGE) {CLog::Proxy(CLog::Exception,__FILE__, __LINE__,__FUNCSIG__).Log LOGMESSAGE;}
/// Important Log
#define LOG_IMPORTANT(LOGMESSAGE) {CLog::Proxy(CLog::Important,__FILE__, __LINE__,__FUNCSIG__).Log LOGMESSAGE;} 
