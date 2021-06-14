#include "TimeFunction.h"
#include <intrinsic.h>
#include <stdint.h>
#include <stdio.h>
#include <Semaphore.h>
#include <SystemAPI.h>

extern unsigned int g_tickCount;
unsigned int g_startTickCount = 0;
Semaphore gSleepSemaphore("sleep_sem", 0);


extern "C" void kSleep(DWORD dwMilliseconds)
{
#if SKY_EMULATOR
	//g_platformAPI._processInterface.sky_Sleep(dwMilliseconds);
	//gSleepSemaphore.Wait(dwMilliseconds);
	Semaphore sleepSemaphore("sleep_sem", 0);
	sleepSemaphore.Wait(dwMilliseconds);
	return;
#else
	Semaphore sleepSemaphore("sleep_sem", 0);
	sleepSemaphore.Wait(dwMilliseconds);
	//gSleepSemaphore.Wait(dwMilliseconds);
#endif
}


extern "C" DWORD kGetTickCount()
{
#if SKY_EMULATOR
	if (g_startTickCount == 0)
	{
		g_startTickCount = g_platformAPI._processInterface.sky_GetTickCount();
		return 0;
	}
	
	return g_platformAPI._processInterface.sky_GetTickCount() - g_startTickCount;
#endif
	//kSleep(0);
	//int32to64 a;
	//read_tsc(&a.i32[0], &a.i32[1]);
	//return a.i32[1];
	return (g_tickCount - g_startTickCount);
}

BOOL kGetLocalTime(LPSYSTEMTIME lpSystemTime)
{
#if SKY_EMULATOR
	return g_platformAPI._processInterface.sky_GetLocalTime(lpSystemTime);
#endif

	/* Checking whether we can read the time now or not according to some documentation the MSB in Status A will remain 1 (invalid time) only a millisecond*/
	int TimeOut;

	OutPortByte(RTC_INDEX_REG, RTC_STATUS_A);    //check status - read access
	TimeOut = 1000;
	while (InPortByte(RTC_VALUE_REG) & 0x80)
		if (TimeOut < 0)
			return false;
		else
			TimeOut--;

	OutPortByte(RTC_INDEX_REG, RTC_DAY);         //get day value
	lpSystemTime->wDay = InPortByte(RTC_VALUE_REG);
	OutPortByte(RTC_INDEX_REG, RTC_MONTH);       //get month value
	lpSystemTime->wMonth = InPortByte(RTC_VALUE_REG);
	OutPortByte(RTC_INDEX_REG, RTC_YEAR);        //get year
	lpSystemTime->wYear = InPortByte(RTC_VALUE_REG);

	OutPortByte(RTC_INDEX_REG, RTC_DAY_OF_WEEK); //get day of week - **** problem
	lpSystemTime->wDayOfWeek = InPortByte(RTC_VALUE_REG);

	OutPortByte(RTC_INDEX_REG, RTC_SECOND);
	lpSystemTime->wSecond = InPortByte(RTC_VALUE_REG);
	OutPortByte(RTC_INDEX_REG, RTC_MINUTE);
	lpSystemTime->wMinute = InPortByte(RTC_VALUE_REG);
	OutPortByte(RTC_INDEX_REG, RTC_HOUR);
	lpSystemTime->wHour = InPortByte(RTC_VALUE_REG);

	OutPortByte(RTC_INDEX_REG, RTC_STATUS_B);
	OutPortByte(RTC_VALUE_REG, 2);


	lpSystemTime->wYear = (lpSystemTime->wYear / 16) * 10 + (lpSystemTime->wYear % 16);
	lpSystemTime->wMonth = (lpSystemTime->wMonth / 16) * 10 + (lpSystemTime->wMonth % 16);
	lpSystemTime->wDay = (lpSystemTime->wDay / 16) * 10 + (lpSystemTime->wDay % 16);
	lpSystemTime->wHour = (lpSystemTime->wHour / 16) * 10 + (lpSystemTime->wHour % 16);
	lpSystemTime->wMinute = (lpSystemTime->wMinute / 16) * 10 + (lpSystemTime->wMinute % 16);
	lpSystemTime->wSecond = (lpSystemTime->wSecond / 16) * 10 + (lpSystemTime->wSecond % 16);

	lpSystemTime->wYear += 2000;

	return true;
}

