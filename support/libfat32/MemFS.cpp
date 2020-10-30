#pragma warning(disable:4786)
#include "MemFS.h"
#include <map>
#include <string>
#include "AnsiMemFile.h"
#include <minwinconst.h>

//typedef PathMap std::map<std::string,CAnsiMemFile*>;
//typedef HandleMap std::map<HANDLE,std::string>;
//The g_MapName2File and g_MapHandle2Name do not have to  be synchronous.
//The file beging opened must exist in both map.
extern std::map<std::string,CAnsiMemFile*>* g_MapName2File;//map filename to file object
extern std::map<HANDLE,std::string>*  g_MapHandle2Name;   // map file handle to filename

static DWORD g_CurrentHandle=0;
static DWORD g_nFSRefCount=0;
/**************************************************************************
 *	\par function: 
 *  \param   
 *  \return
 *  \attention ignore the dwDesiredAccess,for the sake of simplifying the procedure
 **************************************************************************/
BOOL RAMPathFileExist(LPCSTR lpFileName)
{
	//DumpMemFS();
	//{find whether the file object exist
	std::map<std::string,CAnsiMemFile*>::iterator itF=g_MapName2File->find(std::string(lpFileName));
	if(itF==g_MapName2File->end())
		return FALSE;
	//}
	return TRUE;
}
HANDLE RAMCreateFile(
  LPCSTR lpFileName,          // pointer to name of the file
  DWORD dwDesiredAccess,       // access (read-write) mode
  DWORD dwShareMode,           // share mode
  SECURITY_ATTRIBUTES* lpSecurityAttributes,
                               // pointer to security attributes
  DWORD dwCreationDisposition,  // how to create
  DWORD dwFlagsAndAttributes,  // file attributes
  HANDLE hTemplateFile         // handle to file with attributes to 
                               // copy
)
{
	std::string strFileName(lpFileName);
	//{find whether the file have been opened.
	std::map<HANDLE,std::string>::iterator itH=g_MapHandle2Name->begin();
	for(;itH!=g_MapHandle2Name->end();itH++){
		if(strFileName==itH->second){
			return itH->first;
		}
	}
	//}
	//{find whether the file object exist
	std::map<std::string,CAnsiMemFile*>::iterator itF=g_MapName2File->find(std::string(lpFileName));
	if(itF==g_MapName2File->end()){
		//The file does not exist in the simple memory file system
		if((CREATE_NEW==dwCreationDisposition||CREATE_ALWAYS==dwCreationDisposition
			||OPEN_ALWAYS==dwCreationDisposition))
		{//{create a memory file object
			CAnsiMemFile *pMemFile=new CAnsiMemFile(8*1024*1024);
			//{Read the file from disk
			//}
			(*g_MapName2File)[strFileName]=pMemFile;
			//}
		}else{
			return (HANDLE)-1;
		}
	}
	//}	
	HANDLE hFile=(HANDLE)++g_CurrentHandle;
	(*g_MapHandle2Name)[hFile]=strFileName;
	return hFile;
}
BOOL RAMDeleteFile(
  LPCSTR lpFileName   // pointer to name of file to delete
)
{ 
	//{find whether the file object exist
	std::map<std::string,CAnsiMemFile*>::iterator itF=g_MapName2File->find(std::string(lpFileName));
	if(itF==g_MapName2File->end())
		return FALSE;
	//}
	//{Delete the file and remove it from the MemFS
	CAnsiMemFile *pMemFile=itF->second;
	delete pMemFile;
	g_MapName2File->erase(itF);
    //}
	//{find whether the file have been opened
	std::string strFileName(lpFileName);
	std::map<HANDLE,std::string>::iterator itH=g_MapHandle2Name->begin();
	for(;itH!=g_MapHandle2Name->end();itH++){
		if(strFileName==itH->second){
			g_MapHandle2Name->erase(itH);
			break;
		}
	}
	//}
	return TRUE;        
}

