#pragma warning(disable:4786)
//#include <afx.h>
// #include "LibExt2.h"
// #include "SysfsExt2.h"
// #include <map>
// #include <string>
// #include <shlwapi.h>
// #include <typeinfo.h>
// #include "SysfsExt3.h"
// #include "Sysfs16.h"
// #include "Sysfs32.h"
// #include <afxwin.h>
// #include <iostream>
// #include <exception>
#include "LibExt2.h"
#include "sysfs.h"
#include "Sysfs16.h"
#include "Sysfs32.h"
//#include "SysfsExt2.h"
//#include "SysfsExt3.h"
//#include "SysfsExt4.h"
#include "ImgFileFind.h"
#include "NameMap.h"

#include <map>
#include <string>
//#include <shlwapi.h>
//#include <typeinfo.h>
//#include <afxwin.h>
#include <iostream.h>
//#include <exception>
#include <vector>
#include <sprintf.h>
#include <fileio.h>
//#include <afxtempl.h> 

using namespace std;

//"..\..\sqlite-source-3_3_10\MemSQLiteLib\Debug\MemSQLiteLibD.lib"
//"..\MemFS\Debug\MemFSD.lib"
//extern void ActDbgPrint(const char* format, ...);

#define MAX_BYTE_TOREAD		/*32768*/1048576 //1 MB
#define MAX_BYTE_TOWRITE	/*32768*/1048576 //1 MB

//BOOL Ext2InsertFileInternalFunc(CSysfs *pfs,LPCTSTR pszPathName,const PBYTE pbBuffer,
//								WORD wFileAttrib,DWORD dwLength);

//BOOL FatAddDir(LPCTSTR pszExt2PathName,LPCTSTR pszDestDir,LPCTSTR pszSrcDir);
BOOL FatAddFile(LPCTSTR pszExt2PathName,LPCTSTR pszDestDir,
						PBYTE pbBuffer,DWORD dwLength,LPCTSTR pszFileName);

BOOL InsertFile(CSysfs *pfs ,LPCTSTR pszDesFullPath,PBYTE pbBuffer,DWORD dwLength);
BOOL InsertFile(CSysfs *pfs ,LPCTSTR pszSrcFullPath , LPCTSTR pszDesFullPath);

//BOOL	RecursiveInsert(LPCTSTR  lpSrcFolderPath , LPCTSTR  lpDesFolderPath , CSysfs *pfs );

//wbt del
// BOOL	IsValidFileName(CString csInput);
// BOOL	IsValidFileName(CString csInput)
// {
// 	if (csInput.Left(1)==".")
// 		return FALSE;
// 
// 	return TRUE;
// }

void run_logEx(char* msg)
// 写运行日志文件 run.log
{
	//wbt del
#if 0
	
	FILE *fp;
	char log_file[512];//="e:\\temp\\run.log";
	int filesize, maxsize=1024*1000, delsize=1024*50;
	char *mem;
	int year,month,day,hour,min,sec;
	SYSTEMTIME  runtime;
	char temp[2*1024];

//	return;
	memset(log_file,0,sizeof(log_file));

	DWORD dw;
	char	szModuledir[128];
	DWORD dwLength=GetModuleFileName(NULL,szModuledir,128);
	
	for(dw=dwLength-1;dw>0;dw--)
	{
		if(szModuledir[dw]=='\\')
		{
			szModuledir[dw+1]=0;
			break;
		}
	}

	memcpy(log_file,szModuledir,500);
	//strcat(log_file,"log.txt");
	strcpy(log_file,"c:\\log.txt");

	fp=fopen(log_file,"ab");	// 追加方式打开
	if (fp==NULL)
	{
		printf("open file [%s] with ab fail!\n",log_file);
		return;
	}
	GetSystemTime(&runtime);
	year	= runtime.wYear;
	month	= runtime.wMonth;
	day		= runtime.wDay;
	hour	= runtime.wHour+8;
	min		= runtime.wMinute;
	sec		= runtime.wSecond;
	
	fseek(fp,0,SEEK_END);
	filesize=ftell(fp);
	fseek(fp,0,SEEK_SET);
	
	if (filesize>=maxsize)	// run.log文件大于1M
	{
		fclose(fp);
		fp=fopen(log_file,"rb");	// 为读打开一个二进制文件
		if (fp==NULL)
		{
			printf("open file [%s] with rb fail!\n",log_file);
			return;
		}
		mem =(char*)malloc(filesize);
		if (mem==0)		// 内存不足!
		{
			printf("Error: not enough memory for open [%s]!\n\n",log_file);
			free(mem);
			fclose(fp);
			return;
		}
		fread(mem,filesize,1,fp);	// 读文件
		fclose(fp);
		fp=fopen(log_file,"wb");	// 为输出打开一个二进制文件
		if (fp==NULL)
		{
			printf("open file [%s] with wb fail!\n",log_file);
			return;
		}
		fwrite(&mem[delsize],filesize-delsize,1,fp);	// 删除旧的日志，重新存
		free(mem);
		fclose(fp);
		fp=fopen(log_file,"ab");	// 追加方式打开
		if (fp==NULL)
		{
			printf("open file [%s] with ab fail second time!\n",log_file);
			return;
		}
	}
	memset(temp,0,sizeof(temp));
	sprintf(temp,"%4d-%2d-%2d %2d:%2d:%2d  %s\r\n\r\n",
		year,month,day,hour,min,sec,msg);
	fwrite(temp,strlen(temp),1,fp);
	fclose(fp);

#endif
}
//extern void WINAPI run_log(char* msg);