/* Returns 1 on success and 0 on failue */
BYTE kSetLocalTime(LPSYSTEMTIME lpSystemTime)
{
	/* Checking whether we can read the time now or not according to some documentation the MSB in Status A will   remain 1 (invalid time) only a millisecond*/
	int TimeOut;

	OutPortByte(RTC_INDEX_REG, RTC_STATUS_A);    //checking status -read access
	TimeOut = 1000;
	while (InPortByte(RTC_VALUE_REG) & 0x80)
		if (TimeOut < 0)
			return 0;
		else
			TimeOut--;

	OutPortByte(RTC_INDEX_REG, RTC_DAY);
	OutPortByte(RTC_VALUE_REG, (uchar)lpSystemTime->wDay);
	OutPortByte(RTC_INDEX_REG, RTC_MONTH);
	OutPortByte(RTC_VALUE_REG, (uchar)lpSystemTime->wMonth);
	OutPortByte(RTC_INDEX_REG, RTC_YEAR);
	OutPortByte(RTC_VALUE_REG, (uchar)lpSystemTime->wYear);

	OutPortByte(RTC_INDEX_REG, RTC_SECOND);
	OutPortByte(RTC_VALUE_REG, (uchar)lpSystemTime->wSecond);
	OutPortByte(RTC_INDEX_REG, RTC_MINUTE);
	OutPortByte(RTC_VALUE_REG, (uchar)lpSystemTime->wMinute);
	OutPortByte(RTC_INDEX_REG, RTC_HOUR);
	OutPortByte(RTC_VALUE_REG, (uchar)lpSystemTime->wHour);

	return 1;
}

bool GetMilliTime(bigtime_t& milliTime)
{
	/* Checking whether we can read the time now or not according to some documentation the MSB in Status A will remain 1 (invalid time) only a millisecond*/
	/*int TimeOut;

	OutPortByte(RTC_INDEX_REG, RTC_STATUS_A);    //check status - read access
	TimeOut = 1000;
	while (InPortByte(RTC_VALUE_REG) & 0x80)
		if (TimeOut < 0)
			return false;
		else
			TimeOut--;

	OutPortByte(RTC_INDEX_REG, RTC_SECOND);
	milliTime = InPortByte(RTC_VALUE_REG);
	milliTime *= 1000;*/

	milliTime = kGetTickCount();

	return true;
}

/* Definitions */
#define CMOS_IO_BASE			0x70
#define CMOS_IO_LENGTH			2
#define CMOS_IO_SELECT			0x00
#define CMOS_IO_DATA			0x01

#define CMOS_REGISTER_SECONDS	0x00
#define CMOS_REGISTER_MINUTES	0x02
#define CMOS_REGISTER_HOURS		0x04
#define CMOS_REGISTER_DAYS		0x07
#define CMOS_REGISTER_MONTHS	0x08
#define CMOS_REGISTER_YEARS		0x09

#define CMOS_REGISTER_STATUS_A	0x0A
#define CMOS_REGISTER_STATUS_B	0x0B
#define CMOS_REGISTER_STATUS_C	0x0C
#define CMOS_REGISTER_STATUS_D	0x0D

#define CMOS_CURRENT_YEAR		2017

/* Bits */
#define CMOS_NMI_BIT			0x80
#define CMOS_ALLBITS_NONMI		0x7F
#define CMOSX_BIT_DISABLE_NMI	0x80
#define CMOSA_UPDATE_IN_PROG	0x80
#define CMOSB_BCD_FORMAT		0x04
#define CMOSB_RTC_PERIODIC		0x40

/* Time Conversion */
#define CMOS_BCD_TO_DEC(n)		(((n >> 4) & 0x0F) * 10 + (n & 0x0F))
#define CMOS_DEC_TO_BCD(n)		(((n / 10) << 4) | (n % 10))

/* RTC Irq */
#define CMOS_RTC_IRQ			0x08

typedef enum _DeviceIoType {
	DeviceIoInvalid = 0,
	DeviceIoMemoryBased,            // Usually device memory range
	DeviceIoPortBased,              // Usually a port range
	DeviceIoPinBased                // Usually a port/pin combination
} DeviceIoType_t;

