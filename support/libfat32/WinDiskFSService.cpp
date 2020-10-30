// WinDiskFSService.cpp: implementation of the CWinDiskFSService class.
//
//////////////////////////////////////////////////////////////////////

#include "WinDiskFSService.h"
#include "common.h"
#include <stdio.h>
#include <fileio.h>
#include "sprintf.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWinDiskFSService::CWinDiskFSService()
{
	bEnableLog = FALSE;
}

CWinDiskFSService::~CWinDiskFSService()
{

}
//////////////////////////////////////////////////////////////////////////
BOOL CWinDiskFSService::FSPathFileExist(LPCSTR lpFileName)
{
	//这个函数貌似没用到wbt del
	//return PathFileExists(lpFileName);
	return TRUE;
}
FSHANDLE CWinDiskFSService::FSCreateFile(
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
	return (FSHANDLE)::CreateFile(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,
		dwCreationDisposition,dwFlagsAndAttributes,(HANDLE)hTemplateFile);
}
BOOL CWinDiskFSService::FSDeleteFile(
								 LPCSTR lpFileName   // pointer to name of file to delete
								 )
{ 
	//20190514
	//wbt mod
	//return DeleteFile(lpFileName);
	/*return remove(lpFileName);*/
	return TRUE;
}
DWORD CWinDiskFSService::FSGetFileAttributes(
										 LPCSTR lpFileName   // pointer to the name of a file or directory
										 )
{ 
	//wbt mod
	//return ::GetFileAttributes(lpFileName)
	return 0;
}
BOOL CWinDiskFSService::FSCloseHandle(
								  FSHANDLE hObject   // FSHANDLE to object to close
								  )
{ 
	//wbt mod
	//return ::CloseHandle((HANDLE)hObject);
	if (hObject)
	{
		return fclose((FILE*) hObject);
	}
	return FALSE;
}
BOOL CWinDiskFSService::FSReadFile(
							   FSHANDLE hFile,                // FSHANDLE of file to read
							   void* lpBuffer,             // pointer to buffer that receives data
							   DWORD nNumberOfBytesToRead,  // number of bytes to read
							   DWORD* lpNumberOfBytesRead, // pointer to number of bytes read
							   OVERLAPPED* lpOverlapped    // pointer to structure for data
							   )
{ 
	return ReadFile((HANDLE)hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead
		,lpOverlapped);
}
BOOL CWinDiskFSService::FSWriteFile(
								FSHANDLE hFile,                    // FSHANDLE to file to write to
								const void* lpBuffer,                // pointer to data to write to file
								DWORD nNumberOfBytesToWrite,     // number of bytes to write
								DWORD* lpNumberOfBytesWritten,  // pointer to number of bytes written
								OVERLAPPED* lpOverlapped        // pointer to structure for overlapped I/O
								)
{ 
	if (bEnableLog)
	{
		FILE* f = fopen("d:\\log.txt" , "a+b");
		int tlen = nNumberOfBytesToWrite;		
		if(tlen > 10) tlen = 10;
		BYTE* p = (BYTE*)lpBuffer;
		char szTemp[128];

		sprintf(szTemp , "Write len: %d : \r\n" , nNumberOfBytesToWrite);
		fseek(f , 0 , SEEK_END);
		fwrite(szTemp , 1 , strlen(szTemp) , f);

		for(int i=0;i<tlen;i++)
		{
			sprintf(szTemp , "%02X " , p[i]);				
			fwrite(szTemp , 1 , strlen(szTemp) , f);
		}
//		fwrite(p , 1 , tlen , f);

		sprintf(szTemp , "\r\n");
		fwrite(szTemp , 1 , strlen(szTemp) , f);
		fclose(f);
	}


	return ::WriteFile((HANDLE)hFile,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten
		,lpOverlapped);
}

