// DiskImgFile.cpp: implementation of the DiskImgFile class.
//
//////////////////////////////////////////////////////////////////////

#include "DiskImgFile.h"
#include <string.h>
#include "common.h"
#include "sprintf.h"


extern void WINAPI run_log(char* msg);

BOOL IsVolLabel(Flag fAttr)
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

BOOL IsFile(Flag fAttr)
{
	return ((fAttr& (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0x00);
}

BOOL IsLongDir(Flag fAttr)
{
	return ((fAttr& ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME);	
}

BOOL IsFoulder(Flag fAttr)
{
	if(IsFile(fAttr))
		return FALSE;
	return ((fAttr& (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == ATTR_DIRECTORY);
}

BOOL IsReadOnly(Flag fAttr)
{
	return ((fAttr & ATTR_READ_ONLY) == ATTR_READ_ONLY);
}
BOOL IsHide(Flag fAttr)
{
	return ((fAttr& ATTR_HIDDEN) == ATTR_HIDDEN);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern DskszToSecperClus DskTableFAT16[];
DiskImgFile::DiskImgFile()
{
	_lpImgFileName = new TCHAR[MAX_PATH];
	memset(_lpImgFileName, 0x00, MAX_PATH * sizeof(TCHAR));
	_imgSpace = DISK_16M;
	_imgContent = NULL;
	_curDirectory.clear();
	_hFile = NULL;
	

#ifdef _MAC_UNIX
	m_CodePage = 0;
#else
	m_CodePage = CP_ACP;
#endif

}

DiskImgFile::~DiskImgFile()
{
	_curDirectory.clear();
	_fats.resize(0);
	if(_imgContent)
	{
		delete _imgContent;
	}
	if(_lpImgFileName)
	{
		delete _lpImgFileName;
	}
}


unsigned char DiskImgFile::ChkSum (LPCSTR pFcbName)
{
	short FcbNameLen;
	unsigned char Sum;
	Sum = 0;
	for (FcbNameLen=11; FcbNameLen!=0; FcbNameLen--) 
	{
	// NOTE: The operation is an unsigned char rotate right
		Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *pFcbName++;
	}
	return (Sum);
}

BOOL DiskImgFile::IsExit(LPCTSTR lptName)											// 判断目录(文件)名在当前目录下是否存在
{
	int nIndex = 0;
	Fat_Directory dir;
	TCHAR lpLongName[MAX_PATH];

	while(1)
	{
		memset(lpLongName, 0x00, MAX_PATH * sizeof(TCHAR));
		if(!GetDirectoryTabEx(dir, lpLongName, nIndex))
		{
			break;
		}
		if(strcmp(lpLongName, lptName) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL DiskImgFile::GetShortNameNumicTail(LPCSTR basicName, char& tailNumic)
{
/*	char firstName[8];
	char cmpName[8];

	memcpy(firstName, basicName, 6);
	firstName[6] = '~';
	firstName[7] = '\0';
	cmpName[7]='\0';

	int nIndex = 0;
	Fat_Directory dir;
	char cMaxNumic = '0';

	while(1)
	{
		if(!GetDirectoryTab(dir, nIndex))
		{
			tailNumic = (char)0xff;
			break;
		}

		memcpy(cmpName, dir.DIR_Name, 7);

		if(strcmp(firstName, cmpName) == 0)
		{
			if(cMaxNumic < dir.DIR_Name[7])
			{
				cMaxNumic = dir.DIR_Name[7];
			}
		}		
	}
	if(cMaxNumic == '0')
		tailNumic = ' ';
	else
		tailNumic = cMaxNumic + 1;

	return TRUE;
*/
	char firstName[8];
	char cmpName[8];

	memcpy(firstName, basicName, 6);
	firstName[6] = '~';
	firstName[7] = '\0';
	cmpName[7]='\0';

	int nIndex = 0;
	Fat_Directory dir;
	char cMaxNumic = '0';

	BOOL hasSameName = FALSE;
	BYTE hasNumic[4];
	memset(hasNumic , 0 , 4);

	BYTE bTemp;

	while(1)
	{
		if(!GetDirectoryTab(dir, nIndex))
		{
			tailNumic = (char)0xff;
			break;
		}

		memcpy(cmpName, dir.DIR_Name, 7);

		if(strcmp(firstName, cmpName) == 0)
		{
			hasSameName = TRUE;
			//短命序号为1,2,3或4时, hasNumic[i]赋值  1
			bTemp = dir.DIR_Name[7] - 0x30 - 1;
			
			if( bTemp >= 0 && bTemp <= 3)
				hasNumic[bTemp] = 1;
		}		
	}
	
	if (hasSameName)
	{
		for(int i=0;i<4;i++)
		{
			if(hasNumic[i] == 0)//hasNumic[i]为0时,证明当前序号没有用到,返回当前序号;
			{
				cMaxNumic += (i+1); 
				break;
			}
		}
	}
	
	if(!hasSameName)
		tailNumic = ' ';//没有相同的名字
	else 
	{
		if(cMaxNumic == '0')//序号1,2,3,4都用过了
			tailNumic = (char)0xff;
		else
			tailNumic = cMaxNumic;
	}

	return TRUE;
}

INT DiskImgFile::CalcNeedTabCount(LPCTSTR lpName)									// 根据名字长度计算需要目录项目数目
{
#ifdef _UNICODE
	BOOL bUse = TRUE;
	int len = WideCharToMultiByte(CP_ACP, 0, lpName, -1, NULL, 0, "_", &bUse);
	char* pch = new char[len];
	ZeroMemory(pch, len);
	bUse = TRUE;
	WideCharToMultiByte(CP_ACP, 0, lpName, -1, pch, len, "_", &bUse);
	int wLen = wcslen(lpName); 
	if(len > wLen)
	{
		delete[] pch;
	 	return ((wLen + 25) / 13);
	}

	while (_istspace(*pch))
		//pch = _tcsinc(pch);
		pch = (char*)_mbsinc((unsigned char*)pch);//modify by joelee 2008-03-11		
	

	int nLen =strlen(pch);
	LPSTR pEnd = pch + nLen;
	LPSTR pCur = pEnd;
	
	while (_istspace(*pCur) && (pCur > pch))
	{
		*pCur = '\0'; 
		pCur = pCur-1 ;
	}
	//_tcsupr(pch);
	_strupr(pch);//modify by joelee 2008-03-11		

	nLen = strlen(pch);
	LPSTR pExt = new char[MAX_PATH];
	memset(pExt, 0x00, MAX_PATH);
	
	GetRighStrByFindA(pExt, pch, '.', -1, "");
	if(nLen == 0)
	{

		delete[] pch;
		delete[] pExt;
		return 0;
	}

	if((strlen(pExt) == 0) && (nLen <= 8) )
	{
	
		delete[] pch;
		delete[] pExt;
		return /*1*/2;
	}
	
	else if((strlen(pExt) >= 1) && (strlen(pExt) <=3) && (nLen <= 8) )
	{
	
		delete[] pch;
		delete[] pExt;
		return /*1*/2;	
	}

	int nCount = (nLen + 25) / 13; 
	delete[] pch;
	delete[] pExt;

	//joelee 2008-08-08
	if (nCount == 1) nCount = 2;
	
	return nCount;
#else

	int nLen = strlen(lpName);
	LPSTR pch =  new char[nLen + 1];

	
	int nWChLen;
	LPWSTR pLongName = new WCHAR[nLen+1];
	memset(pLongName, 0x00, (nLen+1) * 2);
	nWChLen = ::MultiByteToWideChar(m_CodePage, 0, lpName, nLen, pLongName, nLen);
// printf("---------diskimgfile nLen=%d,lpName=%s,pLongName=",nLen,lpName,pLongName);
// char* p = (char*)pLongName;
// for(int i=0;i<(nLen+1)*2;i++)
// {
// printf("%c", p[i]);
// }
// printf("\n");	
	//wbt mod
	if(nWChLen != nLen) // 非英文字体
	{
//printf("nWChLen=%d != nLen=%d !! return %d\n",nWChLen, (nWChLen + 25) / 13);
		delete[] pLongName;
		delete[] pch;
    	 return ((nWChLen + 25) / 13);
	}

	memcpy(pch, lpName, nLen);
	pch[nLen] = '\0';

	while (_istspace(*pch))
		pch = (char*)_mbsinc((const unsigned char*)pch);

	nLen =strlen(pch);
	LPSTR pEnd = pch + nLen;
	LPSTR pCur = pEnd;
	
	while (_istspace(*pCur) && (pCur > pch))
	{
		*pCur = '\0'; 
		pCur = pCur-1 ;
	}
	_mbsupr((char*)pch);

	nLen = strlen(pch);
	LPSTR pExt = new char[MAX_PATH];
	memset(pExt, 0x00, MAX_PATH);
	GetRighStrByFind(pExt, pch, '.', -1, "");
	if(nLen == 0)
	{
		delete[] pLongName;
		delete[] pch;
		delete[] pExt;
		return 0;
	}

	if((strlen(pExt) == 0) && (nLen <= 8) && (strcmp(lpName, pch)== 0))
	{
		delete[] pLongName;
		delete[] pch;
		delete[] pExt;
		return /*1*/2;//一定创建长文件名
	}
	
	else if((strlen(pExt) >= 1) && (strlen(pExt) <=3) && (nLen <= 8) && (strcmp(lpName, pch) == 0))
	{
		delete[] pLongName;
		delete[] pch;
		delete[] pExt;
		return /*1*/2;	//一定创建长文件名
	}

	int nCount = (nLen + 25) / 13; 
	delete[] pLongName;
	delete[] pch;
	delete[] pExt;

	//Add by joelee 2008-08-08
	if (nCount == 1) nCount ++;//一定创建长文件名
	return nCount;	
#endif
}


/*
短名生成原则;
(1).取长文件名的前6 个字符加上"~1"形成短文件名，扩展名不变。
(2).如果已存在这个文件名，则符号"~"后的数字递增，直到5。
(3).如果文件名中"~"后面的数字达到5，则短文件名只使用长文件名的
前两个字母。通过 数学操纵( CRC校验 ) 长文件名的剩余字母生成短文件名的后四个字母，
然后加后缀"~1"(如果有必要，或是其他数字以避免重复的文件名)。
*/
BOOL DiskImgFile::GenerateShortName(LPCTSTR lpName, LPSTR lpShortName)
{
#ifdef _UNICODE

	int nLen =  wcslen(lpName);

	BOOL bUse = TRUE;
	
	int l = WideCharToMultiByte(CP_ACP, 0, lpName, -1, NULL, 0,  "_", &bUse);
	LPSTR pch = new char[MAX_PATH];
	ZeroMemory(pch, MAX_PATH);
	bUse = TRUE;
	WideCharToMultiByte(CP_ACP, 0, lpName, -1, pch, l, "_", &bUse);
#else
	int nLen = strlen(lpName);
	LPSTR pch = new char[MAX_PATH];
	memset(pch, 0x00, MAX_PATH);
	memcpy(pch, lpName, nLen);
#endif

	//////////////////////////////////////////////////////////////////////////
	// 变大写
	_mbsupr((char*)pch);

	int pclen = strlen(pch);
	
	LPSTR pstrSource = pch;
	LPSTR pstrDest = pch;	
	//LPTSTR pstrEnd = pch + pclen;
	char* pstrEnd = pch + pclen;//modify by joelee 2008-03-11

	while (pstrSource < pstrEnd)
	{
		if(*pstrSource == '+'
			|| *pstrSource == ','
			|| *pstrSource == ';'
			|| *pstrSource == '='
			|| *pstrSource == '['
			|| *pstrSource == ']'			
			)
			*pstrSource = '_';
		pstrSource++;
	}
	

	LPSTR pExt = new char[4];
	memset(pExt, 0x00, 4);
	GetRighStrByFindA(pExt,  pch, '.', 3, " ");

	//分析后缀名，名字中间不能有空格  
	//Joelee  2008-07-24
	char tempExt[4];
	char p;
	memcpy(tempExt , pExt , 4);
	int i,j;
	
	for (i=0,j=0;i<3;i++)
	{
		p = tempExt[i];
		if (p != ' ')
		{
			pExt[j++] = p;
		}
	}
	
	for (i=j;i<3;i++)
	{
		pExt[i] = ' ';
	}
	
	//////////////////////////////////////////////////////////////////////////
	// 分析名字
	LPSTR pFileName = NULL;
	if(strlen(pExt) == 0) // 表示没有后缀名
	{
		strcat(pExt,"   ");
		pFileName = pch;
	}
	else
	{
		pFileName = GetLeftStrA(pch, '.', FALSE);
	}
	
	if(pFileName == NULL)
	{
		delete[] pch;
		delete[] pExt;
		return NULL;
	}

	//读取文件名最后一个字符
	char lastChar;
	LPSTR tempFileName = pFileName; 
	nLen = strlen(pFileName);
	for(int m = 0; m < nLen; ++m)
	{
		if (m == nLen-1)
		{
			lastChar = *tempFileName;
		}
		++tempFileName;
	}

	//////////////////////////////////////////////////////////////////////////
	// 清除空格和.
 	if (nLen > 8)
 	{
		RemoveCharA(pFileName, ' ');
	}
	RemoveCharA(pFileName, '.');

	//////////////////////////////////////////////////////////////////////////
	// 查找当前目录相同名字目录（长名）
	if(IsExit(lpName))
	{
		if(	pFileName != pch)
			delete pFileName;
		delete[] pch;
		delete[] pExt;
		//wbt dll
		//printmsg("exit the same foulder or file name!");
		return FALSE;
	}
	//////////////////////////////////////////////////////////////////////////
	// 生成短名
	//nLen = strlen(pFileName);

	char* pChar = NULL;
	char* CRCSrcBuf = new char[nLen];
	memcpy(CRCSrcBuf , pFileName , nLen);		

	if(nLen < 8)
	{
		for(int i = 0; i < 8-nLen; i++)
		{
			strcat(pFileName, " ");
		}
	}
	if(nLen >8)
		pFileName[8] = '\0';

	char chNumic;
	GetShortNameNumicTail(pFileName, chNumic);
	if( chNumic == (char)0xff )
	{
		char CRCStr[5];
		pChar = CRCSrcBuf;
		pChar += 2;//取第二个字符以后的字符串
		GetCRCStr(pChar , nLen-2 , CRCStr);//获取CRC校验后个字符

		pChar = pFileName;
		pChar+=2;
		memcpy(pChar , CRCStr , 4);//从pFileName[2]开始 , 复制CRCStr ,  长度4

		GetShortNameNumicTail(pFileName, chNumic);
	}
	if(CRCSrcBuf)
		delete[] CRCSrcBuf;


	if(chNumic != ' ' )
	{
		pFileName[7]= chNumic;
		pFileName[6]= '~';
		pFileName[8]='\0';
	}	
	else
	{
		if(nLen > 8 || lastChar == 0x20)
		{
			pFileName[7]= '1';
			pFileName[6]= '~';
			pFileName[8]='\0';
		}
	}
	strcat(pFileName, pExt);
	strcat(lpShortName, pFileName);
	if(	pFileName != pch)
		delete pFileName;
	delete[] pch;
	delete[] pExt;
	return TRUE;
	
}

UINT  DiskImgFile::RelatAddrToCluster(IN UINT uRetAddr)						// 映射函数从实际实际偏移位置到簇
{
	//_startClusterAddr = _imgBpb.BPB_BytsPerSec + _imgBpb.BPB_FATSz16*_imgBpb.BPB_BytsPerSec*2 - 32*_imgBpb.BPB_BytsPerSec;

	UINT relAddrk = uRetAddr - _stOfClusterAddr;
	
	return (relAddrk / _imgBpb.BPB_BytsPerSec/_imgBpb.BPB_SecPerClus + 2);
	
}

UINT  DiskImgFile::ClusterToRelatAddr(IN UINT uCluNm)						// 映射函数从簇到实际偏移位置
{
	if(uCluNm == 0)
		return _stOfRootAddr;
	UINT relAddr = (uCluNm - 2 )* _imgBpb.BPB_BytsPerSec * _imgBpb.BPB_SecPerClus;
	return relAddr + _stOfClusterAddr;
}

BOOL DiskImgFile::ImgCreateDirectory(IN LPCTSTR lptDirName)					// 建立目录函数   ldDirName 为长名文件夹
{
	int ns = _curDirectory.size();
	int nNeed = this->CalcNeedTabCount(lptDirName); // 需要存放的结构的个数

	UINT len = strlen(lptDirName);
	LPWSTR pLongName = new WCHAR[len + 1];

#ifdef _UNICODE
	wcscpy(pLongName , lptDirName);
	int nl = wcslen(pLongName);
#else	
	memset(pLongName, 0x00, (len + 1) * 2);
	int nl = ::MultiByteToWideChar(m_CodePage, 0, lptDirName, len, pLongName, len+1);
#endif
	
	
	LPSTR pName = new char[MAX_PATH];
	memset(pName, 0x00, MAX_PATH);
	if(!GenerateShortName(lptDirName, pName))// new char[11];
	{
		delete[] pLongName;
		delete[] pName;
		//wbt del
		//printmsg("Generate short name faile!");
		return FALSE;
	}
	
	 len = strlen(pName);
	ASSERT(len <= 12);


	DWORD nRead;
	DWORD nWrite;
		
	BYTE chkFlag = ChkSum(pName);
	if(ns == 0) // 在根目录下创建字目录
	{
		Fat_Directory dir;
		Fat_DirectoryRW dirRw;
	
		INT nIndex = -1;

//////////////////////////////////////////////////////////////////////////
// 开始建立目录项
		memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
		memset(&dir, 0x00, sizeof(Fat_Directory));
		SetDiskFilePointer(NULL, _stOfRootAddr, NULL, (DWORD)FILE_BEGIN);
		int i = 0;

		for( i = 0; i < ROOTENTCNT; i++)
		{			
			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
			if(nRead != sizeof(Fat_DirectoryRW))
			{
				//wbt del
				//printmsg("Error In Create RootDirectroy --Read DirRW Error!");
				return FALSE;
			}
			if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed == 1))   // 判断是否为空项
			{
				nIndex = i;
				break;
			}
		}
		//if(nIndex == -1)
		if(nIndex == -1 || (nIndex + nNeed > ROOTENTCNT) )
		{
			
			//wbt del
			//printmsg("This Directroy is full!");
			return FALSE;
		}		
	
		//////////////////////////////////////////////////////////////////////////
		// 处理长名目录项目
		if(nNeed > 1)
		{
			Fat_LongDirectory longDir;
			memset(&longDir, 0x00, 32);
			longDir.LDIR_Attr = ATTR_LONG_NAME;
			longDir.LDIR_Chksum = chkFlag;
			longDir.LDIR_FstClusLO = 0;
			longDir.LDIR_Ord = LAST_LONG_ENTRY|(BYTE)(nNeed -1);
			longDir.LDIR_Type = 0;		
			int npos = (nNeed - 2) * 13;
			memset(longDir.LDIR_Name1, 0xff, 26);
			memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
			if((nl - npos) < 13)
				memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
			RwInfoFromLongDirInfo(dirRw, longDir);
			SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
			{
				
				//wbt del
				//printmsg("Error In create RootDirectory -- write dir Struct");
				return FALSE;
			}
			nIndex ++;	
		}
		for(i = nNeed - 2; i >0 ; i--)
		{
			Fat_LongDirectory longDir;
			memset(&longDir, 0x00, 32);
			longDir.LDIR_Attr = ATTR_LONG_NAME;
			longDir.LDIR_Chksum = chkFlag;
			longDir.LDIR_FstClusLO = 0;
			longDir.LDIR_Ord = (BYTE)i;
			longDir.LDIR_Type = 0;
		//	int nl = strlen(lpDirName);
			int npos = (i - 1) * 13;
			memset(longDir.LDIR_Name1, 0xff, 26);
			memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
			RwInfoFromLongDirInfo(dirRw, longDir);
		//	SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
			{
				delete[] pName;
				//wbt del
				//printmsg("Error In create RootDirectory -- write dir Struct");
				return FALSE;
			}
			nIndex ++;	
		}
		//////////////////////////////////////////////////////////////////////////
		

		memcpy(&dir.DIR_Name, pName, 11);
		dir.DIR_Attr = 0x10;
	//	dir.DIR_WrtDateTime = CTime::GetCurrentTime();
		UINT clus = GetFirstFreeClusNum();
		
		ASSERT(clus >= 2);
		dir.DIR_FstClusHI = 0;
		dir.DIR_FstClusLO = clus;

		::GetSystemTime(&dir.DIR_WrtDateTime);
		::GetSystemTime(&dir.DIR_CrtDateTime);
		::GetSystemTime(&dir.DIR_LstAcceDate);


	/*
		dir.DIR_WrtDateTime = CTime::GetCurrentTime();
			dir.DIR_CrtDateTime = CTime::GetCurrentTime();
			dir.DIR_LstAcceDate = CTime::GetCurrentTime();
	*/
		SetClus(clus, m_EOC_STATUS);

		RwInfoFromDirInfo(dirRw, dir);
		SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
		{
			
			//wbt del
			//printmsg("Error In create RootDirectory -- write dir Struct");
			return FALSE;
		}
			
		//////////////////////////////////////////////////////////////////////////
		// 初始化分配得到簇的值
	
	    InitializeClus(clus);
	
	
		//////////////////////////////////////////////////////////////////////////
		//  建立 .目录
	
		UINT pos = ClusterToRelatAddr(clus);
		memcpy(dir.DIR_Name, ".       ", 8);
		::GetSystemTime(&dir.DIR_WrtDateTime);
		::GetSystemTime(&dir.DIR_CrtDateTime);
		::GetSystemTime(&dir.DIR_LstAcceDate);
	/*
		dir.DIR_WrtDateTime = CTime::GetCurrentTime();
			dir.DIR_CrtDateTime = CTime::GetCurrentTime();
			dir.DIR_LstAcceDate = CTime::GetCurrentTime();
		*/
		dir.DIR_FstClusHI = 0;
		dir.DIR_FstClusLO = clus;
		SetDiskFilePointer(NULL, pos, NULL, (DWORD)FILE_BEGIN);			
	    RwInfoFromDirInfo(dirRw, dir);
		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
		{
			//wbt del
			//printmsg("Error In create SubDirectory -- write dir Struct");
			return FALSE;
		}
		//
		//////////////////////////////////////////////////////////////////////////
		

		//////////////////////////////////////////////////////////////////////////
		// 建立 ..目录
		memcpy(dir.DIR_Name, "..      ", 8);
		dir.DIR_FstClusHI = 0;
		dir.DIR_FstClusLO = 0;
		::GetSystemTime(&dir.DIR_WrtDateTime);
		::GetSystemTime(&dir.DIR_CrtDateTime);
		::GetSystemTime(&dir.DIR_LstAcceDate);
	/*
		dir.DIR_WrtDateTime = CTime::GetCurrentTime();
			dir.DIR_CrtDateTime = CTime::GetCurrentTime();
			dir.DIR_LstAcceDate = CTime::GetCurrentTime();
		*/
		
	    RwInfoFromDirInfo(dirRw, dir);
		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
		{
			printmsg("Error In create SubDirectory -- write dir Struct");
			return FALSE;
		}
    	SetDiskFilePointer(NULL, 0, NULL, FILE_BEGIN);
		//
		//////////////////////////////////////////////////////////////////////////	
		//////////////////////////////////////////////////////////////////////////
		//
	}
	
	else  //在子目录下创建子目录
	{

		//////////////////////////////////////////////////////////////////////////
		// 建立新目录项
		Fat_Directory curD = _curDirectory.at(_curDirectory.size() - 1);

		UINT dirAdd = this->ClusterToRelatAddr(curD.DIR_FstClusLO);

		Fat_Directory dir;
		Fat_DirectoryRW dirRw;
		INT nIndex = -1;

		memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
		memset(&dir, 0x00, sizeof(Fat_Directory));
		SetDiskFilePointer(NULL, dirAdd, NULL, (DWORD)FILE_BEGIN);
		UINT maxDircnt = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;
		WORD cClus = curD.DIR_FstClusLO;
		int  nDirPos = 0; // 记录目录的相对位置 放置在Fat_Directory.DIR_PosIndex中
		while(1)
		{
			for(int i = 0; i < maxDircnt; i++)
			{	
				ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
				if(nRead != sizeof(Fat_DirectoryRW))
				{
					printmsg("Error In Create RootDirectroy --Read DirRW Error!");
					return FALSE;
				}
				
				if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed == 1))   // 判断是否为空项
				{
					nIndex = i;
					break;
				}	
				nDirPos++;
			}
			if(nIndex != -1)
				break;			
			DWORD dwClus =  _fats.at(cClus); 			
 			if(dwClus == m_EOC_STATUS)
 				break;			
			cClus = dwClus;
			dirAdd = ClusterToRelatAddr(cClus);
			SetDiskFilePointer(NULL, dirAdd, NULL, (DWORD)FILE_BEGIN);
		}
		if(nIndex == -1)
		{
			UINT c = GetFirstFreeClusNum();
			SetClus(cClus, c);
			SetClus(c, m_EOC_STATUS);
			cClus = c;
			InitializeClus(c);
			nIndex = nDirPos;
			dirAdd = ClusterToRelatAddr(c);
		}	
		

		//////////////////////////////////////////////////////////////////////////
		// 处理长名目录项目
		int retIndex = nIndex%maxDircnt;
		if(nNeed > 1)
		{
			Fat_LongDirectory longDir;
			memset(&longDir, 0x00, 32);
			longDir.LDIR_Attr = ATTR_LONG_NAME;
			longDir.LDIR_Chksum = chkFlag;
			longDir.LDIR_FstClusLO = 0;
			longDir.LDIR_Ord = LAST_LONG_ENTRY|(BYTE)(nNeed - 1);
			longDir.LDIR_Type = 0;
		//	int nl = strlen(lpDirName);
			int npos = (nNeed - 2) * 13;
			memset(longDir.LDIR_Name1, 0xff, 26);
			memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
			if((nl - npos) < 13)
				memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
			RwInfoFromLongDirInfo(dirRw, longDir);
			SetDiskFilePointer(NULL, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
			{
				
				printmsg("Error In create RootDirectory -- write dir Struct");
				return FALSE;
			}
			retIndex++;
			nIndex ++;	
			if(retIndex >= maxDircnt)
			{
				UINT c = GetFirstFreeClusNum();
				SetClus(cClus, c);
				SetClus(c, m_EOC_STATUS);
				InitializeClus(c);
				retIndex = 0;
				dirAdd = ClusterToRelatAddr(c);
			}
		}
		for( int i = nNeed - 2; i >0 ; i--)
		{
			Fat_LongDirectory longDir;
			memset(&longDir, 0x00, 32);
			longDir.LDIR_Attr = ATTR_LONG_NAME;
			longDir.LDIR_Chksum = chkFlag;
			longDir.LDIR_FstClusLO = 0;
			longDir.LDIR_Ord = (BYTE)i;
			longDir.LDIR_Type = 0;
			//int nl = strlen(lptDirName);
			int npos = (i - 1) * 13;
			memset(longDir.LDIR_Name1, 0xff, 26);
			memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
			RwInfoFromLongDirInfo(dirRw, longDir);
			SetDiskFilePointer(NULL, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
			{
		
				printmsg("Error In create RootDirectory -- write dir Struct");
				return FALSE;
			}
			retIndex++;
			nIndex ++;	
			if(retIndex >= maxDircnt)
			{
				UINT c = GetFirstFreeClusNum();
				SetClus(cClus, c);
				SetClus(c, m_EOC_STATUS);
				InitializeClus(c);
				retIndex = 0;
				dirAdd = ClusterToRelatAddr(c);
			}
		}
		//////////////////////////////////////////////////////////////////////////
		
		//SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
		memcpy(&dir.DIR_Name, pName, 11);
		dir.DIR_Attr = 0x10;
			::GetSystemTime(&dir.DIR_WrtDateTime);
		::GetSystemTime(&dir.DIR_CrtDateTime);
		::GetSystemTime(&dir.DIR_LstAcceDate);
		/*
		dir.DIR_WrtDateTime = CTime::GetCurrentTime();
				dir.DIR_CrtDateTime = CTime::GetCurrentTime();
				dir.DIR_LstAcceDate = CTime::GetCurrentTime();
		*/
		

		UINT clus = GetFirstFreeClusNum();
		
		ASSERT(clus >= 2);
		dir.DIR_FstClusHI = 0;
		dir.DIR_FstClusLO = clus;
		SetClus(clus, m_EOC_STATUS);

		RwInfoFromDirInfo(dirRw, dir);
		//SetDiskFilePointer(NULL, dirAdd + 32 * nIndex, NULL, FILE_BEGIN);
		SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
		{
			
			printmsg("Error In create RootDirectory -- write dir Struct");
			return FALSE;
		}

		//////////////////////////////////////////////////////////////////////////
		// 初始化分配得到簇的值
		InitializeClus(clus);
		//////////////////////////////////////////////////////////////////////////
		//  建立 .目录

		UINT pos = ClusterToRelatAddr(clus);
		memcpy(dir.DIR_Name, ".          ", 11);
			::GetSystemTime(&dir.DIR_WrtDateTime);
		::GetSystemTime(&dir.DIR_CrtDateTime);
		::GetSystemTime(&dir.DIR_LstAcceDate);
		/*
		dir.DIR_WrtDateTime = CTime::GetCurrentTime();
		dir.DIR_CrtDateTime = CTime::GetCurrentTime();
		dir.DIR_LstAcceDate = CTime::GetCurrentTime();
		*/
		dir.DIR_FstClusHI = 0;
		dir.DIR_FstClusLO = clus;
		SetDiskFilePointer(NULL, pos, NULL, (DWORD)FILE_BEGIN);			
	    RwInfoFromDirInfo(dirRw, dir);
		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
		{
			printmsg("Error In create SubDirectory -- write dir Struct");
			return FALSE;
		}
		//
		//////////////////////////////////////////////////////////////////////////
		

		//////////////////////////////////////////////////////////////////////////
		// 建立 ..目录
		memcpy(dir.DIR_Name, "..         ", 11);
		dir.DIR_FstClusHI = curD.DIR_FstClusHI;
		dir.DIR_FstClusLO = curD.DIR_FstClusLO;
		dir.DIR_WrtDateTime = curD.DIR_WrtDateTime;
		dir.DIR_CrtDateTime = curD.DIR_CrtDateTime;
		dir.DIR_LstAcceDate = curD.DIR_LstAcceDate;

	    RwInfoFromDirInfo(dirRw, dir);
		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
		{
			delete[] pName;
			printmsg("Error In create SubDirectory -- write dir Struct");
			return FALSE;
		}
		SetDiskFilePointer(NULL, 0, NULL, FILE_BEGIN);
		//
		//////////////////////////////////////////////////////////////////////////				
	}	
	delete[] pLongName;
	delete[] pName;
	return TRUE;
	
}


BOOL DiskImgFile::CreateDirectoryEx(IN LPCTSTR lptFullDir)
{
	BOOL ret = FALSE;
	vector<DirPaths> paths;
	ParaPathFromStr(lptFullDir, paths);
	int ns = paths.size();
	LPTSTR lpPath = new TCHAR[MAX_PATH];
	memset(lpPath, 0x00, MAX_PATH * sizeof(TCHAR) );
	for(int i = 0; i < ns; i++)
	{
		strcat(lpPath, "\\");
		LPTSTR pck = GetStrFromChArry(paths.at(i).pName, strlen(paths.at(i).pName));
		strcat(lpPath, pck);
		if(SetCurrentDirectory(lpPath))
		{
			delete pck;
			continue;
		}

		ret = this->ImgCreateDirectory(pck);
		delete pck;	

		if (!ret) break;		

	}
	delete[] lpPath;
	return ret;
}



BOOL  DiskImgFile::DeleteDirectoryEx(LPCTSTR lptDirectoryPath)
{
	LPTSTR lpCurDir = GetLeftStr(lptDirectoryPath, '\\', FALSE);
	LPTSTR lpName = new TCHAR[MAX_PATH];
	memset(lpName, 0x00, MAX_PATH * sizeof(TCHAR));

	if(!GetRighStrByFind(lpName, lptDirectoryPath, '\\', -1, ""))
	{
		delete lpCurDir;
		delete[] lpName;
		return FALSE;
	}
	
	if(lpCurDir == NULL)
	{
		delete lpCurDir;
		delete[] lpName;
		return FALSE;
	}
	
	if(SetCurrentDirectory(lpCurDir))
	{
		BOOL ret = this->ImgDeleteDirectory(lpName);
		if (ret)TrimDirEntrySpace();		
		
		delete lpCurDir;
		delete[] lpName;
		return ret;
	}
	delete lpCurDir;
	delete[] lpName;
	return FALSE;
}

BOOL  DiskImgFile::DeleteFileEx(LPCTSTR lptFilePath)
{
	LPTSTR lpCurDir = GetLeftStr(lptFilePath, '\\', FALSE);
	if(lpCurDir == NULL)
	{
		delete lpCurDir;
		return FALSE;
	}

	LPTSTR lpName = new TCHAR[MAX_PATH];
	memset(lpName, 0x00, MAX_PATH * sizeof(TCHAR) );
	if(!GetRighStrByFind(lpName, lptFilePath, '\\', -1, ""))
	{
		delete lpCurDir;
		delete[] lpName;
		return FALSE;
	}

	if(SetCurrentDirectory(lpCurDir))
	{
		BOOL ret = this->ImgDeleteFile(lpName);
		if (ret) TrimDirEntrySpace();		
		
		delete[] lpName;
		delete lpCurDir;
		return ret;
	}
	delete lpCurDir;
	delete[] lpName;
	return FALSE;
}

BOOL  DiskImgFile::ImgCreateFile(IN const LPCTSTR lptFileName, 
								  IN PBYTE  pBuffer,
								  IN const BYTE bFileAttr,
								  IN const UINT nSize,
								  HANDLE& hFile)// 创建文件函数
{
	int ns = _curDirectory.size();
	int nNeed = this->CalcNeedTabCount(lptFileName); // 需要存放的结构的个数

	UINT len = strlen(lptFileName);
	LPWSTR pLongName = new WCHAR[len + 1];

	int i=0;
#ifdef _UNICODE
	wcscpy(pLongName , lptFileName);
	int nl = strlen(pLongName);
#else	
	memset(pLongName, 0x00, (len + 1) * 2);
	int nl = ::MultiByteToWideChar(m_CodePage, 0, lptFileName, len, pLongName, len+1);
#endif

	LPSTR pName = new char[MAX_PATH];
	memset(pName, 0x00, MAX_PATH);
	
	if (!GenerateShortName(lptFileName, pName))
	{
		delete[] pName;
		return FALSE;
	}
	
	

	DWORD nRead;
	DWORD nWrite;
	
	Fat_Directory dir;
	Fat_DirectoryRW dirRw;

	memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
	memset(&dir, 0x00, sizeof(Fat_Directory));

	UINT fstClus = 0;
	
	BYTE chkFlag = ChkSum(pName);	
	if(ns == 0) // 在根目录下创建文件
	{
		INT nIndex = -1;
		//////////////////////////////////////////////////////////////////////////
		// 开始建立目录项
		SetDiskFilePointer(NULL, _stOfRootAddr, NULL, (DWORD)FILE_BEGIN);
		for( i = 0; i < ROOTENTCNT; i++)
		{			
			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
			if(nRead != sizeof(Fat_DirectoryRW))
			{
				delete[] pName;
				delete[] pLongName;
				printmsg("Error In Create RootFile --Read DirRW Error!");
				return FALSE;
			}
			if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed ==1))    // 判断是否为空项
			{
				nIndex = i;
				break;
			}
		}

		if(nIndex == -1 || (nIndex + nNeed > ROOTENTCNT) )
		{
			delete[] pName;
			delete[] pLongName;
			printmsg("This Directroy is full!");
			return FALSE;
		}		
	
		//////////////////////////////////////////////////////////////////////////
		// 处理长名目录项目
		if(nNeed > 1)
		{
			Fat_LongDirectory longDir;
			memset(&longDir, 0x00, 32);
			longDir.LDIR_Attr = ATTR_LONG_NAME;
			longDir.LDIR_Chksum = chkFlag;
			longDir.LDIR_FstClusLO = 0;
			longDir.LDIR_Ord = LAST_LONG_ENTRY|(BYTE)(nNeed - 1);
			longDir.LDIR_Type = 0;
		//	int nl = strlen(lpFileName);
			int npos = (nNeed - 2) * 13;
			memset(longDir.LDIR_Name1, 0xff, 26);
			memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
			if((nl - npos) < 13)
				memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
			RwInfoFromLongDirInfo(dirRw, longDir);
			SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
			{
				delete[] pName;
				delete[] pLongName;
				printmsg("Error In create RootDirectory -- write dir Struct");
				return FALSE;
			}
			nIndex ++;	
		}
		for( i = nNeed - 2; i >0 ; i--)
		{
			Fat_LongDirectory longDir;
			memset(&longDir, 0x00, 32);
			longDir.LDIR_Attr = ATTR_LONG_NAME;
			longDir.LDIR_Chksum = chkFlag;
			longDir.LDIR_FstClusLO = 0;
			longDir.LDIR_Ord = (BYTE)i;
			longDir.LDIR_Type = 0;
			int npos = (i - 1) * 13;
			memset(longDir.LDIR_Name1, 0xff, 26);
			memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
			RwInfoFromLongDirInfo(dirRw, longDir);
		//	SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
			{
				delete[] pName;
				delete[] pLongName;
				printmsg("Error In create RootDirectory -- write dir Struct");
				return FALSE;
			}
			nIndex ++;	
		}
		//////////////////////////////////////////////////////////////////////////

		memcpy(&dir.DIR_Name, pName, 11);
		dir.DIR_Attr = bFileAttr;
//		dir.DIR_FileSize = nSize;
		dir.DIR_FileSize = 0;
		::GetSystemTime(&dir.DIR_WrtDateTime);
		::GetSystemTime(&dir.DIR_CrtDateTime);
		::GetSystemTime(&dir.DIR_LstAcceDate);
		/*dir.DIR_WrtDateTime = CTime::GetCurrentTime();
		dir.DIR_LstAcceDate = CTime::GetCurrentTime();
		dir.DIR_CrtDateTime = CTime::GetCurrentTime();
*/
		//fstClus = GetFirstFreeClusNum();
		//ASSERT(fstClus >= 2);
		fstClus = 0;
		dir.DIR_FstClusLO = fstClus;
		dir.DIR_FstClusHI = 0;
		dir.DIR_PosIndex = nIndex;


		//////////////////////////////////////////////////////////////////////////
		// 返回文件的句柄
		ImgFileHandle* fh = new ImgFileHandle;
		fh->_exceedsize = 0;
		memcpy(&fh->_fileTab, &dir, sizeof(dir));
		fh->_curRetpos = 0;
		fh->_tabStAddr = _stOfRootAddr + 32 * nIndex;
		fh->_stAddr = ClusterToRelatAddr(dir.DIR_FstClusLO);
		fh->_curPos =fh->_stAddr;
		hFile = (HANDLE)fh;

		RwInfoFromDirInfo(dirRw, dir);
		SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
		{
			delete[] pName;
			delete[] pLongName;
			printmsg("Error In create RootFile -- write dir Struct");
			return FALSE;
		}
		//////////////////////////////////////////////////////////////////////////
		
			
	}
	else		// 在其他目录下建立文件
	{
		Fat_Directory curD = _curDirectory.at(_curDirectory.size() - 1);

		UINT dirAdd = this->ClusterToRelatAddr(curD.DIR_FstClusLO);

		INT nIndex = -1;
		//////////////////////////////////////////////////////////////////////////
		// 开始建立目录项

		SetDiskFilePointer(NULL, dirAdd, NULL, (DWORD)FILE_BEGIN);
		UINT maxDircnt = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;
		WORD cClus = curD.DIR_FstClusLO;
		int  nDirPos = 0; // 记录目录的相对位置 放置在Fat_Directory.DIR_PosIndex中
		while(1)
		{
			for(int i = 0; i < maxDircnt; i++)
			{	
				ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
				if(nRead != sizeof(Fat_DirectoryRW))
				{
					delete[] pLongName;
					delete[] pName;
					printmsg("Error In Create SubFile --Read DirRW Error!");
					return FALSE;
				}
				
				if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed == 1))   // 判断是否为空项
				{
					nIndex = i;
					break;
				}	
				nDirPos++;
			}
			if(nIndex != -1)
				break;
			if(_fats.at(cClus) == m_EOC_STATUS)
				break;
			cClus = _fats.at(cClus);
			dirAdd = ClusterToRelatAddr(cClus);
			SetDiskFilePointer(NULL, dirAdd, NULL, (DWORD)FILE_BEGIN);
		}
		if(nIndex == -1)
		{
			UINT c = GetFirstFreeClusNum();
			SetClus(cClus, c);
			SetClus(c, m_EOC_STATUS);
			InitializeClus(c);
			nIndex = nDirPos;
			dirAdd = ClusterToRelatAddr(c);
		}	
/*
		for(int i = 0; i < maxDircnt; i++)
		{			
			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
			if(nRead != sizeof(Fat_DirectoryRW))
			{
				printmsg("Error In Create SubFile --Read DirRW Error!");
				return FALSE;
			}
			if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed == 1))  // 判断是否为空项
			{
				nIndex = i;
				break;
			}
		}
		
		if(nIndex == -1)
		{
			printmsg("This Directroy is full!");
			return FALSE;
		}
*/
	
		//////////////////////////////////////////////////////////////////////////
		// 处理长名目录项目
		int retIndex = nIndex%maxDircnt;
		if(nNeed > 1)
		{
			Fat_LongDirectory longDir;
			memset(&longDir, 0x00, 32);
			longDir.LDIR_Attr = ATTR_LONG_NAME;
			longDir.LDIR_Chksum = chkFlag;
			longDir.LDIR_FstClusLO = 0;
			longDir.LDIR_Ord = LAST_LONG_ENTRY|(BYTE)(nNeed-1);
			longDir.LDIR_Type = 0;
			int npos = (nNeed - 2) * 13;
			memset(longDir.LDIR_Name1, 0xff, 26);
			memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
			if((nl - npos) < 13)
				memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
			RwInfoFromLongDirInfo(dirRw, longDir);
			SetDiskFilePointer(NULL, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
			{
				delete[] pLongName;
				delete[] pName;
				printmsg("Error In create RootDirectory -- write dir Struct");
				return FALSE;
			}
			retIndex++;
			nIndex ++;
			if(retIndex >= maxDircnt)
			{
				UINT c = GetFirstFreeClusNum();
				SetClus(cClus, c);
				SetClus(c, m_EOC_STATUS);
				InitializeClus(c);
				retIndex = 0;
				dirAdd = ClusterToRelatAddr(c);
			}
		}
		for(int i = nNeed - 2; i >0 ; i--)
		{
			Fat_LongDirectory longDir;
			memset(&longDir, 0x00, 32);
			longDir.LDIR_Attr = ATTR_LONG_NAME;
			longDir.LDIR_Chksum = chkFlag;
			longDir.LDIR_FstClusLO = 0;
			longDir.LDIR_Ord = (BYTE)i;
			longDir.LDIR_Type = 0;
			//int nl = strlen(lptFileName);
			int npos = (i - 1) * 13;
			memset(longDir.LDIR_Name1, 0xff, 26);
			memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
			RwInfoFromLongDirInfo(dirRw, longDir);
			SetDiskFilePointer(NULL, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
			{
				delete[] pLongName;
				delete[] pName;
				printmsg("Error In create RootDirectory -- write dir Struct");
				return FALSE;
			}
			
			nIndex ++;	
			retIndex++;
			if(retIndex >= maxDircnt)
			{
				UINT c = GetFirstFreeClusNum();
				SetClus(cClus, c);
				SetClus(c, m_EOC_STATUS);
				InitializeClus(c);
				retIndex = 0;
				dirAdd = ClusterToRelatAddr(c);
			}
		}
		//////////////////////////////////////////////////////////////////////////
		
	
		memcpy(&dir.DIR_Name, pName, 11);
		dir.DIR_Attr = bFileAttr;
//		dir.DIR_FileSize = nSize;
		dir.DIR_FileSize = 0;
		::GetSystemTime(&dir.DIR_WrtDateTime);
		::GetSystemTime(&dir.DIR_CrtDateTime);
		::GetSystemTime(&dir.DIR_LstAcceDate);
/*		dir.DIR_WrtDateTime = CTime::GetCurrentTime();
		dir.DIR_LstAcceDate = CTime::GetCurrentTime();
		dir.DIR_CrtDateTime = CTime::GetCurrentTime();
*/
// 		fstClus = GetFirstFreeClusNum();
// 		ASSERT(fstClus >= 2);
		fstClus = 0;
		dir.DIR_FstClusLO = fstClus;
		dir.DIR_FstClusHI = 0;
		dir.DIR_PosIndex = nIndex;
		
		ImgFileHandle* fh = new ImgFileHandle;
		memcpy(&fh->_fileTab, &dir, sizeof(dir));
		fh->_exceedsize = 0;
		fh->_curRetpos = 0;
		fh->_tabStAddr = dirAdd + 32 * retIndex;
		fh->_stAddr = ClusterToRelatAddr(dir.DIR_FstClusLO);
		fh->_curPos =fh->_stAddr;

		hFile = (HANDLE)fh;


		RwInfoFromDirInfo(dirRw, dir);
//		SetDiskFilePointer(NULL, dirAdd + 32 * nIndex, NULL, FILE_BEGIN);
		SetDiskFilePointer(NULL, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
		{
			delete[] pName;
			delete[] pLongName;
			printmsg("Error In create RootFile -- write dir Struct");
			return FALSE;
		}
	}

	// 计算文件占用簇的数目
	UINT bysPerClus = _imgBpb.BPB_BytsPerSec * _imgBpb.BPB_SecPerClus;
	UINT nFileClus = (nSize +  bysPerClus - 1) / bysPerClus;
	if((nSize == 0) || (pBuffer == NULL))
	{
		delete[] pName;
		delete[] pLongName;
		SetClus(fstClus, m_EOC_STATUS);
		return TRUE;
	}

	if(nFileClus > 0)
	{
		int k = 0;
		UINT tempValue = 0xffff;
		UINT curClus = fstClus;
		while(1)//k < nFileClus)
		{
			curClus = fstClus;
			// 把文件写入该簇
			UINT reAdd = ClusterToRelatAddr(curClus);
			SetDiskFilePointer(NULL, reAdd, NULL, FILE_BEGIN);
			if(k < nFileClus -1)
				if (!WriteDiskFile(_hFile, pBuffer + k * bysPerClus, bysPerClus, &nWrite, NULL))
				{
					return FALSE;
				}				
			else
				if (!WriteDiskFile(_hFile, pBuffer + k * bysPerClus, nSize%bysPerClus, &nWrite, NULL))
				{
					return FALSE;
				}
				
			k++;
			if(k >= nFileClus)
			{
				break;
			}
			SetClus(fstClus, m_EOC_STATUS);
			fstClus = GetFirstFreeClusNum();
			SetClus(curClus, fstClus);			
		}		
		SetClus(fstClus, m_EOC_STATUS);	
	}
	else
	{
		SetClus(fstClus, m_EOC_STATUS);
	}
	delete[] pLongName;
	delete[] pName;
	return TRUE;
}

BOOL  DiskImgFile::ImgDeleteDirectory(UINT clusNum)
{
	Fat_DirectoryRW  dirRw;
	Fat_Directory dir;
	DWORD nRead;

	memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
	UINT retAddr = this->ClusterToRelatAddr(clusNum);
	
	SetDiskFilePointer(NULL, retAddr, NULL, (DWORD)FILE_BEGIN);

	UINT maxDircnt = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;

	for(int i = 0; i < maxDircnt; i++)
	{	
		SetDiskFilePointer(NULL, retAddr + i*32, NULL, (DWORD)FILE_BEGIN);
		ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
		if(nRead != sizeof(Fat_DirectoryRW))
		{
			printmsg("Error In delete ----Read DirRW Error!");
			return FALSE;
		}
	
		if(dirRw.DIR_Name[0]== (char)0xE5)
			continue;
		if(dirRw.DIR_Name[0] == 0x00)
			break;
		if(IsFile(dirRw.DIR_Attr))  // 检测到是文件
		{
			DirInfoFromRwInfo(dir, dirRw);
			UINT firstClus = dir.DIR_FstClusLO;
			ASSERT(firstClus >=2);
			UINT curc = firstClus;
			if(_fats.at(curc)== m_EOC_STATUS)
			{
				SetClus(curc, FREE_STATUS);
				return TRUE;
			}
			while(_fats.at(curc) != m_EOC_STATUS)
			{
				InitializeClus(curc);
				UINT temp = _fats.at(curc);
				SetClus(curc, FREE_STATUS);
				curc = temp;				
			}
			InitializeClus(curc);
			SetClus(curc, FREE_STATUS);
			memset(dir.DIR_Name, 0x00, 8);			
		}
		else if(IsFoulder(dirRw.DIR_Attr))
		{
		
			LPSTR thisDir = ".       ";
			LPSTR upDir = "..      ";
// 			LPSTR pch = GetStrFromChArry(dirRw.DIR_Name, 8);
// 			if((strcmp(pch, thisDir) == 0) || (strcmp(pch, upDir) == 0))
			if((memcpy(dirRw.DIR_Name, thisDir, 8) == 0) || (memcpy(dirRw.DIR_Name, upDir, 8) == 0))
			{
				//delete pch;
				continue;
			}
			//delete pch;
			DirInfoFromRwInfo(dir, dirRw);
			WORD c = dir.DIR_FstClusLO;//_fats.at(dir.DIR_FstClusLO);
			while(1)
			{
				ImgDeleteDirectory(c);
				InitializeClus(c);
				WORD temp = c;
				c = _fats.at(c);
				SetClus(temp, FREE_STATUS);
				if(c == (WORD)m_EOC_STATUS)
					break;
			}
		}
	}
	return TRUE;
}
				   
BOOL  DiskImgFile::ImgDeleteDirectory(IN LPCTSTR lptDirName)					// 删除目录函数
{

	int ns = _curDirectory.size();
	DWORD nWrite;
	
	LPTSTR lpLongName = new TCHAR[MAX_PATH];
	int i=0;

	int nIndex = 0;
	int entryStartIndex = 0;
	Fat_Directory dir;
	while(1)
	{
		memset(lpLongName, 0x00, MAX_PATH * sizeof(TCHAR) );
		entryStartIndex = nIndex;
		if(!GetDirectoryTabEx(dir, lpLongName, nIndex))
		{
			delete[] lpLongName;
			return FALSE;
		}
		if(strcasecmp(lpLongName, lptDirName) == 0)
		{
			break;
		}
	}

	UINT firstClus = dir.DIR_FstClusLO;
	ASSERT(firstClus >=2);
	UINT curc = firstClus;
	if(_fats.at(curc)== m_EOC_STATUS)
	{
		ImgDeleteDirectory(curc);
		SetClus(curc, FREE_STATUS);
		InitializeClus(curc);
	//	return TRUE;
	}
	else
	{
		while(1)//_fats.at(curc) != m_EOC_STATUS)
		{
			ImgDeleteDirectory(curc);
			UINT temp = _fats.at(curc);
			InitializeClus(curc);
			SetClus(curc, FREE_STATUS);
			curc = temp;				
			if(curc == m_EOC_STATUS)
				break;
		}
	}
//	InitializeClus(curc);
//	SetClus(curc, FREE_STATUS);

/*
	ImgDeleteDirectory(dir.DIR_FstClusLO);
	this->SetClus(dir.DIR_FstClusLO, FREE_STATUS);
	InitializeClus(dir.DIR_FstClusLO);
*/
	//int nNeed = this->CalcNeedTabCount(lpDirName);
	int nNeed = dir.DIR_EntryCount;
	if (nNeed == 0)	
		nNeed = 1;

	if(ns == 0 )	// 在根目录下删除目录
	{	
		BYTE emptyFlag[2] = {0xE5 , 0xFF};
		
		for( i = 0; i < nNeed; i++)
		{
			SetDiskFilePointer(NULL, _stOfRootAddr + (dir.DIR_PosIndex - i) * 32, NULL, FILE_BEGIN);
			WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
		}
// 		if (!DeleteDirEntry(0 , entryStartIndex , nIndex-1))
// 		{
// 			return FALSE;
// 		}
	}

	else			// 在子目录下删除目录
	{
		Fat_Directory& f = _curDirectory.at(ns - 1);
		int nPos = dir.DIR_PosIndex;
		WORD curC = f.DIR_FstClusLO;

		int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
		int nCluIndex = nPos /recPerCluster;  // 记录在该目录的第 几个簇下操作
		UINT retIndex = nPos%recPerCluster;

		UINT retAddr = ClusterToRelatAddr(curC);

		if(retIndex < (nNeed -1))
		{
			retIndex = recPerCluster - (nNeed  - retIndex - 1);					
			nCluIndex--;
		}
		else
			retIndex = retIndex - (nNeed - 1); 
		
		for( i = 0; i < nCluIndex; i++)
		{
			curC = _fats.at(curC);
			if (curC == m_EOC_STATUS)
			{
				return	FALSE;
			}
			retAddr = ClusterToRelatAddr(curC);
		}
		
		BYTE emptyFlag[2] = {0xE5 , 0xFF};

		for(i = 0; i < nNeed; i++)
		{
			SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
			WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
			retIndex++;
			if(retIndex >= recPerCluster)
			{
				curC = _fats.at(curC);
				if (curC == m_EOC_STATUS) return FALSE;
				
				retAddr = ClusterToRelatAddr(curC);
				retIndex = 0;
			}
		}

	}
	delete[] lpLongName;

	return TRUE;
}

BOOL DiskImgFile::DeleteDirEntry(UINT nClus , int nStartIndex , int nEndIndex )
{
	BYTE emptyFlag[2] = {0xE5 , 0xFF};
	DWORD nWrite = 0;
	int ns = _curDirectory.size();
	int nNeed = nEndIndex - nStartIndex + 1;
	int i =0 ;

	if (ns == 0) // root dir
	{	
		//Delete all entry , long name and short name
 		for ( i = 0;i<nNeed;i++)
		{
 			SetDiskFilePointer(NULL, _stOfRootAddr + (nStartIndex + i) * 32, NULL, FILE_BEGIN);
 			if (!WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL))
 			{
 				return FALSE;
 			}
 			
 		}
	
// 		//just delete short name entry
// 		SetDiskFilePointer(NULL, _stOfRootAddr + nEndIndex * 32, NULL, FILE_BEGIN);
// 		if (!WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL))
// 		{
// 			return FALSE;
// 		}
	}
	else
	{
		int i = 0;					
		WORD curC = nClus;
				
		int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值

		//Delete all entry , long name and short name
		int nCluIndex = nStartIndex /recPerCluster;  // 记录在该目录的第 几个簇下操作
		UINT retIndex = nStartIndex%recPerCluster;
		UINT retAddr = ClusterToRelatAddr(curC);
	
		for( i = 0; i < nCluIndex; i++)
		{
			curC = _fats.at(curC);
			retAddr = ClusterToRelatAddr(curC);
		}		
		
		for (i = 0;i<nNeed;i++)
		{
			SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
			if (!WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL))
			{
				return FALSE;
			}			

			retIndex++;
			if(retIndex >= recPerCluster)
			{
			 	curC = _fats.at(curC);
			 	retAddr = ClusterToRelatAddr(curC);
			 	retIndex = 0;
 			}
		}

// 		//just delete short name entry
// 		int nCluIndex = nEndIndex /recPerCluster;  // 记录在该目录的第 几个簇下操作
// 		UINT retIndex = nEndIndex%recPerCluster;
// 		UINT retAddr = ClusterToRelatAddr(curC);
// 		
// 		for( i = 0; i < nCluIndex; i++)
// 		{
// 			curC = _fats.at(curC);
// 			retAddr = ClusterToRelatAddr(curC);
// 		}		
// 		
// 	
// 		SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 		if (!WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL))
// 		{
// 			return FALSE;
// 		}			
			

	}

	return TRUE;
}

BOOL  DiskImgFile::ImgDeleteFile(IN LPCTSTR lptFileName)						// 删除文件函数
{
	int ns = _curDirectory.size();
	DWORD nWrite;
	
	LPTSTR lpLongName = new TCHAR[MAX_PATH];
	
	int nIndex = 0;
	
	Fat_Directory dir;
	while(1)
	{
		memset(lpLongName, 0x00, MAX_PATH * sizeof(TCHAR));
		
		if(!GetDirectoryTabEx(dir, lpLongName, nIndex))
		{
			delete[] lpLongName;
			return FALSE;
		}
		if(strcasecmp(lpLongName, lptFileName) == 0)
		{
			break;
		}
	}

	//int nNeed = CalcNeedTabCount(lpFileName);
	int nNeed = dir.DIR_EntryCount;
	if (nNeed == 0)	
		nNeed = 1;
		
	if(ns == 0 )	// 在根目录下删除文件
	{		
		BYTE emptyFlag[2] = {0xE5 , 0xFF};
		
		for(int i = 0; i < nNeed; i++)
		{
			SetDiskFilePointer(NULL, _stOfRootAddr + (nIndex - i - 1) * 32, NULL, FILE_BEGIN);
			WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
		}
		

		UINT firstClus = dir.DIR_FstClusLO;	
		UINT curc = firstClus;
		SetClusFreeStatus(curc);
	}
	
	else			// 在子目录下删除文件
	{
		int i = 0;
		Fat_Directory& parDir = _curDirectory.at(ns - 1);
		UINT retAddr = ClusterToRelatAddr(parDir.DIR_FstClusLO);
		
		BYTE emptyFlag[2] = {0xE5 , 0xFF};
		
		int nPos = dir.DIR_PosIndex;
		WORD curC = parDir.DIR_FstClusLO;

		int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
		int nCluIndex = nPos /recPerCluster;  // 记录在该目录的第 几个簇下操作
		UINT retIndex = nPos%recPerCluster;

		if(retIndex < (nNeed -1))
		{
			retIndex = recPerCluster - (nNeed  - retIndex - 1);					
			nCluIndex--;
		}
		else
			retIndex = retIndex - (nNeed - 1); 

		for( i = 0; i < nCluIndex; i++)
		{
			curC = _fats.at(curC);
			if (curC == m_EOC_STATUS) return FALSE;
			retAddr = ClusterToRelatAddr(curC);
		}

		
		for(i = 0; i < nNeed; i++)
		{
			SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
			WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
			retIndex++;
			if(retIndex >= recPerCluster)
			{
				curC = _fats.at(curC);
				if (curC == m_EOC_STATUS) /*return FALSE*/break;
				retAddr = ClusterToRelatAddr(curC);
				retIndex = 0;
			}
		}


		UINT firstClus = dir.DIR_FstClusLO;

 		UINT curc = firstClus;

		SetClusFreeStatus(curc);
	}

	delete[] lpLongName;
	return TRUE;
}

BOOL  DiskImgFile::ImgMoveFile(IN LPCSTR lpFileName, IN LPCSTR lpSrcDir, IN LPCSTR lpDesFileName, IN LPCSTR lpDesDir)		// 移动文件函数
{	
	return TRUE;
// 	vector<Fat_Directory> srcDirs;
// 	vector<Fat_Directory> desDirs;
// 	
// 	srcDirs.resize(0);
// 	desDirs.resize(0);
// 
// 	if(!ParaDirectoryFromStr(lpSrcDir, srcDirs))
// 	{
// 		printmsg("The source path is invalide!");
// 		return FALSE;
// 	}
// 	if(!ParaDirectoryFromStr(lpDesDir, desDirs))
// 	{
// 		printmsg("The destion path is invalide");
// 		return FALSE;
// 	}
// 
// /*
// 	if(srcDirs.size() == 0) // 不能将根目录移动
// 		return FALSE;
// */
// 
// //	Fat_Directory& srcTab = srcDirs.at(srcDirs.size() - 1);
// 
// 	SetCurrentDirectory(lpSrcDir);
// 	
// 	Fat_Directory fileTab;
// 	GetDirectoryTab(fileTab, lpFileName);
// 	
// 	SetCurrentDirectory(lpDesDir);
// 	
// 	int ns = _curDirectory.size();
// 	int nNeed = this->CalcNeedTabCount(lpDesFileName); // 需要存放的结构的个数
// 
// 	UINT len = strlen(lpDesFileName);
// 	LPWSTR pLongName = new WCHAR[len + 1];
// 	memset(pLongName, 0x00, (len + 1) * 2);
// 	int nl = ::MultiByteToWideChar(m_CodePage, 0, lpDesFileName, len, pLongName, len+1);
// 	
// 	
// 	LPSTR pName = new char[12];
// 	memset(pName, 0x00, 12);
// 	if(!GenerateShortName(lpDesFileName, pName))// new char[11];
// 	{
// 		printmsg("Generate short name faile!");
// 		delete[] pLongName;
// 		delete[] pName;
// 		return FALSE;
// 	}
// 
// 	 len = strlen(pName);
// 	ASSERT(len <= 12);
// 
// 
// 	DWORD nRead;
// 	DWORD nWrite;
// 		
// 	BYTE chkFlag = ChkSum(pName);
// 	if(ns == 0) // 在根目录下创建文件
// 	{
// 		Fat_Directory dir;
// 		Fat_DirectoryRW dirRw;
// 	
// 		INT nIndex = -1;
// 
// //////////////////////////////////////////////////////////////////////////
// // 开始建立目录项
// 		memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
// 		memset(&dir, 0x00, sizeof(Fat_Directory));
// 		SetDiskFilePointer(NULL, _stOfRootAddr, NULL, (DWORD)FILE_BEGIN);
// 		int i = 0;
// 
// 		for( i = 0; i < ROOTENTCNT; i++)
// 		{			
// 			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
// 			if(nRead != sizeof(Fat_DirectoryRW))
// 			{
// 				printmsg("Error In Create RootDirectroy --Read DirRW Error!");
// 				return FALSE;
// 			}
// 			if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed == 1))   // 判断是否为空项
// 			{
// 				nIndex = i;
// 				break;
// 			}
// 		}
// 		if(nIndex == -1)
// 		{
// 			printmsg("This Directroy is full!");
// 			return FALSE;
// 		}		
// 	
// 		//////////////////////////////////////////////////////////////////////////
// 		// 处理长名目录项目
// 		if(nNeed > 1)
// 		{
// 			Fat_LongDirectory longDir;
// 			memset(&longDir, 0x00, 32);
// 			longDir.LDIR_Attr = ATTR_LONG_NAME;
// 			longDir.LDIR_Chksum = chkFlag;
// 			longDir.LDIR_FstClusLO = 0;
// 			longDir.LDIR_Ord = LAST_LONG_ENTRY|(BYTE)(nNeed -1);
// 			longDir.LDIR_Type = 0;		
// 			int npos = (nNeed - 2) * 13;
// 			memset(longDir.LDIR_Name1, 0xff, 26);
// 			memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
// 			if((nl - npos) < 13)
// 				memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
// 			RwInfoFromLongDirInfo(dirRw, longDir);
// 			SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
// 			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 			{
// 				
// 				printmsg("Error In create RootDirectory -- write dir Struct");
// 				return FALSE;
// 			}
// 			nIndex ++;	
// 		}
// 		for(i = nNeed - 2; i >0 ; i--)
// 		{
// 			Fat_LongDirectory longDir;
// 			memset(&longDir, 0x00, 32);
// 			longDir.LDIR_Attr = ATTR_LONG_NAME;
// 			longDir.LDIR_Chksum = chkFlag;
// 			longDir.LDIR_FstClusLO = 0;
// 			longDir.LDIR_Ord = (BYTE)i;
// 			longDir.LDIR_Type = 0;
// 		//	int nl = strlen(lpDirName);
// 			int npos = (i - 1) * 13;
// 			memset(longDir.LDIR_Name1, 0xff, 26);
// 			memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
// 			RwInfoFromLongDirInfo(dirRw, longDir);
// 		//	SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
// 			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 			{
// 				delete[] pName;
// 				printmsg("Error In create RootDirectory -- write dir Struct");
// 				return FALSE;
// 			}
// 			nIndex ++;	
// 		}
// 		//////////////////////////////////////////////////////////////////////////
// 		
// 
// 		memcpy(&dir.DIR_Name, pName, 11);
// 		dir.DIR_Attr = fileTab.DIR_Attr;
// 		UINT clus = fileTab.DIR_FstClusLO;
// 		
// 		ASSERT(clus >= 2);
// 		dir.DIR_FstClusHI = 0;
// 		dir.DIR_FstClusLO = clus;
// 		dir.DIR_WrtDateTime = fileTab.DIR_WrtDateTime;
// 		dir.DIR_CrtDateTime = fileTab.DIR_CrtDateTime;
// 		dir.DIR_LstAcceDate = fileTab.DIR_LstAcceDate;
// 		dir.DIR_FileSize = fileTab.DIR_FileSize;
// 
// 		RwInfoFromDirInfo(dirRw, dir);
// 		SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
// 		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 		{
// 			printmsg("Error In create RootDirectory -- write dir Struct");
// 			return FALSE;
// 		}			
// 
// 		//////////////////////////////////////////////////////////////////////////
// 		// 取消原来位置的文件表格
// 		ns = srcDirs.size();
// 		UINT retAddr;
// 		if(ns == 0)
// 		{
// 			retAddr = _stOfRootAddr;
// 			int nPos = dir.DIR_PosIndex;
// 			retAddr = retAddr;// + nPos*32;
// 			int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
// 
// 			//BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 			BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 			UINT retIndex = nPos;
// 			retIndex = retIndex - (nNeed - 1);
// 			for(i = 0; i < nNeed; i++)
// 			{
// 				SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 				WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
// 				retIndex++;		
// 			}
// 
// 		}
// 		else
// 		{
// 			retAddr = ClusterToRelatAddr(srcDirs.at(ns - 1).DIR_FstClusLO);
// 
// 			Fat_Directory& f = srcDirs.at(ns - 1);
// 			int nPos = fileTab.DIR_PosIndex;
// 			WORD curC = f.DIR_FstClusLO;
// 
// 			int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
// 			int nCluIndex = nPos /recPerCluster;  // 记录在该目录的第 几个簇下操作
// 			UINT retIndex = nPos%recPerCluster;
// 
// 			UINT retAddr = ClusterToRelatAddr(curC);
// 
// 			for(int i = 0; i < nCluIndex; i++)
// 			{
// 				curC = _fats.at(curC);
// 				if (curC == m_EOC_STATUS) return FALSE;
// 				retAddr = ClusterToRelatAddr(curC);
// 			}
// 
// 			if(retIndex < (nNeed -1))
// 			{
// 				retIndex = recPerCluster - (nNeed - 1 - retIndex);
// 				nCluIndex--;
// 				for(int i = 0; i < nCluIndex; i++)
// 				{
// 					curC = _fats.at(curC);
// 					if (curC == m_EOC_STATUS) return FALSE;
// 					retAddr = ClusterToRelatAddr(curC);
// 				}
// 			}
// 			
// 			//BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 			BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 
// 			retIndex = retIndex - (nNeed - 1);
// 			for(i = 0; i < nNeed; i++)
// 			{
// 				SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 				WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
// 				retIndex++;
// 				if(retIndex >= recPerCluster)
// 				{
// 					curC = _fats.at(curC);
// 					if (curC == m_EOC_STATUS) return FALSE;
// 					retAddr = ClusterToRelatAddr(curC);
// 					retIndex = 0;
// 				}
// 			}
// 		}
// 
// 		
// 	}
// 	
// 	else  //在子目录下创建子目录
// 	{
// 
// 		//////////////////////////////////////////////////////////////////////////
// 		// 建立新目录项
// 		Fat_Directory curD = _curDirectory.at(_curDirectory.size() - 1);
// 
// 		UINT dirAdd = this->ClusterToRelatAddr(curD.DIR_FstClusLO);
// 
// 		Fat_Directory dir;
// 		Fat_DirectoryRW dirRw;
// 		INT nIndex = -1;
// 
// 		memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
// 		memset(&dir, 0x00, sizeof(Fat_Directory));
// 		SetDiskFilePointer(NULL, dirAdd, NULL, (DWORD)FILE_BEGIN);
// 		UINT maxDircnt = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;
// 		WORD cClus = curD.DIR_FstClusLO;
// 		int  nDirPos = 0; // 记录目录的相对位置 放置在Fat_Directory.DIR_PosIndex中
// 		while(1)
// 		{
// 			for(int i = 0; i < maxDircnt; i++)
// 			{	
// 				ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
// 				if(nRead != sizeof(Fat_DirectoryRW))
// 				{
// 					printmsg("Error In Create RootDirectroy --Read DirRW Error!");
// 					return FALSE;
// 				}
// 				
// 				if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed == 1))   // 判断是否为空项
// 				{
// 					nIndex = i;
// 					break;
// 				}	
// 				nDirPos++;
// 			}
// 			if(nIndex != -1)
// 				break;
// 			cClus = _fats.at(cClus);
// 			if(cClus == m_EOC_STATUS)
// 				break;
// 			dirAdd = ClusterToRelatAddr(cClus);
// 			SetDiskFilePointer(NULL, dirAdd, NULL, (DWORD)FILE_BEGIN);
// 		}
// 		if(nIndex == -1)
// 		{
// 			UINT c = GetFirstFreeClusNum();
// 			SetClus(cClus, c);
// 			SetClus(c, m_EOC_STATUS);
// 			InitializeClus(c);
// 			nIndex = nDirPos;
// 			dirAdd = ClusterToRelatAddr(c);
// 		}	
// 		
// 
// 		//////////////////////////////////////////////////////////////////////////
// 		// 处理长名目录项目
// 		int retIndex = nIndex%maxDircnt;
// 		if(nNeed > 1)
// 		{
// 			Fat_LongDirectory longDir;
// 			memset(&longDir, 0x00, 32);
// 			longDir.LDIR_Attr = ATTR_LONG_NAME;
// 			longDir.LDIR_Chksum = chkFlag;
// 			longDir.LDIR_FstClusLO = 0;
// 			longDir.LDIR_Ord = LAST_LONG_ENTRY|(BYTE)(nNeed - 1);
// 			longDir.LDIR_Type = 0;
// 		//	int nl = strlen(lpDirName);
// 			int npos = (nNeed - 2) * 13;
// 			memset(longDir.LDIR_Name1, 0xff, 26);
// 			memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
// 			if((nl - npos) < 13)
// 				memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
// 			RwInfoFromLongDirInfo(dirRw, longDir);
// 			SetDiskFilePointer(NULL, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 			{
// 				
// 				printmsg("Error In create RootDirectory -- write dir Struct");
// 				return FALSE;
// 			}
// 			retIndex++;
// 			nIndex ++;	
// 			if(retIndex >= maxDircnt)
// 			{
// 				UINT c = GetFirstFreeClusNum();
// 				SetClus(cClus, c);
// 				SetClus(c, m_EOC_STATUS);
// 				InitializeClus(c);
// 				retIndex = 0;
// 				dirAdd = ClusterToRelatAddr(c);
// 			}
// 		}
// 		for( int i = nNeed - 2; i >0 ; i--)
// 		{
// 			Fat_LongDirectory longDir;
// 			memset(&longDir, 0x00, 32);
// 			longDir.LDIR_Attr = ATTR_LONG_NAME;
// 			longDir.LDIR_Chksum = chkFlag;
// 			longDir.LDIR_FstClusLO = 0;
// 			longDir.LDIR_Ord = (BYTE)i;
// 			longDir.LDIR_Type = 0;
// 			int nl = strlen(lpFileName);
// 			int npos = (i - 1) * 13;
// 			memset(longDir.LDIR_Name1, 0xff, 26);
// 			memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
// 			RwInfoFromLongDirInfo(dirRw, longDir);
// 			SetDiskFilePointer(NULL, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 			{
// 		
// 				printmsg("Error In create RootDirectory -- write dir Struct");
// 				return FALSE;
// 			}
// 			retIndex++;
// 			nIndex ++;	
// 			if(retIndex >= maxDircnt)
// 			{
// 				UINT c = GetFirstFreeClusNum();
// 				SetClus(cClus, c);
// 				SetClus(c, m_EOC_STATUS);
// 				InitializeClus(c);
// 				retIndex = 0;
// 				dirAdd = ClusterToRelatAddr(c);
// 			}
// 		}
// 		//////////////////////////////////////////////////////////////////////////
// 		
// 		SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 		memcpy(&dir.DIR_Name, pName, 11);
// 		dir.DIR_Attr = fileTab.DIR_Attr;
// /*		dir.DIR_WrtDateTime = CTime::GetCurrentTime();
// 		dir.DIR_CrtDateTime = CTime::GetCurrentTime();
// 		dir.DIR_LstAcceDate = CTime::GetCurrentTime();
// */
// 		::GetSystemTime(&dir.DIR_WrtDateTime);
// 		::GetSystemTime(&dir.DIR_CrtDateTime);
// 		::GetSystemTime(&dir.DIR_LstAcceDate);
// 		dir.DIR_FileSize = fileTab.DIR_FileSize;
// 
// 		UINT clus = fileTab.DIR_FstClusLO;
// 		
// 		ASSERT(clus >= 2);
// 		dir.DIR_FstClusHI = 0;
// 		dir.DIR_FstClusLO = clus;
// 	
// 		RwInfoFromDirInfo(dirRw, dir);
// 		SetDiskFilePointer(NULL, dirAdd + 32 * nIndex, NULL, FILE_BEGIN);
// 		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 		{
// 			
// 			printmsg("Error In create RootDirectory -- write dir Struct");
// 			return FALSE;
// 		}
// 		//
// 		//////////////////////////////////////////////////////////////////////////
// 			
// 		UINT retAddr ;
// 		
// 		//////////////////////////////////////////////////////////////////////////
// 		// 取消原来位置的目录表格
// 
// 		ns = srcDirs.size();
// 		if(ns == 0)
// 		{
// 			retAddr = _stOfRootAddr;
// 			int nPos = fileTab.DIR_PosIndex;
// 			retAddr = retAddr ;//+ nPos*32;
// 			int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
// 
// 			//BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 			BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 			
// 			UINT retIndex = nPos;
// 			retIndex = retIndex - (nNeed - 1);
// 			int i;
// 			for(i = 0; i < nNeed; i++)
// 			{
// 				SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 				WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
// 				retIndex++;		
// 			}
// 
// 		}
// 		else
// 		{
// 			int i=0;
// 			retAddr = ClusterToRelatAddr(srcDirs.at(ns - 1).DIR_FstClusLO);
// 
// 			Fat_Directory& f = srcDirs.at(ns - 1);
// 			int nPos = fileTab.DIR_PosIndex;
// 			WORD curC = f.DIR_FstClusLO;
// 
// 			int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
// 			int nCluIndex = nPos /recPerCluster;  // 记录在该目录的第 几个簇下操作
// 			UINT retIndex = nPos%recPerCluster;
// 
// 			UINT retAddr = ClusterToRelatAddr(curC);
// 
// 			for( i = 0; i < nCluIndex; i++)
// 			{
// 				curC = _fats.at(curC);
// 				if (curC == m_EOC_STATUS) return FALSE;
// 				retAddr = ClusterToRelatAddr(curC);
// 			}
// 
// 			if(retIndex < (nNeed -1))
// 			{
// 				retIndex = recPerCluster - (nNeed - 1 - retIndex);
// 				nCluIndex--;
// 				for(int i = 0; i < nCluIndex; i++)
// 				{
// 					curC = _fats.at(curC);
// 					if (curC == m_EOC_STATUS) return FALSE;
// 					retAddr = ClusterToRelatAddr(curC);
// 				}
// 			}
// 			
// 			BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 
// 			retIndex = retIndex - (nNeed - 1);
// 
// 			for(i = 0; i < nNeed; i++)
// 			{
// 				SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 				WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
// 				retIndex++;
// 				if(retIndex >= recPerCluster)
// 				{
// 					curC = _fats.at(curC);
// 					if (curC == m_EOC_STATUS) return FALSE;
// 					retAddr = ClusterToRelatAddr(curC);
// 					retIndex = 0;
// 				}
// 			}
// 		}
// 
// 	}	
// 	delete[] pName;
// 	delete[] pLongName;
// //	delete[] lpLongName;
// 	return TRUE;
}

BOOL  DiskImgFile::ImgMoveDirectory(IN LPCSTR lpSrcDir, IN LPCSTR lpDesDir)	// 移动目录函数
{
	return TRUE;
// 	vector<Fat_Directory> srcDirs;
// 	vector<Fat_Directory> desDirs;
// 	
// 	srcDirs.resize(0);
// 	desDirs.resize(0);
// 
// 	if(!ParaDirectoryFromStr(lpSrcDir, srcDirs))
// 	{
// 		printmsg("The source path is invalide!");
// 		return FALSE;
// 	}
// 	if(!ParaDirectoryFromStr(lpDesDir, desDirs))
// 	{
// 		printmsg("The destion path is invalide");
// 		return FALSE;
// 	}
// 
// 	LPSTR lpDirName = new char[MAX_PATH];
// 	memset(lpDirName, 0X00, MAX_PATH);
// 	if(!GetRighStrByFind(lpDirName, lpSrcDir, '\\', -1, ""))
// 	{
// 		delete lpDirName;
// 		return FALSE;
// 	}
// 
// 
// 	if(srcDirs.size() == 0) // 不能将根目录移动
// 		return FALSE;
// 
// 	Fat_Directory& srcTab = srcDirs.at(srcDirs.size() - 1);
// 	
// 	SetCurrentDirectory(lpDesDir);
// 
// 	
// 
// 	int ns = _curDirectory.size();
// 	int nNeed = this->CalcNeedTabCount(lpDirName); // 需要存放的结构的个数
// 
// 	UINT len = strlen(lpDirName);
// 	LPWSTR pLongName = new WCHAR[len + 1];
// 	memset(pLongName, 0x00, (len + 1) * 2);
// 	int nl = ::MultiByteToWideChar(m_CodePage, 0, lpDirName, len, pLongName, len+1);
// 	
// 	
// 	LPSTR pName = new char[11];
// 	memset(pName, 0x00, 11);
// 	if(!GenerateShortName(lpDirName, pName))// new char[11];
// 	{
// 		delete[] pName;
// 		delete[] pLongName;
// 		printmsg("Generate short name faile!");
// 		return FALSE;
// 	}
// 
// 	 len = strlen(pName);
// 	ASSERT(len <= 12);
// 
// 
// 	DWORD nRead;
// 	DWORD nWrite;
// 		
// 	BYTE chkFlag = ChkSum(pName);
// 	if(ns == 0) // 在根目录下创建字目录
// 	{
// 		Fat_Directory dir;
// 		Fat_DirectoryRW dirRw;
// 	
// 		INT nIndex = -1;
// 
// //////////////////////////////////////////////////////////////////////////
// // 开始建立目录项
// 		memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
// 		memset(&dir, 0x00, sizeof(Fat_Directory));
// 		SetDiskFilePointer(NULL, _stOfRootAddr, NULL, (DWORD)FILE_BEGIN);
// 		int i = 0;
// 
// 		for( i = 0; i < ROOTENTCNT; i++)
// 		{			
// 			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
// 			if(nRead != sizeof(Fat_DirectoryRW))
// 			{
// 				printmsg("Error In Create RootDirectroy --Read DirRW Error!");
// 				return FALSE;
// 			}
// 			if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed == 1))   // 判断是否为空项
// 			{
// 				nIndex = i;
// 				break;
// 			}
// 		}
// 		if(nIndex == -1)
// 		{
// 			
// 			printmsg("This Directroy is full!");
// 			return FALSE;
// 		}		
// 	
// 		//////////////////////////////////////////////////////////////////////////
// 		// 处理长名目录项目
// 		if(nNeed > 1)
// 		{
// 			Fat_LongDirectory longDir;
// 			memset(&longDir, 0x00, 32);
// 			longDir.LDIR_Attr = ATTR_LONG_NAME;
// 			longDir.LDIR_Chksum = chkFlag;
// 			longDir.LDIR_FstClusLO = 0;
// 			longDir.LDIR_Ord = LAST_LONG_ENTRY|(BYTE)(nNeed -1);
// 			longDir.LDIR_Type = 0;		
// 			int npos = (nNeed - 2) * 13;
// 			memset(longDir.LDIR_Name1, 0xff, 26);
// 			memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
// 			if((nl - npos) < 13)
// 				memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
// 			RwInfoFromLongDirInfo(dirRw, longDir);
// 			SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
// 			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 			{
// 				
// 				printmsg("Error In create RootDirectory -- write dir Struct");
// 				return FALSE;
// 			}
// 			nIndex ++;	
// 		}
// 		for(i = nNeed - 2; i >0 ; i--)
// 		{
// 			Fat_LongDirectory longDir;
// 			memset(&longDir, 0x00, 32);
// 			longDir.LDIR_Attr = ATTR_LONG_NAME;
// 			longDir.LDIR_Chksum = chkFlag;
// 			longDir.LDIR_FstClusLO = 0;
// 			longDir.LDIR_Ord = (BYTE)i;
// 			longDir.LDIR_Type = 0;
// 		//	int nl = strlen(lpDirName);
// 			int npos = (i - 1) * 13;
// 			memset(longDir.LDIR_Name1, 0xff, 26);
// 			memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
// 			RwInfoFromLongDirInfo(dirRw, longDir);
// 		//	SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
// 			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 			{
// 				delete[] pName;
// 				printmsg("Error In create RootDirectory -- write dir Struct");
// 				return FALSE;
// 			}
// 			nIndex ++;	
// 		}
// 		//////////////////////////////////////////////////////////////////////////
// 		
// 
// 		memcpy(&dir.DIR_Name, pName, 11);
// 		dir.DIR_Attr = 0x10;
// 		UINT clus = srcTab.DIR_FstClusLO;
// 		
// 		ASSERT(clus >= 2);
// 		dir.DIR_FstClusHI = 0;
// 		dir.DIR_FstClusLO = clus;
// 		dir.DIR_WrtDateTime = srcTab.DIR_WrtDateTime;
// 		dir.DIR_CrtDateTime = srcTab.DIR_CrtDateTime;
// 		dir.DIR_LstAcceDate = srcTab.DIR_LstAcceDate;
// 
// 		RwInfoFromDirInfo(dirRw, dir);
// 		SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
// 		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 		{
// 			
// 			printmsg("Error In create RootDirectory -- write dir Struct");
// 			return FALSE;
// 		}			
// 
// 		//////////////////////////////////////////////////////////////////////////
// 		// 修改..目录的表格
// 		UINT retAddr = ClusterToRelatAddr(srcTab.DIR_FstClusLO);
// 		SetDiskFilePointer(NULL, retAddr +32, NULL, FILE_BEGIN);
// 		ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
// 		if(nRead != sizeof(Fat_DirectoryRW))
// 		{
// 			printmsg("Error! ");
// 			return FALSE;
// 		}		
// 		DirInfoFromRwInfo(dir, dirRw);
// 		dir.DIR_FstClusLO = 0;
// 		RwInfoFromDirInfo(dirRw, dir);
// 		SetDiskFilePointer(NULL, retAddr +32, NULL, FILE_BEGIN);
// 		WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL);
// 		if(nWrite != sizeof(Fat_DirectoryRW))
// 		{
// 			printmsg("Error! ");
// 			return FALSE;
// 		}	
// 
// 		//////////////////////////////////////////////////////////////////////////
// 		// 取消原来位置的目录表格
// 		ns = srcDirs.size();
// 		if(ns <= 1)
// 		{
// 			retAddr = _stOfRootAddr;
// 			int nPos = srcTab.DIR_PosIndex;
// 			retAddr = retAddr + nPos*32;
// 			int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
// 
// 			BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 			UINT retIndex = nPos;
// 			retIndex = retIndex - (nNeed - 1);
// 			for(i = 0; i < nNeed; i++)
// 			{
// 				SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 				WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
// 				retIndex++;		
// 			}
// 
// 		}
// 		else
// 		{
// 			retAddr = ClusterToRelatAddr(srcDirs.at(ns - 2).DIR_FstClusLO);
// 
// 			Fat_Directory& f = srcDirs.at(ns - 2);
// 			int nPos = srcTab.DIR_PosIndex;
// 			WORD curC = f.DIR_FstClusLO;
// 
// 			int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
// 			int nCluIndex = nPos /recPerCluster;  // 记录在该目录的第 几个簇下操作
// 			UINT retIndex = nPos%recPerCluster;
// 
// 			UINT retAddr = ClusterToRelatAddr(curC);
// 
// 			for(int i = 0; i < nCluIndex; i++)
// 			{
// 				curC = _fats.at(curC);
// 				if (curC == m_EOC_STATUS) return FALSE;
// 				retAddr = ClusterToRelatAddr(curC);
// 			}
// 
// 			if(retIndex < (nNeed -1))
// 			{
// 				retIndex = recPerCluster - (nNeed - 1 - retIndex);
// 				nCluIndex--;
// 				for(int i = 0; i < nCluIndex; i++)
// 				{
// 					curC = _fats.at(curC);
// 					if (curC == m_EOC_STATUS) return FALSE;
// 					retAddr = ClusterToRelatAddr(curC);
// 				}
// 			}
// 			
// 			BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 
// 			retIndex = retIndex - (nNeed - 1);
// 			for(i = 0; i < nNeed; i++)
// 			{
// 				SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 				WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
// 				retIndex++;
// 				if(retIndex >= recPerCluster)
// 				{
// 					curC = _fats.at(curC);
// 					if (curC == m_EOC_STATUS) return FALSE;
// 					retAddr = ClusterToRelatAddr(curC);
// 					retIndex = 0;
// 				}
// 			}
// 		}
// 
// 		
// 	}
// 	
// 	else  //在子目录下创建子目录
// 	{
// 
// 		//////////////////////////////////////////////////////////////////////////
// 		// 建立新目录项
// 		Fat_Directory curD = _curDirectory.at(_curDirectory.size() - 1);
// 
// 		UINT dirAdd = this->ClusterToRelatAddr(curD.DIR_FstClusLO);
// 
// 		Fat_Directory dir;
// 		Fat_DirectoryRW dirRw;
// 		INT nIndex = -1;
// 
// 		memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
// 		memset(&dir, 0x00, sizeof(Fat_Directory));
// 		SetDiskFilePointer(NULL, dirAdd, NULL, (DWORD)FILE_BEGIN);
// 		UINT maxDircnt = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;
// 		WORD cClus = curD.DIR_FstClusLO;
// 		int  nDirPos = 0; // 记录目录的相对位置 放置在Fat_Directory.DIR_PosIndex中
// 		while(1)
// 		{
// 			for(int i = 0; i < maxDircnt; i++)
// 			{	
// 				ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
// 				if(nRead != sizeof(Fat_DirectoryRW))
// 				{
// 					printmsg("Error In Create RootDirectroy --Read DirRW Error!");
// 					return FALSE;
// 				}
// 				
// 				if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed == 1))   // 判断是否为空项
// 				{
// 					nIndex = i;
// 					break;
// 				}	
// 				nDirPos++;
// 			}
// 			if(nIndex != -1)
// 				break;
// 			cClus = _fats.at(cClus);
// 			if(cClus == m_EOC_STATUS)
// 				break;
// 			dirAdd = ClusterToRelatAddr(cClus);
// 			SetDiskFilePointer(NULL, dirAdd, NULL, (DWORD)FILE_BEGIN);
// 		}
// 		if(nIndex == -1)
// 		{
// 			UINT c = GetFirstFreeClusNum();
// 			SetClus(cClus, c);
// 			SetClus(c, m_EOC_STATUS);
// 			InitializeClus(c);
// 			nIndex = nDirPos;
// 			dirAdd = ClusterToRelatAddr(c);
// 		}	
// 		
// 
// 		//////////////////////////////////////////////////////////////////////////
// 		// 处理长名目录项目
// 		int retIndex = nIndex%maxDircnt;
// 		if(nNeed > 1)
// 		{
// 			Fat_LongDirectory longDir;
// 			memset(&longDir, 0x00, 32);
// 			longDir.LDIR_Attr = ATTR_LONG_NAME;
// 			longDir.LDIR_Chksum = chkFlag;
// 			longDir.LDIR_FstClusLO = 0;
// 			longDir.LDIR_Ord = LAST_LONG_ENTRY|(BYTE)(nNeed - 1);
// 			longDir.LDIR_Type = 0;
// 		//	int nl = strlen(lpDirName);
// 			int npos = (nNeed - 2) * 13;
// 			memset(longDir.LDIR_Name1, 0xff, 26);
// 			memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
// 			if((nl - npos) < 13)
// 				memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
// 			RwInfoFromLongDirInfo(dirRw, longDir);
// 			SetDiskFilePointer(NULL, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 			{
// 				
// 				printmsg("Error In create RootDirectory -- write dir Struct");
// 				return FALSE;
// 			}
// 			retIndex++;
// 			nIndex ++;	
// 			if(retIndex >= maxDircnt)
// 			{
// 				UINT c = GetFirstFreeClusNum();
// 				SetClus(cClus, c);
// 				SetClus(c, m_EOC_STATUS);
// 				InitializeClus(c);
// 				retIndex = 0;
// 				dirAdd = ClusterToRelatAddr(c);
// 			}
// 		}
// 		for( int i = nNeed - 2; i >0 ; i--)
// 		{
// 			Fat_LongDirectory longDir;
// 			memset(&longDir, 0x00, 32);
// 			longDir.LDIR_Attr = ATTR_LONG_NAME;
// 			longDir.LDIR_Chksum = chkFlag;
// 			longDir.LDIR_FstClusLO = 0;
// 			longDir.LDIR_Ord = (BYTE)i;
// 			longDir.LDIR_Type = 0;
// 			int nl = strlen(lpDirName);
// 			int npos = (i - 1) * 13;
// 			memset(longDir.LDIR_Name1, 0xff, 26);
// 			memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
// 			RwInfoFromLongDirInfo(dirRw, longDir);
// 			SetDiskFilePointer(NULL, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 			{
// 		
// 				printmsg("Error In create RootDirectory -- write dir Struct");
// 				return FALSE;
// 			}
// 			retIndex++;
// 			nIndex ++;	
// 			if(retIndex >= maxDircnt)
// 			{
// 				UINT c = GetFirstFreeClusNum();
// 				SetClus(cClus, c);
// 				SetClus(c, m_EOC_STATUS);
// 				InitializeClus(c);
// 				retIndex = 0;
// 				dirAdd = ClusterToRelatAddr(c);
// 			}
// 		}
// 		//////////////////////////////////////////////////////////////////////////
// 		
// 		SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 		memcpy(&dir.DIR_Name, pName, 11);
// 		dir.DIR_Attr = 0x10;
// /*		dir.DIR_WrtDateTime = CTime::GetCurrentTime();
// 		dir.DIR_CrtDateTime = CTime::GetCurrentTime();
// 		dir.DIR_LstAcceDate = CTime::GetCurrentTime();
// */	    ::GetSystemTime(&dir.DIR_WrtDateTime);
// 		::GetSystemTime(&dir.DIR_CrtDateTime);
// 		::GetSystemTime(&dir.DIR_LstAcceDate);
// 
// 		UINT clus = srcTab.DIR_FstClusLO;
// 		
// 		ASSERT(clus >= 2);
// 		dir.DIR_FstClusHI = 0;
// 		dir.DIR_FstClusLO = clus;
// 	
// 		RwInfoFromDirInfo(dirRw, dir);
// 		SetDiskFilePointer(NULL, dirAdd + 32 * nIndex, NULL, FILE_BEGIN);
// 		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 		{
// 			
// 			printmsg("Error In create RootDirectory -- write dir Struct");
// 			return FALSE;
// 		}
// 		//
// 		//////////////////////////////////////////////////////////////////////////
// 			
// 		//////////////////////////////////////////////////////////////////////////
// 		// 修改..目录的表格
// 		UINT retAddr = ClusterToRelatAddr(srcTab.DIR_FstClusLO);
// 		SetDiskFilePointer(NULL, retAddr +32, NULL, FILE_BEGIN);
// 		ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
// 		if(nRead != sizeof(Fat_DirectoryRW))
// 		{
// 			printmsg("Error! ");
// 			return FALSE;
// 		}		
// 		DirInfoFromRwInfo(dir, dirRw);
// 		dir.DIR_FstClusLO = curD.DIR_FstClusLO;
// 		RwInfoFromDirInfo(dirRw, dir);
// 		SetDiskFilePointer(NULL, retAddr +32, NULL, FILE_BEGIN);
// 		WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL);
// 		if(nWrite != sizeof(Fat_DirectoryRW))
// 		{
// 			printmsg("Error! ");
// 			return FALSE;
// 		}	
// 
// 		//////////////////////////////////////////////////////////////////////////
// 		// 取消原来位置的目录表格
// 
// 		ns = srcDirs.size();
// 		if(ns == 1)
// 		{
// 			retAddr = _stOfRootAddr;
// 			int nPos = srcTab.DIR_PosIndex;
// 			retAddr = retAddr + nPos*32;
// 			int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
// 
// 			BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 			UINT retIndex = nPos;
// 			retIndex = retIndex - (nNeed - 1);
// 			int i;
// 			for(i = 0; i < nNeed; i++)
// 			{
// 				SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 				WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
// 				retIndex++;		
// 			}
// 
// 		}
// 		else
// 		{
// 			int i=0;
// 			retAddr = ClusterToRelatAddr(srcDirs.at(ns - 2).DIR_FstClusLO);
// 
// 			Fat_Directory& f = srcDirs.at(ns - 2);
// 			int nPos = srcTab.DIR_PosIndex;
// 			WORD curC = f.DIR_FstClusLO;
// 
// 			int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
// 			int nCluIndex = nPos /recPerCluster;  // 记录在该目录的第 几个簇下操作
// 			UINT retIndex = nPos%recPerCluster;
// 
// 			UINT retAddr = ClusterToRelatAddr(curC);
// 
// 			for( i = 0; i < nCluIndex; i++)
// 			{
// 				curC = _fats.at(curC);
// 				if (curC == m_EOC_STATUS) return FALSE;
// 				retAddr = ClusterToRelatAddr(curC);
// 			}
// 
// 			if(retIndex < (nNeed -1))
// 			{
// 				retIndex = recPerCluster - (nNeed - 1 - retIndex);
// 				nCluIndex--;
// 				for(int i = 0; i < nCluIndex; i++)
// 				{
// 					curC = _fats.at(curC);
// 					if (curC == m_EOC_STATUS) return FALSE;
// 					retAddr = ClusterToRelatAddr(curC);
// 				}
// 			}
// 			
// 			BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 
// 			retIndex = retIndex - (nNeed - 1);
// 
// 			for(i = 0; i < nNeed; i++)
// 			{
// 				SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 				WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
// 				retIndex++;
// 				if(retIndex >= recPerCluster)
// 				{
// 					curC = _fats.at(curC);
// 					if (curC == m_EOC_STATUS) return FALSE;
// 					retAddr = ClusterToRelatAddr(curC);
// 					retIndex = 0;
// 				}
// 			}
// 		}
// 
// 	}	
// 	delete lpDirName;
// 	return TRUE;
}

BOOL  DiskImgFile::ImgGetFileStaus(IN LPCTSTR lptFileName,
						  OUT ImgFileStatus& status)
{
	if(lptFileName == NULL)
		return FALSE;
	
	int ns = _curDirectory.size();
	UINT retAddr = 0;
/*
		if(ns == 0)
		{
			retAddr = _stOfRootAddr;
		}
		else
		{
			Fat_Directory& tem = _curDirectory.at(ns - 1);
			retAddr = ClusterToRelatAddr(tem.DIR_FstClusLO);
		}*/
	
	LPTSTR lpName = new TCHAR[MAX_PATH];
	if(!GetRighStrByFind(lpName, lptFileName, '\\', -1, ""))
	{
		delete[] lpName;
		return FALSE;
	}
	
	Fat_Directory fileTab;
	if(!GetDirectoryTab(fileTab,  lpName))
	{
		delete[] lpName;
		return FALSE;
	}
	
	status.m_ctime = fileTab.DIR_CrtDateTime;
	status.m_mtime = fileTab.DIR_WrtDateTime;
	status.m_atime = fileTab.DIR_LstAcceDate;
	status.m_size = fileTab.DIR_FileSize;
	delete[] lpName;
	return TRUE;
}

BOOL DiskImgFile::ImgOpenFile(IN LPCSTR lpFileName,									// 打开文件函数
					  OUT PBYTE pBuffer, 
					  IN DWORD nBufferLen) 
{
	return TRUE;
// 	if(lpFileName == NULL)
// 		return FALSE;
// 	
// 	LPSTR lpCurDir = GetLeftStr(lpFileName, '\\', FALSE);
// 	if(lpCurDir == NULL)
// 		return FALSE;
// 
// 	this->SetCurrentDirectory(lpCurDir);
// 
// 	delete lpCurDir;
// 	int ns = _curDirectory.size();
// 	UINT retAddr = 0;
// 	if(ns == 0)
// 	{
// 		retAddr = _stOfRootAddr;
// 	}
// 	else
// 	{
// 		Fat_Directory& tem = _curDirectory.at(ns - 1);
// 		retAddr = ClusterToRelatAddr(tem.DIR_FstClusLO);
// 	}
// 	LPSTR lpName = new char[MAX_PATH];
// 	memset(lpName, 0x00, MAX_PATH);
// 	if(!GetRighStrByFind(lpName, lpFileName, '\\', -1, ""))
// 	{
// 		delete[] lpName;
// 		return FALSE;
// 	}
// 
// 	Fat_Directory fileTab;
// 	if(!GetDirectoryTab(fileTab,  lpName))
// 	{
// 		delete[] lpName;
// 		return FALSE;
// 	}
// 
// 	
// 	int nSize = fileTab.DIR_FileSize;
// 	UINT bysPerClus = _imgBpb.BPB_BytsPerSec * _imgBpb.BPB_SecPerClus;
// 	UINT nFileClus = (nSize +  bysPerClus - 1) / bysPerClus;
// 	DWORD nRead;
// 	int k = 0;
// 	UINT tempValue = 0xffff;
// 	UINT curClus = fileTab.DIR_FstClusHI;
// 	curClus = curClus<<16|fileTab.DIR_FstClusLO;
// 	while(k < nFileClus)
// 	{
// 		UINT reAdd = ClusterToRelatAddr(curClus);
// 		if(k*bysPerClus > nBufferLen)
// 		{
// 			SetDiskFilePointer(NULL, reAdd, NULL, FILE_BEGIN);
// 			ReadDiskFile(NULL, pBuffer + (k-1) * bysPerClus, nBufferLen%bysPerClus, &nRead, NULL);
// 			delete[] lpName;
// 			return TRUE;
// 		}
// 	
// 		SetDiskFilePointer(NULL, reAdd, NULL, FILE_BEGIN);
// 		if(k < nFileClus -1)
// 			ReadDiskFile(NULL, pBuffer + k * bysPerClus, bysPerClus, &nRead, NULL);
// 		else
// 			ReadDiskFile(NULL, pBuffer + k * bysPerClus, nSize%bysPerClus, &nRead, NULL);
// 		curClus = _fats.at(curClus);
// 		if(curClus == m_EOC_STATUS)
// 		{
// 			delete[] lpName;
// 			return FALSE;
// 		}
// 		k++;
// 	}	
// 
// 	delete[] lpName;
// 	return TRUE;
}


BOOL DiskImgFile::CloseFileEx(HANDLE hFile)
{
	ImgFileHandle* fh = (ImgFileHandle*)hFile;
	if(fh)
	{
		delete fh;
		fh = NULL;
	}
// 	if(hFile)
// 	{
// 		delete (ImgFileHandle*)hFile;
// 		hFile = NULL;
// 	}
	return TRUE;
}


BOOL DiskImgFile::ReadFileEx(HANDLE hFile,   
							   OUT PBYTE pBuffer, 
							   IN DWORD nBufferLen, 
							   OUT PDWORD nImgRead) 
{

	*nImgRead = 0;
	ImgFileHandle* fh = (ImgFileHandle*)hFile;

	DWORD fileSize = fh->_fileTab.DIR_FileSize;
	UINT currentPosition = fh->_curRetpos;
	if (currentPosition + nBufferLen > fileSize)
	{
		nBufferLen = fileSize - currentPosition;
	}

	if((fh->_accMode != GENERIC_READ) && (fh->_accMode != (GENERIC_READ|GENERIC_WRITE)))
	{
		printmsg("Can not read this file!");
		return FALSE;
	}
	DWORD nRead;
	
	UINT bysPerClus = _imgBpb.BPB_BytsPerSec * _imgBpb.BPB_SecPerClus;
	// 先读第一块，不够一簇的那快
	UINT nFirstSize = bysPerClus - fh->_curRetpos % bysPerClus;
	UINT nJClus = fh->_curPos;
	SetDiskFilePointer(NULL, fh->_curPos, NULL, FILE_BEGIN);
	if(nBufferLen <= nFirstSize) // 不用跨簇读
	{
		fh->_curRetpos = fh->_curRetpos + nBufferLen;
		fh->_curPos = fh->_curPos + nBufferLen;
		ReadDiskFile(NULL, pBuffer, nBufferLen, &nRead, NULL);
		*nImgRead = nRead;
		return TRUE;
	}
	ReadDiskFile(NULL, pBuffer, nFirstSize, &nRead, NULL); // 先读第一块，不够一簇的那快
	*nImgRead = *nImgRead + nRead;

	int k = 0;
	UINT tempValue = 0xffff;
	UINT leftSize = nBufferLen - nFirstSize;
	UINT nFileClus = (leftSize +  bysPerClus - 1) / bysPerClus;
	UINT curClus = RelatAddrToCluster(fh->_curPos);
	curClus = _fats.at(curClus);
	if (curClus == m_EOC_STATUS) return FALSE;
	UINT retAddr = ClusterToRelatAddr(curClus);
	DWORD nw;

	while(k < nFileClus)
	{
		SetDiskFilePointer(NULL, retAddr, NULL, FILE_BEGIN);
		if((k + 1)*bysPerClus > leftSize)
		{
			ReadDiskFile(NULL, pBuffer + nFirstSize + k * bysPerClus, leftSize%bysPerClus, &nRead, NULL);
			fh->_curRetpos = fh->_curRetpos + nBufferLen;
			fh->_curPos = retAddr + leftSize%bysPerClus;
			*nImgRead = *nImgRead + nRead;
			Fat_DirectoryRW dirRw;
			::GetSystemTime(&fh->_fileTab.DIR_LstAcceDate);
			//fh->_fileTab.DIR_LstAcceDate = CTime::GetCurrentTime();
			RwInfoFromDirInfo(dirRw, fh->_fileTab);
			SetDiskFilePointer(NULL, fh->_tabStAddr, NULL, FILE_BEGIN);
			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nw, NULL))
			{
				printmsg("Error in WriteFileEx() Function");
				return FALSE;
			}
			return TRUE;
		}		
		
		ReadDiskFile(NULL, pBuffer + nFirstSize + k * bysPerClus, bysPerClus, &nRead, NULL);
		*nImgRead = *nImgRead + nRead;

		if(curClus == m_EOC_STATUS || curClus == FREE_STATUS)
			return FALSE;

		curClus = _fats.at(curClus);

 		//char* p = (char*)(pBuffer + nFirstSize + k * bysPerClus);		
 		//TRACE("%d  , %s\n" , curClus , p);


		fh->_curPos = fh->_curPos + bysPerClus;
		//if(curClus == m_EOC_STATUS)
		//	return FALSE;
		retAddr = ClusterToRelatAddr(curClus);
		k++;
	}	
	fh->_curPos = retAddr + leftSize%bysPerClus;
	fh->_curRetpos = fh->_curRetpos + nBufferLen;
	Fat_DirectoryRW dirRw;

	::GetSystemTime(&fh->_fileTab.DIR_LstAcceDate);
	//fh->_fileTab.DIR_LstAcceDate = CTime::GetCurrentTime();
	RwInfoFromDirInfo(dirRw, fh->_fileTab);
	SetDiskFilePointer(NULL, fh->_tabStAddr, NULL, FILE_BEGIN);
	if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nw, NULL))
	{
		printmsg("Error in WriteFileEx() Function");
		return FALSE;
	}

	return TRUE;
}

