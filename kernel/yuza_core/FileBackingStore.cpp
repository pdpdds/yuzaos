#include "FileBackingStore.h"
#include <string>
#include <stdio.h>
#include <ktypes.h>
#include <StackTracer.h>
#include <SystemAPI.h>

FileBackingStore::FileBackingStore(HANDLE fileDesc)
	: m_fileDesc(fileDesc)
{
}


FileBackingStore::~FileBackingStore()
{
}

bool FileBackingStore::HasPage(off_t offset)
{
	return true;
}

int FileBackingStore::Read(off_t offset, void* va, int)
{
	int res = fseek((FILE*)m_fileDesc, offset, SEEK_SET);

	if (res != 0)
	{
		kprintf("FileBackingStore Seek Fail %d %d\n", res, offset);
		for (;;);
	}

	res = fread(va, 1, PAGE_SIZE, (FILE*)m_fileDesc);

	return E_NO_ERROR;
}

int FileBackingStore::Write(off_t offset, const void* va)
{
	return E_ERROR;
}

off_t FileBackingStore::Commit(off_t size)
{
	return size;
}
