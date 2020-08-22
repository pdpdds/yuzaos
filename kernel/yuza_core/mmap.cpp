/*
 *  mmap.c
 *
 *  copyright (c) 2018 Xiongfei Shi
 *
 *  author: Xiongfei Shi <jenson.shixf(a)gmail.com>
 *  license: MIT
 */

#include <windef.h>
#include <errno.h>
#include <stdio.h>
#include "mmap.h"
#include "LoadDLL.h"
#include <SystemAPI.h>
#include <Thread.h>
#include <Team.h>
#include <SystemCall.h>
#include <Area.h>

#define PROT_NONE       0
#define PROT_READ       1
#define PROT_WRITE      2
#define PROT_EXEC       4

#define MAP_FILE        0
#define MAP_SHARED      1
#define MAP_PRIVATE     2
#define MAP_TYPE        0x0F
#define MAP_FIXED       0x10
#define MAP_ANONYMOUS   0x20
#define MAP_ANON        MAP_ANONYMOUS

#define MAP_FAILED      ((void *)-1)

#define SECTION_QUERY                0x0001
#define SECTION_MAP_WRITE            0x0002
#define SECTION_MAP_READ             0x0004
#define SECTION_MAP_EXECUTE          0x0008
#define SECTION_EXTEND_SIZE          0x0010
#define SECTION_MAP_EXECUTE_EXPLICIT 0x0020 // not included in SECTION_ALL_ACCESS

#define FILE_MAP_WRITE            SECTION_MAP_WRITE
#define FILE_MAP_READ             SECTION_MAP_READ
#define FILE_MAP_ALL_ACCESS       SECTION_ALL_ACCESS


#define MS_ASYNC            1
#define MS_SYNC             2
#define MS_INVALIDATE       4

#ifndef FILE_MAP_EXECUTE
# define FILE_MAP_EXECUTE   0x0020
#endif

static int _mmap_error(DWORD err, int deferr) {
    if (0 == err)
        return deferr;
    return err;
}

static DWORD _mmap_prot_page(int prot) {
    DWORD protect = 0;

    if (PROT_NONE == prot)
        return protect;

    if (prot & PROT_EXEC)
        protect = (prot & PROT_WRITE) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
    else
        protect = (prot & PROT_WRITE) ? PAGE_READWRITE : PAGE_READONLY;

    return protect;
}

static DWORD _mmap_prot_file(int prot) {
    DWORD desiredAccess = 0;

    if (PROT_NONE == prot)
        return desiredAccess;

    if (prot & PROT_READ)
        desiredAccess |= FILE_MAP_READ;

    if (prot & PROT_WRITE)
        desiredAccess |= FILE_MAP_WRITE;

    if (prot & PROT_EXEC)
        desiredAccess |= FILE_MAP_EXECUTE;

    return desiredAccess;
}

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, unsigned long off) {
    HANDLE fm, h;
    void * map = MAP_FAILED;

    DWORD protect = _mmap_prot_page(prot);
    DWORD desiredAccess = _mmap_prot_file(prot);

    DWORD dwFileOffsetHigh = 0;
    DWORD dwFileOffsetLow = (DWORD)off;

    DWORD dwMaxSizeHigh = 0;
    DWORD dwMaxSizeLow = (DWORD)(off + len);

    errno = 0;

    if (!len
        /* Unsupported flag combinations */
        || (flags & MAP_FIXED)
        /* Usupported protection combinations */
        || (PROT_EXEC == prot)) {
        errno = EINVAL;
        return MAP_FAILED;
    }

	/*FileDescriptor* pDesc = (FileDescriptor*)Thread::GetRunningThread()->GetTeam()->GetHandleTable()->GetResource(fildes, OBJ_FD);
	
	if(pDesc == 0)
		return MAP_FAILED;

	int area = CreateFileArea(pDesc->GetName(), pDesc->GetName(), INVALID_PAGE, 0,
		pDesc->GetNode()->GetLength() , EXACT_ADDRESS | MAP_PRIVATE, (USER_READ | USER_WRITE | SYSTEM_READ | SYSTEM_WRITE), *Thread::GetRunningThread()->GetTeam());
	if (area < 0)
	{
		kprintf("Failed to map image file\n");
		return MAP_FAILED;
	}

  Area* pArea = (Area*)Thread::GetRunningThread()->GetTeam()->GetHandleTable()->GetResource(area, -1);
  */
    /*fm = CreateFileMapping(h, NULL, protect, dwMaxSizeHigh, dwMaxSizeLow, NULL);

    if (!fm) {
        errno = _mmap_error(GetLastError(), EPERM);
        return MAP_FAILED;
    }*/

    /*map = MapViewOfFile(fm, desiredAccess, dwFileOffsetHigh, dwFileOffsetLow, len);

    CloseHandle(fm);

    if (!map) {
        errno = _mmap_error(GetLastError(), EPERM);
        return MAP_FAILED;
    }*/

   // return (void*)(pArea->GetBaseAddress() + dwFileOffsetLow);
	return 0;
}

int munmap(void *addr, size_t len) {
    /*if (!UnmapViewOfFile(addr)) {
        errno = _mmap_error(GetLastError(), EPERM);
        return -1;
    }*/
    return 0;
}

int msync(void *addr, size_t len, int flags) {
    /*if (!FlushViewOfFile(addr, len)) {
        errno = _mmap_error(GetLastError(), EPERM);
        return -1;
    }*/
    return 0;
}

extern bool WINAPI VirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD  flNewProtect, PDWORD lpflOldProtect);
int mprotect(void *addr, size_t len, int prot) {
    DWORD newProtect = _mmap_prot_page(prot);
    DWORD oldProtect = 0;

    if (!VirtualProtect(addr, len, newProtect, &oldProtect)) {
        errno = _mmap_error(kGetLastError(), EPERM);
        return -1;
    }
    return 0;
}

int mlock(const void *addr, size_t len) {
    /*if (!VirtualLock((LPVOID)addr, len)) {
        errno = _mmap_error(GetLastError(), EPERM);
        return -1;
    }*/
    return 0;
}

int munlock(const void *addr, size_t len) {
    /*if (!VirtualUnlock((LPVOID)addr, len)) {
        errno = _mmap_error(GetLastError(), EPERM);
        return -1;
    }*/
    return 0;
}
