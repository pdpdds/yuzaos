#pragma once
#include <windef.h>
#include "minwindef.h"
#include <minwinbase.h>

#ifdef _MSC_VER
	typedef __int64	int64;
	typedef unsigned __int64 uint64;
	#define PACKED
	#define NORETURN
#endif

typedef int object_id;
typedef unsigned long mode_t;

#define OS_NAME_LENGTH 30
#define INFINITE_TIMEOUT (((int64) 0x7fffffff << 32) | (int64) 0xffffffff)

#define PAGE_SIZE 0x1000
#define INVALID_PAGE 0xffffffff

// Page protection
typedef uint PageProtection;

#define USER_READ 1
#define USER_WRITE 2
#define USER_EXEC 4
#define SYSTEM_READ 8
#define SYSTEM_WRITE 16
#define SYSTEM_EXEC 32


// Create area flags
#define SEARCH_FROM_TOP 0
#define SEARCH_FROM_BOTTOM 2
#define EXACT_ADDRESS 4

// Map file flags
#define MAP_PRIVATE 8
#define MAP_PHYSMEM 16

typedef enum AreaWiring 
{
	AREA_NOT_WIRED,
	AREA_WIRED
} AreaWiring;


typedef enum StatType 
{
	ST_DIRECTORY,
	ST_FILE
} StatType;

typedef int (*THREAD_START_ENTRY)(void*);

typedef enum ResourceType 
{
	OBJ_ANY = -1,
	OBJ_SEMAPHORE,
	OBJ_TEAM,
	OBJ_THREAD,
	OBJ_AREA,
	OBJ_FD,
	OBJ_IMAGE,
	OBJ_MUTEX,
	OBJ_SPINLOCK,
	OBJ_TIMER,
} ResourceType;

// Wait flags
typedef enum WaitFlags 
{
	WAIT_FOR_ONE = 0,
	WAIT_FOR_ALL = 1
} WaitFlags;


/* Error codes */
#define E_NO_ERROR 0
#define E_ERROR -1
#define E_TIMED_OUT -2
#define E_BAD_HANDLE -3
#define E_BAD_OBJECT_TYPE -4
#define E_NOT_ALLOWED -5
#define E_BAD_ADDRESS -6
#define E_WAIT_COLLISION -7
#define E_IO -8
#define E_NO_SUCH_FILE -9
#define E_NOT_DIR -10
#define E_INVALID_OPERATION -11
#define E_ENTRY_EXISTS -12
#define E_NO_MEMORY -13
#define E_NOT_IMAGE -14
#define E_INTERRUPTED -15
#define E_WOULD_BLOCK -16



