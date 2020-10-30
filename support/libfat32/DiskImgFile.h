#pragma once
#include "common.h"
#include "MirrorFileModel.h"


typedef  SYSTEMTIME CTime;

#ifdef _MAC_UNIX
#include "winapi.h"
#include "Support.h"
typedef  SYSTEMTIME CTime;
#endif
#include "MemFS.h"	// Added by ClassView

//#ifdef _WINDOWS
//#define printmsg(x)	AfxMessageBox(_T(x))
//#endif

#define			DEFAULT_VOLUME_NAME		"NO NAME"

struct ImgFileStatus
{	
	CTime m_ctime;          // creation date/time of file
	CTime m_mtime;          // last modification date/time of file
	CTime m_atime;          // last access date/time of file
	LONG m_size;            // logical size of file in bytes
	BYTE m_attribute;       // logical OR of CFile::Attribute enum values
	BYTE _m_padding;        // pad the structure to a WORD
	TCHAR m_szFullName[MAX_PATH]; // absolute path name
};

struct ImgFileHandle		// 定义文件句柄内容
//class ImageFileHandle
{
	HANDLE _handle;
	UINT  _stAddr;			// 文件的物理起始地址
	UINT  _curPos;			// 在镜像文件中该文件当前的实际地址
	UINT  _tabStAddr;		// 文件目录所在地址
	UINT  _curRetpos;		// 文件相对位置(连续的)--文件当前操作到的位置
	DWORD _accMode;
	DWORD _shareMode;		// 访问信息
	Fat_Directory _fileTab; 
	UINT _exceedsize;
};

struct ImgFindHandle // 定义查找句柄
{
	HANDLE _handle;
	UINT   _nIndex;	// 当前目录下Directory entry 的索引
	UINT   _curCluster; // 当前簇的编号
	UINT   _nRetIndex; // 簇内当前位置
	WIN32_FIND_DATA _findData;
	Fat_Directory _curFileTab;
};



enum
{
	ATTR_READ_ONLY  = 0x01,
	ATTR_HIDDEN		= 0x02,
	ATTR_SYSTEM		= 0x04,
	ATTR_VOLUME_ID  = 0x08,
	ATTR_DIRECTORY  = 0x10,
	ATTR_ARCHIVE    = 0x20,
	ATTR_NORMAL		= 0x80,
	ATTR_LONG_NAME  = ATTR_READ_ONLY 
					  |ATTR_HIDDEN 
					  |ATTR_SYSTEM 
					  |ATTR_VOLUME_ID,
					  
	ATTR_LONG_NAME_MASK = ATTR_READ_ONLY 
					|ATTR_HIDDEN 
					|ATTR_SYSTEM 
					|ATTR_VOLUME_ID 
					|ATTR_DIRECTORY
					|ATTR_ARCHIVE,
	LAST_LONG_ENTRY = 0x40,
};

BOOL IsVolLabel(Flag fAttr);
BOOL IsFile(Flag fAttr);
BOOL IsLongDir(Flag fAttr);
BOOL IsFoulder(Flag fAttr);
BOOL IsReadOnly(Flag fAttr);
/*

BOOL IsHide(Flag fAttr);
BOOL IsSystem(Flag fAttr);
BOOL IsReadOnly(Flag fAttr);
BOOL Is(Flag fAttr);*/

class DiskImgFile:public MirrorFile 
{
public:
	DiskImgFile();
	virtual ~DiskImgFile();
public:
//////////////////////////////////////////////////////////////////////////
// Assite Function
	//-----------------------------------------------------------------------------
	// ChkSum()
	// Returns an unsigned byte checksum computed on an unsigned byte
	// array. The array must be 11 bytes long and is assumed to contain
	// a name stored in the format of a MS-DOS directory entry.
	// Passed: pFcbName Pointer to an unsigned byte array assumed to be
	// 11 bytes long.
	// Returns: Sum An 8-bit unsigned checksum of the array pointed
	// to by pFcbName.
	//------------------------------------------------------------------------------
	unsigned char ChkSum (LPCSTR pFcbName);
	BOOL  GenerateShortName(LPCTSTR lpName, LPSTR lpShortName);//LPSTR GenerateShortName(LPCSTR lpName);
	INT	  CalcNeedTabCount(LPCTSTR lptName);									// 根据名字长度计算需要目录项目数目
	BOOL  IsExit(LPCTSTR lpName);											// 判断目录(文件)名在当前目录下是否存在
	BOOL  GetShortNameNumicTail(LPCSTR basicName, char& tailNumic);			// 短名的数字后缀
	
