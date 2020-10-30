#pragma once
#include "DiskImgFile.h"

class DiskImgFile32 : public DiskImgFile  
{
public:
	DiskImgFile32();
	virtual ~DiskImgFile32();
public:
	virtual UINT  RelatAddrToCluster(IN UINT uRetAddr);								// 映射函数从实际实际偏移位置到簇
	virtual UINT  ClusterToRelatAddr(IN UINT uCluNm);								// 映射函数从簇到实际偏移位置
	
	BOOL SetCurrentDirectory(IN LPCTSTR lptPathName);						// 设置当前路径	
	BOOL ParaDirectoryFromStr(IN LPCTSTR lptDirName, OUT vector<Fat_Directory>& fatDir);

	BOOL GetDirectoryTab(OUT Fat_Directory& dir, IN UINT clus, IN LPCTSTR longName)	;
	BOOL CreateDirectoryEx(IN LPCTSTR lpFullDir);

	UINT  GetFirstFreeClusNum();											// 返回空闲的簇号，从2开始
	BOOL  GetDirectoryTabEx(OUT Fat_Directory& dir, OUT LPTSTR lptLongName, IN OUT INT& nIndex);	// 返回当前目录下第nIndex个目录
	BOOL  GetDirectoryTab(OUT Fat_Directory& dir, IN OUT INT& nIndex);							// 返回当前目录下第nIndex个目录
//	BOOL  GetDirectoryTab(OUT Fat_Directory& dir, IN LPCSTR longName);							// 返回当前目录长名为longName 的目录

	BOOL  GetDirectoryTabEx(OUT Fat_Directory& dir, IN UINT clus, OUT LPTSTR lptLongName, IN OUT INT& nIndex); // 返回簇clus下目录下第nIndex个目录
//	BOOL  GetDirectoryTab(OUT Fat_Directory& dir, IN UINT clus, IN OUT INT& nIndex);						 // 返回簇clus下目录下第nIndex个目录
//	BOOL  GetDirectoryTab(OUT Fat_Directory& dir, IN UINT clus, IN LPCSTR longName);						 // 返回簇clus下目录下长名为longName的目录

	BOOL  TrimDirEntrySpace();		// 整理删除文件或者目录后FAT_Directory Entries的空间

//////////////////////////////////////////////////////////////////////////
// GetFreeSpace 
		
public:
//////////////////////////////////////////////////////////////////////////
// inside operator
	BOOL  ImgCreateDirectory(IN LPCTSTR lptDirName);							// 在当前目录下建立目录函数 -- 

	
	BOOL  ImgCreateFile(IN const LPCTSTR lptFileName, 
						IN PBYTE pbuffer, 
						IN const BYTE bFileAttr,
						IN const UINT nSize,
						HANDLE& hFile);										// 创建文件函数

	BOOL  ImgDeleteDirectory(IN LPCTSTR lptDirName);							// 当前目录下删除目录函数

	BOOL  ImgDeleteDirectory(UINT clusNum);									// 删除在该簇开始地方的目录 递归调用 

	BOOL  ImgDeleteFile(IN LPCTSTR lptFileName);								// 当前目录下删除文件函数

	BOOL  ImgMoveFile(IN LPCSTR lpFileName, 
					  IN LPCSTR lpSrcDir, 
					  IN LPCSTR lpDesFileName,
					  IN LPCSTR lpDesDir);									// 移动文件函数

	BOOL  ImgMoveDirectory(IN LPCSTR lpSrcDir, 
						   IN LPCSTR lpDesDir);								// 移动目录函数


//	BOOL  ImgGetFileStaus(IN LPCSTR lpFileName,
	//					  OUT ImgFileStatus& status);
//	BOOL  ImgOpenFile(IN LPCSTR lpFileName,									// 打开文件函数
//					  OUT PBYTE pBuffer, 
//					  IN DWORD nBufferLen) ;	


