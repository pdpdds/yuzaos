// ImgFileFind.cpp: implementation of the CImgFileFind class.
//
//////////////////////////////////////////////////////////////////////

#include "ImgFileFind.h"
//#include "extfscommon.h"

// #ifdef _DEBUG
// #undef THIS_FILE
// static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
// #endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
// class CImgFileFind
//////////////////////////////////////////////////////////////////////////
CImgFileFind::CImgFileFind(DiskImgFile* pImgFile)
{
	_imgFile = pImgFile;
	_curIndex = -1;
	_lpFileName = NULL;
	_lpCurFileName = NULL;
}

CImgFileFind::CImgFileFind(const CImgFileFind& src)
{
	_imgFile = src._imgFile;
	_curIndex = -1;
	_lpFileName = NULL;
	_lpCurFileName = NULL;
}



CImgFileFind::~CImgFileFind()
{
	if(_lpCurFileName)
		delete _lpCurFileName;
	if(_lpFileName)
		delete _lpFileName;

}

BOOL CImgFileFind::SetImgFile(DiskImgFile* pImgFile)
{
	_imgFile = pImgFile;
	return TRUE;
}
	
BOOL CImgFileFind::SetCurDirectory(LPCSTR lpDirPath)
{
	_imgFile->SetCurrentDirectory(lpDirPath);
	return TRUE;
}

BOOL CImgFileFind::FindFile(LPCSTR lpName)
{
	if(_lpFileName)
	{
		delete _lpFileName;
		_lpFileName = NULL;
	}

	if(lpName == NULL)
	{
		_lpFileName = NULL;
		_curIndex = 0;
		return TRUE;
	}
	int nLen = strlen(lpName);
	if(nLen == 0)
	{
		_lpFileName = NULL;
	}
	else
	{
		_lpFileName = new char[nLen + 1];	
		strcpy(_lpFileName, lpName);
		_lpFileName[nLen] = '\0';
	}
	
	if(_imgFile == NULL)
		return FALSE;

	_curIndex = 0;
	return TRUE;
}

