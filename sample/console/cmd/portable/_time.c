#include "cmd.h"

#include <time.h>

#define EPOCH_YEAR 1970
#define TM_YEAR_BASE 1900

// https://github.com/huluwa/fishjam-template-library/blob/46b4b3048a38034772d7c8a654d372907d7e024d/DocFileViewer/timecvt.h

// FileTimeToLocalFileTime
// GetLocalTime
// SystemTimeToFileTime
// CompareFileTime
// FileTimeToSystemTime
// etc.
// https://github.com/paulopina21/plxJukebox-11/blob/193996ac99b99badab3a1d422806942afca2ad01/xbmc/linux/XTimeUtils.cpp
// https://github.com/paulopina21/plxJukebox-11/blob/193996ac99b99badab3a1d422806942afca2ad01/xbmc/linux/XFileUtils.cpp


#define WIN32_TIME_OFFSET ((unsigned long long)(369 * 365 + 89) * 24 * 3600 * 10000000)

/*
 * A Leap year is any year that is divisible by four, but not by 100 unless also
 * divisible by 400
 */
#define IsLeapYear(y) ((!(y % 4)) ? (((!(y % 400)) && (y % 100)) ? 1 : 0) : 0)

static struct {
	int m_IsDST;
} g_timezone = {0};

VOID GetLocalTime(LPSYSTEMTIME sysTime)
{
  const time_t t = time(NULL);
  struct tm now;

  localtime_r(&t, &now);
  sysTime->wYear = now.tm_year + 1900;
  sysTime->wMonth = now.tm_mon + 1;
  sysTime->wDayOfWeek = now.tm_wday;
  sysTime->wDay = now.tm_mday;
  sysTime->wHour = now.tm_hour;
  sysTime->wMinute = now.tm_min;
  sysTime->wSecond = now.tm_sec;
  sysTime->wMilliseconds = 0;
  // NOTE: localtime_r() is not required to set this, but we Assume that it's set here.
  g_timezone.m_IsDST = now.tm_isdst;
}

BOOL FileTimeToLocalFileTime(const FILETIME* lpFileTime, LPFILETIME lpLocalFileTime)
{
  // TODO: FileTimeToLocalTime not implemented
  *lpLocalFileTime = *lpFileTime;
  return true;
}



static time_t
sub_mkgmt(struct tm* tm)
{
    int y, nleapdays;
    time_t t;
    /* days before the month */
    static const unsigned short moff[12] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };

    /*
     * XXX: This code assumes the given time to be normalized.
     * Normalizing here is impossible in case the given time is a leap
     * second but the local time library is ignorant of leap seconds.
     */

     /* minimal sanity checking not to access outside of the array */
    if ((unsigned)tm->tm_mon >= 12)
        return (time_t)-1;
    if (tm->tm_year < EPOCH_YEAR - TM_YEAR_BASE)
        return (time_t)-1;

    y = tm->tm_year + TM_YEAR_BASE - (tm->tm_mon < 2);
    nleapdays = y / 4 - y / 100 + y / 400 -
        ((EPOCH_YEAR - 1) / 4 - (EPOCH_YEAR - 1) / 100 + (EPOCH_YEAR - 1) / 400);
    t = ((((time_t)(tm->tm_year - (EPOCH_YEAR - TM_YEAR_BASE)) * 365 +
        moff[tm->tm_mon] + tm->tm_mday - 1 + nleapdays) * 24 +
        tm->tm_hour) * 60 + tm->tm_min) * 60 + tm->tm_sec;

    return (t < 0 ? (time_t)-1 : t);
}

time_t
timegm(struct tm* tm)
{
    time_t t, t2;
    struct tm* tm2;
    int sec;

    /* Do the first guess. */
    if ((t = sub_mkgmt(tm)) == (time_t)-1)
        return (time_t)-1;

    /* save value in case *tm is overwritten by gmtime() */
    sec = tm->tm_sec;

    tm2 = gmtime(&t);
    if ((t2 = sub_mkgmt(tm2)) == (time_t)-1)
        return (time_t)-1;

    if (t2 < t || tm2->tm_sec != sec) {
        /*
         * Adjust for leap seconds.
         *
         *     real time_t time
         *           |
         *          tm
         *         /	... (a) first sub_mkgmt() conversion
         *       t
         *       |
         *      tm2
         *     /	... (b) second sub_mkgmt() conversion
         *   t2
         *			--->time
         */
         /*
          * Do the second guess, assuming (a) and (b) are almost equal.
          */
        t += t - t2;
        tm2 = gmtime(&t);

        /*
         * Either (a) or (b), may include one or two extra
         * leap seconds.  Try t, t + 2, t - 2, t + 1, and t - 1.
         */
        if (tm2->tm_sec == sec
            || (t += 2, tm2 = gmtime(&t), tm2->tm_sec == sec)
            || (t -= 4, tm2 = gmtime(&t), tm2->tm_sec == sec)
            || (t += 3, tm2 = gmtime(&t), tm2->tm_sec == sec)
            || (t -= 2, tm2 = gmtime(&t), tm2->tm_sec == sec))
            ;	/* found */
        else {
            /*
             * Not found.
             */
            if (sec >= 60)
                /*
                 * The given time is a leap second
                 * (sec 60 or 61), but the time library
                 * is ignorant of the leap second.
                 */
                ;	/* treat sec 60 as 59,
                       sec 61 as 0 of the next minute */
            else
                /* The given time may not be normalized. */
                t++;	/* restore t */
        }
    }

    return (t < 0 ? (time_t)-1 : t);
}


