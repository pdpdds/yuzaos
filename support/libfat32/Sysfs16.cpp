// Sysfs16.cpp: implementation of the CSysfs16 class.
//
//////////////////////////////////////////////////////////////////////

#include "Sysfs16.h"
#include "DiskImgFile.h"
#include "sprintf.h"
// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSysfs16::CSysfs16()
{
	_pMirrorFile = NULL;
	_diskSize = 0;

	strcpy(_szcurDir , "\\");
}

CSysfs16::~CSysfs16()
{
	if(_pMirrorFile)
	{
		RefreshFatTable();
		_pMirrorFile->CloseImgFile();		
		delete _pMirrorFile;
		_pMirrorFile = NULL;

	}	
}

BOOL CSysfs16::Release()
{
	if (_pMirrorFile)
	{
		RefreshFatTable();
		_pMirrorFile->CloseImgFile();
		delete _pMirrorFile;
		_pMirrorFile = NULL;
		_diskSize = 0;
	}

	return TRUE;
}

BOOL CSysfs16::Mount(
		LPCTSTR lpImageFile,	// in, pointer to name of the image file, if file not exist, create it.
		LPCTSTR lpRoot,		// out,pointer to name of the root
		LONGLONG uVDiskSize,	// in/out, size of virtual disk
		UINT     nBlockSize,
		UINT     nJournalBlkSize,
		CFSService::FSType type
		)
{
	if(_pMirrorFile == NULL)
	{
		_pMirrorFile = new DiskImgFile();
		_pMirrorFile->IniFatType(FAT16_TYPE);
		_pMirrorFile->IniFSService(type);
	}
	else
		_pMirrorFile->CloseImgFile();

	_pMirrorFile->OpenImgFile(lpImageFile,FAT16_TYPE, uVDiskSize);
	_diskSize = uVDiskSize;
	
	return TRUE;
}


BOOL CSysfs16::UnMount(
		LPCTSTR lpRoot		// pointer to name of the root
		)
{
	if(_pMirrorFile== NULL)
		return TRUE;
	RefreshFatTable();
	_pMirrorFile->CloseImgFile();
	delete _pMirrorFile;
	_pMirrorFile = NULL;
	return TRUE;
}

BOOL CSysfs16::Format(
		UINT uFileSystem,	//system id(FAT16:0, FAT32:1)
		LPCTSTR lpLabel,	// pointer to name of the label
		BYTE uSectorPerCluster,	// sector of per cluster
		UINT     nBlockSize
		)
{
	ASSERT(_pMirrorFile);
	_pMirrorFile->FormatImgFile(lpLabel, uFileSystem, _diskSize);
	return TRUE;
}

BOOL CSysfs16::FindClose(
		HANDLE hFindFile   // file search handle
		)
{
//	return _pMirrorFile->CloseFileEx(hFindFile);// _fileFind.CloseFind();
	return _pMirrorFile->FindClose(hFindFile);
}

