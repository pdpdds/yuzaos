// Sysfs32.h: interface for the CSysfs32 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYSFS32_H__12ACA834_EF1F_4556_B547_599216F3359A__INCLUDED_)
#define AFX_SYSFS32_H__12ACA834_EF1F_4556_B547_599216F3359A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "sysfs.h"
#include "DiskImgFile32.h"
//#include "imgfilefind.h"
class CSysfs32 : public CSysfs  
{
public:
	CSysfs32();
	virtual ~CSysfs32();
//////////////////////////////////////////////////////////////////////////
// Interface
public:
	virtual BOOL Release();

	virtual BOOL Mount(
		LPCTSTR lpImageFile,	// in, pointer to name of the image file, if file not exist, create it.
		LPCTSTR lpRoot,		// out,pointer to name of the root
		LONGLONG uVDiskSize,		// in/out, size of virtual disk
		UINT     nBlockSize = 0,
		UINT     nLogBlkSize = 0,
		CFSService::FSType type=CFSService::FS_WIN_DISK
		);

	virtual BOOL UnMount(
		LPCTSTR lpRoot		// pointer to name of the root
		);

	virtual BOOL Format(
		UINT uFileSystem,	//system id(FAT16:0, FAT32:1)
		LPCTSTR lpLabel,	// pointer to name of the label
		BYTE uSectorPerCluster,	// sector of per cluster
		UINT     nBlockSize = 0
		);

	virtual BOOL FindClose(
		HANDLE hFindFile   // file search handle
		);

	virtual BOOL CopyFile(
		LPCTSTR lpExistingFileName,	// pointer to name of an existing file
		LPCTSTR lpNewFileName,		// pointer to filename to copy to
		BOOL bFailIfExists			// flag for operation if file exists
		);

	virtual BOOL CreateDirectory(
		LPCTSTR lpPathName,                         // pointer to directory path string
		LPSECURITY_ATTRIBUTES lpSecurityAttributes  // pointer to security descriptor
		);

	virtual HANDLE CreateFile(
		LPCTSTR lpFileName,          // pointer to name of the file
		DWORD dwDesiredAccess,       // access (read-write) mode
		DWORD dwShareMode,           // share mode
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// pointer to security attributes
		DWORD dwCreationDisposition, // how to create
		DWORD dwFlagsAndAttributes,  // file attributes
		HANDLE hTemplateFile         // handle to file with attributes to copy
		) ;

	virtual BOOL DeleteFile(
		LPCTSTR lpFileName   // pointer to name of file to delete
		);

	virtual HANDLE FindFirstFile(
		LPCTSTR lpFileName,					// pointer to name of file to search for
		LPWIN32_FIND_DATA lpFindFileData	// pointer to returned information
		);
/*
	virtual HANDLE FindFirstFileEx(
		LPCTSTR lpFileName,					// pointer to the name of the file to search for
		FINDEX_INFO_LEVELS fInfoLevelId,	// information level of the returned data
		LPVOID lpFindFileData,				// pointer to the returned information
		FINDEX_SEARCH_OPS fSearchOp,		// type of filtering to perform
		LPVOID lpSearchFilter,				// pointer to search criteria
		DWORD dwAdditionalFlags				// additional search control flags
		);
*/
	virtual BOOL FindNextFile(
		HANDLE hFindFile,					// handle to search
		LPWIN32_FIND_DATA lpFindFileData	// pointer to structure for data on found file
		) ;

	virtual DWORD GetCurrentDirectory(
		DWORD nBufferLength,  // size, in characters, of directory buffer
		LPTSTR lpBuffer       // pointer to buffer for current directory
		);

	virtual BOOL GetDiskFreeSpace(
		LPTSTR lpRootPathName,			// pointer to root path
		LPDWORD lpSectorsPerCluster,	// pointer to sectors per cluster
		LPDWORD lpBytesPerSector,		// pointer to bytes per sector
		LPDWORD lpNumberOfFreeClusters,	// pointer to number of free clusters
		LPDWORD lpTotalNumberOfClusters	// pointer to total number of clusters
		);

	virtual DWORD GetFileSize(
		HANDLE hFile,			// handle of file to get size of
		LPDWORD lpFileSizeHigh	// pointer to high-order word for file size
		);

	virtual DWORD GetFullPathName(
		LPCTSTR lpFileName,  // pointer to name of file to find path for
		DWORD nBufferLength, // size, in characters, of path buffer
		LPTSTR lpBuffer,     // pointer to path buffer
		LPTSTR *lpFilePart   // pointer to filename in path
		);

