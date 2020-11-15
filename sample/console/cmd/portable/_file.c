#include "cmd.h"

#include <unistd.h>
#include <assert.h>
//#include <libgen.h>
//#include <sys/statvfs.h>

// Portable implementations of Windows Kernel32 functions
// https://github.com/sduirc/slippage-free/blob/c7ea1f3d69166e7ee61bd7668b58bee5dd8a8d08/Slippage/libc/BFC/filex.h
// https://github.com/sincoder/libx/blob/c09f390e38c0f3b1b91566654ec3c6cf8e113f48/kernel32.c
// https://github.com/paulopina21/plxJukebox-11/blob/193996ac99b99badab3a1d422806942afca2ad01/xbmc/linux/XFileUtils.cpp
// https://github.com/owen200008/ccbasic/blob/398474ac9d31abac09fc0d8ff2dc2bb714efdeef/src/file/file_linux.cpp


static DWORD
ChangeFileAttributes(struct stat st)
{
	DWORD dwAttr = 0;
	if (st.st_mode & S_IFDIR)
	{
		dwAttr |= FILE_ATTRIBUTE_DIRECTORY;
	}
	if (st.st_mode & S_IFCHR)
	{
		dwAttr |= FILE_ATTRIBUTE_DEVICE;
	}
	if (st.st_mode & S_IFREG)
	{
		dwAttr |= FILE_ATTRIBUTE_NORMAL;
	}
	return dwAttr;
}


DWORD
GetFileAttributes(LPCTSTR lpFileName)
{
	struct stat st;
	char* unix_path = DOSPath2UNIXPath(lpFileName);
	int ret = fstat(unix_path, &st);
	free(unix_path);
	if (ret != 0) {
		return SetLastErrno();
	}
	return ChangeFileAttributes(st);
}

BOOL
SetFileAttributes(LPCTSTR lpFileName, DWORD dwFileAttributes)
{
	struct stat st;
	BOOL ret = FALSE;
	char* unix_path = DOSPath2UNIXPath(lpFileName);

	if (fstat(unix_path, &st) == 0)
	{
		DWORD dwThisAttr = 0;
		if (access(unix_path, W_OK) != 0)
		{
			dwThisAttr |= FILE_ATTRIBUTE_READONLY;
		}

		dwFileAttributes &= FILE_ATTRIBUTE_READONLY;
		/*if (dwFileAttributes != dwThisAttr)
		{
			if (dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
				st.st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
			}
			else {
				st.st_mode |= (S_IWUSR | S_IWGRP | S_IWOTH);
			}
			ret = (chmod(unix_path, st.st_mode) == 0);
			if (!ret) {
				SetLastErrno();
			}
		}
		else {*/
			ret = TRUE;
		//}
	}
	else {
		SetLastError(ERROR_FILE_NOT_FOUND);
	}

	free(unix_path);
	return ret;
}

BOOL WINAPI MoveFile(
	_In_ LPCTSTR lpExistingFileName,
	_In_ LPCTSTR lpNewFileName
) {
	char* unix_existing = DOSPath2UNIXPath(lpExistingFileName);
	char* unix_new = DOSPath2UNIXPath(lpNewFileName);
	BOOL ret = !rename(lpExistingFileName, lpNewFileName);
	if (!ret) {
		SetLastErrno();
	}
	free(unix_existing);
	free(unix_new);
	return ret;
}

BOOL WINAPI ReadFile(
	FILE* hFile,
	LPVOID       lpBuffer,
	DWORD        nNumberOfBytesToRead,
	LPDWORD      lpNumberOfBytesRead,
	LPVOID lpOverlapped
) {
	assert(hFile != NULL);
	assert(lpBuffer != NULL);
	DWORD bytes_read = fread(lpBuffer, 1, nNumberOfBytesToRead, hFile);
	if (lpNumberOfBytesRead) {
		*lpNumberOfBytesRead = bytes_read;
	}
	return bytes_read == nNumberOfBytesToRead;
}

DWORD WINAPI GetFileSize(
	FILE* hFile,
	LPDWORD lpFileSizeHigh
) {
	assert(lpFileSizeHigh == NULL);
	int prev = ftell(hFile);
	fseek(hFile, 0L, SEEK_END);
	int sz = ftell(hFile);
	fseek(hFile, prev, SEEK_SET); //go back to where we were
	return sz;
}

