#include <memory.h>

void *AllocPtr(int nSize)
{
	void *p = malloc(nSize);
	nSize = _msize(p);
	memset(p, 0, nSize);
	return p;
}

void *ReAllocPtr(void *p, int nSize)
{
	int nOldSize = _msize(p);
	p = realloc(p, nSize);
	if(nSize > nOldSize)
	{
		nSize = _msize(p);
		memset((char*)p+nOldSize, 0, nSize-nOldSize);
	}
	return p;
}

void FreePtr(void *p)
{
	free(p);
}

