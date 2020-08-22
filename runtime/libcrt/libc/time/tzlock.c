

#include <_spinlock.h>
#include "local.h"

/* Declare the lock */
Spinlock_t __GlbTzLock = SPINLOCK_INIT;

/* Grab tz lock */
void __tz_lock(void) {
	SpinlockAcquire(&__GlbTzLock);
}

/* Release tz lock */
void __tz_unlock(void) {
	SpinlockRelease(&__GlbTzLock);
}