BOOL	DiskImgFile::IniWrite(HANDLE hFile , DWORD dwWriteLen)
{
	ImgFileHandle* fh = (ImgFileHandle*)hFile;
	
	UINT maxSizePerClus = _imgBpb.BPB_BytsPerSec*_imgBpb.BPB_SecPerClus;
	
	UINT firstClu = fh->_fileTab.DIR_FstClusLO;

	
	if ( firstClu == 0)
	{
		UINT fstClus = GetFirstFreeClusNum();	
		SetClus(fstClus, m_EOC_STATUS);
		
		fh->_fileTab.DIR_FstClusLO = fstClus;
		fh->_fileTab.DIR_FstClusHI = 0;
		
		fh->_stAddr = ClusterToRelatAddr(fstClus);
		fh->_curPos =fh->_stAddr;
		
		firstClu = fstClus;
		//fh->_fileTab.DIR_FileSize = maxSizePerClus;
	}	
	
	DWORD needspace = fh->_curRetpos + dwWriteLen;
	DWORD needclus = needspace / maxSizePerClus;
	UINT	resspace = needspace % maxSizePerClus;
	if (resspace!=0) needclus++;
	
	UINT lastclu = firstClu;	
	
	for (int i=0;i<needclus-1 ;i++)
	{
		//TRACE("%d\n" , _fats.at(lastclu));
		if(_fats.at(lastclu) == m_EOC_STATUS)
		{
			UINT freeclus = GetFirstFreeClusNum();
#ifndef DEV_IMG
			InitializeClus(freeclus);
#endif
			SetClus(lastclu , freeclus);				
			SetClus(freeclus , m_EOC_STATUS);
		}
		
		lastclu = _fats.at(lastclu); 
	}		
	
	if (fh->_curRetpos > fh->_fileTab.DIR_FileSize)
		fh->_fileTab.DIR_FileSize = fh->_curRetpos ;
	
	
	//calc write pos
	UINT curclus = fh->_curRetpos / maxSizePerClus;
	UINT curbyte = fh->_curRetpos % maxSizePerClus;
	lastclu = firstClu;
	for (int j=0;j<curclus;j++)
	{
		if(_fats.at(lastclu) == m_EOC_STATUS)
		{
			return FALSE;//
		}
		
		lastclu = _fats.at(lastclu); 
	}
	
	fh->_curPos = ClusterToRelatAddr(lastclu) + curbyte;
	
	return TRUE;
}