BOOL DeleteFile(LPCTSTR lpFileName) {
	char* unix_path = DOSPath2UNIXPath(lpFileName);
	BOOL ret = 0 == unlink(unix_path);
	if (!ret) {
		SetLastErrno();
	}
	free(unix_path);
	return ret;
}

BOOL WINAPI CloseFile(
	FILE* hFile
) {
	return !fclose(hFile);
}



// https://github.com/timob/linapple/blob/956f6765bdd7508945ba642bd6da3b09148516c6/src/wwrapper.cpp
DWORD SetFilePointer(FILE* hFile,
	LONG lDistanceToMove,
	PLONG lpDistanceToMoveHigh,
	DWORD dwMoveMethod) {
	/* ummm,fseek in Russian */
	fseek((FILE*)hFile, lDistanceToMove, dwMoveMethod);
	return ftell((FILE*)hFile);
}

// CreateFile
// https://github.com/bitcrystal/temp/blob/b6b32e4fa9abfa4007320e571b8beced1e353ed9/src/functions_hook/subhook/windows_defs2.c


int _pipe(
	FILE **pfds,
	unsigned int psize,
	int textmode
)
{
	return 0;
	/*int fds[2];
	int ret = pipe(&fds[0]);
	if( ! ret ) {
		pfds[0] = fdopen(fds[0], "rw");
		pfds[1] = fdopen(fds[1], "rw");
	}
	return ret;*/
}

/*
FILE *_dup2( FILE *oldfp, FILE *newfp )
{
	int oldfd, newfd;
	int res;

	if( oldfp==NULL || newfp==NULL ) return NULL;

	oldfd = fileno( oldfp );
	newfd = fileno( newfp );

	res = dup2( oldfd, newfd );

	if( res==-1) return NULL;
	else return newfp;
}


FILE* fdup(FILE* fp,char*mode)
{
	int fd;
	if(fp==NULL)return NULL;
	fd=dup(fileno(fp));
	return fdopen(fd,mode);
}


FILE *_dup( FILE *handle ) {
	return fdup(handle, "rwb");
}*/

// https://github.com/MathewWi/wiiapple/blob/edcab3f1d6e4c007ebfb4856071df4eee1838aaf/wiiapple/source/wwrapper.cpp
BOOL WriteFile(FILE* hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
	LPDWORD lpNumberOfBytesWritten, LPVOID lpOverlapped) {
	/* write something to file */
	DWORD byteswritten = fwrite(lpBuffer, 1, nNumberOfBytesToWrite, (FILE*)hFile);
	*lpNumberOfBytesWritten = byteswritten;
	return (nNumberOfBytesToWrite == byteswritten);
}


FILE* WINAPI GetStdHandle(
	_In_ DWORD nStdHandle
) {
	switch (nStdHandle)
	{
	case STD_INPUT_HANDLE:
		return STDIN;

	case STD_OUTPUT_HANDLE:
		return STDOUT;

	case STD_ERROR_HANDLE:
		return STDERR;

	default:
		abort();
	}
	return NULL;
}

/*BOOL WINAPI SetCurrentDirectory(
	_In_ LPCTSTR lpPathName
) {
	char* unix_path = DOSPath2UNIXPath(lpPathName);
	BOOL ret = !chdir(unix_path);
	if (!ret) {
		SetLastErrno();
	}
	free(unix_path);
	return ret;
}*/

BOOL WINAPI GetBinaryType(
	_In_  LPCTSTR lpApplicationName,
	_Out_ LPDWORD lpBinaryType
) {
	struct stat sb;
	BOOL ret = FALSE;
	char* unix_path = DOSPath2UNIXPath(lpApplicationName);
	if (!fstat(lpApplicationName, &sb)) {
		ret = sb.st_mode & S_IXUSR;
		if (ret && lpBinaryType) {
			*lpBinaryType = SCS_POSIX_BINARY;
		}
		else {
			SetLastError(ERROR_BAD_EXE_FORMAT);
		}
	}
	else {
		SetLastErrno();
	}
	free(unix_path);
	return ret;
}