//#define _EXT3_LOG
void Log(LPCTSTR pszText)
{
#ifdef _EXT3_LOG
	static CStdioFile file("C:\\Ext3AddFile.Log",CFile::modeCreate|CFile::modeReadWrite);
	file.WriteString(pszText);
	file.WriteString("\n");
#endif
}
//声明全局Ext2的对象表格.
//__Ext2Open,if the file exist,then open with ignoring the other two parameters,
//else Open a new file
typedef std::map<std::string,CSysfs*> SysFSMap;
typedef std::map<CSysfs*, DWORD> FS_Space_Resv_Map;
SysFSMap g_Ext2Table;
FS_Space_Resv_Map g_ResvMap;


//////////////////////////////////////////////////////////////////////////
//2014-02-12添加文件存在则删除接口
int PathFileExistsInImg(CSysfs* pfs , LPCTSTR szFilePath)
{
	//printf("PathFileExistsInImg 1\n");
	HANDLE hf = pfs->CreateFile(szFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);	
	//printf("PathFileExistsInImg 2\n");
	
	if ((unsigned long)-1==(unsigned long)hf)
		return 0;
	
	pfs->CloseFile(hf);
	
	return 1;
	
}

DWORD FatDelEncryptFileSize(LPCTSTR pszExt2PathName,LPCTSTR pszDestDir, LPCTSTR psaFileName)
{
	
	std::string strFileName(pszExt2PathName);
	SysFSMap::iterator it=g_Ext2Table.find(strFileName);
	if(g_Ext2Table.end()==it)
		return FALSE;
	CSysfs *pFS=it->second; 
	
	
	TCHAR tszFullPath[MAX_PATH];
	_tcscpy(tszFullPath , pszDestDir);
	int ilen = _tcslen(tszFullPath);
	
	if (tszFullPath[ilen-1] != '\\')
	{
		_tcscat(tszFullPath , "\\");
	}
	
	if (memcmp(psaFileName , "./" , 2) == 0)
		_tcscat(tszFullPath , psaFileName +2);
	else
		_tcscat(tszFullPath , psaFileName );
	
	int iIndex = 0;
	
	TCHAR tszEncryptFileName[MAX_PATH];
	
	while(1)
	{
		sprintf(tszEncryptFileName , "%s~%d" , tszFullPath  , iIndex);
		//printf("PathFileExistsInImg...: %s\n" , tszEncryptFileName);
		if (PathFileExistsInImg(pFS , tszEncryptFileName))
		{
			//printf("PathFileExistsInImg...done \n");
			pFS->DeleteFile(tszEncryptFileName);
			
		}
		else 
			break;
		
		iIndex++;		
	}
	
	
	return 1;
	
}
//////////////////////////////////////////////////////////////////////////