// BOOL  DiskImgFile::CalcNewPos(HANDLE hFile)
// {
// 	ImgFileHandle* fh = (ImgFileHandle*)hFile;
// 	
// 	int exceedsize = fh->_exceedsize;
// 	
// 	if ( exceedsize > 0)
// 	{
// 		UINT maxSizePerClus = _imgBpb.BPB_BytsPerSec*_imgBpb.BPB_SecPerClus;
// 		int curfilesize = fh->_fileTab.DIR_FileSize;
// 		int retpos = fh->_curRetpos;
// 		
// 		UINT firstClu =	fh->_fileTab.DIR_FstClusLO; 
// 		
// 		UINT lastclu = firstClu;		
// 		
// 		while(1)
// 		{
// 			if(_fats.at(lastclu) == m_EOC_STATUS)
// 			{
// 				break;
// 			}
// 			lastclu = _fats.at(lastclu); 
// 		}
// 		
// 		UINT resspace = maxSizePerClus - fh->_fileTab.DIR_FileSize % maxSizePerClus;
// 		
// 		if (resspace > exceedsize)
// 		{
// 			UINT res = fh->_curRetpos % maxSizePerClus;
// 			fh->_curPos = ClusterToRelatAddr(lastclu) + res;			
// 			fh->_fileTab.DIR_FileSize += exceedsize;
// 			fh->_exceedsize = 0;
// 		}
// 		else
// 		{
// 			exceedsize -= resspace;
// 			UINT needClus = exceedsize /  maxSizePerClus;
// 			if (exceedsize % maxSizePerClus != 0) needClus += 1;
// 			
// 			UINT res = exceedsize % maxSizePerClus;			
// 			
// 			SetClusEx(lastclu , needClus);
// 			
// 			while(1)
// 			{
// 				if(_fats.at(lastclu) == m_EOC_STATUS)
// 				{
// 					break;
// 				}
// 				lastclu = _fats.at(lastclu); 
// 			}
// 			
// 			fh->_curPos = ClusterToRelatAddr(lastclu) + res;
// 			fh->_fileTab.DIR_FileSize += exceedsize + resspace;
// 			fh->_exceedsize = 0;
// 		}		
// 	}
// 	
// 	
// 	return TRUE;
// }

