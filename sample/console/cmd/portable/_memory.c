#include "cmd.h"

#include <assert.h>

// Win32 Equivalents for C Run-Time Functions
// https://support.microsoft.com/en-gb/kb/99456

HANDLE WINAPI GetProcessHeap(void) {
	return NULL;
}

BOOL WINAPI HeapFree(
  HANDLE hHeap,
  DWORD  dwFlags,
  LPVOID lpMem
) {
	assert( hHeap == NULL );
	assert( dwFlags == 0 );
	free(lpMem);
	return TRUE;
}

LPVOID WINAPI HeapReAlloc(
  HANDLE hHeap,
  DWORD  dwFlags,
  LPVOID lpMem,
  size_t dwBytes
) {
	assert( hHeap == NULL );
	assert( dwFlags == 0 );
	return realloc(lpMem, dwBytes);
}

LPVOID WINAPI HeapAlloc(
  HANDLE hHeap,
  DWORD  dwFlags,
  size_t dwBytes
) {
	assert( hHeap == NULL );
  void *data = malloc(dwBytes);
	if( dwFlags == HEAP_ZERO_MEMORY ) {
    memset(data, 0, dwBytes);
  }
  else {
    assert( dwFlags == 0 );
  }
	return data;
}

LPVOID WINAPI VirtualAlloc(
  LPVOID lpAddress,
  SIZE_T dwSize,
  DWORD  flAllocationType,
  DWORD  flProtect
) {
	assert( lpAddress == NULL );
	assert( flAllocationType == MEM_COMMIT );
	assert( flProtect == PAGE_READWRITE );
	return malloc(dwSize);
}


BOOL WINAPI VirtualFree(
  LPVOID lpAddress,
  SIZE_T dwSize,
  DWORD  dwFreeType
) {
	assert( lpAddress != NULL );
	assert( dwSize == 0 );
	assert( dwFreeType == MEM_RELEASE );
	free(lpAddress);
}