DWORD RAMGetFileAttributes(
  LPCSTR lpFileName   // pointer to the name of a file or directory
)
{
	//if the file does not exist,return 0xFFFFFFFF
	//{find whether the file object exist
	std::map<std::string,CAnsiMemFile*>::iterator itF=g_MapName2File->find(std::string(lpFileName));
	if(itF==g_MapName2File->end())
		return 0xFFFFFFFF;
	//}
	return FILE_ATTRIBUTE_NORMAL;
}
BOOL RAMCloseHandle(
  HANDLE hObject   // handle to object to close
)
{
	std::map<HANDLE,std::string>::iterator itH=g_MapHandle2Name->find(hObject);
	if(g_MapHandle2Name->end()!=itH){
		g_MapHandle2Name->erase(itH->first);
		return TRUE;
	}
	return FALSE;
}
BOOL RAMReadFile(
  HANDLE hFile,                // handle of file to read
  void* lpBuffer,             // pointer to buffer that receives data
  DWORD nNumberOfBytesToRead,  // number of bytes to read
  DWORD* lpNumberOfBytesRead, // pointer to number of bytes read
  OVERLAPPED* lpOverlapped        // pointer to structure for overlapped I/O
)
{
	std::map<HANDLE,std::string>::iterator itH=g_MapHandle2Name->find(hFile);
	if(g_MapHandle2Name->end()==itH){
		return FALSE;
	}
	std::string strFileName=itH->second;
	std::map<std::string,CAnsiMemFile*>::iterator itF=g_MapName2File->find(strFileName);
	if(g_MapName2File->end()==itF)
		return FALSE;
	CAnsiMemFile *pMemFile=itF->second;
	DWORD dwRead=pMemFile->Read(lpBuffer,nNumberOfBytesToRead);
	if(lpNumberOfBytesRead)
		*lpNumberOfBytesRead=dwRead;
	return TRUE;
}
BOOL RAMWriteFile(
  HANDLE hFile,                    // handle to file to write to
  const void* lpBuffer,                // pointer to data to write to file
  DWORD nNumberOfBytesToWrite,     // number of bytes to write
  DWORD* lpNumberOfBytesWritten,  // pointer to number of bytes written
  OVERLAPPED* lpOverlapped        // pointer to structure for overlapped I/O
)
{
	std::map<HANDLE,std::string>::iterator itH=g_MapHandle2Name->find(hFile);
	if(g_MapHandle2Name->end()==itH){
		return FALSE;
	}
	std::string strFileName=itH->second;
	std::map<std::string,CAnsiMemFile*>::iterator itF=g_MapName2File->find(strFileName);
	if(g_MapName2File->end()==itF)
		return FALSE;
	CAnsiMemFile *pMemFile=itF->second;
	if(lpNumberOfBytesWritten)
		*lpNumberOfBytesWritten=nNumberOfBytesToWrite;
	pMemFile->Write(lpBuffer,nNumberOfBytesToWrite);
	return TRUE;
}
//lpDistanceToMoveHigh 
//Pointer to the high-order 32 bits of the signed 64-bit distance to move. 
//If you do not need the high-order 32 bits, this pointer may be NULL.
// When non-NULL, this parameter also receives the high-order DWORD of the new value of 
//the file pointer. 
DWORD RAMSetFilePointer(
						HANDLE hFile,          // handle of file
						ULONGLONG lDistanceToMove,  // number of bytes to move file pointer
						LONG* lpDistanceToMoveHigh,
						// pointer to high-order DWORD of 
						// distance to move
						DWORD dwMoveMethod     // how to move
						)
{
	std::map<HANDLE,std::string>::iterator itH=g_MapHandle2Name->find(hFile);
	if(g_MapHandle2Name->end()==itH){
		return  0xFFFFFFFF;
	}
	std::string strFileName=itH->second;
	std::map<std::string,CAnsiMemFile*>::iterator itF=g_MapName2File->find(strFileName);
	if(g_MapName2File->end()==itF)
		return  0xFFFFFFFF;
	CAnsiMemFile *pMemFile=itF->second;
	if(lpDistanceToMoveHigh)
		*lpDistanceToMoveHigh=0;
	DWORD dwRet=pMemFile->Seek(lDistanceToMove,dwMoveMethod);
	return dwRet;
}

BOOL RAMFlushFileBuffers(
  HANDLE hFile   // open handle to file whose buffers are to be 
                 // flushed
)
{
	return TRUE;
}
BOOL RAMSetEndOfFile(
  HANDLE hFile   // handle of file whose EOF is to be set
)
{
	std::map<HANDLE,std::string>::iterator itH=g_MapHandle2Name->find(hFile);
	if(g_MapHandle2Name->end()==itH){
		return  FALSE;
	}
	std::string strFileName=itH->second;
	std::map<std::string,CAnsiMemFile*>::iterator itF=g_MapName2File->find(strFileName);
	if(g_MapName2File->end()==itF)
		return  FALSE;
	CAnsiMemFile *pMemFile=itF->second;
	pMemFile->SeekToEnd();
	return TRUE;
}