BOOL DiskImgFile::WriteFileEx(HANDLE hFile ,
								IN PBYTE pBuffer, 
								IN DWORD nBufferLen,
								OUT PDWORD nWrite) 
{

	if (nBufferLen == 0)
	{
		nWrite = 0;
		return TRUE;
	}

	ImgFileHandle* fh = (ImgFileHandle*)hFile;
	if((fh->_accMode != GENERIC_WRITE) && (fh->_accMode != (GENERIC_READ|GENERIC_WRITE)))
	{
		printmsg("Can not Write this file!");
		return FALSE;
	}
	
	if (nBufferLen == 0) return TRUE;
	
	if (!IniWrite(hFile , nBufferLen)) return FALSE;

	//*****	for making img to device
	HANDLE devHandle =_hFile;
#ifdef DEV_IMG
	devHandle = (HANDLE)-2;
#endif
	//****************
	
	DWORD nw = 0;
	UINT nWriteLen = (UINT)nBufferLen;	
	
	//先写第一块，不够一簇的那快
	UINT bysPerClus = _imgBpb.BPB_BytsPerSec*_imgBpb.BPB_SecPerClus;
	UINT nCurClu = RelatAddrToCluster(fh->_curPos);
	UINT nFirstLeft = bysPerClus - fh->_curRetpos%bysPerClus;

	SetDiskFilePointer(devHandle, fh->_curPos, NULL, FILE_BEGIN);
	if(nWriteLen < nFirstLeft)
	{
		if (!WriteDiskFile(devHandle, pBuffer, nWriteLen, nWrite, NULL))
		{
			return FALSE;
		}
		
		fh->_curPos = fh->_curPos + nWriteLen;
		fh->_curRetpos = fh->_curRetpos + nWriteLen;
		if(fh->_curRetpos >= fh->_fileTab.DIR_FileSize)
			fh->_fileTab.DIR_FileSize = fh->_curRetpos;
		::GetSystemTime(&fh->_fileTab.DIR_WrtDateTime);
//		fh->_fileTab.DIR_WrtDateTime = CTime::GetCurrentTime();
		Fat_DirectoryRW dirRw;
		RwInfoFromDirInfo(dirRw, fh->_fileTab);
		SetDiskFilePointer(_hFile, fh->_tabStAddr, NULL, FILE_BEGIN);
		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nw, NULL))
		{
			printmsg("Error in WriteFileEx() Function");
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		DWORD nw;
		SetDiskFilePointer(devHandle, fh->_curPos, NULL, FILE_BEGIN);
		if (!WriteDiskFile(devHandle, pBuffer, nFirstLeft, &nw, NULL))
		{
			return FALSE;
		}
		
		*nWrite = *nWrite + nw;
	}

	nCurClu = _fats.at(nCurClu);
	UINT retAddr;

	if(nCurClu!=m_EOC_STATUS)
	{
		retAddr	= ClusterToRelatAddr(nCurClu);
		
		SetDiskFilePointer(devHandle, retAddr, NULL, FILE_BEGIN);
		
		int k = 0;
		UINT nLeftNeedWriteSize = nBufferLen - nFirstLeft; // 剩余要写的字节大小
		
		
		while (nLeftNeedWriteSize > k*bysPerClus)//
		{
			if(nLeftNeedWriteSize >= (k + 1)*bysPerClus)
			{
				if (!WriteDiskFile(devHandle, pBuffer + nFirstLeft + k * bysPerClus, bysPerClus, &nw, NULL))
				{
					return FALSE;
				}				
				*nWrite = *nWrite + nw;
			}
			else
			{
				if (!WriteDiskFile(devHandle, pBuffer + nFirstLeft + k * bysPerClus, nLeftNeedWriteSize%bysPerClus, &nw, NULL))
				{
					return FALSE;
				}				
				*nWrite = *nWrite + nw;
				break;
			}
			nCurClu = _fats.at(nCurClu);	
			
			
			if(nCurClu == m_EOC_STATUS)
			{
				//retAddr = ClusterToRelatAddr(GetFirstFreeClusNum());//Add by Joelee 2007.10.12
				break;
			}
			
			retAddr = ClusterToRelatAddr(nCurClu);
			SetDiskFilePointer(devHandle, retAddr, NULL, FILE_BEGIN);
			k++;
		}

		fh->_curPos = retAddr + nLeftNeedWriteSize%bysPerClus;
	}	

	fh->_curRetpos = fh->_curRetpos + nBufferLen;
	
	if(fh->_curRetpos >= fh->_fileTab.DIR_FileSize)
			fh->_fileTab.DIR_FileSize = fh->_curRetpos;
	::GetSystemTime(&fh->_fileTab.DIR_WrtDateTime);
//	fh->_fileTab.DIR_WrtDateTime = CTime::GetCurrentTime();
	Fat_DirectoryRW dirRw;
	RwInfoFromDirInfo(dirRw, fh->_fileTab);
	SetDiskFilePointer(_hFile, fh->_tabStAddr, NULL, FILE_BEGIN);
	if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nw, NULL))
	{
		printmsg("Error in WriteFileEx() Function");
		return FALSE;
	}
	return TRUE;
}

