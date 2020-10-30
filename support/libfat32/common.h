//////////////////////////////////////////////////////////////////////////
// FileName: Common.h 
//
// Describe: define the common param
// Anthor : lhs

#ifdef _MSC_VER 
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp strnicmp
#define strcasecmp stricmp
#endif
// 

// _FAT32_VERSION FAT32
#if !defined _INCLUDE_COMMON_H1
#define _INCLUDE_COMMON_H1
#include <vector>
#include <size_t.h>
#include <minwindef.h>
#include <minwinconst.h>
#include "winapi.h"
#include "Support.h"
//#include "..\wintypedef.h"
using namespace std;
#define FREE_STATUS 0x0000
#define BAD_STATUS12  0x0FF7
#define EOC_STATUS12  0x0FFF
#define BAD_STATUS16  0xFFF7
#define EOC_STATUS16  0xFFFF
#define BYSPERSEC  512			
#define ROOTENTCNT	512			


typedef UINT Flag;

#ifdef _MSC_VER
#pragma pack(push,1)
#endif
 
//xm.BootSector_BPB_RW
typedef struct _tagBootSector_BPB_RW  
{
	BYTE BS_jmpBoot[3];		//0
	CHAR BS_OEMName[8];		//3
	BYTE BPB_BytsPerSec[2];	//11
	BYTE BPB_SecPerClus;	//13
	BYTE BPB_RsvdSecCnt[2];	//14
	BYTE BPB_NumFATs;		//16
	BYTE BPB_RootEntCnt[2];	//17
	BYTE BPB_TotSec16[2];		//19
	BYTE BPB_Media;			//21
	BYTE BPB_FATSz16[2];		//22
	BYTE BPB_SecPerTrk[2];		//24
	BYTE BPB_NumHeads[2];		//26
	BYTE BPB_HiddSec[4];		//28
	BYTE BPB_TotSec32[4];		//32

	BYTE BPB_OTHERS[474];	//36
/*
	BYTE BS_DrvNum;			//36
	BYTE BS_Reserved1;		//37
	BYTE BS_BootSig;		//38 
	BYTE BS_VolID[4];			//39  
	CHAR  BS_VolLab[11];	//43 
	CHAR  BS_FilSysType[8];	//54 
	CHAR BR_RESSERVER[448]; // 62
*/
	BYTE BPB_EOC[2];		// 510
#ifdef _FAT32_VERSION
#endif	

}BootSector_BPB_RW, *PBootSector_BPB_RW;

//xm.BootSector_BPB
typedef struct _tagBootSector_BPB		
{
	BYTE BS_jmpBoot[3];		//0
	CHAR BS_OEMName[8];		//3
	WORD BPB_BytsPerSec;	//11
	BYTE BPB_SecPerClus;	//13
	WORD BPB_RsvdSecCnt;	//14
	BYTE BPB_NumFATs;		//16
	WORD BPB_RootEntCnt;	//17
	WORD BPB_TotSec16;		//19
	BYTE BPB_Media;			//21
	WORD BPB_FATSz16;		//22
	WORD BPB_SecPerTrk;		//24
	WORD BPB_NumHeads;		//26
	DWORD BPB_HiddSec;		//28
	DWORD BPB_TotSec32;		//32

	BYTE BS_DrvNum;			//36
	BYTE BS_Reserved1;		//37
	BYTE BS_BootSig;		//38 
	DWORD BS_VolID;			//39  
	CHAR  BS_VolLab[11];	//43 
	CHAR  BS_FilSysType[8];	//54 
#ifdef _FAT32_VERSION
#endif		
}BootSector_BPB, PBootSector_BPB;

