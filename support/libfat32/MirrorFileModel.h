#pragma once
#include "FSService.h"

class MirrorFile 
{
public:
	MirrorFile();
	virtual ~MirrorFile();
public:
	virtual UINT ImgFileType() = 0;
	virtual LONGLONG TotoleSpace() = 0;
	virtual LPSTR GetMirrorFileName()= 0 ;
	
	BOOL	SetDiskFilePointer(HANDLE hFile,  LONG lDistanceToMove,	PLONG lpDistanceToMoveHigh,	DWORD dwMoveMethod);
	BOOL	ReadDiskFile(HANDLE hFile ,  LPVOID lpBuffer,	DWORD nNumberOfBytesToRead,	LPDWORD lpNumberOfBytesRead , LPOVERLAPPED lpOverlapped);
	BOOL	WriteDiskFile(HANDLE hFile,LPCVOID lpBuffer,DWORD nNumberOfBytesToWrite,LPDWORD lpNumberOfBytesWritten,LPOVERLAPPED lpOverlapped);	
	BOOL	CreateDiskFile(LPCSTR lpFileName);
	BOOL	CloseDiskFile();	
	void	IniFSService(CFSService::FSType type);

protected:
	HANDLE	m_fHandle;

private:
	FSHANDLE	CreateDiskFile(		LPCSTR lpFileName,          // pointer to name of the file
		DWORD dwDesiredAccess,       // access (read-write) mode
		DWORD dwShareMode,           // share mode
		SECURITY_ATTRIBUTES* lpSecurityAttributes,
		// pointer to security attributes
		DWORD dwCreationDisposition,  // how to create
		DWORD dwFlagsAndAttributes,  // file attributes
		FSHANDLE hTemplateFile         // handle to file with attributes to
		);
	CFSService *m_pFSService;	
};
