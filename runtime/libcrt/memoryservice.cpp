#include <minwindef.h>
#include "memory.h"
#include <systemcall_impl.h>

extern "C" void* calloc(size_t nmemb, size_t size)
{
	return (void*)Syscall_Calloc(nmemb, size);
}

extern "C" void* realloc(void* ptr, size_t size)
{
	if (size == 0)
		return 0;

	return (void*)Syscall_Realloc(ptr, size);
}

void *operator new(size_t size)
{
	return	Syscall_Malloc(size);
}

void* __cdecl operator new[](size_t size)
{
	return (void*)Syscall_Malloc(size);
}

void *operator new(size_t, void *p)
{
	return p;
}

void *operator new[](size_t, void *p)
{
	return p;
}


void operator delete[](void *p, size_t size)
{
	Syscall_Free(p);
}

void __cdecl operator delete(void *p)
{
	Syscall_Free(p);
}

void operator delete(void *p, size_t size)
{
	Syscall_Free(p);
}

void operator delete[](void *p)
{
	Syscall_Free(p);
}

extern "C" void* malloc(size_t size)
{
	return (void*)Syscall_Malloc(size);
}

void* malloc_aligned(size_t size, size_t alignment)
{
	return (void*)Syscall_Malloc_Aligned(size, alignment);
}

extern "C" void free(void *p)
{
	if (p == 0)
		return;

	Syscall_Free(p);
}

extern "C" void abort()
{
	Syscall_Panic("error");
}


int __cdecl _purecall()
{
	return 0;
}
