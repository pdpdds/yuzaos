// MemFSService.cpp: implementation of the MemFSService class.
//
//////////////////////////////////////////////////////////////////////

#include "MemFSService.h"
#include "MemFS.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemFSService::CMemFSService()
{
	RAMInitialMemFS();
}

CMemFSService::~CMemFSService()
{
	RAMCloseMemFS();
}
//////////////////////////////////////////////////////////////////////////
BOOL CMemFSService::FSPathFileExist(LPCSTR lpFileName)
{
	return RAMPathFileExist(lpFileName);
}
FSHANDLE CMemFSService::FSCreateFile(
								   LPCSTR lpFileName,          // pointer to name of the file
								   DWORD dwDesiredAccess,       // access (read-write) mode
								   DWORD dwShareMode,           // share mode
								   SECURITY_ATTRIBUTES* lpSecurityAttributes,
								   // pointer to security attributes
								   DWORD dwCreationDisposition,  // how to create
								   DWORD dwFlagsAndAttributes,  // file attributes
								   FSHANDLE hTemplateFile         // FSHANDLE to file with attributes to 
								   // copy
								   )
{ 
	return RAMCreateFile(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,
		dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
}
BOOL CMemFSService::FSDeleteFile(
								 LPCSTR lpFileName   // pointer to name of file to delete
								 )
{ 
	return RAMDeleteFile(lpFileName);
}
DWORD CMemFSService::FSGetFileAttributes(
										 LPCSTR lpFileName   // pointer to the name of a file or directory
										 )
{ 
	return RAMGetFileAttributes(lpFileName);
}
BOOL CMemFSService::FSCloseHandle(
								  FSHANDLE hObject   // FSHANDLE to object to close
								  )
{ 
	return RAMCloseHandle(hObject);
}
BOOL CMemFSService::FSReadFile(
							   FSHANDLE hFile,                // FSHANDLE of file to read
							   void* lpBuffer,             // pointer to buffer that receives data
							   DWORD nNumberOfBytesToRead,  // number of bytes to read
							   DWORD* lpNumberOfBytesRead, // pointer to number of bytes read
							   OVERLAPPED* lpOverlapped    // pointer to structure for data
							   )
{ 
	return RAMReadFile(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead
		,lpOverlapped);
}
BOOL CMemFSService::FSWriteFile(
								FSHANDLE hFile,                    // FSHANDLE to file to write to
								const void* lpBuffer,                // pointer to data to write to file
								DWORD nNumberOfBytesToWrite,     // number of bytes to write
								DWORD* lpNumberOfBytesWritten,  // pointer to number of bytes written
								OVERLAPPED* lpOverlapped        // pointer to structure for overlapped I/O
								)
{ 
	return RAMWriteFile(hFile,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten
		,lpOverlapped);
}

ULONGLONG CMemFSService::FSSetFilePointer(
									  FSHANDLE hFile,          // FSHANDLE of file
									  ULONGLONG lDistanceToMove,  // number of bytes to move file pointer
									  LONG* lpDistanceToMoveHigh,
									  // pointer to high-order DWORD of 
									  // distance to move
									  DWORD dwMoveMethod     // how to move
									  )
{ 
	return RAMSetFilePointer(hFile,lDistanceToMove,lpDistanceToMoveHigh,
		dwMoveMethod);
}

BOOL CMemFSService::FSFlushFileBuffers(
									   FSHANDLE hFile   // open FSHANDLE to file whose buffers are to be 
									   // flushed
									   )
{ 
	return RAMFlushFileBuffers(hFile);
}
BOOL CMemFSService::FSSetEndOfFile(
								   FSHANDLE hFile   // FSHANDLE of file whose EOF is to be set
								   )
{ 
	return RAMSetEndOfFile(hFile);
}

DWORD CMemFSService::FSGetFileSize(
								   FSHANDLE hFile,  // FSHANDLE of file to get size of
								   DWORD* lpFileSizeHigh 
								   // pointer to high-order word for file size
								   )
{ 
	return RAMGetFileSize(hFile,lpFileSizeHigh);
}
BOOL CMemFSService::FSLockFile(
							   FSHANDLE hFile,           // FSHANDLE of file to lock
							   DWORD dwFileOffsetLow,  // low-order word of lock region offset
							   DWORD dwFileOffsetHigh,  // high-order word of lock region offset
							   DWORD nNumberOfBytesToLockLow,
							   // low-order word of length to lock
							   DWORD nNumberOfBytesToLockHigh 
							   // high-order word of length to lock
							   )
{ 
	return RAMLockFile(hFile,dwFileOffsetLow,dwFileOffsetHigh,nNumberOfBytesToLockLow,
		nNumberOfBytesToLockHigh);
}
BOOL CMemFSService::FSLockFileEx(
								 FSHANDLE hFile,      // FSHANDLE of file to lock
								 DWORD dwFlags,     // functional behavior modification flags
								 DWORD dwReserved,  // reserved, must be set to zero
								 DWORD nNumberOfBytesToLockLow,
								 // low-order 32 bits of length to lock
								 DWORD nNumberOfBytesToLockHigh,
								 // high-order 32 bits of length to lock
								 OVERLAPPED* lpOverlapped 
								 // addr. of structure with lock region start 
								 // offset
								 )
{ 
	return RAMLockFileEx(hFile,dwFlags,dwReserved,nNumberOfBytesToLockLow,
		nNumberOfBytesToLockHigh,lpOverlapped);
}
BOOL CMemFSService::FSUnlockFile(
								 FSHANDLE hFile,           // FSHANDLE of file to unlock
								 DWORD dwFileOffsetLow,  // low-order word of lock region offset
								 DWORD dwFileOffsetHigh,  // high-order word of lock region offset
								 DWORD nNumberOfBytesToUnlockLow,
								 // low-order word of length to unlock
								 DWORD nNumberOfBytesToUnlockHigh 
								 // high-order word of length to unlock
								 )
{ 
	return RAMUnlockFile(hFile,dwFileOffsetLow,dwFileOffsetHigh,
		nNumberOfBytesToUnlockLow,nNumberOfBytesToUnlockHigh);
}
BOOL CMemFSService::FSUnlockFileEx(
								   FSHANDLE hFile,      // FSHANDLE of file to unlock
								   DWORD dwReserved,  // reserved, must be set to zero
								   DWORD nNumberOfBytesToUnlockLow,
								   // low order 32-bits of length to unlock
								   DWORD nNumberOfBytesToUnlockHigh,
								   // high order 32-bits of length to unlock
								   OVERLAPPED* lpOverlapped 
								   // addr. of struct. with unlock region start 
								   // offset
								   )
{ 
	return RAMUnlockFileEx(hFile,dwReserved,nNumberOfBytesToUnlockLow,nNumberOfBytesToUnlockHigh,
		lpOverlapped);
}
//////////////////////////////////////////////////////////////////////////