char* DOSBasename(char* path) {
	char* ptr = strrchr(path, '\\');
	if (ptr && *ptr) {
		return &ptr[1];
	}
	return NULL;
}

char*
realpath(const char* path, char resolved[PATH_MAX])
{
	struct stat sb;
	char* p, * q, * s;
	size_t left_len, resolved_len;
	unsigned symlinks;
	int serrno, slen;
	char left[PATH_MAX], next_token[PATH_MAX], symlink[PATH_MAX];

	serrno = errno;
	symlinks = 0;
	if (path[0] == '/') {
		resolved[0] = '/';
		resolved[1] = '\0';
		if (path[1] == '\0')
			return (resolved);
		resolved_len = 1;
		left_len = strlcpy(left, path + 1, sizeof(left));
	}
	else {
		if (getcwd(resolved, PATH_MAX) == NULL) {
			strlcpy(resolved, ".", PATH_MAX);
			return (NULL);
		}
		resolved_len = strlen(resolved);
		left_len = strlcpy(left, path, sizeof(left));
	}
	if (left_len >= sizeof(left) || resolved_len >= PATH_MAX) {
		errno = ENAMETOOLONG;
		return (NULL);
	}

	/*
	 * Iterate over path components in `left'.
	 */
	while (left_len != 0) {
		/*
		 * Extract the next path component and adjust `left'
		 * and its length.
		 */
		p = strchr(left, '/');
		s = p ? p : left + left_len;
		if (s - left >= sizeof(next_token)) {
			errno = ENAMETOOLONG;
			return (NULL);
		}
		memcpy(next_token, left, s - left);
		next_token[s - left] = '\0';
		left_len -= s - left;
		if (p != NULL)
			memmove(left, s + 1, left_len + 1);
		if (resolved[resolved_len - 1] != '/') {
			if (resolved_len + 1 >= PATH_MAX) {
				errno = ENAMETOOLONG;
				return (NULL);
			}
			resolved[resolved_len++] = '/';
			resolved[resolved_len] = '\0';
		}
		if (next_token[0] == '\0')
			continue;
		else if (strcmp(next_token, ".") == 0)
			continue;
		else if (strcmp(next_token, "..") == 0) {
			/*
			 * Strip the last path component except when we have
			 * single "/"
			 */
			if (resolved_len > 1) {
				resolved[resolved_len - 1] = '\0';
				q = strrchr(resolved, '/') + 1;
				*q = '\0';
				resolved_len = q - resolved;
			}
			continue;
		}

		/*
		 * Append the next path component and lstat() it. If
		 * lstat() fails we still can return successfully if
		 * there are no more path components left.
		 */
		resolved_len = strlcat(resolved, next_token, PATH_MAX);
		if (resolved_len >= PATH_MAX) {
			errno = ENAMETOOLONG;
			return (NULL);
		}
		if (fstat(resolved, &sb) != 0) {
			if (errno == ENOENT && p == NULL) {
				errno = serrno;
				return (resolved);
			}
			return (NULL);
		}
		/*if (S_ISLNK(sb.st_mode)) {
			if (symlinks++ > MAXSYMLINKS) {
				errno = ELOOP;
				return (NULL);
			}
			slen = readlink(resolved, symlink, sizeof(symlink) - 1);
			if (slen < 0)
				return (NULL);
			symlink[slen] = '\0';
			if (symlink[0] == '/') {
				resolved[1] = 0;
				resolved_len = 1;
			}
			else if (resolved_len > 1) {
				// Strip the last path component. 
				resolved[resolved_len - 1] = '\0';
				q = strrchr(resolved, '/') + 1;
				*q = '\0';
				resolved_len = q - resolved;
			}

			if (p != NULL) {
				if (symlink[slen - 1] != '/') {
					if (slen + 1 >= sizeof(symlink)) {
						errno = ENAMETOOLONG;
						return (NULL);
					}
					symlink[slen] = '/';
					symlink[slen + 1] = 0;
				}
				left_len = strlcat(symlink, left, sizeof(left));
				if (left_len >= sizeof(left)) {
					errno = ENAMETOOLONG;
					return (NULL);
				}
			}
			left_len = strlcpy(left, symlink, sizeof(left));
		}*/
	}

	/*
	 * Remove trailing slash except when the resolved pathname
	 * is a single "/".
	 */
	if (resolved_len > 1 && resolved[resolved_len - 1] == '/')
		resolved[resolved_len - 1] = '\0';
	return (resolved);
}

