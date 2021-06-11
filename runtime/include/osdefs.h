#pragma once


/* Operation System types below
* these are usually fixed no matter arch and include stuff
* as threading, processing etc */
typedef unsigned int                IntStatus_t;
typedef size_t                      UUId_t;
typedef unsigned int                Flags_t;
typedef unsigned                    DevInfo_t;
typedef void*                       Handle_t;

/* Define some special UUId_t constants
* Especially a constant for invalid */
#define UUID_INVALID                (UUId_t)-1

#if defined(SKYOS32)
#define __BITS                      32
#define __MASK                      0xFFFFFFFF
#endif

typedef struct _DataKey {
	union {
		int     Integer;
		UUId_t  Id;
		struct {
			const char* Pointer;
			size_t      Length;
		} String;
	} Value;
} DataKey_t;

/* This enumeration denotes
* the kind of key that is to be interpreted by the data-structure */
typedef enum _KeyType {
	KeyInteger,
	KeyId,
	KeyString
} KeyType_t;

/* SafeMemoryLock_t
* Custom implementation that is available for the different data-structures in
* the libds. */
typedef struct _SafeMemoryLock {
	bool     SyncObject;
	unsigned        Flags;
} SafeMemoryLock_t;

#define FSEC_PER_NSEC                           1000000L
#define NSEC_PER_MSEC                           1000L
#define MSEC_PER_SEC                            1000L
#define NSEC_PER_SEC                            1000000000L
#define FSEC_PER_SEC                            1000000000000000LL