typedef struct _tagBootSector_BPB32
{
	BYTE BS_jmpBoot[3];		//0
	CHAR BS_OEMName[8];		//3
	WORD BPB_BytsPerSec;	//11
	BYTE BPB_SecPerClus;	//13
	WORD BPB_RsvdSecCnt;	//14
	BYTE BPB_NumFATs;		//16
	WORD BPB_RootEntCnt;	//17
	WORD BPB_TotSec16;		//19
	BYTE BPB_Media;			//21
	WORD BPB_FATSz16;		//22
	WORD BPB_SecPerTrk;		//24
	WORD BPB_NumHeads;		//26
	DWORD BPB_HiddSec;		//28
	DWORD BPB_TotSec32;		//32

	DWORD BPB_FATSz32;		//36 4 
	WORD  PB_ExtFlags;		//40 2 
	WORD  BPB_FSVer;		//42 2 
	DWORD BPB_RootClus;		//44 4 
	WORD  BPB_FSInfo;		//48 2 
	WORD  BPB_BkBootSec;	//50 2 
	CHAR  BPB_Reserved[12]; //52 12 
	BYTE BS_DrvNum;			//64 1 
	BYTE BS_Reserved1;		//65 1 
	BYTE BS_BootSig;		//66 1  
	DWORD BS_VolID;			//67 4   
	CHAR  BS_VolLab[11];	//71 11  
	CHAR  BS_FilSysType[8];	//82 8  
}BootSector_BPB32, *PBootSector_BPB32;


typedef struct _tagFat_DirectoryRW		//目录表结构--原始数据操作结构
{
	CHAR DIR_Name[11];
	BYTE DIR_Attr;
	BYTE DIR_Resrv;
	BYTE DIR_CrtTimeTenth;
	BYTE DIR_CrtTime[2];
	BYTE DIR_CrtDate[2];
	BYTE DIR_LstAccDate[2];
	BYTE DIR_FstClusHI[2]; // FAT16 always is zero 
	BYTE DIR_WrtTime[2];
	BYTE DIR_WrtDate[2];
	BYTE DIR_FstClusLO[2]; 
	BYTE DIR_FileSize[4];	
}Fat_DirectoryRW, *PFat_DirectoryRW;

typedef struct _tagFat_Directory		//目录结构--实际操作
{
	CHAR DIR_Name[11];
	BYTE DIR_Attr;
	BYTE DIR_Resrv;
	BYTE DIR_CrtTimeTenth;
	SYSTEMTIME DIR_CrtDateTime;
	SYSTEMTIME DIR_LstAcceDate;
	WORD DIR_FstClusHI;
	SYSTEMTIME DIR_WrtDateTime;
	WORD DIR_FstClusLO;
	DWORD DIR_FileSize;
	INT  DIR_PosIndex;
	BYTE	DIR_EntryCount;
} Fat_Directory, *PFat_Directory;	

typedef struct _tagFat_LongDirectory_RW		// 长名目录
{
	BYTE LDIR_Ord;
	BYTE LDIR_Name1[10];
	BYTE LDIR_Attr;
	BYTE LDIR_Type;
	BYTE LDIR_Chksum;
	BYTE LDIR_Name2[12];
	BYTE LDIR_FstClusLO[2]; // Must be zero
	BYTE LDIR_Name3[4];
} Fat_LongDirectory_RW, *PFat_LongDirectory_RW;


typedef struct _tagFat_LongDirectory // 长名目录
{
	BYTE  LDIR_Ord;
	WORD  LDIR_FstClusLO;
	WCHAR LDIR_Name1[5];
	WCHAR LDIR_Name2[6];
	WCHAR LDIR_Name3[2];
	BYTE LDIR_Attr;
	BYTE LDIR_Type;
	BYTE LDIR_Chksum;
} Fat_LongDirectory, *PFat_LongDirectory;


struct DskszToSecperClus				//磁盘类型对应簇的扇区数结构体
{
	DWORD DiskSize;
	BYTE SecPerClusVal;
};


typedef struct _tagFat_FsInfo_RW
{
	BYTE FSI_LeadSig[4];
	BYTE FSI_Reserved1[480];
	BYTE FSI_StrucSig[4];
	BYTE FSI_Free_Count[4];
	BYTE FSI_Nxt_Free[4];
	BYTE FSI_Reserved2[12];
	BYTE FSI_TrailSig[4];
}Fat_FsInfo_RW, *PFat_FsInfo_RW;