DWORD WINAPI GetFullPathName(
	_In_  LPCTSTR lpFileName,
	_In_  DWORD   nBufferLength,
	_Out_ LPTSTR  lpBuffer,
	_Out_ LPTSTR* lpFilePart
) {
	assert(lpFileName != NULL);
	assert(nBufferLength > 0);
	assert(lpBuffer != NULL);
	DWORD ret = 0;
	char* unix_path = DOSPath2UNIXPath(lpFileName);
	char resolved_path[PATH_MAX];

	char* tmp_dos_path = strdup(lpFileName);
	char* extra_filename = DOSBasename(tmp_dos_path);
	if (extra_filename != NULL && *extra_filename && strchr(extra_filename, '*')) {
		// Filename part contains wildcard
		// Perform basename on directory only
		char extra_tmp = extra_filename[0];
		extra_filename[0] = 0;
		free(unix_path);
		unix_path = DOSPath2UNIXPath(tmp_dos_path);
		extra_filename[0] = extra_tmp;
	}
	else {
		extra_filename = NULL;
	}

	if (realpath(unix_path, resolved_path))
	{
		if (extra_filename) {
			strcat(resolved_path, "\\");
			strcat(resolved_path, extra_filename);
		}
		char* dos_path = UNIXPath2DOSPath(resolved_path);
		strncpy(lpBuffer, dos_path, nBufferLength);
		free(dos_path);
		if (lpFilePart) {
			*lpFilePart = DOSBasename(lpBuffer);
		}
		ret = strlen(lpBuffer);
	}
	else {
		SetLastErrno();
	}
	free(tmp_dos_path);
	free(unix_path);
	return ret;
}

BOOL WINAPI WriteConsole(
	_In_             FILE* hConsoleOutput,
	_In_       const VOID* lpBuffer,
	_In_             DWORD   nNumberOfCharsToWrite,
	_Out_            LPDWORD lpNumberOfCharsWritten,
	_Reserved_       LPVOID  lpReserved
) {
	size_t nwritten = fwrite(lpBuffer, 1, nNumberOfCharsToWrite, hConsoleOutput);
	if (lpNumberOfCharsWritten) {
		*lpNumberOfCharsWritten = nwritten;
	}
	return nwritten == nNumberOfCharsToWrite;
}


BOOL WINAPI CreateDirectory(
	_In_     LPCTSTR               lpPathName,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
) {
	return !mkdir(lpPathName);
}

DWORD WINAPI GetCurrentDirectory(
	_In_  DWORD  nBufferLength,
	_Out_ LPTSTR lpBuffer
) {
	DWORD ret = 0;
	char wd[PATH_MAX];
	getcwd(wd, sizeof(wd));
	char* dos_path = UNIXPath2DOSPath(wd);
	strncpy(lpBuffer, dos_path, nBufferLength);
	ret = strlen(lpBuffer);
	free(dos_path);
	return ret;
}

UINT WINAPI GetWindowsDirectory(
	_Out_ LPTSTR lpBuffer,
	_In_  UINT   uSize
) {
	strncpy(lpBuffer, "C:\\BIN\\", uSize);
	return strlen(lpBuffer);
}

BOOL WINAPI RemoveDirectory(
	_In_ LPCTSTR lpPathName
) {
	return !unlink(lpPathName);
}

UINT WINAPI GetDriveType(
	_In_opt_ LPCTSTR lpRootPathName
) {
	return DRIVE_FIXED;
}

BOOL WINAPI SetFileTime(
	_In_           HANDLE   hFile,
	_In_opt_ const FILETIME* lpCreationTime,
	_In_opt_ const FILETIME* lpLastAccessTime,
	_In_opt_ const FILETIME* lpLastWriteTime
) {
	return FALSE;
}

BOOL WINAPI FlushFileBuffers(
	_In_ FILE* hFile
) {
	fflush(hFile);
	return TRUE;
}

