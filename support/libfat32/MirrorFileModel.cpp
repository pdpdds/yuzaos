#include <minwinconst.h>
#include "MirrorFileModel.h"
#include "MemFSService.h"
#include "WinDiskFSService.h"
#include <memory.h>



MirrorFile::MirrorFile()
{
	m_pFSService = NULL;
	m_fHandle = NULL;
}

MirrorFile::~MirrorFile()
{
	if (m_pFSService)
	{
		delete m_pFSService;
		m_pFSService = NULL;
	}
}

BOOL	MirrorFile::SetDiskFilePointer(HANDLE hFile,  LONG lDistanceToMove,	PLONG lpDistanceToMoveHigh,	DWORD dwMoveMethod)
{
	m_pFSService->FSSetFilePointer((FSHANDLE)m_fHandle ,lDistanceToMove , lpDistanceToMoveHigh , dwMoveMethod);
	return TRUE;
}

BOOL	MirrorFile::ReadDiskFile(HANDLE hFile ,  LPVOID lpBuffer,	DWORD nNumberOfBytesToRead,	LPDWORD lpNumberOfBytesRead , LPOVERLAPPED lpOverlapped)
{
	return m_pFSService->FSReadFile((FSHANDLE)m_fHandle , lpBuffer , nNumberOfBytesToRead , lpNumberOfBytesRead , lpOverlapped);

}

BOOL	MirrorFile::WriteDiskFile(HANDLE hFile,LPCVOID lpBuffer,DWORD nNumberOfBytesToWrite,LPDWORD lpNumberOfBytesWritten,LPOVERLAPPED lpOverlapped)
{
	return m_pFSService->FSWriteFile((FSHANDLE)m_fHandle , lpBuffer , nNumberOfBytesToWrite , lpNumberOfBytesWritten , lpOverlapped);
}

FSHANDLE	MirrorFile::CreateDiskFile(		LPCSTR lpFileName,          // pointer to name of the file
						   DWORD dwDesiredAccess,       // access (read-write) mode
						   DWORD dwShareMode,           // share mode
						   SECURITY_ATTRIBUTES* lpSecurityAttributes,
						   // pointer to security attributes
						   DWORD dwCreationDisposition,  // how to create
						   DWORD dwFlagsAndAttributes,  // file attributes
						   FSHANDLE hTemplateFile         // handle to file with attributes to
						   )
{
	return m_pFSService->FSCreateFile(lpFileName , dwDesiredAccess , dwShareMode , lpSecurityAttributes ,
			dwCreationDisposition , dwFlagsAndAttributes  , hTemplateFile);
}

void	MirrorFile::IniFSService(CFSService::FSType type)
{
	if (m_pFSService)
	{
		delete m_pFSService;
		m_pFSService = NULL;
	}

	if(CFSService::FS_MEM==type)
	{
		m_pFSService=new CMemFSService();
	}
	else
	{	
		m_pFSService=new CWinDiskFSService();
	}
}

BOOL	MirrorFile::CreateDiskFile(LPCSTR lpFileName)
{
	if (m_fHandle) CloseDiskFile();
	
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle =  0;
	//	_lpImgFileName = new char[MAX_PATH];
	
	// map creation flags
	// map read/write mode
	DWORD dwAccess = 0;
	dwAccess = GENERIC_READ|GENERIC_WRITE;
	
	// map share mode
	DWORD dwShareMode = 0;
	dwShareMode = FILE_SHARE_WRITE|FILE_SHARE_READ;
	// map creation flags
	DWORD dwCreateFlag;
	
	dwCreateFlag = CREATE_ALWAYS;
	// attempt file creation
	// attempt file creation

	int ret = 0;

	FSHANDLE hFile = CreateDiskFile(lpFileName, dwAccess, dwShareMode, &sa,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);  //已有的，就打开，没有的就会返回0xfffffff,就在下面再创建
	

	if(hFile == (FSHANDLE)-1 || hFile == (FSHANDLE)0)
	{
		//printf("hFile is %d", hFile);

		hFile = CreateDiskFile(lpFileName, dwAccess, dwShareMode, &sa,
			dwCreateFlag, FILE_ATTRIBUTE_NORMAL, NULL);

		if(hFile == (FSHANDLE)-1 || hFile == (FSHANDLE)0) 
			return FALSE;
		
		ret = 2;//disk not format

	}
	else ret = 1;//disk exists

	m_fHandle = (HANDLE)hFile;

	return ret;	
}

BOOL	MirrorFile::CloseDiskFile()
{
	if (m_fHandle)
	{
		// fill zero to fit sector
		ULONGLONG dwPos = m_pFSService->FSSetFilePointer((FSHANDLE)m_fHandle , 0 , 0 , FILE_END);
		
		if (dwPos % 512 != 0 )
		{
			ULONGLONG offset = 0;
			offset = 512 - dwPos % 512;
			BYTE buf[512];
			memset(buf , 0 , 512);
			
			DWORD nWritten = 0;
			m_pFSService->FSWriteFile( (FSHANDLE)m_fHandle, buf , (DWORD)offset , &nWritten , NULL);
		}
		
		BOOL ret = m_pFSService->FSCloseHandle((FSHANDLE)m_fHandle);
		m_fHandle = NULL;
		return ret;
	}
	
	return TRUE;
}
