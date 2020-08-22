#include "TestTimer.h"
#include <orangeos.h>
#include <SystemAPI.h>
#include "Timer.h"
#include "cpu_asm.h"


int test_num = -1;
int interrupts_received = 0;
bigtime_t lastPeriodic = 0;
const bigtime_t periodicInterval = 500000;



TestTimer::TestTimer()
{
}


TestTimer::~TestTimer()
{
}

InterruptStatus TestTimer::HandleTimeout()
{
	printf("*\n");
	//ASSERT(!INTERRUPTS_ON());
	switch (test_num) {
	case -1:
		kPanic("spurious timer interrupt\n");
		break;

	case 1:
		kPanic("cancel failed!\n");
		break;

	case 2:
		printf("Test #2: got interrupt correctly\n");
		break;

	case 3:
		interrupts_received++;
		printf(".");
		break;

	case 4: {
		bigtime_t now = SystemTime();
		ASSERT(now > lastPeriodic);

		bigtime_t late = (now - lastPeriodic) - periodicInterval;
		if (late > 1000000) 
		{
			kPanic("periodic timer was %q us late\n", late);
		}

		if (late < -1000000) 
		{
			kPanic("periodic timer was %q us too early\n", -late);
		}

		lastPeriodic = now;
		interrupts_received++;
		break;
	}

	default:
		kPanic("bad timer test num %d\n", test_num);
	}

	return InterruptStatus::kHandledInterrupt;
}
