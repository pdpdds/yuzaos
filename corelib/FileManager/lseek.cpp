#include <stdio.h>
#include <FileManager.h>

int lseek(int fd, int offset, int whence)
{
	long ret = 0;
	FILE* fp = g_pFileManager->GetFile(fd);

	if (fp == 0)
		return -1;

	ret = g_pFileManager->fseek(fp, offset, whence);

	return ret;
}