BOOL WINAPI FlushConsoleInputBuffer(
	_In_ FILE* hConsoleInput
) {
	fflush(hConsoleInput);
	return TRUE;
}

BOOL WINAPI GetDiskFreeSpace(
  _In_  LPCTSTR lpRootPathName,
  _Out_ LPDWORD lpSectorsPerCluster,
  _Out_ LPDWORD lpBytesPerSector,
  _Out_ LPDWORD lpNumberOfFreeClusters,
  _Out_ LPDWORD lpTotalNumberOfClusters
) {
	/*struct statvfs sb;
	if( statvfs(lpRootPathName, &sb) )
		return FALSE;

	if( lpSectorsPerCluster )
		*lpSectorsPerCluster = sb.f_frsize / sb.f_bsize;

	if( lpBytesPerSector )
		*lpBytesPerSector = sb.f_bsize;

	if( lpNumberOfFreeClusters )
		*lpNumberOfFreeClusters = sb.f_bfree / (sb.f_frsize / sb.f_bsize);

	if( lpTotalNumberOfClusters )
		*lpTotalNumberOfClusters = sb.f_blocks;
		*/
	return TRUE;
}

const unsigned int bufferSize = 16384;
BOOL WINAPI CopyFile(
	_In_ LPCTSTR src,
	_In_ LPCTSTR dest,
	_In_ BOOL    bFailIfExists
) {
	if (bFailIfExists) {
		struct stat sb;
		if (!fstat(dest, &sb))
			return FALSE;
	}


	char buffer[16384];
	FILE* hSrc = fopen(src, "rb");
	if (!hSrc) return FALSE;
	FILE* hDest = fopen(dest, "wb");
	if (!hDest)
	{
		fclose(hSrc);
		return FALSE;
	}
	BOOL ret = TRUE;
	while (ret)
	{
		int sizeRead = fread(buffer, 1, bufferSize, hSrc);
		if (sizeRead > 0)
		{
			int sizeWritten = fwrite(buffer, 1, sizeRead, hDest);
			if (sizeWritten != sizeRead)
			{
				ret = FALSE;
			}
		}
		else if (sizeRead < 0)
		{
			ret = FALSE;
		}
		else
			break;  // we're done
	}
	fclose(hSrc);
	fclose(hDest);
	return ret;
}


FILE* CreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	TCHAR szOpenFlag[8];
	memset(szOpenFlag, 0, sizeof(szOpenFlag));
	switch (dwCreationDisposition)
	{
	case CREATE_NEW:
	{
		if (access(lpFileName, F_OK) == 0)
		{
			SetLastError(EEXIST);
			return INVALID_HANDLE_VALUE;
		}
	}
	case CREATE_ALWAYS:
	{
		szOpenFlag[0] = 'w';
		break;
	}
	case OPEN_ALWAYS:
	{
		if (access(lpFileName, F_OK) == 0)
		{
			szOpenFlag[0] = 'r';
		}
		else
		{
			szOpenFlag[0] = 'w';
		}
		break;
	}
	case TRUNCATE_EXISTING:
	{
		if (access(lpFileName, F_OK) != 0)
		{
			SetLastError(ENOENT);
			return INVALID_HANDLE_VALUE;
		}
		szOpenFlag[0] = 'w';
		break;
	}
	default:
	case OPEN_EXISTING:
	{
		szOpenFlag[0] = 'r';
		break;
	}
	}
	szOpenFlag[1] = 'b';
	if (dwDesiredAccess & GENERIC_WRITE)
	{
		szOpenFlag[2] = '+';
	}
	FILE* fp = fopen(lpFileName, szOpenFlag);
	if (fp == NULL)
	{
		return INVALID_HANDLE_VALUE;
	}
	return fp;
}