BOOL  DiskImgFile::RenameFileEx(LPCSTR lpSrcName, LPCSTR lpNewFileName)
{
	return TRUE;
// 	//////////////////////////////////////////////////////////////////////////
// 	// 获取当前的
// 
// 
// 	if(lpSrcName == NULL)
// 		return FALSE;
// /*
// 	LPSTR lpCurDir = GetLeftStr(lpSrcName, '\\', FALSE);
// 
// 	LPSTR lpName = new char[MAX_PATH];
// 	memset(lpName, 0x00, MAX_PATH);
// 		
// 	if(!GetRighStrByFind(lpName, lpSrcName, '\\', -1, ""))
// 	{
// 		delete[] lpName;
// 		return FALSE;
// 	}
// 
// 	if(lpCurDir == NULL)
// 	{
// 		if(!SetCurrentDirectory("\\"))
// 			return FALSE;
// 	}
// 	else 
// 	{
// 		if(!SetCurrentDirectory(lpCurDir))
// 		{	
// 			delete[] lpName;
// 			return FALSE;
// 		}
// 	}
// 	*/
// 	//////////////////////////////////////////////////////////////////////////
// 	// 删除原来的FAT
// 
// 	int ns = _curDirectory.size();
// 	DWORD nWrite;
// 	
// 	LPSTR lpLongName = new char[MAX_PATH];
// 	
// 	int nIndex = 0;
// 	Fat_Directory dir;
// 	while(1)
// 	{
// 		memset(lpLongName, 0x00, MAX_PATH);
// 		if(!GetDirectoryTabEx(dir, lpLongName, nIndex))
// 		{
// 			delete[] lpLongName;
// 			return FALSE;
// 		}
// 		if(strcmp(lpLongName, lpSrcName) == 0)
// 		{
// 			break;
// 		}
// 	}
// 
// 	int nNeed = CalcNeedTabCount(lpSrcName);
// 	
// 	if(ns == 0 )	// 在根目录下删除文件
// 	{
// 		BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 		for(int i = 0; i < nNeed; i++)
// 		{
// 			SetDiskFilePointer(NULL, _stOfRootAddr + (nIndex - i - 1) * 32, NULL, FILE_BEGIN);
// 			WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
// 		}		
// 	}	
// 	else			// 在子目录下删除文件
// 	{
// 
// 		Fat_Directory& parDir = _curDirectory.at(ns - 1);
// 		UINT retAddr = ClusterToRelatAddr(parDir.DIR_FstClusLO);
// 		BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 		int nPos = dir.DIR_PosIndex;
// 		WORD curC = parDir.DIR_FstClusLO;
// 
// 		int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
// 		int nCluIndex = nPos /recPerCluster;  // 记录在该目录的第 几个簇下操作
// 		UINT retIndex = nPos%recPerCluster;
// 
// 		int i;
// 		for( i = 0; i < nCluIndex; i++)
// 		{
// 			curC = _fats.at(curC);
// 			if (curC == m_EOC_STATUS) return FALSE;
// 			
// 			retAddr = ClusterToRelatAddr(curC);
// 		}
// 
// 		if(retIndex < (nNeed -1))
// 		{
// 			retIndex = recPerCluster - (nNeed - 1 - retIndex);
// 			nCluIndex--;
// 			for(int i = 0; i < nCluIndex; i++)
// 			{
// 				curC = _fats.at(curC);
// 				if (curC == m_EOC_STATUS) return FALSE;
// 				retAddr = ClusterToRelatAddr(curC);
// 			}
// 		}
// 
// 		retIndex = retIndex - (nNeed - 1); 
// 		
// 		for(i = 0; i < nNeed; i++)
// 		{
// 			SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 			WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
// 			retIndex++;
// 			if(retIndex >= recPerCluster)
// 			{
// 				curC = _fats.at(curC);
// 				retAddr = ClusterToRelatAddr(curC);
// 				retIndex = 0;
// 			}
// 		}
// 	}
// 	delete[] lpLongName;
// 
// 	//////////////////////////////////////////////////////////////////////////
// 	// 建立新的FAT
// 
// /*	LPSTR lpNewDir = GetLeftStr(lpNewFileName, '\\', FALSE);
// 
// 	if(!SetCurrentDirectory(lpNewDir))
// 	{
// 		this->CreateDirectoryEx(lpNewDir);
// 		this->SetCurrentDirectory(lpNewDir);
// 	}
// 
// 	memset(lpName, 0x00, MAX_PATH);
// 		
// 	if(!GetRighStrByFind(lpName, lpNewFileName, '\\', -1, ""))
// 	{
// 		delete[] lpName;
// 		return FALSE;
// 	}
// */
// 
// 	UINT len = strlen(lpNewFileName);
// 	nNeed = this->CalcNeedTabCount(lpNewFileName); // 需要存放的结构的个数
// 	LPWSTR pLongName = new WCHAR[len + 1];
// 	memset(pLongName, 0x00, len * 2);
// 	int nl = ::MultiByteToWideChar(m_CodePage, 0, lpNewFileName, len, pLongName, len+1);
// 
// 	LPSTR pName = new char[MAX_PATH];
// 	memset(pName, 0x00, MAX_PATH);
// 	if(!GenerateShortName(lpNewFileName, pName))
// 	{
// 		delete[] pName;
// 		return FALSE;
// 	}
// 	
// 
// 	DWORD nRead;
// 
// 	Fat_DirectoryRW dirRw;
// 
// 	memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
// //	memset(&dir, 0x00, sizeof(Fat_Directory));
// 
// 	UINT fstClus = 0;
// 	ns = _curDirectory.size();
// 	
// 	BYTE chkFlag = ChkSum(pName);	
// 	if(ns == 0) // 在根目录下创建文件
// 	{
// 		INT nIndex = -1;
// 		int i;
// 		//////////////////////////////////////////////////////////////////////////
// 		// 开始建立目录项
// 		SetDiskFilePointer(NULL, _stOfRootAddr, NULL, (DWORD)FILE_BEGIN);
// 		for( i = 0; i < ROOTENTCNT; i++)
// 		{			
// 			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
// 			if(nRead != sizeof(Fat_DirectoryRW))
// 			{
// 				delete[] pName;
// 				printmsg("Error In Create RootFile --Read DirRW Error!");
// 				return FALSE;
// 			}
// 			if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed ==1))    // 判断是否为空项
// 			{
// 				nIndex = i;
// 				break;
// 			}
// 		}
// 		if(nIndex == -1)
// 		{
// 			delete[] pName;
// 			printmsg("This Directroy is full!");
// 			return FALSE;
// 		}		
// 	
// 		//////////////////////////////////////////////////////////////////////////
// 		// 处理长名目录项目
// 		if(nNeed > 1)
// 		{
// 			Fat_LongDirectory longDir;
// 			memset(&longDir, 0x00, 32);
// 			longDir.LDIR_Attr = ATTR_LONG_NAME;
// 			longDir.LDIR_Chksum = chkFlag;
// 			longDir.LDIR_FstClusLO = 0;
// 			longDir.LDIR_Ord = LAST_LONG_ENTRY|(BYTE)(nNeed - 1);
// 			longDir.LDIR_Type = 0;
// 		//	int nl = strlen(lpFileName);
// 			int npos = (nNeed - 2) * 13;
// 			memset(longDir.LDIR_Name1, 0xff, 26);
// 			memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
// 			if((nl - npos) < 13)
// 				memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
// 			RwInfoFromLongDirInfo(dirRw, longDir);
// 			SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
// 			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 			{
// 				delete[] pName;
// 				printmsg("Error In create RootDirectory -- write dir Struct");
// 				return FALSE;
// 			}
// 			nIndex ++;	
// 		}
// 		
// 		for( i = nNeed - 2; i >0 ; i--)
// 		{
// 			Fat_LongDirectory longDir;
// 			memset(&longDir, 0x00, 32);
// 			longDir.LDIR_Attr = ATTR_LONG_NAME;
// 			longDir.LDIR_Chksum = chkFlag;
// 			longDir.LDIR_FstClusLO = 0;
// 			longDir.LDIR_Ord = (BYTE)i;
// 			longDir.LDIR_Type = 0;
// 			int npos = (i - 1) * 13;
// 			memset(longDir.LDIR_Name1, 0xff, 26);
// 			memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
// 			RwInfoFromLongDirInfo(dirRw, longDir);
// 		//	SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
// 			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 			{
// 				delete[] pName;
// 				printmsg("Error In create RootDirectory -- write dir Struct");
// 				return FALSE;
// 			}
// 			nIndex ++;	
// 		}
// 		//////////////////////////////////////////////////////////////////////////
// 
// 		memcpy(&dir.DIR_Name, pName, 11);
// 		::GetSystemTime(&dir.DIR_WrtDateTime);
// 		::GetSystemTime(&dir.DIR_LstAcceDate);
// 
// 		RwInfoFromDirInfo(dirRw, dir);
// 		SetDiskFilePointer(NULL, _stOfRootAddr + 32 * nIndex, NULL, FILE_BEGIN);
// 		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 		{
// 			delete[] pName;
// 			printmsg("Error In create RootFile -- write dir Struct");
// 			return FALSE;
// 		}
// 		//////////////////////////////////////////////////////////////////////////		
// 			
// 	}
// 	else		// 在其他目录下建立文件
// 	{
// 		Fat_Directory curD = _curDirectory.at(_curDirectory.size() - 1);
// 
// 		UINT dirAdd = this->ClusterToRelatAddr(curD.DIR_FstClusLO);
// 
// 		INT nIndex = -1;
// 		//////////////////////////////////////////////////////////////////////////
// 		// 开始建立目录项
// 
// 		SetDiskFilePointer(NULL, dirAdd, NULL, (DWORD)FILE_BEGIN);
// 		UINT maxDircnt = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;
// 		WORD cClus = curD.DIR_FstClusLO;
// 		int  nDirPos = 0; // 记录目录的相对位置 放置在Fat_Directory.DIR_PosIndex中
// 		while(1)
// 		{
// 			for(int i = 0; i < maxDircnt; i++)
// 			{	
// 				ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
// 				if(nRead != sizeof(Fat_DirectoryRW))
// 				{
// 					delete[] pLongName;
// 					delete[] pName;
// 					printmsg("Error In Create SubFile --Read DirRW Error!");
// 					return FALSE;
// 				}
// 				
// 				if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed == 1))   // 判断是否为空项
// 				{
// 					nIndex = i;
// 					break;
// 				}	
// 				nDirPos++;
// 			}
// 			if(nIndex != -1)
// 				break;
// 			cClus = _fats.at(cClus);
// 			if(cClus == m_EOC_STATUS)
// 				break;
// 			dirAdd = ClusterToRelatAddr(cClus);
// 			SetDiskFilePointer(NULL, dirAdd, NULL, (DWORD)FILE_BEGIN);
// 		}
// 		if(nIndex == -1)
// 		{
// 			UINT c = GetFirstFreeClusNum();
// 			SetClus(cClus, c);
// 			SetClus(c, m_EOC_STATUS);
// 			InitializeClus(c);
// 			nIndex = nDirPos;
// 			dirAdd = ClusterToRelatAddr(c);
// 		}	
// 	
// 		//////////////////////////////////////////////////////////////////////////
// 		// 处理长名目录项目
// 		int retIndex = nIndex%maxDircnt;
// 		if(nNeed > 1)
// 		{
// 			Fat_LongDirectory longDir;
// 			memset(&longDir, 0x00, 32);
// 			longDir.LDIR_Attr = ATTR_LONG_NAME;
// 			longDir.LDIR_Chksum = chkFlag;
// 			longDir.LDIR_FstClusLO = 0;
// 			longDir.LDIR_Ord = LAST_LONG_ENTRY|(BYTE)(nNeed-1);
// 			longDir.LDIR_Type = 0;
// 			int npos = (nNeed - 2) * 13;
// 			memset(longDir.LDIR_Name1, 0xff, 26);
// 			memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
// 			if((nl - npos) < 13)
// 				memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
// 			RwInfoFromLongDirInfo(dirRw, longDir);
// 			SetDiskFilePointer(NULL, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 			{
// 				delete[] pLongName;
// 				delete[] pName;
// 				printmsg("Error In create RootDirectory -- write dir Struct");
// 				return FALSE;
// 			}
// 			retIndex++;
// 			nIndex ++;
// 			if(retIndex >= maxDircnt)
// 			{
// 				UINT c = GetFirstFreeClusNum();
// 				SetClus(cClus, c);
// 				SetClus(c, m_EOC_STATUS);
// 				InitializeClus(c);
// 				retIndex = 0;
// 				dirAdd = ClusterToRelatAddr(c);
// 			}
// 		}
// 		for(int i = nNeed - 2; i >0 ; i--)
// 		{
// 			Fat_LongDirectory longDir;
// 			memset(&longDir, 0x00, 32);
// 			longDir.LDIR_Attr = ATTR_LONG_NAME;
// 			longDir.LDIR_Chksum = chkFlag;
// 			longDir.LDIR_FstClusLO = 0;
// 			longDir.LDIR_Ord = (BYTE)i;
// 			longDir.LDIR_Type = 0;
// 			int nl = strlen(lpNewFileName);
// 			int npos = (i - 1) * 13;
// 			memset(longDir.LDIR_Name1, 0xff, 26);
// 			memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
// 			RwInfoFromLongDirInfo(dirRw, longDir);
// 			SetDiskFilePointer(NULL, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 			{
// 				delete[] pLongName;
// 				delete[] pName;
// 				printmsg("Error In create RootDirectory -- write dir Struct");
// 				return FALSE;
// 			}
// 			
// 			nIndex ++;	
// 			retIndex++;
// 			if(retIndex >= maxDircnt)
// 			{
// 				UINT c = GetFirstFreeClusNum();
// 				SetClus(cClus, c);
// 				SetClus(c, m_EOC_STATUS);
// 				InitializeClus(c);
// 				retIndex = 0;
// 				dirAdd = ClusterToRelatAddr(c);
// 			}
// 		}
// 		//////////////////////////////////////////////////////////////////////////
// 			
// 		memcpy(&dir.DIR_Name, pName, 11);
// 		::GetSystemTime(&dir.DIR_WrtDateTime);
// 		::GetSystemTime(&dir.DIR_LstAcceDate);
// 			
// 		RwInfoFromDirInfo(dirRw, dir);
// 		SetDiskFilePointer(NULL, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 		{
// 			delete[] pName;
// 			printmsg("Error In create RootFile -- write dir Struct");
// 			return FALSE;
// 		}
// 	}
// 	delete[] pLongName;
// 	delete[] pName;
// 	return TRUE;
}

HANDLE DiskImgFile::CreateFileEx(LPCTSTR lptFileName,     
									DWORD dwDesiredAccess,      
									DWORD dwShareMode,          
									LPSECURITY_ATTRIBUTES lpSecurityAttributes,	
									DWORD dwCreationDisposition, 
									DWORD dwFlagsAndAttributes,  
									HANDLE hTemplateFile         
									)
{

	if(lptFileName == NULL)
			return (HANDLE)-1;

	LPTSTR lpCurDir = GetLeftStr(lptFileName, '\\', FALSE);

	LPTSTR lpName = new TCHAR[MAX_PATH];
	memset(lpName, 0x00, MAX_PATH * sizeof(TCHAR));
		
	if(!GetRighStrByFind(lpName, lptFileName, '\\', -1, ""))
	{
		delete[] lpName;
		return (HANDLE)-1;
	}


	if(lpCurDir == NULL)
	{
		this->SetCurrentDirectory("\\");
		HANDLE handle;
		if (!this->ImgCreateFile(lpName, NULL, dwFlagsAndAttributes, 0, handle))
		{
			delete[] lpName;
			return (HANDLE)-1;
		}
		
		ImgFileHandle* fh = (ImgFileHandle*)handle;
		if(fh)
		{
			fh->_shareMode = dwShareMode;
			fh->_accMode = dwDesiredAccess;
		}
		delete[] lpName;
		return handle;
	}


	if(!SetCurrentDirectory(lpCurDir))
	{
		if(dwCreationDisposition == OPEN_EXISTING)
		{
			//printmsg("No such file exite!");
			delete[] lpName;
			return (HANDLE)-1;
		}
		this->CreateDirectoryEx(lpCurDir);
		this->SetCurrentDirectory(lpCurDir);
		HANDLE handle;
		if (!this->ImgCreateFile(lpName, NULL, dwFlagsAndAttributes, 0, handle))
		{
			delete[] lpName;
			return (HANDLE)-1;
		}
		
		ImgFileHandle* fh = (ImgFileHandle*)handle;
		if(fh)
		{
			fh->_shareMode = dwShareMode;
			fh->_accMode = dwDesiredAccess;
		}
		delete[] lpName;
		return handle;
	}



	
	int ns = _curDirectory.size();
	//Fat_Directory& tem = _curDirectory.at(ns - 1);
	UINT ncu = 0;//tem.DIR_FstClusLO;
	UINT retAddr = 0;
	if(ns == 0)
	{
		retAddr = _stOfRootAddr;
	}
	else
	{
		Fat_Directory& tem = _curDirectory.at(ns - 1);
		ncu = tem.DIR_FstClusHI;
		ncu = ncu<<16|tem.DIR_FstClusLO;
		retAddr = ClusterToRelatAddr(ncu);
	}

	int nIndex = 0;
	LPTSTR lpTemp = new TCHAR[MAX_PATH];
	Fat_Directory dir;
	while(1)
	{
		memset(lpTemp, 0x00, MAX_PATH * sizeof(TCHAR));

		BOOL ret = GetDirectoryTabEx(dir, lpTemp, nIndex);
		
		if(!ret)
		{
			if(dwCreationDisposition == OPEN_EXISTING)
			{
				delete[] lpTemp;
				delete[] lpName;
				delete lpCurDir;
				//printmsg("No such file exite!");
				return (HANDLE)-1;
			}

			HANDLE handle;

			if (this->ImgCreateFile(lpName, NULL, dwFlagsAndAttributes, 0, handle))
			{
				ImgFileHandle* fh = (ImgFileHandle*)handle;
				if(fh)
				{
					fh->_shareMode = dwShareMode;
					fh->_accMode = dwDesiredAccess;
				}
			}			
			else handle = (HANDLE)-1;

			delete[] lpName;
			delete[] lpTemp;
			delete lpCurDir;
			return handle;
		}

		if(strcmpnocase(lpTemp, lpName) == 0)
			break;
	}


	int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;
	int nCluIndex = dir.DIR_PosIndex / recPerCluster;
	if (ns == 0) recPerCluster = ROOTENTCNT; // if root dir , direntry is ROOTENTCNT
	

	for(int i = 0; i < nCluIndex; i++)
	{
		ncu = _fats.at(ncu);
	}
	if(ncu == 0)
	{
		retAddr = _stOfRootAddr + dir.DIR_PosIndex %recPerCluster * 32;
	}
	else
		retAddr = ClusterToRelatAddr(ncu) + dir.DIR_PosIndex %recPerCluster * 32;

	ImgFileHandle* fh = new ImgFileHandle;
	fh->_exceedsize = 0;
	fh->_curRetpos = 0;
	UINT nc = dir.DIR_FstClusHI;
	nc = nc<<16|dir.DIR_FstClusLO;
	fh->_stAddr = ClusterToRelatAddr(nc);
	fh->_curPos = fh->_stAddr;
	memcpy(&fh->_fileTab, &dir, sizeof(dir));
	fh->_tabStAddr = retAddr;
	fh->_shareMode = dwShareMode;
	fh->_accMode = dwDesiredAccess;
	delete[] lpName;
	delete[] lpTemp;
	delete lpCurDir;
	return (HANDLE)fh;
}

DWORD DiskImgFile::SetFilePointerEx(HANDLE hFile,  
								 LONG lDistanceToMove,  
								 PLONG lpDistanceToMoveHigh,  
								 DWORD dwMoveMethod )
{
	ImgFileHandle* fh = (ImgFileHandle*)hFile;
	int method = (int)dwMoveMethod;
	UINT firstClu = fh->_fileTab.DIR_FstClusLO;
	UINT maxSizePerClus = _imgBpb.BPB_BytsPerSec*_imgBpb.BPB_SecPerClus;
	int nClusC = lDistanceToMove /maxSizePerClus; // 计算得到跨越簇的数量

	switch(method)
	{
	case FILE_BEGIN:
		{
			for(int i = 0; i <nClusC; i++)
			{
				firstClu = _fats.at(firstClu);
			}
 			fh->_curRetpos = lDistanceToMove;
 			fh->_curPos = ClusterToRelatAddr(firstClu) + lDistanceToMove%maxSizePerClus;
		}
		break;
	case FILE_CURRENT:
		{
// 			nClusC = (fh->_curRetpos + lDistanceToMove) /maxSizePerClus; // 计算得到跨越簇的数量
// 			for(int i = 0; i <nClusC; i++)
// 			{
// 				firstClu = _fats.at(firstClu);
// 			}
// 			fh->_curPos = ClusterToRelatAddr(firstClu) + (fh->_curRetpos + lDistanceToMove)%maxSizePerClus;			
			fh->_curRetpos = lDistanceToMove + fh->_curRetpos;
		}
		break;
	case FILE_END:
		{
// 			while(_fats.at(firstClu) != m_EOC_STATUS)
// 			{
// 				firstClu = _fats.at(firstClu);
// 			}
// 			fh->_curPos = ClusterToRelatAddr(firstClu) + (fh->_fileTab.DIR_FileSize)%maxSizePerClus;			
			fh->_curRetpos = fh->_fileTab.DIR_FileSize;
		}
		break;
	default:
		;
	}

// 	if (fh->_curRetpos > fh->_fileTab.DIR_FileSize)
// 	{		 
// 		fh->_exceedsize = fh->_curRetpos - fh->_fileTab.DIR_FileSize;
// 	}
// 	else
// 	{
// 		fh->_exceedsize = 0 ;
// 		int curClus = fh->_curRetpos / maxSizePerClus;
// 		int res = fh->_curRetpos % maxSizePerClus;
// 		for (int i=0;i<curClus;i++)
// 		{
// 			firstClu = _fats.at(firstClu);
// 		}
// 		
// 		fh->_curPos = ClusterToRelatAddr(firstClu) + res;
// 	}
	
	return fh->_curRetpos;
	
	
}

BOOL  DiskImgFile::CreateImageFile(IN LPCSTR lpszFileName, IN UINT fatType, IN LONGLONG diskSize)					// 创建镜像文件
{
#ifdef _WINDOWS
	ASSERT(AfxIsValidString(lpszFileName));

	// map read/write mode
	DWORD dwAccess = 0;
	dwAccess = GENERIC_READ|GENERIC_WRITE;

	// map share mode
	DWORD dwShareMode = 0;
	dwShareMode = FILE_SHARE_WRITE|FILE_SHARE_READ;

	// Note: typeText and typeBinary are used in derived classes only.

	// map modeNoInherit flag
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	// map creation flags
	DWORD dwCreateFlag;

	dwCreateFlag = CREATE_ALWAYS;
	// attempt file creation

	HANDLE hFile = CreateFile(lpszFileName, dwAccess, dwShareMode, &sa,
		dwCreateFlag, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printmsg("Crete MirrorImageFile Error!");	
		return FALSE;
	}
	_hFile = hFile;


	Iinitialize("NO NAME ", fatType, diskSize);//FAT16_TYPE, DISK_128M);

	BootSector_BPB_RW bppRw;
	RwInfoCopyFormBpb(bppRw, _imgBpb);
		
	DWORD nCount = sizeof(bppRw);
	DWORD nWritten;

	DWORD dwNew = SetDiskFilePointer((HANDLE)_hFile, 0, NULL, (DWORD)FILE_BEGIN);
//////////////////////////////////////////////////////////////////////////
// Write First Sector	
	if (!WriteDiskFile((HANDLE)_hFile, &bppRw, nCount, &nWritten, NULL))
	{
		printmsg("Write bpb Error!");
		return FALSE;
	}

	if (nWritten != nCount)
	{
		printmsg("Write bpb Error2! 空间不够大！");
		return FALSE;
	}

	for(int i = 1; i < _imgBpb.BPB_RsvdSecCnt; i++)
	{
		BYTE resv[512];
		memset(resv,0x00, 512);
		if (!WriteDiskFile((HANDLE)_hFile, resv, 512, &nWritten, NULL))
		{
			printmsg("Write bpb Error!");
			return FALSE;
		}

	}

//
//////////////////////////////////////////////////////////////////////////

	_stOfFATAddr = _imgBpb.BPB_RsvdSecCnt*_imgBpb.BPB_BytsPerSec;

//////////////////////////////////////////////////////////////////////////
// Write FATs

	int nsFat = _fats.size();
	_fats.at(0) = 0xFFF8;
	_fats.at(1) = 0xFFFF;
	WORD* fs = new WORD[nsFat];
	for(int i = 0; i< nsFat; i++)
	{
		fs[i] = _fats.at(i);
	}
	if (!WriteDiskFile((HANDLE)_hFile, fs, nsFat*sizeof(WORD), &nWritten, NULL))
	{
		printmsg("Write FAT1 Error!");
		return FALSE;
	}

	if (!WriteDiskFile((HANDLE)_hFile, fs, nsFat*sizeof(WORD), &nWritten, NULL))
	{
		printmsg("Write FAT2 Error!");
		return FALSE;
	}
	delete[] fs;
//	delete []fs;

/*	for(int i = 0; i < nsFat; i++)
	{
		WORD f = _fats.at(i);
		
		if (!WriteDiskFile((HANDLE)_hFile, &f, 2, &nWritten, NULL))
		{
			printmsg("Write FAT1 Error!");
			break;
		}
		if(nWritten != 2)
		{
			printmsg("Write FAT1 Error2! 空间不够大！");
			break;
		}
	}

	for(i = 0; i < nsFat; i++)
	{
		WORD f = _fats.at(i);
		if (!WriteDiskFile((HANDLE)_hFile, &f, 2, &nWritten, NULL))
		{
			printmsg("Write FAT2 Error!");
			break;
		}
		if(nWritten != 2)
		{
			printmsg("Write FAT2 Error2! 空间不够大！");
			break;
		}
	}
	*/


//
//////////////////////////////////////////////////////////////////////////

	_stOfRootAddr = _stOfFATAddr + nsFat* 2 * 2;

//////////////////////////////////////////////////////////////////////////
// Write RootDirectory

	BYTE roorDir[512];
	memset(roorDir, 0x00, 512);

	Fat_Directory dir;
	Fat_DirectoryRW dirRw;
	dir.DIR_Attr = ATTR_VOLUME_ID;
	::GetSystemTime(&dir.DIR_WrtDateTime);
	::GetSystemTime(&dir.DIR_CrtDateTime);
	::GetSystemTime(&dir.DIR_LstAcceDate);
	/*
	dir.DIR_CrtDateTime = CTime::GetCurrentTime();
	dir.DIR_LstAcceDate = CTime::GetCurrentTime();
	dir.DIR_WrtDateTime = CTime::GetCurrentTime();
	*/
	dir.DIR_FileSize = 0;
	dir.DIR_FstClusHI = 0;
	dir.DIR_FstClusLO = 0;
	memcpy(dir.DIR_Name, "NO NAME    ", 11);
	RwInfoFromDirInfo(dirRw, dir);
	WriteDiskFile(_hFile, &dirRw, 32, &nWritten, NULL);

	for(int i = 1; i < 32/*;ROOTENTCNT*/; i++)
	{
		if (!WriteDiskFile((HANDLE)_hFile, roorDir, 32, &nWritten, NULL))
		{
			printmsg("Write RootDir Error!");
			return FALSE;
		}
	}
//
//////////////////////////////////////////////////////////////////////////
	
	_stOfClusterAddr = _stOfRootAddr + ROOTENTCNT * nWritten;

//////////////////////////////////////////////////////////////////////////
// Wrtite Content

/*	int nsContent = (nsFat -2) * _imgBpb.BPB_SecPerClus * BYSPERSEC;
	BYTE* content = new BYTE[nsContent];
	memset(content, 0x00, nsContent);
	if (!WriteDiskFile((HANDLE)_hFile, content, nsContent, &nWritten, NULL))
	{
		printmsg("Write Content Error!");
		return false;
	}
	delete []content;*/

//
//////////////////////////////////////////////////////////////////////////
//	::CloseHandle(_hFile);
#endif
	return TRUE;
}


void  DiskImgFile::Iinitialize(LPCSTR lpVolab, IN UINT fatType, IN LONGLONG diskSize)											// 初始化
{
/*
*	diskSize的值为 枚举DISKSIZE
	*/
	_imgSpace = DiskSizeType(diskSize);
	
	ZeroMemory(&_imgBpb, sizeof(_imgBpb));
	
	//------ fix param ---------
	_imgBpb.BPB_RsvdSecCnt = 1;	
	_imgBpb.BPB_NumFATs = 2;
	_imgBpb.BPB_RootEntCnt = ROOTENTCNT;
	//--------------

	UINT SecPerClus = GenSecPerClus(diskSize);
	UINT  totsec = diskSize/ BYSPERSEC;

	ASSERT(_imgSpace <= DISK_2G && _imgSpace >= 0);

	
// 	UINT RootDirSectors = ((ROOTENTCNT * 32) + (BYSPERSEC- 1)) / BYSPERSEC; 	
// 	UINT TmpVal1 = diskSize/BYSPERSEC - (_imgBpb.BPB_RsvdSecCnt + RootDirSectors);
// 	UINT TmpVal2 = (256 * DskTableFAT16[_imgSpace].SecPerClusVal) + /*BPB_NumFATs*/2;
// 	UINT FATSz = (TmpVal1 + (TmpVal2 -1)) / TmpVal2;

	UINT RootDirSectors = ((ROOTENTCNT * 32) + (BYSPERSEC - 1)) / BYSPERSEC;  //根目录所占字节数
	UINT TmpVal1 = totsec - (_imgBpb.BPB_RsvdSecCnt + RootDirSectors);
	UINT TmpVal2 = (256 * SecPerClus) + _imgBpb.BPB_NumFATs;
	UINT FATSz = (TmpVal1 + (TmpVal2 - 1)) / TmpVal2;
	FATSz = LOWORD(FATSz);

	//code from xiaoma
// 	UINT TempVar1=diskSize/BYSPERSEC-33+2*SecPerClus;
//     UINT TempVar2=256*SecPerClus+2;
//     UINT  FatSector=TempVar1/TempVar2;
//     if(TempVar1%TempVar2)
//         FatSector++;


#ifdef _DEBUG
	//check if BPB value is right

	UINT DataSec = totsec - (_imgBpb.BPB_RsvdSecCnt + (_imgBpb.BPB_NumFATs * FATSz) + RootDirSectors);	
	UINT CountofClusters = DataSec / SecPerClus;

	//TRACE("CountofClusters : %d\n" , CountofClusters);
	if (fatType == FAT12_TYPE)
	{
		ASSERT( CountofClusters < 4085 && CountofClusters >= 0);
	}
	else
		ASSERT( CountofClusters >= 4085 && CountofClusters < 65525);
	
// 	if(CountofClusters < 4085)
// 	{
// 		/* Volume is FAT12 */
// 	} else if(CountofClusters < 65525) 
// 	{
// 		/* Volume is FAT16 */
// 	} else {
// 		/* Volume is FAT32 */
//	}
#endif
	
	
	//////////////////////////////////////////////////////////////////////////
	//Initilize bpb Begin 
	
	_imgBpb.BS_jmpBoot[0] = 0xEB;		
	_imgBpb.BS_jmpBoot[1] = 0x3C;
	_imgBpb.BS_jmpBoot[2] = 0x90;		//待定
	
	memcpy(_imgBpb.BS_OEMName, "MSDOS5.0", 8);		
	
	_imgBpb.BPB_BytsPerSec = BYSPERSEC;	
	_imgBpb.BPB_SecPerClus = SecPerClus;		

	
	if(totsec >= 0x10000)
	{
		_imgBpb.BPB_TotSec32 = totsec;
		_imgBpb.BPB_TotSec16 = 0;
	}
	else		
	{
		_imgBpb.BPB_TotSec16 = totsec;//diskSize/ BYSPERSEC;//DskTableFAT16[diskSize].DiskSize;		
		_imgBpb.BPB_TotSec32 = 0;
	}
	
	_imgBpb.BPB_Media = 0xF8;			
	_imgBpb.BPB_FATSz16 = FATSz;		
	
	//////////////////////////////////////////////////////////////////////////
	// Wanted Modified
	_imgBpb.BPB_SecPerTrk = 0;		
	_imgBpb.BPB_NumHeads = 0;		
	//////////////////////////////////////////////////////////////////////////
	
	_imgBpb.BPB_HiddSec = 0;		
	//	_imgBpb.BPB_TotSec32 = 0;	
	_imgBpb.BS_DrvNum = 0x80;		
	_imgBpb.BS_Reserved1 = 0;	
	_imgBpb.BS_BootSig = 0x29;		
	_imgBpb.BS_VolID = 111;		//  随机生成
	int nLen = strlen(lpVolab);
	char volLab[11];
	memcpy(volLab, lpVolab, nLen);
	int i;
	for( i = 0; i < 11 - nLen; i++)
	{
		volLab[nLen + i] = ' ';
	}
	memcpy(_imgBpb.BS_VolLab, volLab, 11);	
	if (fatType == FAT12_TYPE)
	{
		memcpy(_imgBpb.BS_FilSysType, "FAT12   ", 8);
	}
	else
		memcpy(_imgBpb.BS_FilSysType, "FAT16   ", 8);	
	
	// Initilize bpb End
	//////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////
	// Initilize FAT Begin
	if (fatType == FAT12_TYPE)
	{
		int isize= (_imgBpb.BPB_FATSz16*BYSPERSEC*2)/3;
		_fats.resize(isize); // 计算得到FAT项目的数目
	}
	else
		_fats.resize(_imgBpb.BPB_FATSz16*BYSPERSEC/2); // 计算得到FAT项目的数目

	for( i = 0; i < _fats.size(); i++)
	{
		_fats.at(i) = 0X00;
	}
	// Initialize FAT End
	//////////////////////////////////////////////////////////////////////////
	_curDirectory.resize(0);	
}



