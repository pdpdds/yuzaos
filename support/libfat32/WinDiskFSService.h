// WinDiskFSService.h: interface for the CWinDiskFSService class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINDISKFSSERVICE_H__C9E0E90A_472C_498E_9B46_B2E02222CBF7__INCLUDED_)
#define AFX_WINDISKFSSERVICE_H__C9E0E90A_472C_498E_9B46_B2E02222CBF7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FSService.h"

class CWinDiskFSService : public CFSService  
{
public:
	BOOL FSPathFileExist(LPCSTR lpFileName);
	FSHANDLE FSCreateFile(
		LPCSTR lpFileName,          // pointer to name of the file
		DWORD dwDesiredAccess,       // access (read-write) mode
		DWORD dwShareMode,           // share mode
		SECURITY_ATTRIBUTES* lpSecurityAttributes,
		// pointer to security attributes
		DWORD dwCreationDisposition,  // how to create
		DWORD dwFlagsAndAttributes,  // file attributes
		FSHANDLE hTemplateFile         // FSHANDLE to file with attributes to 
		// copy
		);
	
	BOOL FSDeleteFile(
		LPCSTR lpFileName   // pointer to name of file to delete
		);
	DWORD FSGetFileAttributes(
		LPCSTR lpFileName   // pointer to the name of a file or directory
		);
	BOOL FSCloseHandle(
		FSHANDLE hObject   // FSHANDLE to object to close
		);
	BOOL FSReadFile(
		FSHANDLE hFile,                // FSHANDLE of file to read
		void* lpBuffer,             // pointer to buffer that receives data
		DWORD nNumberOfBytesToRead,  // number of bytes to read
		DWORD* lpNumberOfBytesRead, // pointer to number of bytes read
		OVERLAPPED* lpOverlapped    // pointer to structure for data
		);
	BOOL FSWriteFile(
		FSHANDLE hFile,                    // FSHANDLE to file to write to
		const void* lpBuffer,                // pointer to data to write to file
		DWORD nNumberOfBytesToWrite,     // number of bytes to write
		DWORD* lpNumberOfBytesWritten,  // pointer to number of bytes written
		OVERLAPPED* lpOverlapped        // pointer to structure for overlapped I/O
		);
	ULONGLONG FSSetFilePointer(
		FSHANDLE hFile,          // FSHANDLE of file
		ULONGLONG lDistanceToMove,  // number of bytes to move file pointer
		LONG* lpDistanceToMoveHigh,
		// pointer to high-order DWORD of 
		// distance to move
		DWORD dwMoveMethod     // how to move
		);
	BOOL FSFlushFileBuffers(
		FSHANDLE hFile   // open FSHANDLE to file whose buffers are to be 
		// flushed
		);
	BOOL FSSetEndOfFile(
		FSHANDLE hFile   // FSHANDLE of file whose EOF is to be set
		);
	
	DWORD FSGetFileSize(
		FSHANDLE hFile,  // FSHANDLE of file to get size of
		DWORD* lpFileSizeHigh 
		// pointer to high-order word for file size
		);
	BOOL FSLockFile(
		FSHANDLE hFile,           // FSHANDLE of file to lock
		DWORD dwFileOffsetLow,  // low-order word of lock region offset
		DWORD dwFileOffsetHigh,  // high-order word of lock region offset
		DWORD nNumberOfBytesToLockLow,
		// low-order word of length to lock
		DWORD nNumberOfBytesToLockHigh 
		// high-order word of length to lock
		);
	BOOL FSLockFileEx(
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
		);
	BOOL FSUnlockFile(
		FSHANDLE hFile,           // FSHANDLE of file to unlock
		DWORD dwFileOffsetLow,  // low-order word of lock region offset
		DWORD dwFileOffsetHigh,  // high-order word of lock region offset
		DWORD nNumberOfBytesToUnlockLow,
		// low-order word of length to unlock
		DWORD nNumberOfBytesToUnlockHigh 
		// high-order word of length to unlock
		);
	BOOL FSUnlockFileEx(
		FSHANDLE hFile,      // FSHANDLE of file to unlock
		DWORD dwReserved,  // reserved, must be set to zero
		DWORD nNumberOfBytesToUnlockLow,
		// low order 32-bits of length to unlock
		DWORD nNumberOfBytesToUnlockHigh,
		// high order 32-bits of length to unlock
		OVERLAPPED* lpOverlapped 
		// addr. of struct. with unlock region start 
		// offset
		);
public:
	CWinDiskFSService();
	virtual ~CWinDiskFSService();

};

#endif // !defined(AFX_WINDISKFSSERVICE_H__C9E0E90A_472C_498E_9B46_B2E02222CBF7__INCLUDED_)
