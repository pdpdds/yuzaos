#include <stdio.h>
#include <FileManager.h>

int close(int fd)
{
	FILE* fp = g_pFileManager->GetFile(fd);
	int ret = g_pFileManager->CloseFile(fp);
	g_pFileManager->RemoveFile(fd);
	return 0;
}