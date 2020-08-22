#ifndef _MEMORY_H_
#define _MEMORY_H_

#define HEAP_ZERO_MEMORY 0x00000008

typedef struct _MEMORY_BASIC_INFORMATION {
  PVOID  BaseAddress;
  PVOID  AllocationBase;
  DWORD  AllocationProtect;
  SIZE_T RegionSize;
  DWORD  State;
  DWORD  Protect;
  DWORD  Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

#define MEM_COMMIT 0x00001000
#define MEM_RESERVE 0x00002000
#define MEM_RESET 0x00080000
#define MEM_RELEASE 0x8000

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa366786(v=vs.85).aspx
#define PAGE_NOACCESS 0x01
#define PAGE_READONLY 0x02
#define PAGE_READWRITE 0x04


HANDLE WINAPI GetProcessHeap(void);

BOOL WINAPI HeapFree(
  HANDLE hHeap,
  DWORD  dwFlags,
  LPVOID lpMem
);

LPVOID WINAPI HeapAlloc(
  HANDLE hHeap,
  DWORD  dwFlags,
  size_t dwBytes
);

LPVOID WINAPI HeapReAlloc(
  HANDLE hHeap,
  DWORD  dwFlags,
  LPVOID lpMem,
  size_t dwBytes
);

LPVOID WINAPI VirtualAlloc(
  LPVOID lpAddress,
  SIZE_T dwSize,
  DWORD  flAllocationType,
  DWORD  flProtect
);


BOOL WINAPI VirtualFree(
  LPVOID lpAddress,
  SIZE_T dwSize,
  DWORD  dwFreeType
);


#endif