	virtual UINT  RelatAddrToCluster(IN UINT uRetAddr);								// 映射函数从实际实际偏移位置到簇
	virtual UINT  ClusterToRelatAddr(IN UINT uCluNm);								// 映射函数从簇到实际偏移位置
	virtual void  InitializeClus(IN UINT clusNum);									// 初始化簇
	virtual BOOL  SetClus(IN UINT clusNum, IN UINT nValue);							// 设置簇链
	virtual BOOL  SetClusEx(IN UINT StartClusNum, IN UINT nNeedMoreClus);			// 设置簇链 , Add by Joelee
	virtual BOOL  SetClusFreeStatus(IN UINT StartClusNum);							// 删除簇链 , Add by Joelee
	virtual UINT  GetFirstFreeClusNum();									// 返回空闲的簇号，从2开始
	virtual UINT  GetNextFreeClusNum(UINT StartClusNum);						// 返回空闲的簇号，从StartClusNum开始 , Add by Joelee

	BOOL  SetCurrentDirectory(IN LPCTSTR lptPathName);						// 设置当前路径
	BOOL  GetCurrentDirectory(OUT LPTSTR lpBuffer, IN DWORD nBufferLen);		// 返回当前路径

	BOOL  ParaDirectoryFromStr(IN LPCTSTR lptDirName, OUT vector<Fat_Directory>& fatDir); // 解析img目录路径
	BOOL  ParaPathFromStr(IN LPCTSTR lpPath, OUT vector<DirPaths>& paths);			// 解析路径
	virtual BOOL  TrimDirEntrySpace();		// 整理删除文件或者目录后FAT_Directory Entries的空间


	virtual BOOL  GetDirectoryTabEx(OUT Fat_Directory& dir, OUT LPTSTR lptLongName, IN OUT INT& nIndex);	// 返回当前目录下第nIndex个目录
	virtual BOOL  GetDirectoryTab(OUT Fat_Directory& dir, IN OUT INT& nIndex);							// 返回当前目录下第nIndex个目录
	virtual BOOL  GetDirectoryTab(OUT Fat_Directory& dir, IN LPCTSTR longName);							// 返回当前目录长名为longName 的目录

	
	virtual BOOL  GetDirectoryTabEx(OUT Fat_Directory& dir, IN UINT clus, OUT LPTSTR lptLongName, IN OUT INT& nIndex); // 返回簇clus下目录下第nIndex个目录
	virtual BOOL  GetDirectoryTab(OUT Fat_Directory& dir, IN UINT clus, IN OUT INT& nIndex);						 // 返回簇clus下目录下第nIndex个目录
	virtual BOOL  GetDirectoryTab(OUT Fat_Directory& dir, IN UINT clus, IN LPCTSTR longName);						 // 返回簇clus下目录下长名为longName的目录

	virtual BOOL  GetDirectoryLongName(OUT LPTSTR lpBuffer, IN DWORD nBufferLen, IN Fat_Directory dir);	 // 获取长名

	virtual int  strcmpnocase(LPCTSTR string1,LPCTSTR string2 );//strcmp ,忽略大小写

	virtual BOOL RefreshFatTable();
	virtual BOOL  CheckFatType(IN LPCSTR lpFileName , OUT UINT& FATTYPE);

//////////////////////////////////////////////////////////////////////////
// GetFreeSpace 

	virtual LPTSTR RootPath();
	virtual DWORD SectorsPerCluster();
	virtual DWORD BytesPerSector();
	virtual DWORD NumberOfFreeClusters();
	virtual DWORD TotalNumberOfClusters();
		
public:
//////////////////////////////////////////////////////////////////////////
// inside operator
	virtual BOOL  ImgCreateDirectory(IN LPCTSTR lptDirName);							// 在当前目录下建立目录函数 -- 

	
	virtual BOOL  ImgCreateFile(IN const LPCTSTR lpFileName, 
						IN PBYTE pbuffer, 
						IN const BYTE bFileAttr,
						IN const UINT nSize,
						HANDLE& hFile);										// 创建文件函数

	virtual BOOL  ImgDeleteDirectory(IN LPCTSTR lptDirName);							// 当前目录下删除目录函数

	virtual BOOL  ImgDeleteDirectory(UINT clusNum);									// 删除在该簇开始地方的目录 递归调用 

	virtual BOOL  ImgDeleteFile(IN LPCTSTR lptFileName);								// 当前目录下删除文件函数

	virtual BOOL  ImgMoveFile(IN LPCSTR lpFileName, 
					  IN LPCSTR lpSrcDir, 
					  IN LPCSTR lpDesFileName,
					  IN LPCSTR lpDesDir);									// 移动文件函数

	virtual BOOL  ImgMoveDirectory(IN LPCSTR lpSrcDir, 
						   IN LPCSTR lpDesDir);								// 移动目录函数


	virtual BOOL  ImgGetFileStaus(IN LPCTSTR lptFileName,
						  OUT ImgFileStatus& status);
	virtual BOOL  ImgOpenFile(IN LPCSTR lpFileName,									// 打开文件函数
					  OUT PBYTE pBuffer, 
					  IN DWORD nBufferLen) ;	


//////////////////////////////////////////////////////////////////////////
// Outside Operator
	virtual BOOL  CreateImageFile(IN LPCSTR lpFileName, IN UINT fatType = FAT16_TYPE, IN LONGLONG diskSize = 134217728);	 // 134217728 = 128M						// 创建镜像文件

