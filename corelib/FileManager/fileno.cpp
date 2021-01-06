#include <stdio.h>
#include <FileManager.h>

int fileno(FILE* fp)
{
	return g_pFileManager->GetFd(fp);
}