UINT  DiskImgFile::GetFirstFreeClusNum() // 返回空闲的簇号，从2开始
{
	int nsFat = _fats.size();
	for(int i = 0; i < nsFat; i++)
	{
		WORD ws = (WORD)_fats.at(i);
		if(ws == 0x00)
		{
			return i;
		}
	}
	return 0;
}

UINT  DiskImgFile::GetNextFreeClusNum(UINT StartClusNum)
{
	int nsFat = _fats.size();
	for(int i = StartClusNum+1; i < nsFat; i++)
	{
		WORD ws = (WORD)_fats.at(i);
		if(ws == 0x00)
		{
			return i;
		}
	}
	return 0;
}

BOOL  DiskImgFile::SetClusFreeStatus(IN UINT StartClusNum)
{
	if (StartClusNum == 0) return TRUE;

	WORD firstClus , lastClus  , nextClus , tmp;

	firstClus = lastClus = nextClus = StartClusNum;		
		 
	while(1)
	{			
		tmp = _fats.at(nextClus);			
		_fats.at(nextClus) = FREE_STATUS;			

		if(tmp == m_EOC_STATUS || tmp == FREE_STATUS)
		{
			break;
		}

		if (tmp < firstClus) firstClus = tmp;
		else if(tmp > lastClus) lastClus = tmp;	

		nextClus = tmp;				
	}
	
//	DWORD pdw;
	DWORD totallen = (lastClus - firstClus+1) * 2;
	BYTE* buf = new BYTE[totallen];
	BYTE* p = buf;
	
	for (int i=StartClusNum;i<=lastClus;i++)
	{
		tmp = _fats.at(i);
		memcpy(p , &tmp , 2);
		p+=2;
	}
	
	UINT retValue = _stOfFATAddr + StartClusNum * 2;
	//SetDiskFilePointer(NULL, retValue, NULL, FILE_BEGIN);
	//WriteDiskFile(_hFile, buf, totallen,&pdw , NULL);
	
	delete buf;
	return TRUE;	
}

BOOL  DiskImgFile::SetClusEx(IN UINT StartClusNum, IN UINT nNeedMoreClus)// 设置簇链 , Add by Joelee
{
	if (nNeedMoreClus == 0) return TRUE;	

	if(StartClusNum < 2)
		return FALSE;
	
	int nsFat = _fats.size();
	
	WORD firstClus , lastClus  , nextClus , tmp;

	tmp = GetFirstFreeClusNum();
	_fats.at(StartClusNum) = tmp;

	firstClus = StartClusNum;
	nextClus = lastClus = tmp;	

	int i;
	if(nNeedMoreClus>=2)
	{
		for(i = 0; i < nNeedMoreClus-1 ; i++)
		{
			tmp = GetNextFreeClusNum(nextClus);		 
			_fats.at(nextClus) = tmp;
			nextClus = tmp;

			if (tmp < firstClus) firstClus = tmp;
			else if(tmp > lastClus) lastClus = tmp;	
		}				
	}	
	
	_fats.at(lastClus) = m_EOC_STATUS;

	//if(StartClusNum <  firstClus) firstClus = StartClusNum;

//	DWORD pdw;
	DWORD totallen = (lastClus - firstClus+1) * 2;
	BYTE* buf = new BYTE[totallen];
	BYTE* p = buf;

	for (i=StartClusNum;i<=lastClus;i++)
	{
		tmp = _fats.at(i);
		memcpy(p , &tmp , 2);
		p+=2;
	}

	UINT retValue = _stOfFATAddr + StartClusNum * 2;
	//SetDiskFilePointer(NULL, retValue, NULL, FILE_BEGIN);
	//WriteDiskFile(_hFile, buf, totallen,&pdw , NULL);

	delete buf;
	return TRUE;
}

BOOL  DiskImgFile::SetClus(IN UINT clusNum, IN UINT nValue)	// 设置簇链
{
//	DWORD nWrite;
	if(clusNum < 2)
		return FALSE;
	if(_fats.size() <= clusNum )
	{
		//TRACE0("InValidate ClusNumber;\n");
		return FALSE;
	}
	UINT retValue;
	if (m_FatType==FAT12_TYPE)
	{
		retValue=clusNum+clusNum/2;
		_fats.at(clusNum) = LOWORD(nValue);
	}
	else
	{
		retValue = _stOfFATAddr + clusNum * 2;
		_fats.at(clusNum) = LOWORD(nValue);
		//SetDiskFilePointer(NULL, retValue, NULL, FILE_BEGIN);
	}
	

	WORD v = _fats.at(clusNum);
	//WriteDiskFile(_hFile, &v, 2, &nWrite, NULL);

	return TRUE;
	
}

void  DiskImgFile::InitializeClus(IN UINT clusNum)
{
	UINT pos = ClusterToRelatAddr(clusNum);
	SetDiskFilePointer(NULL, pos, NULL, (DWORD)FILE_BEGIN);
	DWORD nWrite;
	UINT  len = BYSPERSEC * SecPerClus();// _imgBpb.BPB_SecPerClus;

	BYTE* byts = new BYTE[len];
	memset(byts, 0x00, len);
	WriteDiskFile(_hFile, byts, len, &nWrite, NULL);
	SetDiskFilePointer(NULL, 0, NULL, (DWORD)FILE_BEGIN);
	delete []byts;
}



//////////////////////////////////////////////////////////////////////////
//Function: ParaDirectoryFromStr 对目录路径字符串进行解析。
// 注：根目录的表示为前面没有任何其他字符 如 \\abc\\ab 表示从根目录开始的abc目录下的ab目录
//
BOOL DiskImgFile::ParaDirectoryFromStr(IN LPCTSTR lptDirName, OUT vector<Fat_Directory>& fatDir) 
{
	int nLen = strlen(lptDirName);
	LPTSTR lpsz = NULL;
	LPTSTR lpFirstDir = NULL;

	BOOL isLast = FALSE;
	fatDir.resize(0);

	if(lptDirName == NULL)
		return FALSE;
	if(strcmp(lptDirName, "\\") == 0)
		return TRUE;

	lpsz = new TCHAR[nLen + 1];
	memset(lpsz, 0x00, (nLen + 1) * sizeof(TCHAR) );
	LPTSTR lpTem = lpsz;
	strcpy(lpsz, lptDirName);
	lpsz[nLen] = '\0';
	UINT curClus = 0;
	
	lpsz = (TCHAR*)_tcsinc(lpsz);

	while(lpsz)
	{
		LPTSTR longName =  GetLeftStr(lpsz, '\\', TRUE);
		if(longName == NULL)
			longName = lpsz;
	
		lpsz = (TCHAR*)strstr((LPCTSTR)lpsz, (LPCTSTR)"\\");
		if(lpsz)
			lpsz =(TCHAR*) _tcsinc(lpsz);

		int ns = fatDir.size();
		if(ns == 0)
			curClus ;
		else
		{
			curClus = fatDir.at(ns - 1).DIR_FstClusLO;
		}
		Fat_Directory dir;
		
		if(!GetDirectoryTab(dir, curClus, longName))
		{
			delete longName;
			longName = NULL;
			delete lpTem;
		//	delete lpsz;
			return FALSE;
		}
		delete longName;
		longName = NULL;
		fatDir.resize(ns + 1);

		Fat_Directory& tFat = fatDir.at(ns);
		memcpy(&tFat, &dir, sizeof(Fat_Directory));
	}
	delete lpTem;
	return TRUE;
}


BOOL DiskImgFile::ParaPathFromStr(IN LPCTSTR lptDirName, OUT vector<DirPaths>& paths)				// 解析路径
{

	int nLen = strlen(lptDirName);
	LPTSTR lpsz = NULL;
	LPTSTR lpFirstDir = NULL;

	BOOL isLast = FALSE;

	if(lptDirName == NULL)
		return FALSE;

	paths.resize(0);
	if(strcmp(lptDirName, "\\") == 0)
		return TRUE;

	lpsz = new TCHAR[nLen + 1];
	memset(lpsz, 0x00, (nLen + 1) * sizeof(TCHAR) );
	LPTSTR lpTem = lpsz;
	strcpy(lpsz, lptDirName);
	lpsz[nLen] = '\0';
	UINT curClus = 0;
	
	lpsz = (TCHAR*)_tcsinc(lpsz);

	while(lpsz)
	{
		LPTSTR longName =  GetLeftStr(lpsz, '\\', TRUE);
		if(longName == NULL)
			longName = lpsz;
		
		lpsz = (TCHAR*)_tcsstr((LPCTSTR)lpsz, (LPCTSTR)"\\");
		if(lpsz)
			lpsz = (TCHAR*)_tcsinc(lpsz);

		int ns = paths.size();
		
		
		int len = strlen(longName);	
		paths.resize(ns + 1);
		memset(paths.at(ns).pName, 0X00, MAX_PATH * sizeof(TCHAR));
		memcpy(paths.at(ns).pName, longName, len * sizeof(TCHAR));	
		delete longName;
		longName = NULL;

	}
	delete lpTem;
	return TRUE;

}
extern int g_StartLog;
BOOL DiskImgFile::GetDirectoryTabEx(OUT Fat_Directory& dir, OUT LPTSTR lptLongName, IN OUT INT& nIndex)			// 返回当前目录下第nIndex个目录
{
	Fat_DirectoryRW dirRw;
	DWORD nRead;
	int  ns = _curDirectory.size();
//	int nNeed = this->CalcNeedTabCount(lpLongName);
	int iEntryCount = 0;
	dir.DIR_EntryCount = iEntryCount;

	if(ns == 0) // 在根目录下
	{
		if(nIndex > ROOTENTCNT)
			return FALSE;

		UINT retAddr = _stOfRootAddr;
		SetDiskFilePointer(NULL, retAddr + nIndex * 32, NULL, FILE_BEGIN);
		ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
	
		char ch = (char)0xE5;
		if(dirRw.DIR_Name[0] == 0x00)
			return FALSE;
	//	if(dirRw.DIR_Name[0] == ch)
			
		if(dirRw.DIR_Name[0] == (char)0xE5)
		{
			while(1)
			{
				SetDiskFilePointer(NULL, retAddr + nIndex * 32, NULL, FILE_BEGIN);
				ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
				if(dirRw.DIR_Name[0] == 0x00)
					return FALSE;
				if(dirRw.DIR_Name[0] != (char)0xE5)
					break;
				nIndex++;
			}			
		}


		if((IsFile(dirRw.DIR_Attr) ||IsFoulder(dirRw.DIR_Attr)) && (dirRw.DIR_Name[0]!= ch))
		{
			char lpLName[MAX_PATH];
			ZeroMemory(lpLName, MAX_PATH);
			memcpy(lpLName, dirRw.DIR_Name, 8);
			if(IsFile(dirRw.DIR_Attr))
			{
				lpLName[8] = '.';
				memcpy(lpLName + 9,dirRw.DIR_Name+8, 3);
			}

#ifdef _UNICODE			
			int nw = MultiByteToWideChar(CP_ACP, 0, lpLName, -1, NULL, 0);
			MultiByteToWideChar(CP_ACP, 0, lpLName, -1, lptLongName, nw);
#else
			strcpy(lptLongName , lpLName);
#endif
			
			RemoveTChar(lptLongName, ' ');
			DirInfoFromRwInfo(dir,  dirRw);

			dir.DIR_PosIndex = nIndex;
			nIndex++;
			iEntryCount++;
			dir.DIR_EntryCount = iEntryCount;

			return TRUE;
		}

		BOOL bLongNameChkSumMatch = TRUE;
		BYTE* bTemp = (BYTE*)&dirRw;
		BYTE chksumbyte_l = *(bTemp+0x0D);
		
		while(IsLongDir(dirRw.DIR_Attr) && (dirRw.DIR_Name[0]!= ch))
		{
			Fat_LongDirectory longDir;
			LongDirInfoFromRwInfo(longDir, dirRw);

			if( chksumbyte_l != longDir.LDIR_Chksum && bLongNameChkSumMatch)
					bLongNameChkSumMatch = FALSE;	


#ifdef _UNICODE
			WCHAR wTmp[14];
			WCHAR wLTmp[MAX_PATH];
			ZeroMemory(wTmp, 28);
			ZeroMemory(wLTmp, MAX_PATH*2);
        
			memcpy(wTmp, longDir.LDIR_Name1, 26);
			wcscat(wLTmp, wTmp);
			wcscat(wLTmp, lptLongName);
			wcscpy(lptLongName, wLTmp);
#else
			char pchar[MAX_PATH];
			memset(pchar, 0x00, MAX_PATH);
			int nWidth = WideCharToMultiByte(m_CodePage, 0, longDir.LDIR_Name1, 13, pchar, MAX_PATH, "", FALSE);
			LPSTR lpTem = GetStrFromChArry(pchar, MAX_PATH);
			strcat(lpTem, lptLongName);
			strcpy(lptLongName, lpTem);
			delete lpTem;			
#endif

			nIndex ++;	
			iEntryCount++;

			SetDiskFilePointer(NULL, retAddr + nIndex * 32, NULL, FILE_BEGIN);
			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
			if(dirRw.DIR_Name[0] == 0x00)
				return FALSE;

			if(IsFile(dirRw.DIR_Attr) || IsFoulder(dirRw.DIR_Attr))
			{
				BYTE chksumbyte_s = ChkSum(dirRw.DIR_Name);
				
				if (chksumbyte_s!=chksumbyte_l || !bLongNameChkSumMatch)//long name check sum is not match the short name
				{
#ifdef _UNICODE
					char szShortName[16];
					char filename[9];
					memcpy(filename , dirRw.DIR_Name , 8);
					filename[8] = 0x00;
					strcpy(szShortName , filename);
					strcat(szShortName , ".");
					
					char ext[4];
					memcpy(ext , dirRw.DIR_Name+8 , 3);
					ext[3] = 0x00;					
					strcat(szShortName , ext);

					int nw = MultiByteToWideChar(CP_ACP, 0, szShortName, -1, NULL, 0);
					MultiByteToWideChar(CP_ACP, 0, szShortName, -1, lptLongName, nw);

					RemoveTChar(lptLongName , ' ');
#else	

					char filename[9];
					memcpy(filename , dirRw.DIR_Name , 8);
					filename[8] = 0x00;
					RemoveTChar(filename , ' ');
					strcpy(lptLongName , filename);
					strcat(lptLongName , ".");
					
					char ext[4];
					memcpy(ext , dirRw.DIR_Name+8 , 3);
					ext[3] = 0x00;
					RemoveTChar(ext , ' ');
					strcat(lptLongName , ext);
#endif
				}
				
				DirInfoFromRwInfo(dir,  dirRw);
				dir.DIR_PosIndex = nIndex;
				nIndex++;
				iEntryCount++;
				dir.DIR_EntryCount = iEntryCount;
				return TRUE;
			}
		} //while(IsLongDir(dirRw.DIR_Attr) && (dirRw.DIR_Name[0]!= ch))
	//	DirInfoFromRwInfo(dir, dirRw);
		nIndex++;
	}
	else //if(ns == 0) 
	{
	//	if(nIndex > BYSPERSEC * _imgBpb.BPB_SecPerClus / 32)
	//		return FALSE;
		
		int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
		int nCluIndex = 0;  // 记录在该目录的第 几个簇下操作

		Fat_Directory& fat = _curDirectory.at(ns - 1);		
		UINT retAddr = ClusterToRelatAddr(fat.DIR_FstClusLO);
		
		WORD curClu = fat.DIR_FstClusLO;
		nCluIndex = nIndex / recPerCluster;
		int retIndex = 	nIndex % recPerCluster;	

		for(int  i = 0; i < nCluIndex; i++)
		{
			curClu = _fats.at(curClu);
			if (curClu == m_EOC_STATUS)
			{	
				return FALSE;
			}
			
			retAddr = ClusterToRelatAddr(curClu);
		}

		
		while(1)
		{			
			SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);

			retIndex++;
			nIndex++;

			if(dirRw.DIR_Name[0] == 0x00)
				return FALSE;
			if(dirRw.DIR_Name[0] == (char)0xE5)
			{
				while(1)
				{

					if(retIndex >= recPerCluster)
					{
						retIndex = 0;
						curClu = _fats.at(curClu);
						if(curClu == m_EOC_STATUS)
							return FALSE;
						nCluIndex++;						
						retAddr = ClusterToRelatAddr(curClu);
					}
					
					SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
					ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
					if(dirRw.DIR_Name[0] == 0x00)
						return FALSE;
					retIndex++;
					nIndex++;

					if(dirRw.DIR_Name[0] != (char)0xE5)
						break;					
				}			
			}	
			
			char ch = (char)0xE5;
			if((IsFile(dirRw.DIR_Attr) ||IsFoulder(dirRw.DIR_Attr)) && (dirRw.DIR_Name[0]!= ch))
			{
				char lpLName[MAX_PATH];
				ZeroMemory(lpLName, MAX_PATH);
				memcpy(lpLName, dirRw.DIR_Name, 8);
				if(IsFile(dirRw.DIR_Attr))
				{
					lpLName[8] = '.';
					memcpy(lpLName + 9,dirRw.DIR_Name+8, 3);
				}

#ifdef _UNICODE			
				int nw = MultiByteToWideChar(CP_ACP, 0, lpLName, -1, NULL, 0);
				MultiByteToWideChar(CP_ACP, 0, lpLName, -1, lptLongName, nw);
#else
				strcpy(lptLongName , lpLName);
#endif
				
				RemoveTChar(lptLongName, ' ');
				DirInfoFromRwInfo(dir,  dirRw);
		
				dir.DIR_PosIndex = nIndex-1;
				iEntryCount++;
				dir.DIR_EntryCount = iEntryCount;

				return TRUE;
			} 

			BOOL bLongNameChkSumMatch = TRUE;
			BYTE* bTemp = (BYTE*)&dirRw;
			BYTE chksumbyte_l = *(bTemp+0x0D);
			
			while(IsLongDir(dirRw.DIR_Attr) && (dirRw.DIR_Name[0]!= ch))
			{
				Fat_LongDirectory longDir;
				LongDirInfoFromRwInfo(longDir, dirRw);

				if( chksumbyte_l != longDir.LDIR_Chksum && bLongNameChkSumMatch)
					bLongNameChkSumMatch = FALSE;	
			
				if (bLongNameChkSumMatch)
				{
#ifdef _UNICODE
					WCHAR wTmp[14];
					WCHAR wLTmp[MAX_PATH];
					ZeroMemory(wTmp, 28);
					ZeroMemory(wLTmp, MAX_PATH*2);
        
					memcpy(wTmp, longDir.LDIR_Name1, 26);
					wcscat(wLTmp, wTmp);
					wcscat(wLTmp, lptLongName);
					wcscpy(lptLongName, wLTmp);
#else
					char pchar[MAX_PATH];
					memset(pchar, 0x00, MAX_PATH);
					int nWidth = WideCharToMultiByte(m_CodePage, 0, longDir.LDIR_Name1, 13, pchar, MAX_PATH, "", FALSE);
					LPSTR lpTem = GetStrFromChArry(pchar, MAX_PATH);
					strcat(lpTem, lptLongName);
					strcpy(lptLongName, lpTem);
					delete lpTem;			
#endif

	
				}
				iEntryCount++;

				if(retIndex >= recPerCluster)
				{
					retIndex = 0;
					curClu = _fats.at(curClu);
					if(curClu == m_EOC_STATUS)
						return FALSE;
					nCluIndex++;						
					retAddr = ClusterToRelatAddr(curClu);
				}
				SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
				ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
				if(dirRw.DIR_Name[0] == 0x00)
					return FALSE;
			
				if(IsFile(dirRw.DIR_Attr) || IsFoulder(dirRw.DIR_Attr))
				{
					BYTE chksumbyte_s = ChkSum(dirRw.DIR_Name);
					
					if (chksumbyte_s!=chksumbyte_l || !bLongNameChkSumMatch)//long name check sum is not match the short name
					{
#ifdef _UNICODE
						char szShortName[16];
						char filename[9];
						memcpy(filename , dirRw.DIR_Name , 8);
						filename[8] = 0x00;
						strcpy(szShortName , filename);
						strcat(szShortName , ".");
						
						char ext[4];
						memcpy(ext , dirRw.DIR_Name+8 , 3);
						ext[3] = 0x00;					
						strcat(szShortName , ext);

						int nw = MultiByteToWideChar(CP_ACP, 0, szShortName, -1, NULL, 0);
						MultiByteToWideChar(CP_ACP, 0, szShortName, -1, lptLongName, nw);

						RemoveTChar(lptLongName , ' ');
#else	
						char filename[9];
						memcpy(filename , dirRw.DIR_Name , 8);
						filename[8] = 0x00;
						RemoveTChar(filename , ' ');
						strcpy(lptLongName , filename);
						strcat(lptLongName , ".");
						
						char ext[4];
						memcpy(ext , dirRw.DIR_Name+8 , 3);
						ext[3] = 0x00;
						RemoveTChar(ext , ' ');
						strcat(lptLongName , ext);
#endif
					}

					DirInfoFromRwInfo(dir,  dirRw);
					dir.DIR_PosIndex = nIndex;
					nIndex++;
					iEntryCount++;
					dir.DIR_EntryCount = iEntryCount;
					return TRUE;
				}
				retIndex++;
				nIndex++;
			}//while(IsLongDir(dirRw.DIR_Attr) && (dirRw.DIR_Name[0]!= ch))
			return FALSE;
		}
	}
	DirInfoFromRwInfo(dir,  dirRw);
	dir.DIR_PosIndex = nIndex - 1;
	return TRUE;
}

BOOL DiskImgFile::GetDirectoryTab(OUT Fat_Directory& dir, IN OUT INT& nIndex)
{
	Fat_DirectoryRW dirRw;
	DWORD nRead;
	int  ns = _curDirectory.size();
	
	if(ns == 0) // 在根目录下
	{
		if(nIndex > ROOTENTCNT)
			return FALSE;

		UINT retAddr = _stOfRootAddr;
		SetDiskFilePointer(NULL, retAddr + nIndex * 32, NULL, FILE_BEGIN);
		ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
	
		if(dirRw.DIR_Name[0] == 0x00)
			return FALSE;
	
	//	Fat_Directory temDir;
	//	DirInfoFromRwInfo(temDir, dirRw);
		while(dirRw.DIR_Name[0] == (char)0xE5)
		{
			nIndex++;
			SetDiskFilePointer(NULL, retAddr + nIndex * 32, NULL, FILE_BEGIN);
			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);	
			if(dirRw.DIR_Name[0] == 0x00)
				return FALSE;
		}
		while(IsLongDir(dirRw.DIR_Attr) && (dirRw.DIR_Name[0]!= (char)0xE5))
		{
			nIndex ++;	
			SetDiskFilePointer(NULL, retAddr + nIndex * 32, NULL, FILE_BEGIN);
			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
			if(dirRw.DIR_Name[0] == 0x00)
				return FALSE;
		}
	
	//	DirInfoFromRwInfo(dir, dirRw);
		nIndex++;
	}
	else
	{
	/*
		if(nIndex > BYSPERSEC * _imgBpb.BPB_SecPerClus / 32)
				return FALSE;
	
	*/
		if(nIndex < 2)
			nIndex = 2;
		Fat_Directory& fat = _curDirectory.at(ns - 1);
		UINT retAddr = ClusterToRelatAddr(fat.DIR_FstClusLO);
		
		int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
		int nCluIndex = 0;  // 记录在该目录的第 几个簇下操作
	
		WORD curClu = fat.DIR_FstClusLO;
		nCluIndex = nIndex / recPerCluster;
		int retIndex = 	nIndex % recPerCluster;	
		for(int  i = 0; i < nCluIndex; i++)
		{
			curClu = _fats.at(curClu);
			if (curClu == m_EOC_STATUS) return FALSE;
			retAddr = ClusterToRelatAddr(curClu);
		}

		SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
		ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
		nIndex ++;	
		retIndex++;
		if(retIndex >= recPerCluster)
		{
			retIndex = 0;
			curClu = _fats.at(curClu);
			if(curClu == m_EOC_STATUS)
				return FALSE;
			nCluIndex++;						
			retAddr = ClusterToRelatAddr(curClu);
		}
		if(dirRw.DIR_Name[0] == 0x00)
			return FALSE;
		
		while(dirRw.DIR_Name[0] == (char)0xE5)
		{
			SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);	
			nIndex ++;	
			retIndex++;
			if(retIndex >= recPerCluster)
			{
				retIndex = 0;
				curClu = _fats.at(curClu);
				if(curClu == m_EOC_STATUS)
					return FALSE;
				nCluIndex++;						
				retAddr = ClusterToRelatAddr(curClu);
			}
			if(dirRw.DIR_Name[0] == 0x00)
				return FALSE;
		}
	
		while(IsLongDir(dirRw.DIR_Attr) && (dirRw.DIR_Name[0]!= (char)0xE5))
		{			
			SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
			nIndex ++;	
			retIndex++;
			if(retIndex >= recPerCluster)
			{
				retIndex = 0;
				curClu = _fats.at(curClu);
				if(curClu == m_EOC_STATUS)
					return FALSE;
				nCluIndex++;						
				retAddr = ClusterToRelatAddr(curClu);
			}
			if(dirRw.DIR_Name[0] == 0x00)
				return FALSE;
		}
		//nIndex++;
	}
	DirInfoFromRwInfo(dir,  dirRw);
	dir.DIR_PosIndex = nIndex - 1;
	return TRUE;
}

BOOL DiskImgFile::GetDirectoryTab(OUT Fat_Directory& dir, IN LPCTSTR longName)
{
	int nIndex = 0;
	LPTSTR lptTemp = new TCHAR[MAX_PATH];
	memset(lptTemp, 0x00, MAX_PATH * sizeof(TCHAR));
	while(1)
	{
		memset(lptTemp, 0x00, MAX_PATH * sizeof(TCHAR));

		if(!GetDirectoryTabEx(dir, lptTemp, nIndex))
		{
			delete lptTemp;
			return FALSE;
		}
		if(strcmp(lptTemp, longName) == 0)		
		{
			delete lptTemp;
			return TRUE;
		}
	}
	delete lptTemp;
	return FALSE;
	
}


BOOL DiskImgFile::SetCurrentDirectory(IN LPCTSTR lptPathName)						// 设置当前路径
{
	if(!ParaDirectoryFromStr(lptPathName, _curDirectory))
		return FALSE;

	return TRUE;
}


BOOL DiskImgFile::GetCurrentDirectory(OUT LPTSTR lpBuffer, IN DWORD nBufferLen)
{
	memset(lpBuffer, 0x00, nBufferLen * sizeof(TCHAR));
	
	if(_curDirectory.size() == 0)
	{
		strcat(lpBuffer, "\\");
	}

	LPTSTR lpLongName = new TCHAR[MAX_PATH];
	memset(lpLongName, 0x00,  MAX_PATH * sizeof(TCHAR));

	for(int i = 0; i < _curDirectory.size(); i++)
	{
		strcat(lpBuffer, "\\");
		memset(lpLongName, 0x00,  MAX_PATH * sizeof(TCHAR));

		if(!GetDirectoryLongName(lpLongName, MAX_PATH,_curDirectory.at(i)))
			return FALSE;	

		strcat(lpBuffer, lpLongName);
	
	}
	delete[] lpLongName;
	return TRUE;
}