void FSOpen(BYTE bType,LPCTSTR pszPathName,DWORD dwBlockSize,DWORD dwLogSize,
			ULONGLONG dwSize  , DWORD resvSize ,  BOOL bToMen )
{//这里的逻辑要思考一下，特别是涉及到内存文件系统的交互问题.
	//printf("eeeeeeeeeee FSOpen!\n");
	std::string strFileName(pszPathName);
	SysFSMap::iterator it=g_Ext2Table.find(strFileName);
	if(g_Ext2Table.end()==it){
		CSysfs *pSys=NULL;
		switch(bType)
		{
		case 0x01://FAT16
			pSys=new CSysfs16;
			//printf("fat16!");
			break;
		case 0x02://FAT32
			pSys=new CSysfs32;
			//printf("fat32!");
			break;
// 		case 0x03://EXT2
// 			pSys=new CSysfsExt2;
// 			break;
// 		case 0x04://EXT3
// 			pSys=new CSysfsExt3;
// 			break;
// 		case 0x12://FAT12
// 			pSys=new CSysfs12;
// 			break;
		default:
			{
				//wbt mod
// 				CString strErr;
// 				strErr.Format("DO NOT support file system type:%s",bType);
// 				exception e(strErr);throw e;
				char szInfo[256]={0};
				sprintf(szInfo, "DO NOT support file system type:%c",bType);
				printf(szInfo);
			}
			break;
		}
		std::string strPathName=strFileName;
		//std::string strPathName=GetTempDirPath()+strFileName;
		//::DeleteFile(strPathName.c_str());
		//pSys->Mount(strPathName.c_str(),NULL,dwSize,dwBlockSize,dwLogSize,CFSService::FS_WIN_DISK);
		if(bToMen)
			pSys->Mount(strPathName.c_str(),NULL,dwSize,dwBlockSize,dwLogSize,CFSService::FS_MEM);
		else
			pSys->Mount(strPathName.c_str(),NULL,dwSize,dwBlockSize,dwLogSize,CFSService::FS_WIN_DISK);

		//////////////////////////////////////////////////////////////////////////
		//wbt del --need to check!
		//pSys->m_Type = bType;
		//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
		//pSys->SetCurrentDirectory("\\Font.bin");
		//pSys->SetCurrentDirectory("\\Hello");
		//pSys->SetCurrentDirectory("World");
#endif
		g_Ext2Table[strFileName]=pSys;

		if (resvSize!=0) g_ResvMap[pSys]=resvSize;
		
	}
}


void FSClose(LPCTSTR pszExt2PathName)
{
	

	std::string strFileName(pszExt2PathName);
	SysFSMap::iterator it=g_Ext2Table.find(strFileName);
	if(g_Ext2Table.end()==it){
		return;
	}

	CSysfs *pExt2=it->second;

	FS_Space_Resv_Map::iterator resvit=g_ResvMap.find(pExt2);
	if(g_ResvMap.end()!=resvit){
		g_ResvMap.erase(resvit);
	}

	
	delete pExt2;
	pExt2=NULL;
	g_Ext2Table.erase(it);
	//strFileName = GetTempDirPath()+strFileName;
}



BOOL InsertFile(CSysfs *pFat ,LPCTSTR pszSrcFullPath , LPCTSTR pszDesFullPath)
{
	//wbt mod
// 	CFile file;
// 	if (!file.Open(pszSrcFullPath , CFile::modeRead)) return FALSE;
// 	DWORD dwFileSize = file.GetLength();
// 	BYTE* pbBuffer = new BYTE[dwFileSize];	
// 	file.Read(pbBuffer , dwFileSize);
// 	file.Close();

	FILE* fp = NULL;
	fp = fopen(pszSrcFullPath, "rb");
	if (fp == NULL)
	{
		return FALSE;
	}
	fseek(fp, 0, SEEK_END);
	DWORD dwFileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	BYTE* pbBuffer = new BYTE[dwFileSize];	
	fread(pbBuffer, 0, dwFileSize, fp);
	fclose(fp);

	BOOL ret = InsertFile(pFat ,pszDesFullPath,pbBuffer,dwFileSize);
	delete pbBuffer ;
	return ret;

}

