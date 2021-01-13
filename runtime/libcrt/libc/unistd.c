#include "unistd.h"
#include <SystemCall_Impl.h>
#include <string.h>
#include <FileService.h>

int access(const char *pathname, int mode)
{
	struct stat info;
	if (stat(pathname, &info) == 0)
		return 0;

	return -1;
}

int exit(int errorCode)
{
	return Syscall_exit(errorCode);
}



int chmod(const char* filename, int pmode)
{
	return 0;
}

int lstat(const char* filename, struct stat* buf)
{
	struct stat info;
	if (stat(filename, &info) == 0)
		return 0;

	return -1;

}

unsigned int getpid()
{
	return Syscall_GetCurrentProcessId();
}

unsigned int getppid()
{
	return 0;
}

int gethostname(char* name, size_t len)
{
	if (len < 5)
		return -1;

	strcpy(name, "YUZA");
	return 0;
}

//not implemented
int sethostname(const char* name, size_t len)
{
	return -1;
}

int utime(const char* filename, struct utimbuf* buf)
{
	return -1;
}

void perror(const char* str)
{
	printf("%s", str);
}
