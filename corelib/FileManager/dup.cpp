#include <stdio.h>
#include <FileManager.h>

int dup(int fd) 
{
	FILE* fp = g_pFileManager->GetFile(fd);
	if (fp == 0)
		return -1;

	int new_fd = g_pFileManager->AddFile(fp);
	fp->_referenceCount++;
	
	return new_fd;
}

int dup2(int old_fd, int new_fd) 
{
    if (old_fd == new_fd) 
    {    
        return old_fd;
    }

	FILE* fp = g_pFileManager->GetFile(old_fd);

	if (fp == 0)
		return -1;

	g_pFileManager->RemoveFile(new_fd);
	g_pFileManager->AddFile(new_fd, fp);
	fp->_referenceCount++;

	return new_fd;
}