/* ReadDirectIo
* Reads a value from the given raw io source. Accepted values in width are 1, 2, 4 or 8. */
int
ReadDirectIo(
	_In_  DeviceIoType_t    Type,
	_In_  uintptr_t         Address,
	_In_  size_t            Width,
	_Out_ size_t* Value)
{
	switch (Type) {
	case DeviceIoPortBased: {
		if (Width == 1) {
			*Value = ((size_t)InPortByte((uint16_t)Address) & 0xFF);
		}
		else if (Width == 2) {
			*Value = ((size_t)InPortWord((uint16_t)Address) & 0xFFFF);
		}
		else if (Width == 4) {
			*Value = ((size_t)InPortDWord((uint16_t)Address) & 0xFFFFFFFF);
		}
#if __BITS == 64
		else if (Width == 8) {
			uint64_t Temporary = inl((uint16_t)Address + 4);
			Temporary <<= 32;
			Temporary |= inl((uint16_t)Address);
			*Value = Temporary;
		}
#endif
		else {
			//ERROR(" > invalid port width %u for reading", Width);
			return -1;
		}
	} break;

	default: {
		//FATAL(FATAL_SCOPE_KERNEL, " > invalid direct io read type %u", Type);
	} break;
	}
	return 0;
}

/* WriteDirectIo
* Writes a value to the given raw io source. Accepted values in width are 1, 2, 4 or 8. */
int
WriteDirectIo(
	_In_ DeviceIoType_t     Type,
	_In_ uintptr_t          Address,
	_In_ size_t             Width,
	_In_ size_t             Value)
{
	switch (Type) {
	case DeviceIoPortBased: {
		if (Width == 1) {
			OutPortByte((uint16_t)Address, (uint8_t)(Value & 0xFF));
		}
		else if (Width == 2) {
			OutPortWord((uint16_t)Address, (uint16_t)(Value & 0xFFFF));
		}
		else if (Width == 4) {
			OutPortDWord((uint16_t)Address, (uint32_t)(Value & 0xFFFFFFFF));
		}
#if __BITS == 64
		else if (Width == 8) {
			outl((uint16_t)Address + 4, HIDWORD(Value));
			outl((uint16_t)Address, LODWORD(Value));
		}
#endif
		else {
			//ERROR(" > invalid port width %u for writing", Width);
			return -1;
		}
	} break;

	default: {
		//FATAL(FATAL_SCOPE_KERNEL, " > invalid direct io write type %u", Type);
	} break;
	}
	return 0;
}


uint8_t
CmosRead(_In_ uint8_t Register)
{
	// Variables
	size_t Storage = 0;
	uint8_t Tmp = 0;

	// Keep NMI if disabled
	ReadDirectIo(DeviceIoPortBased, CMOS_IO_BASE + CMOS_IO_SELECT, 1, &Storage);
	Storage &= CMOS_NMI_BIT;
	Tmp = Storage & 0xFF;

	// Select Register (but do not change NMI)
	WriteDirectIo(DeviceIoPortBased, CMOS_IO_BASE + CMOS_IO_SELECT, 1,
		(Tmp | (Register & CMOS_ALLBITS_NONMI)));
	ReadDirectIo(DeviceIoPortBased, CMOS_IO_BASE + CMOS_IO_DATA, 1, &Storage);
	Tmp = Storage & 0xFF;
	return Tmp;
}

