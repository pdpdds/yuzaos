#include <stdio.h>
#include <fcntl.h>
#include <FileManager.h>
#include <errno.h>
#include <unistd.h>

int open(const char* filename, int flags, ...)
{
	mode_t mode = 0;

	if (flags & O_CREAT) 
	{
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}

	if (flags & O_CLOEXEC)
		assert(0);

	if (strcmp(filename, "/") == 0) {
		return -1;
	}

	struct stat info;
	int res = stat(filename, &info);
	int fd = -1;
	FILE* fp = 0;

	if (res < 0) 
	{
		if (flags & O_CREAT) 
		{


			fp = g_pFileManager->OpenFile(filename, "w");

			if(!fp)
				return -ENOENT;

			flags &= ~O_CREAT;
			g_pFileManager->CloseFile(fp);
		}
		else
			return -ENOENT;
	}

	if (res == 0 && S_ISDIR(info.st_mode))
	{
		return -EISDIR;
	}

	char fopenFlag[5] = { 0, };
	if (flags & O_RDONLY)
	{
		fp = g_pFileManager->OpenFile(filename, "r");
		strcpy(fopenFlag, "r");
	}
	else if (flags & O_WRONLY)
	{
		fp = g_pFileManager->OpenFile(filename, "w+");
		strcpy(fopenFlag, "w+");
	}
	else if (flags & O_APPEND)
	{
		fp = g_pFileManager->OpenFile(filename, "ab");
		strcpy(fopenFlag, "ab");
	}
	else if (flags & O_TRUNC && ((flags & O_ACCESS) == O_RDWR || (flags & O_ACCESS) == O_WRONLY))
	{
		fp = g_pFileManager->OpenFile(filename, "rw");
		if (!fp)
			assert(0);

		g_pFileManager->ftruncate(fp);
		g_pFileManager->CloseFile(fp);
		strcpy(fopenFlag, "rw");
		fp = g_pFileManager->OpenFile(filename, fopenFlag);
	}
	else if (flags & O_RDWR)
	{
		fp = g_pFileManager->OpenFile(filename, "rw+");
		strcpy(fopenFlag, "rw+");
	}
	

	if (fp == 0) 
		return -ENOENT;

	if (stat(filename, &info) < 0)
	{
		g_pFileManager->CloseFile(fp);
		return -ENOENT;
	}

	fd = g_pFileManager->AddFile(fp);

	return fd;
}