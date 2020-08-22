#include "cmd.h"

static DWORD g_lasterror = 0;

#define EOVERFLOW       132

DWORD WINAPI SetLastErrno( void )
{
	DWORD error = ERROR_SUCCESS;
	switch (errno) {
	case 0:
		error = ERROR_SUCCESS;
		break;
	case EFAULT:
	  	error = ERROR_INVALID_ADDRESS;
	  	break;
	case EACCES:
	    error = ERROR_ACCESS_DENIED;
	    break;
	case EDQUOT:
	    error = ERROR_DISK_FULL;
	    break;
	case EEXIST:
	    error = ERROR_FILE_EXISTS;
	    break;
	case EIO:
	    error = ERROR_GEN_FAILURE;
	    break;
	case ELOOP:
	    error = ERROR_STOPPED_ON_SYMLINK;
	    break;
	case ENAMETOOLONG:
	    error = ERROR_BAD_PATHNAME;
	    break;
	case ENOENT:
	    error = ERROR_FILE_NOT_FOUND;
	    break;
	case ENOMEM:
	    error = ERROR_OUTOFMEMORY;
	    break;
	case ENOTDIR:
	    error = ERROR_INVALID_NAME;
	    break;
	case ENOSPC:
	    error = ERROR_DISK_FULL;
	    break;
	case EPERM:
	    error = ERROR_GEN_FAILURE;
	    break;
	case EOVERFLOW:
	    error = ERROR_BUFFER_OVERFLOW;
	    break;
	default:
		abort();
	  	error = ERROR_INVALID_FUNCTION;
	}
	SetLastError(error);
	return error;
}

DWORD WINAPI GetLastError(void) {
  return g_lasterror;
}

void WINAPI SetLastError(
  _In_ DWORD dwErrCode
) {
  g_lasterror = dwErrCode;
}