/* CmosGetTime
* Retrieves the current time and stores it in
* the c-library time structure */
void CmosGetTime(_Out_ struct tm* Time)
{
	// Variables
	int Sec, Counter;
	uint8_t Century = 0;

	// Do we support century?
	/*if (CmosUnit.AcpiCentury != 0) {
		Century = CmosRead(CmosUnit.AcpiCentury);
	}*/

	// Get Clock (Stable, thats why we loop)
	while (CmosRead(CMOS_REGISTER_SECONDS) != Time->tm_sec
		|| CmosRead(CMOS_REGISTER_MINUTES) != Time->tm_min
		|| CmosRead(CMOS_REGISTER_HOURS) != Time->tm_hour
		|| CmosRead(CMOS_REGISTER_DAYS) != Time->tm_mday
		|| CmosRead(CMOS_REGISTER_MONTHS) != Time->tm_mon
		|| CmosRead(CMOS_REGISTER_YEARS) != Time->tm_year) {
		// Reset variables
		Sec = -1;
		Counter = 0;

		// Update Seconds
		while (Counter < 2) {
			if (CmosRead(CMOS_REGISTER_STATUS_A) & CMOSA_UPDATE_IN_PROG) {
				continue;
			}
			Time->tm_sec = CmosRead(CMOS_REGISTER_SECONDS);

			// Seconds changed.  First from -1, then because the
			// clock ticked, which is what we're waiting for to
			// get a precise reading.
			if (Time->tm_sec != Sec) {
				Sec = Time->tm_sec;
				Counter++;
			}
		}

		// Read the other registers.
		Time->tm_min = CmosRead(CMOS_REGISTER_MINUTES);
		Time->tm_hour = CmosRead(CMOS_REGISTER_HOURS);
		Time->tm_mday = CmosRead(CMOS_REGISTER_DAYS);
		Time->tm_mon = CmosRead(CMOS_REGISTER_MONTHS);
		Time->tm_year = CmosRead(CMOS_REGISTER_YEARS);
	}

	// Convert time format? 
	// - Convert BCD to binary (default RTC mode).
	if (!(CmosRead(CMOS_REGISTER_STATUS_B) & CMOSB_BCD_FORMAT)) {
		Time->tm_year = CMOS_BCD_TO_DEC(Time->tm_year);
		Time->tm_mon = CMOS_BCD_TO_DEC(Time->tm_mon);
		Time->tm_mday = CMOS_BCD_TO_DEC(Time->tm_mday);
		Time->tm_hour = CMOS_BCD_TO_DEC(Time->tm_hour);
		Time->tm_min = CMOS_BCD_TO_DEC(Time->tm_min);
		Time->tm_sec = CMOS_BCD_TO_DEC(Time->tm_sec);
		if (Century != 0) {
			Century = CMOS_BCD_TO_DEC(Century);
		}
	}

	// Counts from 0
	Time->tm_mon--;

	// Correct the year
	if (Century != 0) {
		Time->tm_year += Century * 100;
	}
	else {
		Time->tm_year += (CMOS_CURRENT_YEAR / 100) * 100;
		if (Time->tm_year < CMOS_CURRENT_YEAR) {
			Time->tm_year += 100;
		}
	}
}

extern int GetSystemTime(tm* lpTime);
extern bool GetMilliTime(bigtime_t& milliTime);
// Speed of the machine in MHz.  This is currently hard coded for my test machine.
// It needs to be calculated at boot time.
static bigtime_t timeBase = 150;
/*extern "C" bigtime_t SystemTime()
{
return rdtsc() / timeBase;
}*/
extern "C" bigtime_t SystemTime()
{
	//SYSTEMTIME time;
	//GetLocalTime(&time);
	//LONG time_ms = (time.wSecond * 1000) + time.wMilliseconds;

	bigtime_t milliTime;
	GetMilliTime(milliTime);
	return milliTime;
}

void PrintCurrentTime()
{
	SYSTEMTIME time;
	kGetLocalTime(&time);

	char buffer[256];
	sprintf(buffer, "Current Time : %d/%d/%d %d:%d:%d\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	kprintf("%s", buffer);
}


extern void CmosGetTime(_Out_ struct tm* Time);

int yisleap(int year)
{
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int get_yday(int mon, int day, int year)
{
	static const int days[2][13] = {
		{ 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },
		{ 0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 }
	};
	int leap = yisleap(year);

	return days[leap][mon] + day;
}

extern "C" int kGetSystemTime(tm* lpTime)
{
	SYSTEMTIME sysTime;
	memset(lpTime, 0, sizeof(tm));


#if SKY_EMULATOR
	g_platformAPI._processInterface.sky_GetLocalTime(&sysTime);
#else
	kGetLocalTime(&sysTime);
	//CmosGetTime(lpTime);
#endif

	lpTime->tm_sec = sysTime.wSecond;
	lpTime->tm_min = sysTime.wMinute;
	lpTime->tm_hour = sysTime.wHour;
	lpTime->tm_mday = sysTime.wDay;
	lpTime->tm_mon = sysTime.wMonth;
	lpTime->tm_year = sysTime.wYear;
	lpTime->tm_wday = sysTime.wDayOfWeek;

	//int day = get_yday(sysTime.wMonth, sysTime.wDay, sysTime.wYear);
	//lpTime->tm_yday = day;

	lpTime->tm_isdst = 0;
	lpTime->tm_gmtoff = 0;
	lpTime->tm_zone = 0;

	return 0;
}

extern "C" int kGetTime(time_t* lptime)
{
#if SKY_EMULATOR
	g_platformAPI._processInterface.sky_Time(lptime);
#else
	time(lptime);
#endif

	return 0;
}