	virtual BOOL MoveFile(
		LPCTSTR lpExistingFileName, // pointer to the name of the existing file
		LPCTSTR lpNewFileName       // pointer to the new name for the file
		) ;

	virtual BOOL ReadFile(
		HANDLE hFile,                // handle of file to read
		LPVOID lpBuffer,             // pointer to buffer that receives data
		DWORD nNumberOfBytesToRead,  // number of bytes to read
		LPDWORD lpNumberOfBytesRead, // pointer to number of bytes read
		LPOVERLAPPED lpOverlapped    // pointer to structure for data
		) ;

	virtual BOOL RemoveDirectory(
		LPCTSTR lpPathName   // pointer to directory to remove
		) ;

	virtual BOOL SetCurrentDirectory(
		LPCTSTR lpPathName   // pointer to name of new current directory
		) ;

	virtual DWORD SetFilePointer(
		HANDLE hFile,				// handle of file
		LONG lDistanceToMove,		// number of bytes to move file pointer
		PLONG lpDistanceToMoveHigh,	// pointer to high-order DWORD of distance to move
		DWORD dwMoveMethod			// how to move
		);

	virtual BOOL SetVolumeLabel(
		LPCTSTR lpRootPathName,  // pointer to name of root directory for volume
		LPCTSTR lpVolumeName	// name for the volume
		);

	virtual BOOL WriteFile(
		HANDLE hFile,                    // handle to file to write to
		LPCVOID lpBuffer,                // pointer to data to write to file
		DWORD nNumberOfBytesToWrite,     // number of bytes to write
		LPDWORD lpNumberOfBytesWritten,  // pointer to number of bytes written
		LPOVERLAPPED lpOverlapped        // pointer to structure for overlapped I/O
		);

	virtual BOOL LockFile(
		HANDLE hFile,           // handle of file to lock
		DWORD dwFileOffsetLow,  // low-order word of lock region offset
		DWORD dwFileOffsetHigh,  // high-order word of lock region offset
		DWORD nNumberOfBytesToLockLow,		// low-order word of length to lock
		DWORD nNumberOfBytesToLockHigh 		// high-order word of length to lock
		);

	virtual BOOL UnlockFile(
		HANDLE hFile,           // handle of file to unlock
		DWORD dwFileOffsetLow,  // low-order word of lock region offset
		DWORD dwFileOffsetHigh,	// high-order word of lock region offset
		DWORD nNumberOfBytesToUnlockLow,	// low-order word of length to unlock
		DWORD nNumberOfBytesToUnlockHigh	// high-order word of length to unlock
		);

	virtual BOOL RenameFile(LPCTSTR lpSrcFileName,  LPCTSTR lpNewFileName);
	//////////////////////////////////////////////////////////////////////////
	// Add by lhs. Just for test. 
	virtual BOOL CreateDirectoryEx(					// In order to increase speed of creating directory in current directory
		LPCTSTR lpPathName,                         // pointer to directory path string
		LPSECURITY_ATTRIBUTES lpSecurityAttributes  // pointer to security descriptor
		);

	virtual BOOL InsertFile(IN const LPCSTR lpFileName, 
		IN PBYTE  pBuffer,
		IN const WORD bFileAttr,
		IN const UINT nSize,
        HANDLE& hFile
		);	//
	virtual BOOL  GetVolLabel(LPSTR lpLabel);
	virtual BOOL  CloseFile(HANDLE handle);
	virtual void* MirrorFile();
	virtual BOOL IsFoulder(Flag fAttr);
	virtual BOOL IsVolLabel(Flag fAttr);

//	BOOL IsFileExist(LPCSTR lpFileName);
	BOOL  RefreshFatTable();
   // virtual	BOOL  WriteFile();
	virtual	BOOL PathFileExist(LPCTSTR pszPath);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Arrtirbute
private:
	DiskImgFile32* _pMirrorFile;				// ¾µÏñÎÄ¼þ
	//CImgFileFind _fileFind;
	ULONGLONG		 _diskSize;
	
//	BOOL PathFileExistsEx(LPCTSTR pszPath){return FALSE;};
	
	char	_szcurDir[MAX_PATH];
};


#endif // !defined(AFX_SYSFS32_H__12ACA834_EF1F_4556_B547_599216F3359A__INCLUDED_)
