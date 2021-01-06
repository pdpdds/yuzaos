#include <stdio.h>
#include <FileManager.h>

ssize_t write(int fd, const void* buf, size_t count)
{
	FILE* fp = g_pFileManager->GetFile(fd);
	int ret = g_pFileManager->WriteFile(fp, (unsigned char*)buf, 1, count);
	return ret;
}