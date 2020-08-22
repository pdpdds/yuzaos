#pragma once

#include <windef.h>
#include <osdefs.h>
#include <crtdefs.h>

/* Spinlock Definitions
 * The definition of a spinlock handle used for primitive lock access */
typedef struct _Spinlock {
    int                 Value;
    UUId_t              Owner;
    int                 References;
} Spinlock_t;
#define SPINLOCK_INIT   { 0 }



_CODE_BEGIN
/* SpinlockReset
 * This initializes a spinlock handle and sets it to default value (unlocked) */
CRTDECL( 
int,
SpinlockReset(
	_In_ Spinlock_t *Lock));

/* SpinlockAcquire
 * Acquires the spinlock while busy-waiting for it to be ready if neccessary */
CRTDECL( 
	int,
SpinlockAcquire(
	_In_ Spinlock_t *Lock));

/* SpinlockTryAcquire
 * Makes an attempt to acquire the spinlock without blocking */
CRTDECL( 
	int,
SpinlockTryAcquire(
	_In_ Spinlock_t *Lock));

/* SpinlockRelease
 * Releases the spinlock, and lets other threads access the lock */
CRTDECL( 
	int,
SpinlockRelease(
	_In_ Spinlock_t *Lock));
_CODE_END
