#include <stdio.h>
#include <FileManager.h>

size_t read(int fd, void* buf, size_t count)
{
	FILE* fp = g_pFileManager->GetFile(fd);
	if (fp == 0)
		return 0;

	size_t readLength = g_pFileManager->ReadFile(fp, (unsigned char*)buf, 1, count);

	return readLength;
}