BOOL DiskImgFile::FormatImgFile(LPCSTR lpVolLab, IN UINT fatType /* = FAT16_TYPE */, IN LONGLONG diskSize /* = 134217728 */)
{
	Iinitialize(lpVolLab, fatType, diskSize);//FAT16_TYPE, DISK_128M);

	BootSector_BPB_RW bppRw;
	RwInfoCopyFormBpb(bppRw, _imgBpb);
	
	
	DWORD nCount = sizeof(bppRw);
	DWORD nWritten;

	DWORD dwNew = SetDiskFilePointer((HANDLE)_hFile, 0, NULL, (DWORD)FILE_BEGIN);
//////////////////////////////////////////////////////////////////////////
// Write First Sector	
	if (!WriteDiskFile((HANDLE)_hFile, &bppRw, nCount, &nWritten, NULL))
	{
		//printmsg(L"Write bpb Error!");
		return FALSE;
	}

	if (nWritten != nCount)
	{
		//printmsg(L"Write bpb Error2! 空间不够大！");
		return FALSE;
	}
	int i;
	for( i = 1; i < _imgBpb.BPB_RsvdSecCnt; i++)
	{
		BYTE resv[512];
		memset(resv,0x00, 512);
		if (!WriteDiskFile((HANDLE)_hFile, resv, 512, &nWritten, NULL))
		{
			//printmsg(L"Write bpb Error!");
			return FALSE;
		}

	}

//
//////////////////////////////////////////////////////////////////////////

	_stOfFATAddr = _imgBpb.BPB_RsvdSecCnt*_imgBpb.BPB_BytsPerSec;

//////////////////////////////////////////////////////////////////////////
// Write FATs
	int nsFat = _fats.size();	
	if (fatType == FAT12_TYPE)
	{
		//_fats.at(0) = 0xFFF8;
		//_fats.at(1) = 0x00FF;
		int iFatBytes = _imgBpb.BPB_FATSz16 * BYSPERSEC;
		BYTE* fs = new BYTE[iFatBytes];	
		fs[0]=0xF8;
		fs[1]=0xFF;
		fs[2]=0xFF;
//		int iFatOffset;
//		int iThisFatEntryOffset;
//		WORD wFat12ClusEntryVal;
		for (i=3; i<iFatBytes;i++)
		{
			//iFatOffset = i+(i/2);
			fs[i]=0x00;
#ifdef _MAC_UNIX
			fs[i] = Swap16Host2Little(fs[i]);
#endif	
			
		}

		if (!WriteDiskFile((HANDLE)_hFile, fs, iFatBytes*sizeof(BYTE), &nWritten, NULL))
		{
			//printmsg(L"Write FAT1 Error!");
			return FALSE;
		}
		
		if (!WriteDiskFile((HANDLE)_hFile, fs, iFatBytes*sizeof(BYTE), &nWritten, NULL))
		{
			//printmsg(L"Write FAT2 Error!");
			return FALSE;
		}
		delete []fs;
		_stOfRootAddr = _stOfFATAddr + iFatBytes* _imgBpb.BPB_NumFATs;
	}
	else
	{
		_fats.at(0) = 0xFFF8;
		_fats.at(1) = 0xFFFF;
		WORD* fs = new WORD[nsFat];
		for( i = 0; i< nsFat; i++)
		{
			fs[i] = _fats.at(i);
#ifdef _MAC_UNIX
			fs[i] = Swap16Host2Little(fs[i]);
#endif
		}
		if (!WriteDiskFile((HANDLE)_hFile, fs, nsFat*sizeof(WORD), &nWritten, NULL))
		{
			//printmsg(L"Write FAT1 Error!");
			return FALSE;
		}
		
		if (!WriteDiskFile((HANDLE)_hFile, fs, nsFat*sizeof(WORD), &nWritten, NULL))
		{
			//printmsg(L"Write FAT2 Error!");
			return FALSE;
		}
		delete []fs;
		_stOfRootAddr = _stOfFATAddr + nsFat* 2 * _imgBpb.BPB_NumFATs;
	}

//
//////////////////////////////////////////////////////////////////////////

	

//////////////////////////////////////////////////////////////////////////
// Write RootDirectory

	Fat_Directory dir;
	Fat_DirectoryRW dirRw;
	dir.DIR_Attr = ATTR_VOLUME_ID;
	::GetSystemTime(&dir.DIR_WrtDateTime);
	::GetSystemTime(&dir.DIR_CrtDateTime);
	::GetSystemTime(&dir.DIR_LstAcceDate);

/*	dir.DIR_CrtDateTime = CTime::GetCurrentTime();
	dir.DIR_LstAcceDate = CTime::GetCurrentTime();
	dir.DIR_WrtDateTime = CTime::GetCurrentTime();
*/
	dir.DIR_FstClusHI = 0;
	dir.DIR_FstClusLO = 0;
	dir.DIR_FileSize = 0;
	memcpy(dir.DIR_Name, lpVolLab, 11);
	RwInfoFromDirInfo(dirRw, dir);
	WriteDiskFile(_hFile, &dirRw, 32, &nWritten, NULL);

	DWORD roorDirlen = 511 * 32; //512 direntry counts - 1 VOLUMN ID
	BYTE* roorDir = new BYTE[roorDirlen];
	ZeroMemory(roorDir , roorDirlen);

	if (!WriteDiskFile((HANDLE)_hFile, roorDir, roorDirlen, &nWritten, NULL))
	{
		//printmsg(L"Write RootDir Error!");
		delete[] roorDir;
		return FALSE;
	}
	delete[] roorDir;
//
//////////////////////////////////////////////////////////////////////////
	
	_stOfClusterAddr = _stOfRootAddr + ROOTENTCNT * nWritten;

//////////////////////////////////////////////////////////////////////////
// Wrtite Content
/*
	int nsContent = (nsFat -2) * _imgBpb.BPB_SecPerClus * BYSPERSEC;
	BYTE* content = new BYTE[nsContent];
	memset(content, 0x00, nsContent);
	if (!::WriteFile((HANDLE)_hFile, content, nsContent, &nWritten, NULL))
	{
		printmsg(L"Write Content Error!");
		return false;
	}
	delete []content;*/

//
//////////////////////////////////////////////////////////////////////////
//	
	return TRUE;
}

BOOL DiskImgFile::RefreshFatTable()
{
//	return TRUE;
	if (m_FatType==FAT12_TYPE)
	{
		return RefreshFat12Table();
	}
	//Write Fat to disk
	int nsFat = _fats.size();
	DWORD nWritten = 0;
	WORD* fs = new WORD[nsFat];
	for(int i = 0; i< nsFat; i++)
	{
		fs[i] = _fats.at(i);
#ifdef _MAC_UNIX
		fs[i] = Swap16Host2Little(fs[i]);
#endif
	}
		
	SetDiskFilePointer(_hFile , _stOfFATAddr , 0 , FILE_BEGIN);	
	if (!WriteDiskFile((HANDLE)_hFile, fs, nsFat*sizeof(WORD), &nWritten, NULL))
	{
		delete []fs;			
		return FALSE;		
	}
	
	if (!WriteDiskFile((HANDLE)_hFile, fs, nsFat*sizeof(WORD), &nWritten, NULL))
	{
		delete []fs;			
		return FALSE;		
	}
	
	
	delete []fs;	

	return TRUE;
	
// 	DWORD nRead;
// 
// 	SetDiskFilePointer(NULL, _stOfFATAddr, NULL, FILE_BEGIN);
// 	
// 	int nFats = _imgBpb.BPB_FATSz16 * BYSPERSEC / 2;
// 	_fats.resize(nFats);
// 	WORD* fs = new WORD[_fats.size()];
// 	memset(fs, 0x00, _fats.size()*2);
// 
// 	ReadDiskFile(NULL, fs, _fats.size(), &nRead, NULL);
// 	
// 	for(int i = 0; i < nFats; i++)
// 	{
// 		_fats.at(i) = fs[i];
// 	}
// 
// 
// 	delete[] fs;
// 
// 	return TRUE;
}

BOOL DiskImgFile::OpenImgFile(IN LPCSTR lpFileName, IN UINT fatType, IN LONGLONG diskSize)
{
// 	m_FatType=fatType;
// 	if (fatType==FAT12_TYPE)
// 	{
// 		m_EOC_STATUS=EOC_STATUS12;
// 	}
// 	else
// 		m_EOC_STATUS=EOC_STATUS16;
	//if ( !CreateDiskFile(lpFileName) ) return FALSE;	
	int ret = CreateDiskFile(lpFileName);	
	if(ret == 0) return FALSE;
	else if (ret == 2)
		FormatImgFile(DEFAULT_VOLUME_NAME,fatType,diskSize);


	SetDiskFilePointer(NULL, 0, NULL, FILE_BEGIN);	
	DWORD nRead;

	BootSector_BPB_RW bpbRw;
	ReadDiskFile(NULL, &bpbRw, sizeof(BootSector_BPB_RW), &nRead, NULL);
	
	if(nRead != sizeof(BootSector_BPB_RW))
	{
// 		if(printmsg("This img file isn't formatted, \n would like fromat is now?", MB_YESNO) == IDYES)
// 		{
// 			//CloseHandle(_hFile);
// 			//::DeleteFile(lpFileName);
// 			CloseDiskFile();
// 			goto OpFlag;			
// 		}
		return FALSE;
	}
	BpbCobyFromRwInfo(_imgBpb, bpbRw);
	
	_stOfFATAddr = _imgBpb.BPB_RsvdSecCnt*_imgBpb.BPB_BytsPerSec;;
	
	SetDiskFilePointer(_hFile, _stOfFATAddr, NULL, FILE_BEGIN);
	int nFats;
	if (fatType==FAT12_TYPE)
	{
		nFats= (_imgBpb.BPB_FATSz16*BYSPERSEC*2)/3; //Fat最多包含簇号个数
		_fats.resize(nFats);

		int nFatBytes = _imgBpb.BPB_FATSz16*BYSPERSEC; //Fat表占有字节数
		BYTE *fs=new BYTE[nFatBytes];
		memset(fs,0x0,nFatBytes);

		ReadDiskFile(_hFile, fs, nFatBytes, &nRead, NULL);

		int iFatOffset, iThisFatEntOffset;
		WORD wFAT12ClusEntryVal;
		for (int i = 0; i<nFats; i++)
		{
			iFatOffset= i+i/2;
			iThisFatEntOffset= iFatOffset/*%BYSPERSEC*/;
			
			wFAT12ClusEntryVal = *((WORD *) &fs[iThisFatEntOffset]); //65528=0xfff8

			if(i & 0x0001)
				wFAT12ClusEntryVal = wFAT12ClusEntryVal >> 4;    /* Cluster number is ODD *///4095=0xfff
			else
				wFAT12ClusEntryVal = wFAT12ClusEntryVal & 0x0FFF; /* Cluster number is EVEN *///4088=0xff8

			_fats.at(i) = wFAT12ClusEntryVal;
		}

		delete[] fs;

		_stOfRootAddr = _imgBpb.BPB_FATSz16 * BYSPERSEC*2 + _stOfFATAddr; //16896
		
		//_stOfClusterAddr = _stOfRootAddr + 32 * BYSPERSEC;
		_stOfClusterAddr = _stOfRootAddr + 32 * ROOTENTCNT; //33280
	}
	else
	{
		nFats= _imgBpb.BPB_FATSz16 * BYSPERSEC / 2;
		_fats.resize(nFats);
		WORD* fs = new WORD[_fats.size()];
		memset(fs, 0x00, _fats.size()*2);
		
		ReadDiskFile(_hFile, fs, _fats.size() * 2, &nRead, NULL);
		
		for(int i = 0; i < nFats; i++)
		{
#ifdef _MAC_UNIX
			fs[i] = Swap16Little2Host(fs[i]);
#endif
			_fats.at(i) = fs[i];
		}
		
		
		delete[] fs;
		_stOfRootAddr = nFats * 2 *2 + _stOfFATAddr;
		
		_stOfClusterAddr = _stOfRootAddr + 32 * ROOTENTCNT;
	}
	

	return TRUE;
}


BOOL DiskImgFile::CloseImgFile()
{
//	if(_hFile)
//		::CloseHandle(_hFile);
//	_hFile = NULL;

	CloseDiskFile();
	return TRUE;
}

BOOL DiskImgFile::GetDirectoryLongName(OUT LPTSTR lpBuffer, IN DWORD nBufferLen, IN Fat_Directory dir)
{
	memset(lpBuffer, 0x00, nBufferLen);
	
	UINT c = dir.DIR_FstClusHI;
	c = c<<16|dir.DIR_FstClusLO;
	UINT retAddr = ClusterToRelatAddr(c);
	
	DWORD nRead;
	Fat_DirectoryRW dirRw;
	Fat_LongDirectory longDir;
	
	SetDiskFilePointer(NULL, retAddr + 32, NULL, FILE_BEGIN);
	ReadDiskFile(NULL, &dirRw, 32, &nRead, NULL);
	DWORD pClus ; 
	WORD fsc;
	memcpy(&fsc, dirRw.DIR_FstClusHI, 2);
	pClus = fsc;
	memcpy(&fsc, dirRw.DIR_FstClusLO, 2);
	pClus = pClus<<16|fsc;

	if(pClus == 0)
	{
		retAddr = _stOfRootAddr + dir.DIR_PosIndex * 32;
		if(dir.DIR_PosIndex == 0)
		{
			char sztemp[16];

			memcpy(sztemp, dir.DIR_Name, 8);
			if(IsFile(dir.DIR_Attr))
			{
				sztemp[8] = '.';
				memcpy(sztemp + 9, dir.DIR_Name + 8, 3);
			}

#ifdef _UNICODE
			int nw = MultiByteToWideChar(CP_ACP, 0, sztemp, -1, NULL, 0);
			MultiByteToWideChar(CP_ACP, 0, sztemp, -1, lpBuffer, nw);
#else
			strcpy(lpBuffer , sztemp);
#endif			
			
			RemoveTChar(lpBuffer, ' ');

			return TRUE;
		}

	//	GetStrFromChArry(dir.DIR_Name, 11);
		char ShortName[12];
		ShortName[11] = '\0';
		memcpy(ShortName,dir.DIR_Name, 11);
		BYTE sum = ChkSum(ShortName);
		SetDiskFilePointer(NULL, retAddr - 32, NULL, FILE_BEGIN);
		ReadDiskFile(NULL, &dirRw,  32, &nRead, NULL);
		LongDirInfoFromRwInfo(longDir, dirRw);

		if(longDir.LDIR_Chksum != sum)
		{
			char sztemp[16];

			memcpy(sztemp, dir.DIR_Name, 8);
			if(IsFile(dir.DIR_Attr))
			{
				sztemp[8] = '.';
				memcpy(sztemp + 9, dir.DIR_Name + 8, 3);
			}

#ifdef _UNICODE
			int nw = MultiByteToWideChar(CP_ACP, 0, sztemp, -1, NULL, 0);
			MultiByteToWideChar(CP_ACP, 0, sztemp, -1, lpBuffer, nw);
#else
			strcpy(lpBuffer , sztemp);
#endif			
			
			RemoveTChar(lpBuffer, ' ');
			return TRUE;
		}
		
		int n = 1;
		while(1)
		{
			SetDiskFilePointer(NULL, retAddr - 32*n, NULL, FILE_BEGIN);
			ReadDiskFile(NULL, &dirRw,  32, &nRead, NULL);
			LongDirInfoFromRwInfo(longDir, dirRw);
			if(longDir.LDIR_Ord == (char)0xE5)
			{
				printmsg("Error in getting long name function!");
				return FALSE;
			}
			if(longDir.LDIR_Ord&LAST_LONG_ENTRY)
			{
				Fat_LongDirectory longDir;
				LongDirInfoFromRwInfo(longDir, dirRw);

#ifdef _UNICODE
				WCHAR wTmp[14];
				ZeroMemory(wTmp, 28);
    
				memcpy(wTmp, longDir.LDIR_Name1, 26);
				wcscat(lpBuffer, wTmp);
#else
				char* pchar = new char[MAX_PATH];
				memset(pchar, 0x00, MAX_PATH);
				int nWidth = WideCharToMultiByte(m_CodePage, 0, longDir.LDIR_Name1, 13, pchar, MAX_PATH, "", FALSE);
				strcat(lpBuffer, pchar);
				delete[] pchar;				
#endif
				break;
			}

			Fat_LongDirectory longDir;
			LongDirInfoFromRwInfo(longDir, dirRw);

#ifdef _UNICODE
				WCHAR wTmp[14];
				ZeroMemory(wTmp, 28);
    
				memcpy(wTmp, longDir.LDIR_Name1, 26);
				wcscat(lpBuffer, wTmp);	
#else
				char* pchar = new char[MAX_PATH];
				memset(pchar, 0x00, MAX_PATH);
				int nWidth = WideCharToMultiByte(m_CodePage, 0, longDir.LDIR_Name1, 13, pchar, MAX_PATH, "", FALSE);
				strcat(lpBuffer, pchar);
				delete[] pchar;						
#endif	
				n++;
		}
	}
	else
	{
		if(dir.DIR_PosIndex == 0)
		{
			char sztemp[16];

			memcpy(sztemp, dir.DIR_Name, 8);
			if(IsFile(dir.DIR_Attr))
			{
				sztemp[8] = '.';
				memcpy(sztemp + 9, dir.DIR_Name + 8, 3);
			}

#ifdef _UNICODE
			int nw = MultiByteToWideChar(CP_ACP, 0, sztemp, -1, NULL, 0);
			MultiByteToWideChar(CP_ACP, 0, sztemp, -1, lpBuffer, nw);
#else
			strcpy(lpBuffer , sztemp);
#endif					
			RemoveTChar(lpBuffer, ' ');
			
			return TRUE;
		}
	

		int recPerCluster = BYSPERSEC * SecPerClus() / 32;	//一簇包含最大的值
		int nCluIndex = 0;  // 记录在该目录的第 几个簇下操作

		DWORD curClu = pClus;
		
		
		nCluIndex = dir.DIR_PosIndex / recPerCluster;
		
		int retIndex = 	dir.DIR_PosIndex % recPerCluster;	
		

		retAddr = ClusterToRelatAddr(curClu);
		for(int  i = 0; i < nCluIndex; i++)
		{
			curClu = _fats.at(curClu);
			if (curClu == m_EOC_STATUS) return FALSE;
			retAddr = ClusterToRelatAddr(curClu);
		}

		retAddr = retAddr + retIndex * 32;
	
	//	GetStrFromChArry(dir.DIR_Name, 11);
		char ShortName[12];
		ShortName[11] = '\0';
		memcpy(ShortName,dir.DIR_Name, 11);
		BYTE sum = ChkSum(ShortName);
		SetDiskFilePointer(NULL, retAddr - 32, NULL, FILE_BEGIN);
		ReadDiskFile(NULL, &dirRw,  32, &nRead, NULL);
		LongDirInfoFromRwInfo(longDir, dirRw);

		if(longDir.LDIR_Chksum != sum)
		{
			char sztemp[16];

			memcpy(sztemp, dir.DIR_Name, 8);
			if(IsFile(dir.DIR_Attr))
			{
				sztemp[8] = '.';
				memcpy(sztemp + 9, dir.DIR_Name + 8, 3);
			}

#ifdef _UNICODE
			int nw = MultiByteToWideChar(CP_ACP, 0, sztemp, -1, NULL, 0);
			MultiByteToWideChar(CP_ACP, 0, sztemp, -1, lpBuffer, nw);
#else
			strcpy(lpBuffer , sztemp);
#endif					
			RemoveTChar(lpBuffer, ' ');

			return TRUE;
		}
		
		int n = 1;
		while(1)
		{
			if(retIndex == 0)
			{
				retIndex = recPerCluster;
				n = 1;
				nCluIndex--;
				if(nCluIndex < 0)
					return FALSE;
				curClu = dir.DIR_FstClusLO;
				for(int  i = 0; i < nCluIndex; i++)
				{
					curClu = _fats.at(curClu);
					if (curClu == m_EOC_STATUS) return FALSE;
					retAddr = ClusterToRelatAddr(curClu);
				}	
				retAddr = retAddr + retIndex*32;
			}

			SetDiskFilePointer(NULL, retAddr - 32*n, NULL, FILE_BEGIN);
			ReadDiskFile(NULL, &dirRw,  32, &nRead, NULL);
			LongDirInfoFromRwInfo(longDir, dirRw);
			if(longDir.LDIR_Ord == (char)0xE5)
			{
				printmsg("Error in getting long name function!");
				return FALSE;
			}
			if(longDir.LDIR_Ord&LAST_LONG_ENTRY)
			{
				Fat_LongDirectory longDir;
				LongDirInfoFromRwInfo(longDir, dirRw);

#ifdef _UNICODE
				WCHAR wTmp[14];
				ZeroMemory(wTmp, 28);
    
				memcpy(wTmp, longDir.LDIR_Name1, 26);
				wcscat(lpBuffer, wTmp);
#else
				char* pchar = new char[MAX_PATH];
				memset(pchar, 0x00, MAX_PATH);
				int nWidth = WideCharToMultiByte(m_CodePage, 0, longDir.LDIR_Name1, 13, pchar, MAX_PATH, "", FALSE);
				strcat(lpBuffer, pchar);
				delete[] pchar;				
#endif
				break;
			}
			Fat_LongDirectory longDir;
			LongDirInfoFromRwInfo(longDir, dirRw);

#ifdef _UNICODE
			WCHAR wTmp[14];
			ZeroMemory(wTmp, 28);

			memcpy(wTmp, longDir.LDIR_Name1, 26);
			wcscat(lpBuffer, wTmp);				
#else
			char* pchar = new char[MAX_PATH];
			memset(pchar, 0x00, MAX_PATH);
			int nWidth = WideCharToMultiByte(m_CodePage, 0, longDir.LDIR_Name1, 13, pchar, MAX_PATH, "", FALSE);
			strcat(lpBuffer, pchar);
			delete[] pchar;
#endif
			n++;
		}
	}
	

	return TRUE;
	

}

BOOL DiskImgFile::GetDirectoryTab(OUT Fat_Directory& dir, IN UINT clus, IN OUT INT& nIndex)
{
	Fat_DirectoryRW dirRw;
	DWORD nRead;
	UINT retAddr = 0;
	if(clus == 0)
		retAddr = _stOfRootAddr;
	else
		retAddr = ClusterToRelatAddr(clus);
	SetDiskFilePointer(NULL, retAddr + nIndex * 32, NULL, FILE_BEGIN);
	ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		

	if(dirRw.DIR_Name[0] == 0x00)
		return FALSE;

	Fat_Directory temDir;
	DirInfoFromRwInfo(temDir, dirRw);

	while(IsLongDir(temDir.DIR_Attr)&& (temDir.DIR_Name[0]!= (char)0xE5))
	{
		nIndex ++;	
		SetDiskFilePointer(NULL, retAddr + nIndex * 32, NULL, FILE_BEGIN);
		ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
		if(dirRw.DIR_Name[0] == 0x00)
			return FALSE;
	}
	nIndex++;
	DirInfoFromRwInfo(dir,  dirRw);
	return TRUE;

}

BOOL DiskImgFile::GetDirectoryTabEx(OUT Fat_Directory& dir, 
									IN UINT clus, 
									OUT LPTSTR lptLongName, 
									IN OUT INT& nIndex) // 返回簇clus下目录下第nIndex个目录			
{
	Fat_DirectoryRW dirRw;
	DWORD nRead;
	int  ns = _curDirectory.size();
	UINT retAddr = 0;
	UINT maxDirCount = 0;
	int iEntryCount = 0;
	dir.DIR_EntryCount = iEntryCount;

	if(clus == 0)
	{	
		maxDirCount = 512;
		retAddr = _stOfRootAddr;

		//check if ATTR_VOLUME_ID dir tab
		if (nIndex == 0 )
		{
			SetDiskFilePointer(_hFile, retAddr , NULL, FILE_BEGIN);
			ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);	
			if (dirRw.DIR_Attr == ATTR_VOLUME_ID)
			{
				nIndex++;
			}
		}
	}
	else
	{
	//	ASSERT(nIndex>=2);
	//	if(nIndex <= 2)
		//	nIndex = 2;

		int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
		int nCluIndex = 0;  // 记录在该目录的第 几个簇下操作
		nCluIndex = nIndex / recPerCluster;

		UINT retAddr = ClusterToRelatAddr(clus);
		WORD curClu = clus;	
		int retIndex = 	nIndex % recPerCluster;	


		for(int  i = 0; i < nCluIndex; i++)
		{
			curClu = _fats.at(curClu);
			if(curClu == m_EOC_STATUS)
				return FALSE;
			retAddr = ClusterToRelatAddr(curClu);
		}
	//	retAddr = ClusterToRelatAddr(clus);
		maxDirCount = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;

		SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);		
		ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);	
		
		retIndex++;
		nIndex++;

		if(dirRw.DIR_Name[0] == 0x00)
			return FALSE;

	//	Fat_Directory temDir;
	//	DirInfoFromRwInfo(temDir, dirRw);

		if(dirRw.DIR_Name[0] == (char)0xE5)
		{
			while(1)
			{

				if(retIndex >= recPerCluster)
				{
					curClu = _fats.at(curClu);
					if(curClu == m_EOC_STATUS)
						return FALSE;
					retIndex = 0;
					retAddr = ClusterToRelatAddr(curClu);
				}

				SetDiskFilePointer(NULL ,  retAddr + retIndex * 32, NULL, FILE_BEGIN);
				ReadDiskFile(NULL ,  &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
				retIndex++;
				nIndex++;

				
				if(dirRw.DIR_Name[0] == 0x00)
					return FALSE;
				if(dirRw.DIR_Name[0] != (char)0xE5)
					break;
			}			
		}

		if((IsFile(dirRw.DIR_Attr) ||IsFoulder(dirRw.DIR_Attr)) && (dirRw.DIR_Name[0]!= (char)0xE5))
		{
			char lpLName[MAX_PATH];
			ZeroMemory(lpLName, MAX_PATH);
			memcpy(lpLName, dirRw.DIR_Name, 8);
			if(IsFile(dirRw.DIR_Attr))
			{
				lpLName[8] = '.';
				memcpy(lpLName + 9,dirRw.DIR_Name+8, 3);
			}

#ifdef _UNICODE			
			int nw = MultiByteToWideChar(CP_ACP, 0, lpLName, -1, NULL, 0);
			MultiByteToWideChar(CP_ACP, 0, lpLName, -1, lptLongName, nw);
#else
			strcpy(lptLongName , lpLName);
#endif

			RemoveTChar(lptLongName, ' ');
			DirInfoFromRwInfo(dir,  dirRw);


			dir.DIR_PosIndex = nIndex - 1;
			iEntryCount++;
			dir.DIR_EntryCount = iEntryCount;
		//	nIndex++;
			return TRUE;
		}

		BOOL bLongNameChkSumMatch = TRUE;
		BYTE* bTemp = (BYTE*)&dirRw;
		BYTE chksumbyte_l = *(bTemp+0x0D);
		
		while(IsLongDir(dirRw.DIR_Attr) && (dirRw.DIR_Name[0]!= (char)0xE5))
		{
			Fat_LongDirectory longDir;
			LongDirInfoFromRwInfo(longDir, dirRw);

			if( chksumbyte_l != longDir.LDIR_Chksum && bLongNameChkSumMatch)
				bLongNameChkSumMatch = FALSE;				
			
			if (bLongNameChkSumMatch)
			{
#ifdef _UNICODE
				WCHAR wTmp[14];
				WCHAR wLTmp[MAX_PATH];
				ZeroMemory(wTmp, 28);
				ZeroMemory(wLTmp, MAX_PATH*2);
    
				memcpy(wTmp, longDir.LDIR_Name1, 26);
				wcscat(wLTmp, wTmp);
				wcscat(wLTmp, lptLongName);
				wcscpy(lptLongName, wLTmp);
#else
				char pchar[MAX_PATH];
				memset(pchar, 0x00, MAX_PATH);
				int nWidth = WideCharToMultiByte(m_CodePage, 0, longDir.LDIR_Name1, 13, pchar, MAX_PATH, "", FALSE);
				LPSTR lpTem = GetStrFromChArry(pchar, MAX_PATH);
				strcat(lpTem, lptLongName);
				strcpy(lptLongName, lpTem);
				delete lpTem;			
#endif
			}
			iEntryCount++;

			if(retIndex >= recPerCluster)
			{
				curClu = _fats.at(curClu);
				if(curClu == m_EOC_STATUS)
					return FALSE;
				retIndex = 0;
				retAddr = ClusterToRelatAddr(curClu);
			}
			SetDiskFilePointer(NULL, retAddr + retIndex * 32, NULL, FILE_BEGIN);
			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
			if(dirRw.DIR_Name[0] == 0x00)
				return FALSE;
			if((IsFoulder(dirRw.DIR_Attr)||IsFile(dirRw.DIR_Attr)) && dirRw.DIR_Name[0]!= (char)0xE5)
			{
				BYTE chksumbyte_s = ChkSum(dirRw.DIR_Name);
				
				if (chksumbyte_s!=chksumbyte_l || !bLongNameChkSumMatch )//long name check sum is not match the short name
				{
#ifdef _UNICODE
					char szShortName[16];
					char filename[9];
					memcpy(filename , dirRw.DIR_Name , 8);
					filename[8] = 0x00;
					strcpy(szShortName , filename);
					strcat(szShortName , ".");
					
					char ext[4];
					memcpy(ext , dirRw.DIR_Name+8 , 3);
					ext[3] = 0x00;					
					strcat(szShortName , ext);

					int nw = MultiByteToWideChar(CP_ACP, 0, szShortName, -1, NULL, 0);
					MultiByteToWideChar(CP_ACP, 0, szShortName, -1, lptLongName, nw);

					RemoveTChar(lptLongName , ' ');
#else	

					char filename[9];
					memcpy(filename , dirRw.DIR_Name , 8);
					filename[8] = 0x00;
					RemoveTChar(filename , ' ');
					strcpy(lptLongName , filename);
					strcat(lptLongName , ".");
					
					char ext[4];
					memcpy(ext , dirRw.DIR_Name+8 , 3);
					ext[3] = 0x00;
					RemoveTChar(ext , ' ');
					strcat(lptLongName , ext);
#endif		
				}
				iEntryCount++;
				break;				
			}
			nIndex ++;	
			retIndex++;
		}
		DirInfoFromRwInfo(dir,  dirRw);
		dir.DIR_PosIndex = nIndex;
		nIndex++;
		dir.DIR_EntryCount= iEntryCount;
		return TRUE;
	}


	if(nIndex >= maxDirCount)
		return FALSE;



	SetDiskFilePointer(NULL, retAddr + nIndex * 32, NULL, FILE_BEGIN);
	ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		

	if(dirRw.DIR_Name[0] == 0x00)
		return FALSE;