BOOL WINAPI GetVolumeInformation(
	_In_opt_  LPCTSTR lpRootPathName,
	_Out_opt_ LPTSTR  lpVolumeNameBuffer,
	_In_      DWORD   nVolumeNameSize,
	_Out_opt_ LPDWORD lpVolumeSerialNumber,
	_Out_opt_ LPDWORD lpMaximumComponentLength,
	_Out_opt_ LPDWORD lpFileSystemFlags,
	_Out_opt_ LPTSTR  lpFileSystemNameBuffer,
	_In_      DWORD   nFileSystemNameSize
) {
	if (lpVolumeNameBuffer) {
		strncpy(lpVolumeNameBuffer, "ROOT", nVolumeNameSize);
	}
	if (lpVolumeSerialNumber)
		*lpVolumeSerialNumber = 0x12345678;
	if (lpMaximumComponentLength)
		*lpMaximumComponentLength = 260;
	if (lpFileSystemFlags)
		*lpFileSystemFlags = 0x00000002 | 0x00000001 | 0x00000040;
	if (lpFileSystemNameBuffer)
		strncpy(lpFileSystemNameBuffer, "FAT", nFileSystemNameSize);
	return TRUE;
}

BOOL WINAPI SetConsoleCtrlHandler(
	_In_opt_ PHANDLER_ROUTINE HandlerRoutine,
	_In_     BOOL             Add
) {
	//abort();
	return FALSE;
}

