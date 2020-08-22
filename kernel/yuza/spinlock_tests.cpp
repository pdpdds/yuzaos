#include <SpinLock.h>

void spinlock_tests()
{
	SpinLock spinlock;
	spinlock.Lock();
	spinlock.Unlock();
}