BOOL InsertFile(CSysfs *pFat ,LPCTSTR lpDesFilePath,PBYTE pbBuffer,DWORD dwLength)
{
	BOOL ret = FALSE;
	BYTE* pTemp = pbBuffer;
	DWORD resvSize = 0;
	
	FS_Space_Resv_Map::iterator it=g_ResvMap.find(pFat);
	if(g_ResvMap.end()!=it)
		resvSize = it->second;

	//try
	//{
		char lpRootPathName[MAX_PATH];			// pointer to root path
		DWORD lpSectorsPerCluster;	// pointer to sectors per cluster
		DWORD lpBytesPerSector;		// pointer to bytes per sector
		DWORD lpNumberOfFreeClusters;	// pointer to number of free clusters
		DWORD lpTotalNumberOfClusters;
		
		if (resvSize)
		{
			if (pFat->GetDiskFreeSpace(lpRootPathName , &lpSectorsPerCluster , &lpBytesPerSector , &lpNumberOfFreeClusters , &lpTotalNumberOfClusters))
			{
				DWORD freebytes = lpNumberOfFreeClusters * lpSectorsPerCluster * lpBytesPerSector ;
				if ( resvSize + dwLength > freebytes )
				{
					return FALSE;
				}
			}
		}
		


		DWORD TotalTrans = 0;
		DWORD pdwlen = 0;
		DWORD filesize = dwLength;		


		BYTE* buf = new BYTE[MAX_BYTE_TOWRITE];
		HANDLE hfile = NULL;
		//BOOL ret = FALSE;
		BOOL bWriFail = FALSE;

		int div = filesize / MAX_BYTE_TOWRITE;
		int res = filesize % MAX_BYTE_TOWRITE;

		do 
		{
			//Create File on security area
			hfile = pFat->CreateFile(lpDesFilePath, GENERIC_WRITE|GENERIC_READ, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
			if ( (HANDLE)-1 == hfile)
			{
				//AfxMessageBox("CreateFile Fail");
				break;
			}
			

			for (int i=0;i<div;i++)
			{
				//file.Read(buf , MAX_BYTE_TOWRITE);
				memcpy(buf , pTemp, MAX_BYTE_TOWRITE);
				pTemp += MAX_BYTE_TOWRITE;

				pdwlen = 0;
				if (!pFat->WriteFile(hfile, buf, MAX_BYTE_TOWRITE, &pdwlen, NULL))
				{
					//AfxMessageBox("Write Fail");
					bWriFail = TRUE;
					break;
				}

				TotalTrans += MAX_BYTE_TOWRITE;
				
			}		
			
			if (bWriFail) break;

			if (res!=0 )
			{
				//file.Read(buf , res);
				memcpy(buf , pTemp, res);
				pTemp += res;
				pdwlen = 0;
				if (!pFat->WriteFile(hfile, buf, res, &pdwlen, NULL))
				{
					//AfxMessageBox("Write Fail");
					break;
				}

				TotalTrans += res;
				
			}

			
			ret = TRUE;//Write Success
		} while(FALSE);

		delete buf;
		if(hfile!=(HANDLE)-1) pFat->CloseFile(hfile);
		

		/*}//try
	catch (exception &e)
	{
		//AfxMessageBox(e.what());		
		return FALSE;
	}	*/

	return ret;
}

BOOL FatAddFile(LPCTSTR pszExt2PathName,LPCTSTR pszDestDir,
						PBYTE pbBuffer,DWORD dwLength,LPCTSTR pszFileName)
{
	std::string strFileName(pszExt2PathName);
	SysFSMap::iterator it=g_Ext2Table.find(strFileName);
	if(g_Ext2Table.end()==it)
		return FALSE;

	//wbt mod
	//CString strDir(pszDestDir);
	char strDir[MAX_PATH] = {0};
	strcpy(strDir, pszDestDir);

	//strDir.Replace("/","\\");
	//for (int i=0;i<5;i++)	
		//strDir.Replace("\\\\","\\");
	
	CSysfs *pFat=it->second;

	char lpDesFilePath[MAX_PATH];
	strcpy(lpDesFilePath , strDir);
	strcat(lpDesFilePath , pszFileName);

	BOOL ret = FALSE;

	BYTE* pTemp = pbBuffer;

	if (strstr(lpDesFilePath ,"\\.svn")!=NULL) return TRUE;	


	return InsertFile(pFat , lpDesFilePath , pbBuffer , dwLength);
	
	//try
	//{

		DWORD TotalTrans = 0;
		DWORD pdwlen = 0;
		DWORD filesize = dwLength;		


		BYTE* buf = new BYTE[MAX_BYTE_TOWRITE];
		HANDLE hfile = NULL;
		//BOOL ret = FALSE;
		BOOL bWriFail = FALSE;

		int div = filesize / MAX_BYTE_TOWRITE;
		int res = filesize % MAX_BYTE_TOWRITE;

		do 
		{
			//Create File on security area
			hfile = pFat->CreateFile(lpDesFilePath, GENERIC_WRITE|GENERIC_READ, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
			if ( (HANDLE)-1 == hfile)
			{
				//AfxMessageBox("CreateFile Fail");
				break;
			}
			

			for (int i=0;i<div;i++)
			{
				//file.Read(buf , MAX_BYTE_TOWRITE);
				memcpy(buf , pTemp, MAX_BYTE_TOWRITE);
				pTemp += MAX_BYTE_TOWRITE;

				pdwlen = 0;
				if (!pFat->WriteFile(hfile, buf, MAX_BYTE_TOWRITE, &pdwlen, NULL))
				{
					//AfxMessageBox("Write Fail");
					bWriFail = TRUE;
					break;
				}

				TotalTrans += MAX_BYTE_TOWRITE;
				
			}		
			
			if (bWriFail) break;

			if (res!=0 )
			{
				//file.Read(buf , res);
				memcpy(buf , pTemp, res);
				pTemp += res;
				pdwlen = 0;
				if (!pFat->WriteFile(hfile, buf, res, &pdwlen, NULL))
				{
					//AfxMessageBox("Write Fail");
					break;
				}

				TotalTrans += res;
				
			}

			
			ret = TRUE;//Write Success
		} while(FALSE);

		delete buf;
		if(hfile!=(HANDLE)-1) pFat->CloseFile(hfile);
		

	/*}//try
	catch (exception &e)
	{
		//AfxMessageBox(e.what());		
		return FALSE;
	}	*/

	return ret;
}

#define MAX_BLOCK_SIZE 10485760
BOOL FatAddEncryptFile(LPCTSTR pszExt2PathName,LPCTSTR pszDestDir,
						PBYTE pbBuffer,DWORD dwLength,LPCTSTR psaFileName)
{
	//printf("FatAddEncryptFile 1\n");
	FatDelEncryptFileSize(pszExt2PathName , pszDestDir , psaFileName);
	//printf("FatAddEncryptFile 2\n");
	std::string strFileName(pszExt2PathName);
	SysFSMap::iterator it=g_Ext2Table.find(strFileName);
	if(g_Ext2Table.end()==it)
		return FALSE;
	CSysfs *pExt2=it->second;

	int iBlock = dwLength / MAX_BLOCK_SIZE;
	DWORD dwTotal = dwLength;
	char szFileName[128];
	int i = 0;
	BYTE* pEncryptbuffer= new BYTE[MAX_BLOCK_SIZE + 4096];
	DWORD dwEncryptlen = 0;
	BYTE* pOffset = pbBuffer;
	BOOL ret ; 
	
	//printf("FatAddEncryptFile 3\n");
	for (i=0;i<iBlock;i++)
	{
		dwEncryptlen = MAX_BLOCK_SIZE;
		sprintf(szFileName , "%s~%d" , psaFileName , i);
//printf("================now szFileName=%s,psaFileName=%s\n",szFileName,psaFileName);
		memcpy(pEncryptbuffer , pOffset , dwEncryptlen);

		//encrypt	
		//printf("FastEncrypt...\n");
		char chEncryptedResult = FastEncrypt(pEncryptbuffer , (long*)&dwEncryptlen);
		//printf("FastEncrypt... done\n");
		//char chEncryptedResult = 0;
		
		if (chEncryptedResult!=0) return FALSE;

		//printf("FatAddFile...\n");
		ret = FatAddFile( pszExt2PathName, pszDestDir, pEncryptbuffer, dwEncryptlen, szFileName);
		ASSERT(ret == TRUE);
		//printf("FatAddFile...done \n");
		
		dwTotal -= MAX_BLOCK_SIZE;
		pOffset += MAX_BLOCK_SIZE;
	}

	if (dwTotal > 0)
	{
		dwEncryptlen = dwTotal;
		sprintf(szFileName , "%s~%d" , psaFileName , i);
		memcpy(pEncryptbuffer , pOffset , dwEncryptlen);

		//encrypt		
		//printf("FastEncrypt...\n");
		char chEncryptedResult = FastEncrypt(pEncryptbuffer , (long*)&dwEncryptlen);
		//printf("FastEncrypt... done\n");
		//char chEncryptedResult = 0;
		
		if (chEncryptedResult!=0) return FALSE;
		
		//printf("FatAddFile...\n");
		ret = FatAddFile( pszExt2PathName, pszDestDir, pEncryptbuffer, dwEncryptlen, szFileName);
		ASSERT(ret == TRUE);
		//printf("FatAddFile...done \n");
	}

	delete pEncryptbuffer;

	//printf("FatAddEncryptFile 4\n");
	return TRUE;

}

BOOL ReadBlock(CSysfs* pfs ,LPCTSTR psaFileName , DWORD dwBlock , BYTE*& pBuffer , DWORD& blocksize)
{
	//printf("eee ReadBlock \n");
	char szFileName[128];

	sprintf(szFileName , "\\%s~%d" , psaFileName , dwBlock);

	//printf("============================bbb CreateFile szFileName=%s, dwBlock=%d\n", szFileName, dwBlock);
	HANDLE hf = pfs->CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);	
	//printf("--------------CreateFile hf=%d\n",hf);

	if ((unsigned long)-1==(unsigned long)hf)
		return FALSE;

	unsigned int ulFileLengthHigh=0;
	unsigned int ulLength = pfs->GetFileSize(hf,(LPDWORD)&ulFileLengthHigh);
	//printf("GetFileSize \n");
	unsigned int ulLengthReaded = 0;

	unsigned char* pbDecryptBuffer =new unsigned char[ulLength];
	pfs->ReadFile(hf,pbDecryptBuffer,ulLength,(LPDWORD)&ulLengthReaded,NULL);
	//printf("ReadFile \n");
	pfs->CloseFile(hf);
	//printf("CloseFile \n");

	//decrypt
	char ret = FastDecrypt(pbDecryptBuffer , (long*)&ulLength);
	//char ret = 0;

	if (ret!=0) return FALSE;	

	blocksize = ulLength;
	pBuffer = pbDecryptBuffer;


	return TRUE;
}

int FatReadEncryptFile(LPCTSTR pszExt2PathName,LPCTSTR pszDestDir,DWORD dwPos,
						PBYTE pbBuffer,DWORD dwLength, LPCTSTR psaFileName)
{
	//printf("eee FatReadEncryptFile\n");
/*
	printf("%s , %s , %d , %d , %d , %s\n" ,pszExt2PathName , pszDestDir ,dwPos,dwLength,dwRetReadLen,psaFileName);
	//printf("%d , %d , %d ,%s\n" ,dwPos,dwLength,dwRetReadLen , psaFileName);
	
	for(int i=0;i<9;i++)	
	{
		printf("%x " , psaFileName[i]);
	}
	printf("\n");
*/	
	DWORD dwRetReadLen =0;
	std::string strFileName(pszExt2PathName);
	SysFSMap::iterator it=g_Ext2Table.find(strFileName);
	if(g_Ext2Table.end()==it)
		return -1;
	CSysfs *pFS=it->second;

	BOOL bStartAddressAlign = (dwPos % MAX_BLOCK_SIZE == 0) ? TRUE : FALSE;

	DWORD dwTotalRead =  dwLength;
	BYTE* pBlockBuf = NULL;	
	DWORD dwBlockSize;
	BYTE* pCur = pbBuffer;
	int iCurBlock  = 0;

	//printf("bbb !bStartAddressAlign\n");
	//Read first block	
	if (!bStartAddressAlign)
	{
	//printf("if !bStartAddressAlign\n");
		iCurBlock = dwPos / MAX_BLOCK_SIZE;

		if (!ReadBlock(pFS , psaFileName , iCurBlock , pBlockBuf , dwBlockSize))
		{
			//printf("ReadBlock\n");
			if(pBlockBuf) delete pBlockBuf;
			return -1;
		}

		int dwHeadLen = MAX_BLOCK_SIZE - dwPos % MAX_BLOCK_SIZE;
		
		if (dwHeadLen > dwLength)
		{
			dwHeadLen = dwLength;
		}

		memcpy(pCur , pBlockBuf + dwPos % MAX_BLOCK_SIZE , dwHeadLen);
		//printf("memcpy pBlockBuf + dwPos % MAX_BLOCK_SIZE\n");

		dwRetReadLen += dwHeadLen;
		pCur += dwHeadLen;
		dwTotalRead -= dwHeadLen;
		dwPos += dwHeadLen;
		delete pBlockBuf; pBlockBuf = NULL;
		
	}


	//Read blocks in middle
	int iBlocks = dwTotalRead / MAX_BLOCK_SIZE;

	//printf("iBlocks=%d\n", iBlocks);
	for (int i=0;i<iBlocks;i++)
	{
		iCurBlock = dwPos / MAX_BLOCK_SIZE;
		
		if (!ReadBlock(pFS , psaFileName , iCurBlock , pBlockBuf , dwBlockSize))
		{
			if(pBlockBuf) delete pBlockBuf;
			return -1;
		}

		memcpy(pCur , pBlockBuf , dwBlockSize);
		//printf("memcpy pCur , pBlockBuf , dwBlockSize\n");
		dwRetReadLen += dwBlockSize;
		pCur += dwBlockSize;
		dwTotalRead -= dwBlockSize;
		dwPos += dwBlockSize;
		delete pBlockBuf; pBlockBuf = NULL;
		
	}

	//read last
	int iRest = dwLength % MAX_BLOCK_SIZE;
	//printf("iRest=%d, dwTotalRead=%d\n",iRest, dwTotalRead);
	if (dwTotalRead !=0 && iRest!=0)
	{
		iCurBlock = dwPos / MAX_BLOCK_SIZE;
		//printf("ReadBlock 1 \n");
		if (!ReadBlock(pFS , psaFileName , iCurBlock , pBlockBuf , dwBlockSize))
		{
			//printf("ReadBlock 2 \n");
			if(pBlockBuf) delete pBlockBuf;
			return -1;
		}
		//printf("ReadBlock 3 , %d\n" , dwTotalRead );
		memcpy(pCur , pBlockBuf , dwTotalRead);
		//printf("ReadBlock 3.1 \n");
		//printf("memcpy dwTotalRead !=0 && iRest!=0\n");
		dwRetReadLen += dwTotalRead;
		//printf("ReadBlock 3.2 \n");
		pCur += dwTotalRead;
		dwTotalRead -= dwTotalRead;
		dwPos += dwTotalRead;
		delete pBlockBuf; pBlockBuf = NULL;
		//printf("ReadBlock 4 \n");
		
	}

	return dwRetReadLen;
}


//wbt del
// BOOL IsLnkFile(PBYTE pbBuffer,DWORD dwLength,CString &strLnk)
// {
// 	BYTE szInkFlag[] = {0x21,0x3C, 0x73 , 0x79, 0x6D , 0x6C , 0x69 , 0x6E , 0x6B , 0x3E , 0xFF , 0xFE};
// 	BOOL bIsInkFile = FALSE;
// 
// 
// 	if (memcmp(pbBuffer , szInkFlag , 12)==0)
// 	{
// 		bIsInkFile = TRUE;
// 		strLnk = "";
// 
// 		char *pLink=new char[(dwLength-12)/2];
// 		WORD srcindex=12;
// 		WORD dstindex=0;
// 		while(srcindex < dwLength)
// 		{
// 			pLink[dstindex]=pbBuffer[srcindex];
// 			srcindex+= 2;
// 			dstindex +=1;
// 		}
// 		strLnk=pLink;
// 
// 		return TRUE;
// 	}
// 
// 	if(dwLength<0x50)
// 		return FALSE;
// 	if(!(pbBuffer[0x00]==0x4C&&(pbBuffer[0x14]==0x0C||pbBuffer[0x14]==0x0D)))
// 	{
// 		return FALSE;
// 	}
// 
// 	WORD wOffset=0x4C;
// 	WORD wSize=*(WORD*)(pbBuffer+wOffset);
// 	if(pbBuffer[0x14]==0x0D){
// 		wOffset+=wSize+sizeof(WORD);
// 		wSize=*(WORD*)(pbBuffer+wOffset);
// 	}
// 	char *pLink=new char[wSize+1];
// 	memcpy(pLink,pbBuffer+wOffset+sizeof(WORD),wSize);
// 	pLink[wSize]=0;
// 	strLnk=pLink;
// 	delete[] pLink;
// 	bIsInkFile = TRUE;
// 
// 
// 	
// 
// 	return TRUE;
// }


BOOL IsDirEmpty(int nType,const std::string &strImagePath,const std::string &strPath)
{

	if (!strImagePath.empty())
	{
		//查找Image.

	}
	else
	{

	}
	return TRUE;
}