BOOL CImgFileFind::FindNextFile()
{
	if(_curIndex == -1)
		return FALSE;

	char lpLongName[MAX_PATH];
	memset(lpLongName, 0x00, MAX_PATH);

	if(_imgFile->GetDirectoryTabEx(_curDirTab, lpLongName, _curIndex))
	{
		if(_lpCurFileName)
			delete _lpCurFileName;
		_lpCurFileName = GetStrFromChArry(lpLongName, MAX_PATH);
		if(_lpFileName == NULL)
			return TRUE;
		else
		{
			if(strcmp(lpLongName, _lpFileName) == 0)
				return TRUE;
			else
			{
				while(strcmp(lpLongName, _lpFileName))
				{
					memset(lpLongName, 0x00, MAX_PATH);
					if(!_imgFile->GetDirectoryTabEx(_curDirTab, lpLongName, _curIndex))
						return FALSE;
				}
			}
		}
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CImgFileFind::CloseFind()
{
	_curIndex = -1;
	_lpCurFileName = NULL;

	return TRUE;
}
	
BOOL CImgFileFind::IsDots()
{
	char lpsz[9];
	memset(lpsz, 0x00,9) ;
	memcpy(lpsz, _curDirTab.DIR_Name, 8);
	lpsz[8] = '\0';
	return((strcmp(lpsz, ".       ") == 0) || (strcmp(lpsz, "..      ") == 0));
}

BOOL CImgFileFind::IsDirectory()
{
	return(IsFoulder(_curDirTab.DIR_Attr));
}

BOOL CImgFileFind::IsVolID()
{
	return(IsVolLabel(_curDirTab.DIR_Attr));
}

BOOL CImgFileFind::GetFullPathName(LPSTR lpPathName, int nLen)
{
	//LPSTR pch = new char[MAX_PATH];
	memset(lpPathName, 0x00, nLen);
	if(!_imgFile->GetCurrentDirectory(lpPathName, nLen))
		return FALSE;
	if(strcmp(lpPathName, "\\") == 0)
		strcat(lpPathName, _lpCurFileName);
	else
	{
		strcat(lpPathName, "\\");
		strcat(lpPathName, _lpCurFileName);
	}	
	return TRUE;
}

LPSTR CImgFileFind::GetFileName()
{
	return _lpCurFileName;	
}



CTime CImgFileFind::GetFileCrtTime()
{
	return this->_curDirTab.DIR_CrtDateTime;
}

CTime CImgFileFind::GetFileLstAcceDate()
{
	return this->_curDirTab.DIR_LstAcceDate;
}
	
CTime CImgFileFind::GetFileWrTime()
{
	return this->_curDirTab.DIR_WrtDateTime;
}

DWORD CImgFileFind::GetFileSize()
{
	return this->_curDirTab.DIR_FileSize;
}



CFileFindEx::CFileFindEx(CSysfs* pFSysfs)
{
	_pSysfs = pFSysfs;
	_curIndex = -1;
	_lpFileName = NULL;
	_lpCurFileName = new char[MAX_PATH];
}

CFileFindEx::CFileFindEx(const CFileFindEx& src)
{
	_pSysfs = src._pSysfs;
	_curIndex = -1;
	_lpFileName = NULL;
	_lpCurFileName = NULL;
	memset(&_findData, 0x00, sizeof(_findData));
}



CFileFindEx::~CFileFindEx()
{
	if(_lpCurFileName)
		delete _lpCurFileName;
	if(_lpFileName)
		delete _lpFileName;

}

BOOL CFileFindEx::SetSysfs(CSysfs* pSysfs)
{
//	_imgFile = pImgFile;
	_pSysfs = pSysfs;
	return TRUE;
}
	
BOOL CFileFindEx::SetCurDirectory(LPCSTR lpDirPath)
{
	_pSysfs->SetCurrentDirectory(lpDirPath);
	return TRUE;
}

BOOL CFileFindEx::FindFile(LPCSTR lpName)
{
	if(_lpFileName)
	{
		delete _lpFileName;
		_lpFileName = NULL;
	}

	if(lpName == NULL)
	{
		if(_lpFileName)
			delete _lpFileName;
		_lpFileName = NULL;
		_curIndex = 0;
		_hFindHandle = _pSysfs->FindFirstFile(NULL, NULL);
		if(!_hFindHandle )
			return FALSE;
		return TRUE;
	}
	int nLen = strlen(lpName);
	if(nLen == 0)
	{
		if(_lpFileName)
			delete _lpFileName;
		_lpFileName = NULL;
	}
	else
	{
		_lpFileName = new char[nLen + 1];	
		strcpy(_lpFileName, lpName);
		_lpFileName[nLen] = '\0';
	}
	
	if(_pSysfs == NULL)
		return FALSE;

	_hFindHandle = _pSysfs->FindFirstFile(NULL, NULL);
	if(!_hFindHandle )
		return FALSE;
	_curIndex = 0;
	return TRUE;
}
BOOL CFileFindEx::FindNextFile()
{
	if(_curIndex == -1)
	{
		return FALSE;
	}

	BOOL ret =  _pSysfs->FindNextFile(_hFindHandle, &_findData);
	if(ret)
	{
		memset(_lpCurFileName, 0x00, MAX_PATH);
		strcpy(_lpCurFileName, _findData.cFileName);
	}
	return ret;

}
BOOL CFileFindEx::CloseFind()
{
	_curIndex = -1;
//	_lpCurFileName = NULL;
	_pSysfs->FindClose(_hFindHandle);
	return TRUE;
}
	
BOOL CFileFindEx::IsDots()
{
	return((strcmp(_findData.cFileName, ".") == 0) || (strcmp(_findData.cFileName, "..") == 0));
}

BOOL CFileFindEx::IsDirectory()
{
	return(_pSysfs->IsFoulder(_findData.dwFileAttributes));
}

BOOL CFileFindEx::IsVolID()
{
	return (_pSysfs->IsVolLabel(_findData.dwFileAttributes));
}

BOOL CFileFindEx::GetFullPathName(LPSTR lpPathName, int nLen)
{
	//LPSTR pch = new char[MAX_PATH];
	memset(lpPathName, 0x00, nLen);
	if(!_pSysfs->GetCurrentDirectory(nLen, lpPathName))
		return FALSE;
	if(strcmp(lpPathName, "\\") == 0)
		strcat(lpPathName, _lpCurFileName);
	else
	{
		strcat(lpPathName, "\\");
		strcat(lpPathName, _lpCurFileName);
	}	
	return TRUE;
}

LPSTR CFileFindEx::GetFileName()
{
	return _lpCurFileName;	
}



CTime CFileFindEx::GetFileCrtTime()
{
/*
	__u32 t;
	FileTimeToTime_t(_findData.ftCreationTime, t);
*/
	//wbt mod
	//return CTime(_findData.ftCreationTime);
	return CTime();
}

CTime CFileFindEx::GetFileLstAcceDate()
{
/*
	__u32 t;
	FileTimeToTime_t(_findData.ftLastAccessTime, t);
*/
	//wbt mod
	//return CTime(_findData.ftLastAccessTime);
	return CTime();
}
	
CTime CFileFindEx::GetFileWrTime()
{
/*
	__u32 t;
	FileTimeToTime_t(_findData.ftLastWriteTime, t);
*/
	//wbt mod
	//return CTime(_findData.ftLastWriteTime);
	return CTime();
}

DWORD CFileFindEx::GetFileSize()
{
	return _findData.nFileSizeLow;
}
