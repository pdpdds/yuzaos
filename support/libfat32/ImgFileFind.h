// ImgFileFind.h: interface for the CImgFileFind class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMGFILEFIND_H__EB76A210_0CB0_46A6_8A92_FF1A7E35E06A__INCLUDED_)
#define AFX_IMGFILEFIND_H__EB76A210_0CB0_46A6_8A92_FF1A7E35E06A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "DiskImgFile.h"
//#include "Ext4ImgFile.h"
#include "sysfs.h"

class CImgFileFind
{
public:
	CImgFileFind(){_lpFileName = NULL; _lpCurFileName = NULL;_imgFile = NULL; _curIndex = -1;}
	CImgFileFind(DiskImgFile* pImgFile);
	CImgFileFind(const CImgFileFind& src);
	~CImgFileFind();
public:
	BOOL SetImgFile(DiskImgFile* pImgFile);
	BOOL SetCurDirectory(LPCSTR lpDirPath);

	BOOL FindFile(LPCSTR lpName);
	BOOL FindNextFile();
	BOOL CloseFind();
	
	BOOL IsDots();
	BOOL IsDirectory();
	BOOL IsVolID();

	BOOL GetFullPathName(LPSTR lpPathName, int nLen);
	LPSTR GetFileName();

	CTime GetFileCrtTime();
	CTime GetFileLstAcceDate();
	CTime GetFileWrTime();
	DWORD GetFileSize();

public:
	DiskImgFile* _imgFile;
	Fat_Directory _curDirTab;
	LPSTR		  _lpFileName;
	LPSTR		  _lpCurFileName;
	int  _curIndex;
	
};

class CFileFindEx
{
public:
	CFileFindEx(){_lpFileName = NULL; _lpCurFileName = NULL; _curIndex = -1;}
	CFileFindEx(CSysfs*  pFSysfs);
	CFileFindEx(const CFileFindEx& src);
	~CFileFindEx();
public:
	BOOL SetSysfs(CSysfs* pSysfs);
	BOOL SetCurDirectory(LPCSTR lpDirPath);

	BOOL FindFile(LPCSTR lpName);
	BOOL FindNextFile();
	BOOL CloseFind();
	
	BOOL IsDots();
	BOOL IsDirectory();
	BOOL IsVolID();

	BOOL GetFullPathName(LPSTR lpPathName, int nLen);
	LPSTR GetFileName();

	CTime GetFileCrtTime();
	CTime GetFileLstAcceDate();
	CTime GetFileWrTime();
	DWORD GetFileSize();

public:
	CSysfs*		  _pSysfs;
	LPSTR		  _lpFileName;
	LPSTR		  _lpCurFileName;
	int			  _curIndex;
	HANDLE		  _hFindHandle;
	WIN32_FIND_DATA _findData;

};


#endif // !defined(AFX_IMGFILEFIND_H__EB76A210_0CB0_46A6_8A92_FF1A7E35E06A__INCLUDED_)
