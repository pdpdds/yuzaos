#ifndef _TIME_H__
#define _TIME_H__
#include <minwindef.h>

VOID GetLocalTime(LPSYSTEMTIME sysTime);
BOOL FileTimeToLocalFileTime(const FILETIME* lpFileTime, LPFILETIME lpLocalFileTime);
BOOL SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime,  LPFILETIME lpFileTime);
LONG CompareFileTime(const FILETIME* lpFileTime1, const FILETIME* lpFileTime2);
BOOL FileTimeToSystemTime( const FILETIME* lpFileTime, LPSYSTEMTIME lpSystemTime);
BOOL LocalFileTimeToFileTime( const FILETIME* lpLocalFileTime, LPFILETIME lpFileTime);
void GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime);
void WINAPI GetSystemTime(
  _Out_ LPSYSTEMTIME lpSystemTime
);
BOOL WINAPI SetLocalTime(
  _In_ const SYSTEMTIME *lpSystemTime
);

#endif
