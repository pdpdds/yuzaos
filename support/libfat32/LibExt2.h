#ifndef _LIB_EXT2_INCLUDED_H_
#define _LIB_EXT2_INCLUDED_H_
//__Ext2Open,if the file exist,then open with ignoring the other two parameters,
//else Open a new file
#include <vector>
#include <string>
#include <map>
#include "winapi.h"

extern "C"
{

extern BOOL FatAddEncryptFile(LPCTSTR pszExt2PathName,LPCTSTR pszDestDir,
						PBYTE pbBuffer,DWORD dwLength,LPCTSTR psaFileName);
extern int FatReadEncryptFile(LPCTSTR pszExt2PathName,LPCTSTR pszDestDir,DWORD dwPos,
						PBYTE pbBuffer,DWORD dwLength,  LPCTSTR psaFileName);

extern void FSOpen(BYTE bType,LPCTSTR pszPathName,DWORD dwBlockSize,DWORD dwLogSize,ULONGLONG dwSize ,
				   DWORD resvSize = 0 , BOOL bToMen = TRUE);
extern void FSClose(LPCTSTR pszExt2PathName);
}

#endif	
			
