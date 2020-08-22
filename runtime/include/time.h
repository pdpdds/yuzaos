#pragma once
#include "windef.h"
#include <gettimeofday.h>

#ifdef __cplusplus
extern "C"
{
#else
#include <stdbool.h>
#endif

#ifndef _CLOCK_T_DEFINED
#define _CLOCK_T_DEFINED
//typedef __SIZE_TYPE__ clock_t;
typedef long clock_t;
#endif //!_CLOCK_T_DEFINED

//RTC Command registers
#define RTC_INDEX_REG 0x70
#define RTC_VALUE_REG 0x71

//RTC Data register index
#define RTC_SECOND 0
#define RTC_MINUTE 2
#define RTC_HOUR   4

#define RTC_DAY_OF_WEEK  6

#define RTC_DAY   7
#define RTC_MONTH 8
#define RTC_YEAR  9

#define RTC_STATUS_A 0x0A
#define RTC_STATUS_B 0x0B
#define RTC_STATUS_C 0x0C
#define RTC_STATUS_D 0x0D

#define CLOCKS_PER_SEC      1000
#define TIME_UTC            0 // The epoch for this clock is 1970-01-01 00:00:00 in Coordinated Universal Time (UTC)
#define TIME_TAI            1 // The epoch for this clock is 1970-01-01 00:00:00 in International Atomic Time (TAI)
#define TIME_MONOTONIC      2 // The epoch is when the computer was booted.
#define TIME_PROCESS        3 // The epoch for this clock is at some time during the generation of the current process.
#define TIME_THREAD         4 // The epic is like TIME_PROCESS, but locally for the calling thread.

struct tm
{
	int tm_sec;			// Seconds.	[0-60] (1 leap second) 
	int tm_min;			// Minutes.	[0-59] 
	int tm_hour;			// Hours.	[0-23] 
	int tm_mday;			// Day.		[1-31] 
	int tm_mon;			// Month.	[0-11] 
	int tm_year;			// Year - 1900. 
	int tm_wday;			// Day of week.	[0-6] 
	int tm_yday;			// Days in year.[0-365]	
	int tm_isdst;			// DST.		[-1/0/1]

	long int tm_gmtoff;		// Seconds east of UTC.  
	const char *tm_zone;		// Timezone abbreviation.  
};

struct timespec {
	time_t tv_sec;
	long tv_nsec;
};

extern long _timezone;

extern time_t time(time_t *timer);
extern char *ctime(const time_t *timer);
struct tm *localtime_r(const time_t *__restrict tim_p, struct tm *__restrict res);
extern struct tm *localtime(const time_t * tim_p);
time_t mktime(struct tm *tim_p);
struct tm *gmtime_r(const time_t *__restrict tim_p, struct tm *__restrict res);
struct tm *gmtime(const time_t *tim_p);
size_t strftime(char *__restrict s, size_t maxsize, const char*__restrict format, const struct tm *__restrict tim_p);
char* asctime(const struct tm* tim_p);
clock_t clock(void);
int clock_gettime(int clock_id, struct timespec* tp);
double difftime(time_t end, time_t start);

#ifdef __cplusplus
}


#endif