//	Fat_Directory temDir;
//	DirInfoFromRwInfo(temDir, dirRw);

	if(dirRw.DIR_Name[0] == (char)0xE5)
	{
		while(1)
		{
			SetDiskFilePointer(NULL, retAddr + nIndex * 32, NULL, FILE_BEGIN);
			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
			if(dirRw.DIR_Name[0] == 0x00)
				return FALSE;
			if(dirRw.DIR_Name[0] != (char)0xE5)
				break;
			nIndex++;
		}			
	}

	if((IsFile(dirRw.DIR_Attr) ||IsFoulder(dirRw.DIR_Attr)) && (dirRw.DIR_Name[0]!= (char)0xE5))
	{
		char lpLName[MAX_PATH];
		ZeroMemory(lpLName, MAX_PATH);
		memcpy(lpLName, dirRw.DIR_Name, 8);
		if(IsFile(dirRw.DIR_Attr))
		{
			lpLName[8] = '.';
			memcpy(lpLName + 9,dirRw.DIR_Name+8, 3);
		}

#ifdef _UNICODE			
		int nw = MultiByteToWideChar(CP_ACP, 0, lpLName, -1, NULL, 0);
		MultiByteToWideChar(CP_ACP, 0, lpLName, -1, lptLongName, nw);
#else
		strcpy(lptLongName , lpLName);
#endif

		RemoveTChar(lptLongName, ' ');
		DirInfoFromRwInfo(dir,  dirRw);

		dir.DIR_PosIndex = nIndex;
		nIndex++;
		iEntryCount++;
		dir.DIR_EntryCount = iEntryCount;
		return TRUE;
	}

	BOOL bLongNameChkSumMatch = TRUE;
	BYTE* bTemp = (BYTE*)&dirRw;
	BYTE chksumbyte_l = *(bTemp+0x0D);

	while(IsLongDir(dirRw.DIR_Attr) && (dirRw.DIR_Name[0]!= (char)0xE5) && nIndex < maxDirCount)
	{
		Fat_LongDirectory longDir;
		LongDirInfoFromRwInfo(longDir, dirRw);

		if( chksumbyte_l != longDir.LDIR_Chksum && bLongNameChkSumMatch)
				bLongNameChkSumMatch = FALSE;	
		
		if (bLongNameChkSumMatch)
		{
#ifdef _UNICODE
			WCHAR wTmp[14];
			WCHAR wLTmp[MAX_PATH];
			ZeroMemory(wTmp, 28);
			ZeroMemory(wLTmp, MAX_PATH*2);

			memcpy(wTmp, longDir.LDIR_Name1, 26);
			wcscat(wLTmp, wTmp);
			wcscat(wLTmp, lptLongName);
			wcscpy(lptLongName, wLTmp);
#else
			char pchar[MAX_PATH];
			memset(pchar, 0x00, MAX_PATH);
			int nWidth = WideCharToMultiByte(m_CodePage, 0, longDir.LDIR_Name1, 13, pchar, MAX_PATH, "", FALSE);
			LPSTR lpTem = GetStrFromChArry(pchar, MAX_PATH);
			strcat(lpTem, lptLongName);
			strcpy(lptLongName, lpTem);
			delete lpTem;			
#endif	
		}
		iEntryCount++;
		nIndex ++;	
		SetDiskFilePointer(NULL, retAddr + nIndex * 32, NULL, FILE_BEGIN);
		ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
		if(dirRw.DIR_Name[0] == 0x00)
			return FALSE;
		if( (IsFoulder(dirRw.DIR_Attr)||IsFile(dirRw.DIR_Attr)) && dirRw.DIR_Name[0]!= (char)0xE5)
		{
			BYTE chksumbyte_s = ChkSum(dirRw.DIR_Name);
			
			if (chksumbyte_s!=chksumbyte_l || !bLongNameChkSumMatch)//long name check sum is not match the short name
			{
#ifdef _UNICODE
					char szShortName[16];
					char filename[9];
					memcpy(filename , dirRw.DIR_Name , 8);
					filename[8] = 0x00;
					strcpy(szShortName , filename);
					strcat(szShortName , ".");
					
					char ext[4];
					memcpy(ext , dirRw.DIR_Name+8 , 3);
					ext[3] = 0x00;					
					strcat(szShortName , ext);

					int nw = MultiByteToWideChar(CP_ACP, 0, szShortName, -1, NULL, 0);
					MultiByteToWideChar(CP_ACP, 0, szShortName, -1, lptLongName, nw);

					RemoveTChar(lptLongName , ' ');
#else	

					char filename[9];
					memcpy(filename , dirRw.DIR_Name , 8);
					filename[8] = 0x00;
					RemoveTChar(filename , ' ');
					strcpy(lptLongName , filename);
					strcat(lptLongName , ".");
					
					char ext[4];
					memcpy(ext , dirRw.DIR_Name+8 , 3);
					ext[3] = 0x00;
					RemoveTChar(ext , ' ');
					strcat(lptLongName , ext);
#endif		
			}
			iEntryCount++;
			break;
		}
		
	}
	DirInfoFromRwInfo(dir,  dirRw);
	dir.DIR_PosIndex = nIndex;
	nIndex++;
	dir.DIR_EntryCount = iEntryCount;
	return TRUE;

}

BOOL DiskImgFile::GetDirectoryTab(OUT Fat_Directory& dir, IN UINT clus, IN LPCTSTR longName)						 // 返回簇clus下目录下长名为longName的目录
{
	int nIndex = 0;
	TCHAR lpTemp[MAX_PATH];
	
	while(1)
	{
		memset(lpTemp, 0x00, MAX_PATH * sizeof(TCHAR) );
	
		if(!GetDirectoryTabEx(dir, clus, lpTemp, nIndex))
			return FALSE;
		if(strcmpnocase(lpTemp, longName) == 0)
			return TRUE;
	}

	return FALSE;
}

BOOL DiskImgFile::SetVolLabel(LPCSTR lpVolLabel)
{
	UINT retAddr = _stOfRootAddr;
	DWORD nWrite, nRead;
	Fat_DirectoryRW dirRw;
	SetDiskFilePointer(NULL, retAddr, NULL, FILE_BEGIN);
	int i;
	for( i = 0; i < ROOTENTCNT; i++)
	{
		if(!ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL))
		{
			printmsg("Error in SetVolLabel --- read dirEntry");
			return FALSE;
		}
		if(dirRw.DIR_Name[0] == (char)0xE5)
			continue;
		if(dirRw.DIR_Name[0] == 0X00 && i > 0)
			break;
		if(IsVolLabel(dirRw.DIR_Attr))
		{
			memset(dirRw.DIR_Name, ' ', 11);
			int nLen = strlen(lpVolLabel);
			memcpy(dirRw.DIR_Name, lpVolLabel, nLen);
			SetDiskFilePointer(NULL, _stOfRootAddr + i*32, NULL, FILE_BEGIN);
			WriteDiskFile(_hFile, dirRw.DIR_Name, 11, &nWrite, NULL);
			SetDiskFilePointer(NULL, 43, NULL, FILE_BEGIN);	// 根据BPB的偏移量来确定
			WriteDiskFile(_hFile, dirRw.DIR_Name, 11, &nWrite, NULL);
			return TRUE;
		}
	}

	if(i < ROOTENTCNT)
	{
		Fat_Directory dir;
		Fat_DirectoryRW dirRw;
		DWORD nWritten;
		dir.DIR_Attr = ATTR_VOLUME_ID;
		::GetSystemTime(&dir.DIR_WrtDateTime);
		::GetSystemTime(&dir.DIR_CrtDateTime);
		::GetSystemTime(&dir.DIR_LstAcceDate);
		dir.DIR_FstClusHI = 0;
		dir.DIR_FstClusLO = 0;
		dir.DIR_FileSize = 0;
		memcpy(dir.DIR_Name, lpVolLabel, 11);
		RwInfoFromDirInfo(dirRw, dir);
		SetDiskFilePointer(NULL, retAddr + 32*i, NULL, FILE_BEGIN);
		WriteDiskFile(_hFile, &dirRw, 32, &nWritten, NULL);
	}
	
	return FALSE;

}

LPTSTR DiskImgFile::RootPath()
{
	return "\\";
}

DWORD DiskImgFile::SectorsPerCluster()
{
	return _imgBpb.BPB_SecPerClus;
}

DWORD DiskImgFile::BytesPerSector()
{
	return _imgBpb.BPB_BytsPerSec;
}

DWORD DiskImgFile::NumberOfFreeClusters()
{
	UINT nFreeSum= 0;
	int ns = _fats.size();
	for(int i = 0; i < ns; i++)
	{
		if(_fats.at(i) == FREE_STATUS)
			nFreeSum++;
	}
	return (DWORD)nFreeSum;
}

BOOL DiskImgFile::GetVolLabel(LPSTR lpLabel)
{
	memset(lpLabel, 0x00, 12);
	memcpy(lpLabel, _imgBpb.BS_VolLab, 11);

	UINT retAddr = _stOfRootAddr;
	DWORD nRead;
	Fat_DirectoryRW dirRw;
	SetDiskFilePointer(NULL, retAddr, NULL, FILE_BEGIN);
	UINT maxcnt = _imgBpb.BPB_BytsPerSec * _imgBpb.BPB_SecPerClus / 32;
	for(int i = 0; i < maxcnt; i++)
	{
		if(!ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL))
		{
			printmsg("Error in SetVolLabel --- read dirEntry");
		}
		if(dirRw.DIR_Name[0] == (char)0xE5)
			continue;
		if(dirRw.DIR_Name[0] == 0X00 && i > 0)
			return FALSE;
		if(IsVolLabel(dirRw.DIR_Attr))
		{
			memcpy(lpLabel, dirRw.DIR_Name, 11);
			TrimString(lpLabel, FALSE);
			return TRUE;
		}	
	}	

	TrimString(lpLabel, FALSE);
	return true;


/*
		DWORD nRead;
		memset(lpLabel, 0x00, 12);
		SetDiskFilePointer(NULL, 43, NULL, FILE_BEGIN);
		ReadDiskFile(NULL, lpLabel, 11, &nRead, NULL);
		RemoveChar(lpLabel, ' ');
		return true;*/
	
}

DWORD DiskImgFile::TotalNumberOfClusters()
{
	return _fats.size();
}
	
BOOL  DiskImgFile::TrimDirEntrySpace()
{
	Fat_DirectoryRW dirRw;
	DWORD nRead;
	int  ns = _curDirectory.size();

	vector<Fat_DirectoryRW> dirs;
	dirs.resize(0);
	
	UINT nIndex = 0;
	if(ns == 0) // 在根目录下
	{
		UINT retAddr = _stOfRootAddr;
		SetDiskFilePointer(NULL, retAddr , NULL, FILE_BEGIN);
	
		while (1)
		{
			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);			
			if(dirRw.DIR_Name[0] == 0x00)
				break;
			if(dirRw.DIR_Name[0] != (char)0xE5)
			{
				int ds  = dirs.size();
				dirs.resize(ds + 1);
				memcpy(&dirs.at(ds), &dirRw, sizeof(dirRw));
			}
			nIndex++;			
		}
		
		int ds = dirs.size();
		SetDiskFilePointer(NULL, retAddr, NULL, FILE_BEGIN);
		int i;
		for( i = 0; i < ds; i++)
		{
			WriteDiskFile(_hFile, &dirs.at(i), sizeof(Fat_DirectoryRW), &nRead, NULL);
		}
		char blank[32];
		memset(blank, 0x00, 32);

		for(i = 0; i < (nIndex - ds); i++)
		{
			WriteDiskFile(_hFile, blank, 32, &nRead, NULL);
		}
		
	}

	else
	{
		return TRUE;//no need to trim direntry space if not in rootpath
		UINT curClu =  _curDirectory.at(ns - 1).DIR_FstClusLO;
		UINT retAddr =  ClusterToRelatAddr(curClu);
		UINT maxCntPerClu = BytesPerSector()*SectorsPerCluster()/32;

		UINT retIndex = 0;
		nIndex = 0;
		SetDiskFilePointer(NULL, retAddr , NULL, FILE_BEGIN);
	
		while (1)
		{
			if(retIndex >= maxCntPerClu)
			{
				curClu = _fats.at(curClu);
				if(curClu == m_EOC_STATUS)
					break;
				retAddr = ClusterToRelatAddr(curClu);
				SetDiskFilePointer(NULL, retAddr, NULL, FILE_BEGIN);
			}

			ReadDiskFile(NULL, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);			
			if(dirRw.DIR_Name[0] == 0x00)
				break;
			if(dirRw.DIR_Name[0] != (char)0xE5)
			{
				int ds  = dirs.size();
				dirs.resize(ds + 1);
				memcpy(&dirs.at(ds), &dirRw, sizeof(dirRw));
			}
			retIndex++;
			nIndex++;			
		}
		
		int ds = dirs.size();
		curClu =  _curDirectory.at(ns - 1).DIR_FstClusLO;
		retAddr =  ClusterToRelatAddr(curClu);
		SetDiskFilePointer(NULL, retAddr, NULL, FILE_BEGIN);
		retIndex = 0;
		int i;
		for( i = 0; i < ds; i++)
		{
			if(retIndex >= maxCntPerClu)
			{
				curClu = _fats.at(curClu);
				if(curClu == m_EOC_STATUS)
					break;
				retAddr = ClusterToRelatAddr(curClu);
				SetDiskFilePointer(NULL, retAddr, NULL, FILE_BEGIN);
			}
			WriteDiskFile(_hFile, &dirs.at(i), sizeof(Fat_DirectoryRW), &nRead, NULL);
			retIndex++;
		}
		UINT endClu = curClu;
	
		char blank[32];
		memset(blank, 0x00, 32);
		
		for(i = 0; i < (nIndex - ds); i++)
		{
			if(retIndex >= maxCntPerClu)
			{
				curClu = _fats.at(curClu);
				if(curClu == m_EOC_STATUS)
					break;
				retAddr = ClusterToRelatAddr(curClu);
				SetDiskFilePointer(NULL, retAddr, NULL, FILE_BEGIN);
			}		
			WriteDiskFile(_hFile, blank, 32, &nRead, NULL);
			retIndex++;
		}

		curClu = _fats.at(endClu);

		SetClus(endClu, m_EOC_STATUS);
		
		while(curClu != m_EOC_STATUS)
		{
			UINT temp = _fats.at(curClu);
			SetClus(curClu, FREE_STATUS);
			curClu = temp;
		}		

	}
	return TRUE;
	
}


BOOL DiskImgFile::MoveFileEx(LPCSTR lpSrcName, LPCSTR lpNewFileName)
{
	return TRUE;
// 	LPSTR lpSrcFileName = new char[MAX_PATH];
// 	memset(lpSrcFileName, 0x00, MAX_PATH);
// 	if(!GetRighStrByFind(lpSrcFileName, lpSrcName, '\\', -1, ""))
// 	{
// 		delete lpSrcFileName;
// 		return TRUE;
// 	}
// 
// 	LPSTR lpDesFileName = new char[MAX_PATH];
// 	memset(lpDesFileName, 0x00, MAX_PATH);
// 	if(!GetRighStrByFind(lpDesFileName, lpNewFileName, '\\', -1, ""))
// 	{
// 		delete lpDesFileName;
// 		return TRUE;
// 	}
// 
// 	LPSTR lpsrcDir = GetLeftStr(lpSrcName, '\\', FALSE);
// 	if(lpsrcDir == NULL)
// 	{
// 		lpsrcDir = new char[MAX_PATH];
// 		memset(lpsrcDir, 0x00, MAX_PATH);
// 		strcat(lpsrcDir, "\\");
// 	}
// 
// 	LPSTR lpdesDir = GetLeftStr(lpNewFileName, '\\', FALSE);
// 	if(lpdesDir == NULL)
// 	{
// 		lpdesDir = new char[MAX_PATH];
// 		memset(lpdesDir, 0x00, MAX_PATH);
// 		strcat(lpdesDir, "\\");
// 	}
// 
// 	if(!ImgMoveFile(lpSrcFileName, lpsrcDir, lpDesFileName, lpdesDir))
// 	{
// 		delete lpsrcDir;
// 		delete lpdesDir;
// 		delete lpDesFileName;
// 		delete lpSrcFileName;
// 		return FALSE;
// 	}
// 
// 	delete lpsrcDir;
// 	delete lpdesDir;
// 	delete lpDesFileName;
// 	delete lpSrcFileName;
// 
// 	return TRUE;
}

HANDLE DiskImgFile::FindFirstFile(LPCTSTR lptFileName,					// pointer to name of file to search for
		LPWIN32_FIND_DATA lpFindFileData	// pointer to returned information
		)
{
	ImgFindHandle* fh = new ImgFindHandle;
	fh->_nIndex = 0;
	fh->_handle = (HANDLE)fh;

	if(_curDirectory.size() == 0)
		fh->_curCluster = 0; // 当前簇的编号
	else
	{
		UINT cc = _curDirectory.at(_curDirectory.size() - 1).DIR_FstClusHI;
		cc = cc<<16;
		cc = cc|_curDirectory.at(_curDirectory.size() - 1).DIR_FstClusLO;
		fh->_curCluster = cc;
	}

	fh->_nRetIndex = 0; // 簇内当前索引


	memset(&fh->_curFileTab, 0x00, sizeof(Fat_Directory));
	memset(&fh->_findData, 0x00, sizeof(WIN32_FIND_DATA));
	if (lptFileName == NULL)
		lptFileName = "*.*";
	if(_tcsstr((LPCTSTR)lptFileName, (LPCTSTR)"*") != NULL)
		fh->_findData.dwReserved0 = 1;			// 表示找相似like
	lstrcpy((LPTSTR)fh->_findData.cFileName, lptFileName);
	memset(fh->_findData.cAlternateFileName, 0x00, 14 * sizeof(TCHAR));
	lstrcpy((LPTSTR)fh->_findData.cAlternateFileName, lptFileName);
	_tcsupr((LPTSTR)fh->_findData.cAlternateFileName);
//	memcpy(lpFindFileData, &fh->_findData, sizeof(WIN32_FIND_DATA));

	return (HANDLE)fh;
}
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
BOOL DiskImgFile::FindNextFile(
		HANDLE hFindFile,					// handle to search
		LPWIN32_FIND_DATA lpFindFileData	// pointer to structure for data on found file
		) 
{
//#ifdef _WINDOWS
#ifdef wbt
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#endif
	UINT maxdircnt = _imgBpb.BPB_BytsPerSec*_imgBpb.BPB_SecPerClus / 32;
	ImgFindHandle* fh =  (ImgFindHandle*)hFindFile;
	if(fh->_curCluster == m_EOC_STATUS)
	{
		return FALSE;
	}
	//UINT retAddr = ClusterToRelatAddr(fh->_curCluster);
	Fat_Directory dir;
	LPTSTR longName = new TCHAR[MAX_PATH];
	LPTSTR cmpStr = new TCHAR[MAX_PATH];
	memset(cmpStr, 0x00, MAX_PATH * sizeof(TCHAR));
	memcpy(cmpStr, fh->_findData.cAlternateFileName, 14 * sizeof(TCHAR));
//	RemoveChar(cmpStr, '.');
	RemoveTChar(cmpStr, ' ');
	RemoveTChar(cmpStr, '*');
	RemoveTChar(cmpStr, '?');
	
	int cmpLen = strlen(cmpStr);
	int n = fh->_nRetIndex ;
	int k = 0;
	while (1)
	{
		k++;
		memset(longName, 0x00, MAX_PATH * sizeof(TCHAR));
	
		if(!GetDirectoryTabEx(dir, fh->_curCluster,longName, n))
		{
			delete[] longName;
			delete[] cmpStr;
			return FALSE;
		}
		fh->_nRetIndex = n;
		memset(fh->_findData.cFileName, 0x00, MAX_PATH * sizeof(TCHAR));
		lstrcpy((LPTSTR)fh->_findData.cFileName ,longName);
		_tcsupr((LPTSTR)longName);
		fh->_findData.dwFileAttributes = dir.DIR_Attr;
		//fh->_findData.ftCreationTime = 
		fh->_findData.nFileSizeHigh = 0;
		fh->_findData.nFileSizeLow = dir.DIR_FileSize;

		SYSTEMTIME sysTm;
		FILETIME flTime;
		sysTm = dir.DIR_CrtDateTime;
		SystemTimeToFileTime(&sysTm, &flTime);
		LocalFileTimeToFileTime(&flTime, &fh->_findData.ftCreationTime);
		
		sysTm = dir.DIR_CrtDateTime;
		SystemTimeToFileTime(&sysTm, &flTime);
		LocalFileTimeToFileTime(&flTime, &fh->_findData.ftLastWriteTime);
		
		sysTm = dir.DIR_CrtDateTime;
		SystemTimeToFileTime(&sysTm, &flTime);
		LocalFileTimeToFileTime(&flTime, &fh->_findData.ftLastAccessTime);//FileTimeToLocalFileTime

		memcpy(lpFindFileData, &fh->_findData, sizeof(WIN32_FIND_DATA));

		if(fh->_nRetIndex >= maxdircnt)
		{
			if(fh->_curCluster == 0)
			{
				delete[] longName;
				delete[] cmpStr;
				return FALSE;
			}
			fh->_curCluster = _fats.at(fh->_curCluster);
		/*	if(fh->_curCluster == m_EOC_STATUS)
			{
				delete longName;
				delete cmpStr;
				return FALSE;
			}
		*/
			//fh->_nRetIndex = 0;
			fh->_nRetIndex -= maxdircnt;
			n = 0;
		}
		fh->_nIndex++;
	/*	if(fh->_findData.dwReserved0 == 1)
		{
			if(_mbsstr(longName, cmpStr) == NULL)
				continue;		
			break;
		}
		else
		{
			if(strcmp(longName, cmpStr) != 0 && cmpLen >= 1)
			{
				continue;
			}
			break;
		}
		*/
		break;
	}
	delete[] longName;
	delete[] cmpStr;
	return TRUE;	
}

BOOL DiskImgFile::FindClose(HANDLE hFindFile)
{
	ImgFindHandle* fh =  (ImgFindHandle*)hFindFile;	
	delete fh;
	return TRUE;
}


UINT DiskImgFile::ImgFileType()
{
	return FatType();
}

LPTSTR DiskImgFile::GetMirrorFileName()
{
	return _lpImgFileName;
}

LONGLONG DiskImgFile::TotoleSpace()
{
	return (LONGLONG)this->TolSec() * this->BytesPerSec();
}

unsigned short DiskImgFile::crc16(char *crcarray,int Length)
{
	
	unsigned short crcbyte=0xffff;
	
	unsigned char tempbyte;
	
	int i,j;	
	
	for (i=0;i<Length;i++)		
	{
		
		tempbyte=((unsigned char)(crcbyte%256))^((unsigned char)crcarray[i]);
		
		crcbyte=crcbyte-(crcbyte%256)+tempbyte;
		
		for (j=0;j<8;j++)			
		{
			
			if ((crcbyte&0x0001)==1) crcbyte=0xA001^(crcbyte>>1);
			
			else crcbyte=crcbyte>>1;			
		}		
	}
	
	return crcbyte;	
};

void	DiskImgFile::GetCRCStr(char* src , UINT srclen , char* CRCStr)//CRCStr为4个BYTE
{
	unsigned short ret = crc16(src , srclen);
	sprintf(CRCStr , "%04X" , ret);
}

int  DiskImgFile::strcmpnocase(LPCTSTR string1,LPCTSTR string2 )
{
	return strcasecmp(string1 , string2);
}

BOOL	DiskImgFile::CheckFatType(IN LPCSTR lpFileName , OUT UINT& FATTYPE)
{
	int ft = 0;
	BOOL ftUnMatch = FALSE;
	BOOL ret = FALSE;

	if ( !CreateDiskFile(lpFileName) ) return FALSE;

	SetDiskFilePointer(NULL, 0, NULL, FILE_BEGIN);	
	DWORD nRead;

	//Get NonSecurity Area Boot sector
	BootSector_BPB_RW bpbRw_1;
	ReadDiskFile(NULL, &bpbRw_1, sizeof(BootSector_BPB_RW), &nRead, NULL);
	
	if(nRead != sizeof(BootSector_BPB_RW))
	{
		CloseDiskFile();	
		return FALSE;
	}

	BootSector_BPB imgBpb16_1;
	BootSector_BPB32 imgBpb32_1;

	BpbCobyFromRwInfo(imgBpb16_1, bpbRw_1);

	if(nRead != sizeof(BootSector_BPB_RW))
	{		
		return FALSE;
	}

		
	BpbCobyFromRwInfo(imgBpb16_1, bpbRw_1);
	Bpb32CobyFromRwInfo(imgBpb32_1, bpbRw_1);

	DWORD FATSz = 0;
	DWORD TotSec = 0;
	if(imgBpb16_1.BPB_FATSz16 != 0)
		FATSz = imgBpb16_1.BPB_FATSz16;
	else			
		FATSz = imgBpb32_1.BPB_FATSz32;
	

	if(imgBpb16_1.BPB_TotSec16 != 0)
		TotSec = imgBpb16_1.BPB_TotSec16;
	else
		TotSec = imgBpb16_1.BPB_TotSec32;

	UINT DataSec = TotSec - (imgBpb16_1.BPB_RsvdSecCnt + (imgBpb16_1.BPB_NumFATs * FATSz) + imgBpb16_1.BPB_RootEntCnt);	
	UINT CountofClusters = DataSec / imgBpb16_1.BPB_SecPerClus;

	if(CountofClusters < 4085) {
		/* Volume is FAT12 */
		ft = FAT12_TYPE;
		ret = FALSE;
	} else if(CountofClusters < 65525) {
		/* Volume is FAT16 */
		ft = FAT16_TYPE;
		ret = TRUE;
	} else {
		/* Volume is FAT32 */
		ft =  FAT32_TYPE;
		ret = TRUE;
	}	
	
	
	FATTYPE = ft;
	CloseDiskFile();
	return ret;

}

BOOL DiskImgFile::TestFunc()
{
	return FALSE;
}

BOOL DiskImgFile::RefreshFat12Table()
{
	int nFats = _fats.size();
	DWORD nWritten = 0;
	int nFatBytes = _imgBpb.BPB_FATSz16*BYSPERSEC; 
	BYTE *fs=new BYTE[nFatBytes];
	memset(fs,0x0,nFatBytes);

	int iFatOffset, iThisFatEntOffset;
	WORD wFAT12ClusEntryVal;

	for (int i = 0; i<nFats; i++)
	{
		iFatOffset= i+i/2;
		iThisFatEntOffset= iFatOffset/*%BYSPERSEC*/; //取余操作错误
		
		wFAT12ClusEntryVal = _fats.at(i); //65528=0xfff8

		if(i & 0x0001)
		{
			wFAT12ClusEntryVal = wFAT12ClusEntryVal<<4;	 			
		}		
		*((WORD*) &fs[iThisFatEntOffset]) = (*((WORD*) &fs[iThisFatEntOffset]))| (WORD)wFAT12ClusEntryVal; /* Cluster number is EVEN *///4088=0xff8
		
	//	*((WORD*) &fs[iThisFatEntOffset]) = (*((WORD*) &fs[iThisFatEntOffset])) | (WORD)wFAT12ClusEntryVal;
	}

	
	SetDiskFilePointer(_hFile , _stOfFATAddr , 0 , FILE_BEGIN);	
	if (!WriteDiskFile((HANDLE)_hFile, fs, nFatBytes*sizeof(BYTE), &nWritten, NULL))
	{
		delete []fs;			
		return FALSE;		
	}
	
	if (!WriteDiskFile((HANDLE)_hFile, fs, nFatBytes*sizeof(BYTE), &nWritten, NULL))
	{
		delete []fs;			
		return FALSE;		
	}
	
	
	delete []fs;	
	
	return TRUE;
}

void DiskImgFile::IniFatType(UINT fatType)
{
	if (fatType==FAT12_TYPE)
	{
		m_EOC_STATUS = EOC_STATUS12;
		m_FatType=FAT12_TYPE;
	}
	else
	{
		m_EOC_STATUS = EOC_STATUS16;
		m_FatType=FAT16_TYPE;
	}
}
