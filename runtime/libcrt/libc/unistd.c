#include "unistd.h"
#include <SystemCall_Impl.h>
#include <string.h>
#include <FileService.h>

/*int stat(const char* path, struct stat* buf)
{
	return 0;
}*/

int access(const char *pathname, int mode)
{
	struct stat info;
	if (fstat(pathname, &info) == 0)
		return 0;

	return -1;
}


int exit(int errorCode)
{
	return Syscall_exit(errorCode);
}



