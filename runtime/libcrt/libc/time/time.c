#include <orangeos.h>
#include <time.h>
#include <stddef.h>
#include <stringdef.h>
#include <../include/SystemCall_Impl.h>

extern time_t mktime(struct tm *tim_p);

time_t time(time_t *timer)
{	
	struct tm TimeStruct;
	time_t Converted = 0;

	if (Syscall_GetSystemTime(&TimeStruct) != 0) 
	{
		return 0;
	}

//#endif 
	Converted = mktime(&TimeStruct);
	if (timer != NULL) {
		*timer = Converted;
	}
	return Converted;
 	
}
extern int timespec_get(struct timespec* ts, int base);
int clock_gettime(int clock_id, struct timespec* tp)
{

	timespec_get(tp, TIME_THREAD);

	return 0;
}