typedef struct _tagFat_FsInfo
{
	DWORD FSI_LeadSig;
	DWORD FSI_StrucSig;
	DWORD FSI_Free_Count;
	DWORD FSI_Nxt_Free;
	DWORD FSI_TrailSig;
}Fat_FsInfo, *PFat_FsInfo;

struct DirPaths
{
	UINT nindex;
	char pName[MAX_PATH];
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif

enum eFatType
{
	FAT12_TYPE,
	FAT16_TYPE,
	FAT32_TYPE,
	EXT2_TYPE,
	EXT3_TYPE,
	EXT4_TYPE
};

enum DISKSIZE
{
	DISK_4M,
	DISK_16M,
	DISK_128M,
	DISK_256M,
	DISK_512M,
	DISK_1G,
	DISK_2G,
	DISK_U2G
};

enum DISKSIZE32
{
	DISK32_32M,
	DISK32_260M,
	DISK32_8G,
	DISK32_16G,
	DISK32_32G,
	DISK32_U32G
};

UINT DiskSizeType(LONGLONG disksize);
UINT Disk32SizeType(LONGLONG disksize);

UINT GenSecPerClus(LONGLONG disksize);
UINT GenSecPerClus32(LONGLONG disksize);


UINT GetImgType(LPCSTR lpFileName);

void BpbCobyFromRwInfo(OUT BootSector_BPB& bpb, IN const BootSector_BPB_RW& mbr);
void RwInfoCopyFormBpb(OUT BootSector_BPB_RW& mbr, IN const BootSector_BPB& bpb);

void Bpb32CobyFromRwInfo(OUT BootSector_BPB32& bpb, IN const BootSector_BPB_RW& mbr);
void RwInfoCopyFormBpb32(OUT BootSector_BPB_RW& mbr, IN const BootSector_BPB32& bpb);

void DirInfoFromRwInfo(OUT Fat_Directory& dir, IN const Fat_DirectoryRW& rw);
void RwInfoFromDirInfo(OUT Fat_DirectoryRW& rw, IN const Fat_Directory& dir);
void LongDirInfoFromRwInfo(Fat_LongDirectory& dir, const Fat_DirectoryRW& rw);
void RwInfoFromLongDirInfo(Fat_DirectoryRW& rw, const Fat_LongDirectory& dir); 
 

void RWFsInfoFromFsInfo(OUT Fat_FsInfo_RW& rw, IN const Fat_FsInfo fs);
void FsInfoFromRWFsInfo(OUT Fat_FsInfo fs, IN const Fat_FsInfo_RW rw);

void TrimString(LPSTR lpSrc, BOOL isLeft);   // 整理字符串中的空格字符
void RemoveCharA(LPSTR lptSrc, CHAR ch);   // 清除字符串中的ch 字符
void RemoveTChar(LPTSTR lptSrc, char ch);   // 清除字符串中的ch 字符
LPSTR ReplaceChar(LPCSTR lpSrc, char srcCh, char desCh);   // 清除字符串中的srcch 字符 为desCh
BOOL GetRighStrByFind(LPTSTR lptReturn, LPCTSTR lptSrc, char ch, int len, LPCTSTR defLps);
BOOL GetRighStrByFindA(LPSTR lpReturn, LPCSTR lpSrc, CHAR ch, int len, LPCSTR defLps);
LPTSTR GetLeftStr(LPCTSTR lpSrc, char ch, BOOL isFromLeft); // isFromLeft -- TRUE从左边开始查找 FALSE从右边开始查找
LPSTR GetLeftStrA(LPCSTR lpSrc, CHAR ch, BOOL isFromLeft); // isFromLeft -- TRUE从左边开始查找 FALSE从右边开始查找
LPTSTR GetStrFromChArry(char* pch, INT len);
BOOL  IsNeedLongEntry(LPCSTR lpstr);

//实现文件的64bit操作
ULONGLONG GetPosition(HANDLE hFile);
ULONGLONG MakeUnsignedInt64(DWORD nHigh, DWORD nLow);
void SplitUnsignedInt64(const ULONGLONG& nBigInt, DWORD& nHigh, DWORD& nLow);
ULONGLONG GetLength(HANDLE hFile);


void printmsg(char* msg);


#endif
