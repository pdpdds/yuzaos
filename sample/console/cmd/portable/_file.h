#ifndef _FILE_H__
#define _FILE_H__

#define FILE_BEGIN SEEK_SET
#define FILE_CURRENT SEEK_CUR
#define FILE_END SEEK_END

BOOL WINAPI ReadFile(
  FILE         *hFile,
  LPVOID       lpBuffer,
  DWORD        nNumberOfBytesToRead,
  LPDWORD      lpNumberOfBytesRead,
  LPVOID lpOverlapped
);

BOOL DeleteFile(LPCTSTR lpFileName);

BOOL WINAPI CloseFile(
  FILE *hFile
);

DWORD GetFileAttributes(LPCTSTR lpFileName);

DWORD WINAPI GetFileSize(
  FILE  *hFile,
  LPDWORD lpFileSizeHigh
);

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365541(v=vs.85).aspx
DWORD WINAPI SetFilePointer(
  FILE   *hFile,
  LONG   lDistanceToMove,
  PLONG  lpDistanceToMoveHigh,
  DWORD  dwMoveMethod
);

FILE *_dup( FILE *handle );
  
int _pipe(  
	FILE **pfds,
	unsigned int psize,
	int textmode
);

BOOL WINAPI WriteFile(
	FILE*       hFile,
	LPCVOID      lpBuffer,
	DWORD        nNumberOfBytesToWrite,
    LPDWORD      lpNumberOfBytesWritten,
  	LPVOID lpOverlapped
);

FILE* WINAPI GetStdHandle(
  _In_ DWORD nStdHandle
);

/*BOOL WINAPI SetCurrentDirectory(
  _In_ LPCTSTR lpPathName
);*/

BOOL WINAPI GetBinaryType(
  _In_  LPCTSTR lpApplicationName,
  _Out_ LPDWORD lpBinaryType
);

DWORD WINAPI GetFullPathName(
  _In_  LPCTSTR lpFileName,
  _In_  DWORD   nBufferLength,
  _Out_ LPTSTR  lpBuffer,
  _Out_ LPTSTR  *lpFilePart
);

/*BOOL WINAPI CreateDirectory(
  _In_     LPCTSTR               lpPathName,
  _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
);*/


BOOL WINAPI WriteConsole(
  _In_             FILE*  hConsoleOutput,
  _In_       const VOID    *lpBuffer,
  _In_             DWORD   nNumberOfCharsToWrite,
  _Out_            LPDWORD lpNumberOfCharsWritten,
  _Reserved_       LPVOID  lpReserved
);

DWORD WINAPI GetCurrentDirectory(
  _In_  DWORD  nBufferLength,
  _Out_ LPTSTR lpBuffer
);

UINT WINAPI GetWindowsDirectory(
  _Out_ LPTSTR lpBuffer,
  _In_  UINT   uSize
);

BOOL WINAPI RemoveDirectory(
  _In_ LPCTSTR lpPathName
);

UINT WINAPI GetDriveType(
  _In_opt_ LPCTSTR lpRootPathName
);


BOOL WINAPI SetFileTime(
  _In_           HANDLE   hFile,
  _In_opt_ const FILETIME *lpCreationTime,
  _In_opt_ const FILETIME *lpLastAccessTime,
  _In_opt_ const FILETIME *lpLastWriteTime
);


BOOL WINAPI FlushFileBuffers(
  _In_ FILE* hFile
);

BOOL WINAPI FlushConsoleInputBuffer(
  _In_ FILE* hConsoleInput
);

BOOL WINAPI MoveFile(
  _In_ LPCTSTR lpExistingFileName,
  _In_ LPCTSTR lpNewFileName
);

BOOL SetFileAttributes(LPCTSTR lpFileName,DWORD dwFileAttributes);

BOOL WINAPI GetDiskFreeSpace(
  _In_  LPCTSTR lpRootPathName,
  _Out_ LPDWORD lpSectorsPerCluster,
  _Out_ LPDWORD lpBytesPerSector,
  _Out_ LPDWORD lpNumberOfFreeClusters,
  _Out_ LPDWORD lpTotalNumberOfClusters
);

BOOL WINAPI CopyFile(
  _In_ LPCTSTR lpExistingFileName,
  _In_ LPCTSTR lpNewFileName,
  _In_ BOOL    bFailIfExists
);

/*FILE* WINAPI CreateFile(
  _In_     LPCTSTR               lpFileName,
  _In_     DWORD                 dwDesiredAccess,
  _In_     DWORD                 dwShareMode,
  _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  _In_     DWORD                 dwCreationDisposition,
  _In_     DWORD                 dwFlagsAndAttributes,
  _In_opt_ HANDLE                hTemplateFile
);*/

BOOL WINAPI GetVolumeInformation(
  _In_opt_  LPCTSTR lpRootPathName,
  _Out_opt_ LPTSTR  lpVolumeNameBuffer,
  _In_      DWORD   nVolumeNameSize,
  _Out_opt_ LPDWORD lpVolumeSerialNumber,
  _Out_opt_ LPDWORD lpMaximumComponentLength,
  _Out_opt_ LPDWORD lpFileSystemFlags,
  _Out_opt_ LPTSTR  lpFileSystemNameBuffer,
  _In_      DWORD   nFileSystemNameSize
);


DWORD WINAPI SearchPath(
  _In_opt_  LPCTSTR lpPath,
  _In_      LPCTSTR lpFileName,
  _In_opt_  LPCTSTR lpExtension,
  _In_      DWORD   nBufferLength,
  _Out_     LPTSTR  lpBuffer,
  _Out_opt_ LPTSTR  *lpFilePart
);

typedef BOOL(*PHANDLER_ROUTINE)(DWORD);

BOOL WINAPI SetConsoleCtrlHandler(
  _In_opt_ PHANDLER_ROUTINE HandlerRoutine,
  _In_     BOOL             Add
);


#endif
