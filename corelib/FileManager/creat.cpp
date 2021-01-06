#include <stdio.h>
#include <fcntl.h>
#include <FileManager.h>

int creat(const char* filename, int mode)
{
	return open(filename, O_CREAT | O_WRONLY | O_TRUNC, mode);
}