ULONGLONG CWinDiskFSService::FSSetFilePointer(
											  FSHANDLE hFile,          // FSHANDLE of file
											  ULONGLONG lDistanceToMove,  // number of bytes to move file pointer
											  LONG* lpDistanceToMoveHigh,
											  // pointer to high-order DWORD of 
											  // distance to move
											  DWORD dwMoveMethod     // how to move
											  )
{
	if (bEnableLog)
	{
		FILE* f = fopen("d:\\log.txt" , "a+b");
		fseek(f , 0 , SEEK_END);
		char szTemp[128];
		sprintf(szTemp , "seek : mode %d , dis %d\r\n" , dwMoveMethod , (UINT)lDistanceToMove);
		fwrite(szTemp , 1 , strlen(szTemp) , f);
		fclose(f);
	}	
	
	ULONGLONG nNewPos;
	
	switch (dwMoveMethod)
	{
    case FILE_BEGIN:   
		{
			nNewPos = lDistanceToMove; 
			break;
		}
    case FILE_CURRENT: //deliberate fall through
		{
			nNewPos = GetPosition((HANDLE)hFile) + lDistanceToMove; 
			break;
		}
    case FILE_END:     
		{
			
			nNewPos = GetLength((HANDLE)hFile) + lDistanceToMove; 
			break;
		}
    default:      
		{
			ASSERT(FALSE);        
			break;
		}
	}
	DWORD dwDistanceToMoveLow;
	DWORD dwDistanceToMoveHigh;
	SplitUnsignedInt64(nNewPos, dwDistanceToMoveHigh, dwDistanceToMoveLow);
	
	//WINBUG: What if dwDistanceToMoveLow is > LONG_MAX, then SetFilePointer 
	//will interpret that as a negative value
	DWORD dwNewLow = ::SetFilePointer((HANDLE)hFile, dwDistanceToMoveLow, (LONG*) &dwDistanceToMoveHigh, FILE_BEGIN);
	//wbt del
	//if (dwNewLow  == (DWORD)-1)
	//	CFileException::ThrowOsError((LONG)::GetLastError());
	
	return MakeUnsignedInt64(dwDistanceToMoveHigh, dwNewLow);
}

BOOL CWinDiskFSService::FSFlushFileBuffers(
									   FSHANDLE hFile   // open FSHANDLE to file whose buffers are to be 
									   // flushed
									   )
{ 
	//wbt mod
	//return ::FlushFileBuffers((HANDLE)hFile);
	return 1;
}
BOOL CWinDiskFSService::FSSetEndOfFile(
								   FSHANDLE hFile   // FSHANDLE of file whose EOF is to be set
								   )
{ 
	//wbt mod 
	//return ::SetEndOfFile((HANDLE)hFile);
	return 1;
}

DWORD CWinDiskFSService::FSGetFileSize(
								   FSHANDLE hFile,  // FSHANDLE of file to get size of
								   DWORD* lpFileSizeHigh 
								   // pointer to high-order word for file size
								   )
{ 
	return ::GetFileSize((HANDLE)hFile,lpFileSizeHigh);
}
BOOL CWinDiskFSService::FSLockFile(
							   FSHANDLE hFile,           // FSHANDLE of file to lock
							   DWORD dwFileOffsetLow,  // low-order word of lock region offset
							   DWORD dwFileOffsetHigh,  // high-order word of lock region offset
							   DWORD nNumberOfBytesToLockLow,
							   // low-order word of length to lock
							   DWORD nNumberOfBytesToLockHigh 
							   // high-order word of length to lock
							   )
{ 
	//wbt mod
	//return ::LockFile((HANDLE)hFile,dwFileOffsetLow,dwFileOffsetHigh,nNumberOfBytesToLockLow,nNumberOfBytesToLockHigh);
	return 1;
}
BOOL CWinDiskFSService::FSLockFileEx(
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
	//wbt mod
	//return ::LockFileEx((HANDLE)hFile,dwFlags,dwReserved,nNumberOfBytesToLockLow,nNumberOfBytesToLockHigh,lpOverlapped);
	return 1;
}
BOOL CWinDiskFSService::FSUnlockFile(
								 FSHANDLE hFile,           // FSHANDLE of file to unlock
								 DWORD dwFileOffsetLow,  // low-order word of lock region offset
								 DWORD dwFileOffsetHigh,  // high-order word of lock region offset
								 DWORD nNumberOfBytesToUnlockLow,
								 // low-order word of length to unlock
								 DWORD nNumberOfBytesToUnlockHigh 
								 // high-order word of length to unlock
								 )
{ 
	//wbt mod
	//return ::UnlockFile((HANDLE)hFile,dwFileOffsetLow,dwFileOffsetHigh,nNumberOfBytesToUnlockLow,nNumberOfBytesToUnlockHigh);
	return 1;
}
BOOL CWinDiskFSService::FSUnlockFileEx(
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
	//wbt mod
	//return ::UnlockFileEx((HANDLE)hFile,dwReserved,nNumberOfBytesToUnlockLow,nNumberOfBytesToUnlockHigh,lpOverlapped);
	return 1;
}
//////////////////////////////////////////////////////////////////////////