BOOL CSysfs16::CopyFile(
		LPCTSTR lpExistingFileName,	// pointer to name of an existing file
		LPCTSTR lpNewFileName,		// pointer to filename to copy to
		BOOL bFailIfExists			// flag for operation if file exists
		)
{
	HANDLE hf = this->CreateFile(lpExistingFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if(hf == INVALID_HANDLE_VALUE)
		return FALSE;
	DWORD fSize = 0;
	this->GetFileSize(hf, &fSize);

	LPBYTE pBuffer = new BYTE[fSize];
	DWORD nRead;
	if(!ReadFile(hf, pBuffer, fSize,  &nRead, NULL))
		return FALSE;
	FindClose(hf);

	hf = this->CreateFile(lpNewFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if(hf == INVALID_HANDLE_VALUE)
		return FALSE;

	if(!this->WriteFile(hf, pBuffer, fSize, &nRead, NULL))
		return FALSE;
	FindClose(hf);
	return TRUE;
}

BOOL CSysfs16::CreateDirectory(
		LPCTSTR lpPathName,                         // pointer to directory path string
		LPSECURITY_ATTRIBUTES lpSecurityAttributes  // pointer to security descriptor
		)
{
     return _pMirrorFile->CreateDirectoryEx(lpPathName);
	 
}

HANDLE CSysfs16::CreateFile(
		LPCTSTR lpFileName,          // pointer to name of the file
		DWORD dwDesiredAccess,       // access (read-write) mode
		DWORD dwShareMode,           // share mode
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// pointer to security attributes
		DWORD dwCreationDisposition, // how to create
		DWORD dwFlagsAndAttributes,  // file attributes
		HANDLE hTemplateFile         // handle to file with attributes to copy
		) 
{
	HANDLE hFile = _pMirrorFile->CreateFileEx(lpFileName, dwDesiredAccess,
								dwShareMode,
								NULL,
								dwCreationDisposition,
								dwFlagsAndAttributes,
								NULL);
	return hFile;
}

BOOL CSysfs16::DeleteFile(
		LPCTSTR lpFileName   // pointer to name of file to delete
		)
{
	return _pMirrorFile->DeleteFileEx(lpFileName);
	//return TRUE;
}

HANDLE CSysfs16::FindFirstFile(
		LPCTSTR lpFileName,					// pointer to name of file to search for
		LPWIN32_FIND_DATA lpFindFileData	// pointer to returned information
		)
{
	char	szFullPath[MAX_PATH];

	if (lpFileName == NULL )
	{
		strcpy(szFullPath , _szcurDir);
	}
	else if (lpFileName[0] != '\\')
	{
		strcpy(szFullPath , _szcurDir);
		//sprintf(szFullPath , "%s%s" , _szcurDir , lpFileName);
	}
	else
	{
		strcpy(szFullPath , lpFileName);
	}
	
	HANDLE hf = _pMirrorFile->FindFirstFile(szFullPath, lpFindFileData);
	return hf;
}
/*
HANDLE CSysfs16::FindFirstFileEx(
		LPCTSTR lpFileName,					// pointer to the name of the file to search for
		FINDEX_INFO_LEVELS fInfoLevelId,	// information level of the returned data
		LPVOID lpFindFileData,				// pointer to the returned information
		FINDEX_SEARCH_OPS fSearchOp,		// type of filtering to perform
		LPVOID lpSearchFilter,				// pointer to search criteria
		DWORD dwAdditionalFlags				// additional search control flags
		)
{
	return NULL;
}
*/
BOOL CSysfs16::FindNextFile(
		HANDLE hFindFile,					// handle to search
		LPWIN32_FIND_DATA lpFindFileData	// pointer to structure for data on found file
		) 
{
	return _pMirrorFile->FindNextFile(hFindFile, lpFindFileData);
	//return TRUE;
}

DWORD CSysfs16::GetCurrentDirectory(
		DWORD nBufferLength,  // size, in characters, of directory buffer
		LPTSTR lpBuffer       // pointer to buffer for current directory
		)
{
	_pMirrorFile->GetCurrentDirectory(lpBuffer, nBufferLength);
	return 1;
}

BOOL CSysfs16::GetDiskFreeSpace(
		LPTSTR lpRootPathName,			// pointer to root path
		LPDWORD lpSectorsPerCluster,	// pointer to sectors per cluster
		LPDWORD lpBytesPerSector,		// pointer to bytes per sector
		LPDWORD lpNumberOfFreeClusters,	// pointer to number of free clusters
		LPDWORD lpTotalNumberOfClusters	// pointer to total number of clusters
		)
{
	strcpy(lpRootPathName ,_pMirrorFile->RootPath());
	*lpSectorsPerCluster = _pMirrorFile->SectorsPerCluster();
	*lpBytesPerSector = _pMirrorFile->BytesPerSector();
	*lpNumberOfFreeClusters = _pMirrorFile->NumberOfFreeClusters();
	*lpTotalNumberOfClusters = _pMirrorFile->TotalNumberOfClusters();
	return TRUE;
}

DWORD CSysfs16::GetFileSize(
		HANDLE hFile,			// handle of file to get size of
		LPDWORD lpFileSizeHigh	// pointer to high-order word for file size
		)
{
	ImgFileHandle* handle= (ImgFileHandle*)(hFile);
	if(handle)
	{
		*lpFileSizeHigh = handle->_fileTab.DIR_FileSize;
	}
	return *lpFileSizeHigh;
}

DWORD CSysfs16::GetFullPathName(
		LPCTSTR lpFileName,  // pointer to name of file to find path for
		DWORD nBufferLength, // size, in characters, of path buffer
		LPTSTR lpBuffer,     // pointer to path buffer
		LPTSTR *lpFilePart   // pointer to filename in path
		)
{
	return 0;
}

BOOL CSysfs16::MoveFile(
		LPCTSTR lpExistingFileName, // pointer to the name of the existing file
		LPCTSTR lpNewFileName       // pointer to the new name for the file
		) 
{
	_pMirrorFile->MoveFileEx(lpExistingFileName, lpNewFileName);
	return TRUE;
}

BOOL CSysfs16::ReadFile(
		HANDLE hFile,                // handle of file to read
		LPVOID lpBuffer,             // pointer to buffer that receives data
		DWORD nNumberOfBytesToRead,  // number of bytes to read
		LPDWORD lpNumberOfBytesRead, // pointer to number of bytes read
		LPOVERLAPPED lpOverlapped    // pointer to structure for data
		) 
{
	return _pMirrorFile->ReadFileEx(hFile, (LPBYTE)lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead);
}

BOOL CSysfs16::RemoveDirectory(
		LPCTSTR lpPathName   // pointer to directory to remove
		) 
{
	_pMirrorFile->DeleteDirectoryEx(lpPathName);
	return TRUE;
}

BOOL CSysfs16::SetCurrentDirectory(
		LPCTSTR lpPathName   // pointer to name of new current directory
		) 
{
	char szPath[MAX_PATH];
	strcpy(szPath , lpPathName);

	int len = strlen(szPath);
	if (len>1)
	{
		if (szPath[len-1] == '\\')
			szPath[len-1] = 0;
	}

	if (_pMirrorFile->SetCurrentDirectory(szPath))
	{
		strcpy(_szcurDir , szPath);
		return TRUE;	
	}
	else
	{
		strcpy(_szcurDir , "\\");
		return FALSE;
	}	
	
}

DWORD CSysfs16::SetFilePointer(
		HANDLE hFile,				// handle of file
		LONG lDistanceToMove,		// number of bytes to move file pointer
		PLONG lpDistanceToMoveHigh,	// pointer to high-order DWORD of distance to move
		DWORD dwMoveMethod			// how to move
		)
{
	return _pMirrorFile->SetFilePointerEx(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);	
}

BOOL CSysfs16::SetVolumeLabel(
		LPCTSTR lpRootPathName,  // pointer to name of root directory for volume
		LPCTSTR lpVolumeName	// name for the volume
		)
{
	_pMirrorFile->SetVolLabel(lpVolumeName);
	return TRUE;
}

BOOL CSysfs16::WriteFile(
		HANDLE hFile,                    // handle to file to write to
		LPCVOID lpBuffer,                // pointer to data to write to file
		DWORD nNumberOfBytesToWrite,     // number of bytes to write
		LPDWORD lpNumberOfBytesWritten,  // pointer to number of bytes written
		LPOVERLAPPED lpOverlapped        // pointer to structure for overlapped I/O
		)
{
	return _pMirrorFile->WriteFileEx(hFile, (LPBYTE)lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten);
}

BOOL CSysfs16::LockFile(
		HANDLE hFile,           // handle of file to lock
		DWORD dwFileOffsetLow,  // low-order word of lock region offset
		DWORD dwFileOffsetHigh,  // high-order word of lock region offset
		DWORD nNumberOfBytesToLockLow,		// low-order word of length to lock
		DWORD nNumberOfBytesToLockHigh 		// high-order word of length to lock
		)
{
	return TRUE;
}

BOOL CSysfs16::UnlockFile(
		HANDLE hFile,           // handle of file to unlock
		DWORD dwFileOffsetLow,  // low-order word of lock region offset
		DWORD dwFileOffsetHigh,	// high-order word of lock region offset
		DWORD nNumberOfBytesToUnlockLow,	// low-order word of length to unlock
		DWORD nNumberOfBytesToUnlockHigh	// high-order word of length to unlock
		)
{
	return TRUE;
}

BOOL CSysfs16::CreateDirectoryEx(					// In order to increase speed of creating  directory in current directory
		LPCTSTR lpPathName,                         // pointer to directory path string
		LPSECURITY_ATTRIBUTES lpSecurityAttributes  // pointer to security descriptor
		)
{
	return _pMirrorFile->ImgCreateDirectory(lpPathName);
}

BOOL  CSysfs16::PathFileExist(LPCTSTR pszPath)
{
	HANDLE hfile = CreateFile(pszPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	
	if ( (HANDLE)-1 == hfile) return FALSE;
	else
	{
		CloseFile(hfile);
		return TRUE;
	}
}

BOOL CSysfs16::InsertFile(IN const LPCSTR lpFileName, 
		IN PBYTE  pBuffer,
		IN const WORD bFileAttr,
		IN const UINT nSize,
        HANDLE& hFile
		)
{
	DWORD dwNumerOfFreeCluster,dwBytesPerSector,dwSectorPerCluster
		,dwTotalNumberOfClusters;
	char chRoot[256];
	if(GetDiskFreeSpace(chRoot,&dwSectorPerCluster,&dwBytesPerSector,
		&dwNumerOfFreeCluster,&dwTotalNumberOfClusters)){
		if(dwNumerOfFreeCluster*dwSectorPerCluster*dwBytesPerSector<nSize)
			return FALSE;
		}
	//return _pMirrorFile->ImgCreateFile(lpFileName, pBuffer, (BYTE)bFileAttr, nSize, hFile);

	char	szFullPath[MAX_PATH];
	if (lpFileName[0] != '\\')
	{
		sprintf(szFullPath , "%s\\%s" , _szcurDir , lpFileName);
	}
	else
	{
		strcpy(szFullPath , lpFileName);
	}

	if (PathFileExist(szFullPath))
	{
		DeleteFile(szFullPath);
	}

	HANDLE h = CreateFile(szFullPath, GENERIC_WRITE|GENERIC_READ, NULL, NULL, CREATE_ALWAYS, ATTR_ARCHIVE, NULL);
	if ( (HANDLE)-1 == h)
	{
		//AfxMessageBox("CreateFile Fail");
		return FALSE;
	}		

	DWORD nWritten = 0;
	if (!WriteFile(h , pBuffer , nSize , &nWritten , NULL)) 
		return FALSE;
	
	//CloseFile(h);	
	hFile = h;

	return TRUE;
}
	//

BOOL CSysfs16::GetVolLabel(LPSTR lpLabel)
{
	return _pMirrorFile->GetVolLabel(lpLabel);
}

BOOL CSysfs16::CloseFile(HANDLE handle)
{
	return _pMirrorFile->CloseFileEx(handle);
}

void* CSysfs16::MirrorFile()
{
	return _pMirrorFile;
}


BOOL CSysfs16::IsFoulder(Flag fAttr)
{
	if(IsFile(fAttr))
		return FALSE;
	return ((fAttr& (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_DIRECTORY);
}

BOOL CSysfs16::IsVolLabel(Flag fAttr)
{
	BOOL isVol = FALSE;
	if(IsLongDir(fAttr))
		return FALSE;
	else if(IsFile(fAttr))
		return FALSE;
	else if((fAttr& (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_DIRECTORY)
		return FALSE;
	else if((fAttr& (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_VOLUME_ID)
		return TRUE;
	return FALSE;
}

	
BOOL CSysfs16::RenameFile(LPCTSTR lpSrcFileName,  LPCTSTR lpNewFileName)
{
	return _pMirrorFile->RenameFileEx(lpSrcFileName, lpNewFileName);
}

BOOL  CSysfs16::RefreshFatTable()
{
	return _pMirrorFile->RefreshFatTable();
}

// BOOL  CSysfs16::IsFileExist(LPCSTR lpFileName)
// {
// 	return _pMirrorFile->IsFileExist(lpFileName);
//}

// BOOL  CSysfs16::WriteFile()
// {
// 	return FALSE;
// }