	//////////////////////////////////////////////////////////////////////////
	// virtual Function
	virtual int BytesPerSec()	{return _imgBpb.BPB_BytsPerSec;}
	virtual int SecPerClus()	{return _imgBpb.BPB_SecPerClus;}
	virtual int RsvdSecCnt()	{return _imgBpb.BPB_RsvdSecCnt;}
	virtual int RootEntCnt()	{return _imgBpb.BPB_RootEntCnt;}
	virtual int TolSec()		{return _imgBpb.BPB_TotSec16 + _imgBpb.BPB_TotSec32;}
	virtual int FatSz()			{return _imgBpb.BPB_FATSz16;}
	virtual UINT FatType()		{return FAT32_TYPE;}

	virtual BOOL  SetVolLabel(LPCSTR lpVolLabel);
	virtual BOOL  GetVolLabel(LPSTR lpLabel);

	virtual void  InitializeClus(IN UINT clusNum);									// 初始化簇
	virtual BOOL  SetClus(IN UINT clusNum, IN UINT nValue);							// 设置簇链
	virtual BOOL  SetClusEx(IN UINT StartClusNum, IN UINT nNeedMoreClus);			// 设置簇链 , Add by Joelee
	virtual BOOL  SetClusFreeStatus(IN UINT StartClusNum);

	virtual DWORD SetFilePointerEx(HANDLE hFile,				// handle of file
						LONG lDistanceToMove,		// number of bytes to move file pointer
						PLONG lpDistanceToMoveHigh,	// pointer to high-order DWORD of distance to move
						DWORD dwMoveMethod			// how to move
						);
	virtual BOOL  ReadFileEx(HANDLE hFile,  OUT PBYTE pBuffer, IN DWORD nBufferLen,   OUT PDWORD nRead);

	virtual BOOL  WriteFileEx(HANDLE hFile, IN PBYTE pBuffer, IN DWORD nBufferLen,OUT PDWORD nWrite);

	virtual BOOL  RenameFileEx(LPCSTR lpSrcName, LPCSTR lpNewFileName);

	virtual HANDLE  CreateFileEx(LPCTSTR lpFileName,     
						DWORD dwDesiredAccess,      
						DWORD dwShareMode,          
						LPSECURITY_ATTRIBUTES lpSecurityAttributes,	
						DWORD dwCreationDisposition, 
						DWORD dwFlagsAndAttributes,  
						HANDLE hTemplateFile         
						); 

	virtual HANDLE FindFirstFile(LPCTSTR lptFileName,					// pointer to name of file to search for
		LPWIN32_FIND_DATA lpFindFileData	// pointer to returned information
		);
/*
	virtual HANDLE FindFirstFileEx(
		LPCSTR lpFileName,					// pointer to the name of the file to search for
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

	virtual BOOL FindClose(HANDLE hFindFile);	

//////////////////////////////////////////////////////////////////////////
// GetFreeSpace 

	virtual LPTSTR RootPath();
	virtual DWORD SectorsPerCluster();
	virtual DWORD BytesPerSector();
	virtual DWORD NumberOfFreeClusters();

	
	virtual UINT ImgFileType();


	virtual LPTSTR GetMirrorFileName();
	virtual LONGLONG TotoleSpace();
//////////////////////////////////////////////////////////////////////////
// Outside Operator
	BOOL  CreateImageFile(IN LPCSTR lpFileName, IN UINT fatType = FAT32_TYPE, IN LONGLONG diskSize = 536870912);	 // 536870912 = 256M						// 创建镜像文件

	BOOL  FormatImgFile(IN LPCSTR lpVolLab, IN UINT fatType = FAT16_TYPE, IN LONGLONG diskSize = 536870912);		// 格式化镜像文件

	void  Iinitialize(LPCSTR lpVolab, IN UINT fatType, IN LONGLONG diskSize);					// 初始化

	

	BOOL  OpenImgFile(IN LPCSTR lpFileName, IN LONGLONG diskSize = 536870912 );								// 打开镜像文件

	BOOL RefreshFatTable();
	//BOOL IsFileExist(LPCSTR lpFileName);
	BootSector_BPB32 _imgBpb;	//保留扇区信息
	
protected:
	//virtual BOOL  CalcNewPos(HANDLE hFile);
	virtual BOOL	IniWrite(HANDLE hFile , DWORD dwWriteLen) override;
private:
		
		//BootSector_BPB32 _imgBpb_sFat;	//保留扇区信息
		Fat_FsInfo  _fsInfo;
	//	vector<DWORD>  _fats;
		BYTE*	_imgContent;
};