	virtual BOOL  FormatImgFile(IN LPCSTR lpVolLab, IN UINT fatType = FAT16_TYPE, IN LONGLONG diskSize = 134217728);		// 格式化镜像文件

	virtual void  Iinitialize(LPCSTR lpVolab, IN UINT fatType, IN LONGLONG diskSize);					// 初始化

	

	virtual BOOL  OpenImgFile(IN LPCSTR lpFileName, IN UINT fatType, IN LONGLONG diskSize = 134217728);								// 打开镜像文件

	virtual BOOL  CloseImgFile();

	virtual DWORD SetFilePointerEx(HANDLE hFile,				// handle of file
						LONG lDistanceToMove,		// number of bytes to move file pointer
						PLONG lpDistanceToMoveHigh,	// pointer to high-order DWORD of distance to move
						DWORD dwMoveMethod			// how to move
						);

	virtual BOOL  SetVolLabel(LPCSTR lpVolLabel);
	virtual BOOL  GetVolLabel(LPSTR lpLabel);


	BOOL  DeleteDirectoryEx(LPCTSTR lptDirectoryPath);
	BOOL  DeleteFileEx(LPCTSTR lptFilePath);

	BOOL  CreateDirectoryEx(IN LPCTSTR lptFullDir);						// 建立一个路径目录

	virtual HANDLE  CreateFileEx(LPCTSTR lptFileName,     
						DWORD dwDesiredAccess,      
						DWORD dwShareMode,          
						LPSECURITY_ATTRIBUTES lpSecurityAttributes,	
						DWORD dwCreationDisposition, 
						DWORD dwFlagsAndAttributes,  
						HANDLE hTemplateFile         
						); 

	BOOL  CloseFileEx(HANDLE hFile);										// 关闭文件函数

	virtual BOOL  ReadFileEx(HANDLE hFile,  OUT PBYTE pBuffer, IN DWORD nBufferLen,   OUT PDWORD nRead);

	virtual BOOL  WriteFileEx(HANDLE hFile, IN PBYTE pBuffer, IN DWORD nBufferLen,OUT PDWORD nWrite);
	
	virtual BOOL  MoveFileEx(LPCSTR lpSrcName, LPCSTR lpNewFileName);
	
	virtual BOOL  RenameFileEx(LPCSTR lpSrcName, LPCSTR lpNewFileName);
	
	
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
	// virtual Function
	virtual int BytesPerSec()	{return _imgBpb.BPB_BytsPerSec;}
	virtual int SecPerClus()	{return _imgBpb.BPB_SecPerClus;}
	virtual int RsvdSecCnt()	{return _imgBpb.BPB_RsvdSecCnt;}
	virtual int RootEntCnt()	{return _imgBpb.BPB_RootEntCnt;}
	virtual int TolSec()		{return _imgBpb.BPB_TotSec16 + _imgBpb.BPB_TotSec32;}
	virtual int FatSz()			{return _imgBpb.BPB_FATSz16;}

	virtual UINT FatType(){return FAT16_TYPE;}
	virtual UINT ImgFileType() ;
	virtual LPTSTR GetMirrorFileName();
	virtual LONGLONG TotoleSpace();
	virtual BOOL TestFunc();
	virtual BOOL DeleteDirEntry(UINT nClus , int nStartIndex , int nEndIndex );//delete short name entry
//////////////////////////////////////////////////////////////////////////
//	virtual BOOL  IsFileExist(LPCSTR lpFileName);
//////////////////////////////////////////////////////////////////////////
//

public:
	void IniFatType(UINT fatType);
	BOOL RefreshFat12Table();

	LPTSTR	_lpImgFileName;			// 文件名字
	BootSector_BPB _imgBpb;			//保留扇区信息
	UINT	_imgSpace;				// 镜像文件的空间类型 		/*  DISK_32M,	DISK_64M,	DISK_128M,	DISK_256M DISK_512M,	DISK_1G,	DISK_2G*/
	UINT	_stOfClusterAddr;		// 当前镜像文件簇的开始位置（相对地址）
	UINT    _stOfRootAddr;			// 根目录的相对位置
	UINT	_stOfFATAddr;			// FAT的相对位置
	vector<Fat_Directory> _curDirectory;	//当前的操作路径

//#ifdef _FAT32_VERSION
	vector<DWORD>  _fats;
//#else
//	vector<WORD>   _fats;		
//#endif	
	BYTE*	_imgContent;

protected:
	HANDLE _hFile;
	UINT	m_CodePage;
	//virtual BOOL  CalcNewPos(HANDLE hFile);
	virtual BOOL	IniWrite(HANDLE hFile , DWORD dwWriteLen);

private:
	void	GetCRCStr(char* CRCSrcBuf , UINT srclen , char* CRCStr);//CRCStr为4个BYTE
	unsigned short crc16(char *crcarray,int Length);//CRC校验
	UINT m_FatType;
	WORD m_EOC_STATUS;
	

};