BOOL SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime,  LPFILETIME lpFileTime)
{
  static const int dayoffset[12] = {0, 31, 59, 90, 120, 151, 182, 212, 243, 273, 304, 334};

  struct tm sysTime;
  sysTime.tm_year = lpSystemTime->wYear - 1900;
  sysTime.tm_mon = lpSystemTime->wMonth - 1;
  sysTime.tm_wday = lpSystemTime->wDayOfWeek;
  sysTime.tm_mday = lpSystemTime->wDay;
  sysTime.tm_hour = lpSystemTime->wHour;
  sysTime.tm_min = lpSystemTime->wMinute;
  sysTime.tm_sec = lpSystemTime->wSecond;
  sysTime.tm_yday = dayoffset[sysTime.tm_mon] + (sysTime.tm_mday - 1);
  sysTime.tm_isdst = g_timezone.m_IsDST;

  // If this is a leap year, and we're past the 28th of Feb, increment tm_yday.
  if (IsLeapYear(lpSystemTime->wYear) && (sysTime.tm_yday > 58))
    sysTime.tm_yday++;

  time_t t = timegm(&sysTime);

  LARGE_INTEGER result;
  result.QuadPart = (long long) t * 10000000 + (long long) lpSystemTime->wMilliseconds * 10000;
  result.QuadPart += WIN32_TIME_OFFSET;

  lpFileTime->dwLowDateTime = result.u.LowPart;
  lpFileTime->dwHighDateTime = result.u.HighPart;

  return 1;
}

LONG CompareFileTime(const FILETIME* lpFileTime1, const FILETIME* lpFileTime2)
{
  ULARGE_INTEGER t1;
  t1.u.LowPart = lpFileTime1->dwLowDateTime;
  t1.u.HighPart = lpFileTime1->dwHighDateTime;

  ULARGE_INTEGER t2;
  t2.u.LowPart = lpFileTime2->dwLowDateTime;
  t2.u.HighPart = lpFileTime2->dwHighDateTime;

  if (t1.QuadPart == t2.QuadPart)
     return 0;
  else if (t1.QuadPart < t2.QuadPart)
     return -1;
  else
     return 1;
}

BOOL FileTimeToSystemTime( const FILETIME* lpFileTime, LPSYSTEMTIME lpSystemTime)
{
  LARGE_INTEGER fileTime;
  fileTime.u.LowPart = lpFileTime->dwLowDateTime;
  fileTime.u.HighPart = lpFileTime->dwHighDateTime;

  fileTime.QuadPart -= WIN32_TIME_OFFSET;
  fileTime.QuadPart /= 10000; /* to milliseconds */
  lpSystemTime->wMilliseconds = fileTime.QuadPart % 1000;
  fileTime.QuadPart /= 1000; /* to seconds */

  time_t ft = fileTime.QuadPart;

  struct tm tm_ft;
  gmtime_r(&ft,&tm_ft);

  lpSystemTime->wYear = tm_ft.tm_year + 1900;
  lpSystemTime->wMonth = tm_ft.tm_mon + 1;
  lpSystemTime->wDayOfWeek = tm_ft.tm_wday;
  lpSystemTime->wDay = tm_ft.tm_mday;
  lpSystemTime->wHour = tm_ft.tm_hour;
  lpSystemTime->wMinute = tm_ft.tm_min;
  lpSystemTime->wSecond = tm_ft.tm_sec;

  return 1;
}

#define timezone 1970
BOOL LocalFileTimeToFileTime( const FILETIME* lpLocalFileTime, LPFILETIME lpFileTime)
{
  ULARGE_INTEGER l;
  l.u.LowPart = lpLocalFileTime->dwLowDateTime;
  l.u.HighPart = lpLocalFileTime->dwHighDateTime;

  l.QuadPart += (unsigned long long) timezone * 10000000;

  lpFileTime->dwLowDateTime = l.u.LowPart;
  lpFileTime->dwHighDateTime = l.u.HighPart;

  return 1;
}

BOOL FileTimeToTimeT(const FILETIME* lpLocalFileTime, time_t *pTimeT) {

  if (lpLocalFileTime == NULL || pTimeT == NULL)
  return false;

  ULARGE_INTEGER fileTime;
  fileTime.u.LowPart  = lpLocalFileTime->dwLowDateTime;
  fileTime.u.HighPart = lpLocalFileTime->dwHighDateTime;

  fileTime.QuadPart -= WIN32_TIME_OFFSET;
  fileTime.QuadPart /= 10000; /* to milliseconds */
  fileTime.QuadPart /= 1000; /* to seconds */

  time_t ft = fileTime.QuadPart;

  struct tm tm_ft;
  localtime_r(&ft,&tm_ft);

  *pTimeT = mktime(&tm_ft);
  return true;
}

BOOL  TimeTToFileTime(time_t timeT, FILETIME* lpLocalFileTime) {

  if (lpLocalFileTime == NULL)
  return false;

  ULARGE_INTEGER result;
  result.QuadPart = (unsigned long long) timeT * 10000000;
  result.QuadPart += WIN32_TIME_OFFSET;

  lpLocalFileTime->dwLowDateTime  = result.u.LowPart;
  lpLocalFileTime->dwHighDateTime = result.u.HighPart;

  return true;
}

void GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
  TimeTToFileTime(time(NULL), lpSystemTimeAsFileTime);
}

void WINAPI GetSystemTime(
  _Out_ LPSYSTEMTIME lpSystemTime
) {
	GetLocalTime(lpSystemTime);
}

BOOL WINAPI SetLocalTime(
  _In_ const SYSTEMTIME *lpSystemTime
) {
  return FALSE;
}