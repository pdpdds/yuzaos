#include <SystemAPI.h>
#include <assert.h>
#include "Timer.h"
#include "cpu_asm.h"
#include "TestTimer.h"

extern int test_num;
extern int interrupts_received;
extern bigtime_t lastPeriodic;
const bigtime_t periodicInterval = 500;

void test_timers()
{
	printf("Running timer tests\n");
	
	// 1. Test cancelling a timer
	test_num = 1;
	TestTimer timer1;
	timer1.SetTimeout(SystemTime() + 100000, kOneShotTimer);
	if (timer1.CancelTimeout() != true)
		kPanic("failed to cancel timer #1\n");
	
	test_num = -1;
	kSleep(2000);
	printf("TIMER TEST #1 PASSED\n");

	// 2. Test cancelling a timer that has already expired
	test_num = 2;
	TestTimer timer2;
	timer2.SetTimeout(SystemTime() + 100, kOneShotTimer);
	kSleep(1000);
	if (timer2.CancelTimeout() != false)
		kPanic("Cancelled expired timer!\n");

	// Make sure we don't get spurious interrupts
	test_num = -1;
	kSleep(2000);
	
	printf("TIMER TEST #2 PASSED\n");
	
	// 3. Test 3 code paths for inserting timers.
	test_num = 3;
	interrupts_received = 0;
	TestTimer timer3;
	TestTimer timer4;
	TestTimer timer5;
	bigtime_t now = SystemTime();
	timer3.SetTimeout(now + 100, kOneShotTimer);	// Only element
	timer4.SetTimeout(now + 500, kOneShotTimer);	// New head of queue
	timer5.SetTimeout(now + 750, kOneShotTimer);	// traverse queue
	kSleep(2000);
	if (interrupts_received != 3)
		kPanic("Interrupt insertion failed\n");

	// Make sure we don't get spurious interrupts
	test_num = -1;
	kSleep(2000);
			

	// 4. Test periodic timer
	test_num = 4;
	interrupts_received = 0;
	TestTimer timer6;
	lastPeriodic = SystemTime();
	timer6.SetTimeout(periodicInterval, kPeriodicTimer);
	kSleep(periodicInterval * 10 + periodicInterval / 2);
	if (timer6.CancelTimeout() != true)
		kPanic("failed to cancel periodic timer\n");
		
	test_num = -1;
	if (interrupts_received != 10)
		kPanic("received wrong number of periodic interrupts\n");

	// Make sure we don't get spurious interrupts
	kSleep(2000);

	printf("TIMER TESTS PASSED\n");		
}