DWORD RAMGetFileSize(
  HANDLE hFile,  // handle of file to get size of
  DWORD* lpFileSizeHigh 
                 // pointer to high-order word for file size
)
{
	if(lpFileSizeHigh)
		*lpFileSizeHigh=0;
	std::map<HANDLE,std::string>::iterator itH=g_MapHandle2Name->find(hFile);
	if(g_MapHandle2Name->end()==itH){
		return  0;
	}
	std::string strFileName=itH->second;
	std::map<std::string,CAnsiMemFile*>::iterator itF=g_MapName2File->find(strFileName);
	if(g_MapName2File->end()==itF)
		return  0;
	CAnsiMemFile *pMemFile=itF->second;
	DWORD dwRet=pMemFile->GetLength();
	return dwRet;
}
/**************************************************************************
 *	\par function: 
 *  \param   
 *  \return
 *  \attention Simplely return TRUE,because we do not need to lock a file.
 **************************************************************************/
BOOL RAMLockFile(
  HANDLE hFile,           // handle of file to lock
  DWORD dwFileOffsetLow,  // low-order word of lock region offset
  DWORD dwFileOffsetHigh,  // high-order word of lock region offset
  DWORD nNumberOfBytesToLockLow,
                          // low-order word of length to lock
  DWORD nNumberOfBytesToLockHigh 
                          // high-order word of length to lock
)
{
	return TRUE;
}
BOOL RAMLockFileEx(
  HANDLE hFile,      // handle of file to lock
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
	return TRUE;
}
BOOL RAMUnlockFile(
  HANDLE hFile,           // handle of file to unlock
  DWORD dwFileOffsetLow,  // low-order word of lock region offset
  DWORD dwFileOffsetHigh,  // high-order word of lock region offset
  DWORD nNumberOfBytesToUnlockLow,
                          // low-order word of length to unlock
  DWORD nNumberOfBytesToUnlockHigh 
                          // high-order word of length to unlock
						  )
{
	return TRUE;
}
BOOL RAMUnlockFileEx(
  HANDLE hFile,      // handle of file to unlock
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
	return TRUE;
}
BOOL RAMInitialMemFS()
{
	g_nFSRefCount++;
	return TRUE;
}
BOOL RAMCloseMemFS()
{
	g_nFSRefCount--;
	if(g_nFSRefCount>0)
		return FALSE;
	std::map<std::string,CAnsiMemFile*>::iterator itF=g_MapName2File->begin();
	CAnsiMemFile *pMemFile=NULL;
	for(;itF!=g_MapName2File->end();itF++){
		//TRACE("%s\n",itF->first.c_str());
		pMemFile=itF->second;
		delete pMemFile;pMemFile=NULL;
		//g_MapName2File.erase(itF);
	}
	g_MapName2File->clear();
	g_MapHandle2Name->clear();
//	g_MapName2File.erase(g_MapName2File.begin(),g_MapName2File.end());
//	g_MapHandle2Name.erase(g_MapHandle2Name.begin(),g_MapHandle2Name.end());
	//}
	return TRUE;
}
BOOL RAMAddFile2MemFS(
  LPCSTR lpFileName,
  const BYTE*  pbFileBuffer,
  DWORD   dwBufferLength
)
{
	std::string strFileName(lpFileName);
	std::map<std::string,CAnsiMemFile*>::iterator itF=g_MapName2File->find(std::string(lpFileName));
	if(g_MapName2File->end()!=itF){
		return FALSE;
	}
	CAnsiMemFile *pMemFile=new CAnsiMemFile(8*1024*1024);
	pMemFile->Write(pbFileBuffer,dwBufferLength);
	pMemFile->SeekToBegin();
//	CAnsiMemFile *pMemFile=new CAnsiMemFile(pbFileBuffer,dwBufferLength);
	(*g_MapName2File)[strFileName]=pMemFile;
	return TRUE;
}
/* There is not necessary to using RAMGetFileFromMemFS,because  
 * you can use create to get a handle from MemFS,and then SetFilePointer,
 * Read and Write the file.
*/
BOOL RAMGetFileFromMemFS(
  LPCSTR lpFileName,
  BYTE*  pbFileBuffer,
  DWORD   dwBufferLength
)
{
	std::string strFileName(lpFileName);
	std::map<std::string,CAnsiMemFile*>::iterator itF=g_MapName2File->find(std::string(lpFileName));
	if(g_MapName2File->end()==itF){
		return FALSE;
	}
	CAnsiMemFile *pMemFile=itF->second;
	pMemFile->SeekToBegin();
	DWORD dwRead=pMemFile->Read(pbFileBuffer,dwBufferLength);
	return dwRead==dwBufferLength;
}
void DumpMemFS()
{
	std::map<std::string,CAnsiMemFile*>::iterator itF=g_MapName2File->begin();
	CAnsiMemFile *pMemFile=NULL;
	for(;itF!=g_MapName2File->end();itF++){
		std::string strName=itF->first;
		pMemFile=itF->second;
#ifdef SQLITE_DEBUG
		TRACE3("name:%s,size:%d\n",itF->first.c_str(),pMemFile->GetLength());
#endif
	}
}