// https://github.com/erick2red/coreclr/blob/d872d88defabcce0f9a1a92d2ddd4b1ded105d46/src/pal/src/file/path.cpp
DWORD
SearchPath(
	IN LPCSTR lpPath,
	IN LPCSTR lpFileName,
	IN LPCSTR lpExtension,
	IN DWORD nBufferLength,
	OUT LPSTR lpBuffer,
	OUT LPSTR* lpFilePart
)
{
	DWORD nRet = 0;
	CHAR* FullPath;
	size_t FullPathLength = 0;
	//PathCharString FullPathPS;
	//PathCharString CanonicalFullPathPS;
	CHAR* CanonicalFullPath;
	LPCSTR pPathStart;
	LPCSTR pPathEnd;
	size_t PathLength;
	size_t FileNameLength;
	DWORD length;
	DWORD dw;

	/*
	ENTRY("SearchPathA(lpPath=%p (%s), lpFileName=%p (%s), lpExtension=%p, "
		  "nBufferLength=%u, lpBuffer=%p, lpFilePart=%p)\n",
	  lpPath,
	  lpPath, lpFileName, lpFileName, lpExtension, nBufferLength, lpBuffer,
		  lpFilePart);
	 */

	 /* validate parameters */

	if (NULL == lpPath)
	{
		//assert("lpPath may not be NULL\n");
		SetLastError(ERROR_INVALID_PARAMETER);
		goto done;
	}
	if (NULL == lpFileName)
	{
		//assert("lpFileName may not be NULL\n");
		SetLastError(ERROR_INVALID_PARAMETER);
		goto done;
	}
	if (NULL != lpExtension)
	{
		//assert("lpExtension must be NULL, is %p instead\n", lpExtension);
		SetLastError(ERROR_INVALID_PARAMETER);
		goto done;
	}

	FileNameLength = strlen(lpFileName);

	/* special case : if file name contains absolute path, don't search the
	   provided path */
	if ('\\' == lpFileName[0] || '/' == lpFileName[0])
	{
		/* Canonicalize the path to deal with back-to-back '/', etc. */
		length = FileNameLength;
		CanonicalFullPath = malloc(length); //CanonicalFullPathPS.OpenStringBuffer(length);
		if (NULL == CanonicalFullPath)
		{
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			goto done;
		}
		dw = GetFullPathName(lpFileName, length + 1, CanonicalFullPath, NULL);
		//CanonicalFullPathPS.CloseBuffer(dw);

		if (length + 1 < dw)
		{
			CanonicalFullPath = realloc(CanonicalFullPath, dw - 1); // CanonicalFullPathPS.OpenStringBuffer(dw-1);
			if (NULL == CanonicalFullPath)
			{
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				goto done;
			}
			dw = GetFullPathName(lpFileName, dw,
				CanonicalFullPath, NULL);
			//CanonicalFullPathPS.CloseBuffer(dw);
		}

		if (dw == 0)
		{
			/*WARN("couldn't canonicalize path <%s>, error is %#x. failing.\n",
				 lpFileName, GetLastError());*/
			SetLastError(ERROR_INVALID_PARAMETER);
			goto done;
		}

		/* see if the file exists */
		if (0 == access(CanonicalFullPath, F_OK))
		{
			/* found it */
			nRet = dw;
		}
	}
	else
	{
		LPCSTR pNextPath;
		pNextPath = lpPath;

		while (*pNextPath)
		{
			pPathStart = pNextPath;

			/* get a pointer to the end of the first path in pPathStart */
			pPathEnd = strchr(pPathStart, ':');
			if (!pPathEnd)
			{
				pPathEnd = pPathStart + strlen(pPathStart);
				/* we want to break out of the loop after this pass, so let
				   *pNextPath be '\0' */
				pNextPath = pPathEnd;
			}
			else
			{
				/* point to the next component in the path string */
				pNextPath = pPathEnd + 1;
			}

			PathLength = pPathEnd - pPathStart;

			if (0 == PathLength)
			{
				/* empty component : there were 2 consecutive ':' */
				continue;
			}

			/* Construct a pathname by concatenating one path from lpPath, '/'
			   and lpFileName */
			FullPathLength = PathLength + FileNameLength;
			FullPath = malloc(FullPathLength + 1); // FullPathPS.OpenStringBuffer(FullPathLength+1);
			if (NULL == FullPath)
			{
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				goto done;
			}
			memcpy(FullPath, pPathStart, PathLength);
			FullPath[PathLength] = '/';
			if (NULL == strncpy(&FullPath[PathLength + 1], lpFileName, FullPathLength + 1 - PathLength))
			{
				//ERROR("strcpy failed!\n");
				SetLastError(ERROR_FILENAME_EXCED_RANGE);
				nRet = 0;
				goto done;
			}

			//FullPathPS.CloseBuffer(FullPathLength+1);            
			/* Canonicalize the path to deal with back-to-back '/', etc. */
			length = MAX_PATH; //MAX_LONGPATH; //Use it for first try
			CanonicalFullPath = malloc(length); //CanonicalFullPathPS.OpenStringBuffer(length);
			if (NULL == CanonicalFullPath)
			{
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				goto done;
			}
			dw = GetFullPathName(FullPath, length + 1,
				CanonicalFullPath, NULL);
			//CanonicalFullPathPS.CloseBuffer(dw);

			if (length + 1 < dw)
			{
				CanonicalFullPath = realloc(CanonicalFullPath, dw - 1); //CanonicalFullPathPS.OpenStringBuffer(dw-1);
				dw = GetFullPathName(FullPath, dw,
					CanonicalFullPath, NULL);
				//CanonicalFullPathPS.CloseBuffer(dw);
			}

			if (dw == 0)
			{
				/* Call failed - possibly low memory.  Skip the path */
				/*WARN("couldn't canonicalize path <%s>, error is %#x. "
					 "skipping it\n", FullPath, GetLastError());*/
				continue;
			}

			/* see if the file exists */
			if (0 == access(CanonicalFullPath, F_OK))
			{
				/* found it */
				nRet = dw;
				break;
			}
		}
	}

	if (nRet == 0)
	{
		/* file not found anywhere; say so. in Windows, this always seems to say
		   FILE_NOT_FOUND, even if path doesn't exist */
		SetLastError(ERROR_FILE_NOT_FOUND);
	}
	else
	{
		if (nRet < nBufferLength)
		{
			if (NULL == lpBuffer)
			{
				/* Windows merily crashes here, but let's not */
				/*ERROR("caller told us buffer size was %d, but buffer is NULL\n",
					  nBufferLength);*/
				SetLastError(ERROR_INVALID_PARAMETER);
				nRet = 0;
				goto done;
			}

			if (strncpy(lpBuffer, CanonicalFullPath, nBufferLength))
			{
				//ERROR("strcpy failed!\n");
				SetLastError(ERROR_FILENAME_EXCED_RANGE);
				nRet = 0;
				goto done;
			}

			if (NULL != lpFilePart)
			{
				*lpFilePart = strrchr(lpBuffer, '/');
				if (NULL == *lpFilePart)
				{
					//assert("no '/' in full path!\n");
					abort();
				}
				else
				{
					/* point to character after last '/' */
					(*lpFilePart)++;
				}
			}
		}
		else
		{
			/* if buffer is too small, report required length, including
			   terminating null */
			nRet++;
		}
	}
done:
	//LOGEXIT("SearchPathA returns DWORD %u\n", nRet);
	return nRet;
}
