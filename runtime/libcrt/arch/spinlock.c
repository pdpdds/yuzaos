#include <minwindef.h>
#include <_spinlock.h>
#include <crtdefs.h>
#include <assert.h>
#include <SystemCall_Impl.h>

/* Externs
 * Access to platform specifics */
__EXTERN int _spinlock_acquire(Spinlock_t *Spinlock);
__EXTERN int _spinlock_test(Spinlock_t *Spinlock);
__EXTERN void _spinlock_release(Spinlock_t *Spinlock);

/* SpinlockReset
 * This initializes a spinlock handle and sets it to default value (unlocked) */
int 
SpinlockReset(
	_In_ Spinlock_t *Lock)
{
	ASSERT(Lock != NULL);
	Lock->Value         = 0;
    Lock->References    = 0;
    Lock->Owner         = UUID_INVALID;
	return 0;
}

/* SpinlockAcquire
 * Acquires the spinlock while busy-waiting for it to be ready if neccessary */
int
SpinlockAcquire(
	_In_ Spinlock_t *Lock)
{
	ASSERT(Lock != NULL);

	DWORD currentThreadId = Syscall_GetCurrentThreadId();
    // Reentrancy
    if (Lock->Owner == currentThreadId) {
        Lock->References++;
        return 0;
    }

    // Value is updated by _acquire
	if (!_spinlock_acquire(Lock)) {
        return -1;
    }
    Lock->Owner         = currentThreadId;
    Lock->References    = 1;
    return 0;
}

/* SpinlockTryAcquire
 * Makes an attempt to acquire the spinlock without blocking */
int
SpinlockTryAcquire(
	_In_ Spinlock_t *Lock)
{
	ASSERT(Lock != NULL);

    // Reentrancy
    if (Lock->Owner == Syscall_GetCurrentThreadId()) {
        Lock->References++;
        return 0;
    }

    // Value is updated by _acquire
	if (!_spinlock_test(Lock)) {
        return -1;
    }
    Lock->Owner         = Syscall_GetCurrentThreadId();
    Lock->References    = 1;
    return 0;
}

/* SpinlockRelease
 * Releases the spinlock, and lets other threads access the lock */
int 
SpinlockRelease(
	_In_ Spinlock_t *Lock)
{
	ASSERT(Lock != NULL);

    // Reduce the number of references
    Lock->References--;
    if (Lock->References == 0) {
        Lock->Owner = UUID_INVALID;
        _spinlock_release(Lock);
    }
	return 0;
}
