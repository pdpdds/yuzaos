#include "DiskImgFile32.h"

#define  EOC_STATUS32  (DWORD)0x0FFFFFFF
#define  FREE_STATUS32 (DWORD)0x00000000




extern DskszToSecperClus DskTableFAT32[];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DiskImgFile32::DiskImgFile32()
{
//	_lpImgFileName = new char[MAX_PATH];
	memset(_lpImgFileName, 0x00, MAX_PATH);
	_imgSpace = DISK32_32M;
	_imgContent = NULL;
	_curDirectory.clear();
	_hFile = NULL;

#ifdef _MAC_UNIX
	m_CodePage = 0;
#else
	m_CodePage = CP_ACP;
#endif

}

DiskImgFile32::~DiskImgFile32()
{


	_fats.resize(0);
}



UINT  DiskImgFile32::RelatAddrToCluster(IN UINT uRetAddr)						// 映射函数从实际实际偏移位置到簇
{
	//_startClusterAddr = _imgBpb.BPB_BytsPerSec + _imgBpb.BPB_FATSz16*_imgBpb.BPB_BytsPerSec*2 - 32*_imgBpb.BPB_BytsPerSec;

	UINT relAddrk = uRetAddr - _stOfClusterAddr;
	
	return (relAddrk / _imgBpb.BPB_BytsPerSec/_imgBpb.BPB_SecPerClus + 2);
	
}

UINT  DiskImgFile32::ClusterToRelatAddr(IN UINT uCluNm)						// 映射函数从簇到实际偏移位置
{
	if(uCluNm == 0)
		return _stOfRootAddr;
	UINT relAddr = (uCluNm - 2 )* _imgBpb.BPB_BytsPerSec * _imgBpb.BPB_SecPerClus;
	return relAddr + _stOfClusterAddr;
}



UINT  DiskImgFile32::GetFirstFreeClusNum() // 返回空闲的簇号，从2开始
{
	int nsFat = _fats.size();
	for(int i = 0; i < nsFat; i++)
	{
		DWORD ws = _fats.at(i);
		if((ws&0x0FFFFFFF) == (DWORD)0x00000000)
		{
			return i;
		}
	}
	return 0;
}

BOOL DiskImgFile32::GetDirectoryTabEx(OUT Fat_Directory& dir, 
									IN UINT clus, 
									OUT LPTSTR lptLongName, 
									IN OUT INT& nIndex) // 返回簇clus下目录下第nIndex个目录			

{
	//printf("GetDirectoryTabEx 1\n");
	Fat_DirectoryRW dirRw;
	DWORD nRead;
	int  ns = _curDirectory.size();
	UINT retAddr = 0;
	UINT maxDirCount = 0;
	int iEntryCount = 0;
	if(clus == 0)
	{
		clus = _imgBpb.BPB_RootClus;		
	}

	int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
	int nCluIndex = 0;  // 记录在该目录的第 几个簇下操作
	nCluIndex = nIndex / recPerCluster;

	retAddr = ClusterToRelatAddr(clus);
	DWORD curClu = clus;	
	int retIndex = 	nIndex % recPerCluster;	


	for(int  i = 0; i < nCluIndex; i++)
	{
		curClu = _fats.at(curClu);
		if(curClu == EOC_STATUS32)	return FALSE;
		if(ns == 0 && curClu == 0xffffffff)	return FALSE;

		retAddr = ClusterToRelatAddr(curClu);
	}

	maxDirCount = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;

	//check if ATTR_VOLUME_ID dir tab
	if (nIndex == 0 && clus == _imgBpb.BPB_RootClus)
	{
		SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
		ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);	
		if (dirRw.DIR_Attr == ATTR_VOLUME_ID)
		{
			retIndex++;
			nIndex++;
		}
	}
	//printf("GetDirectoryTabEx 2\n");	
	SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
	ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);	
	retIndex++;
	nIndex++;

	//////////////////////////////////////////////////////////////////////////
	// Added on 10/10/06
// 	while(dirRw.DIR_Name[0]=='.' || IsVolLabel(dirRw.DIR_Attr))
// 	{
// 		SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 		ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);	
// 		retIndex++;
// 		if(retIndex >= recPerCluster)
// 		{
// 			curClu = _fats.at(curClu);
// 			if(curClu == EOC_STATUS32)
// 				return FALSE;
// 			retIndex = 0;
// 			retAddr = ClusterToRelatAddr(curClu);
// 		}
// 		nIndex++;
// 	}//////////////////////////////////////////////////////////////////////////
	

	if(dirRw.DIR_Name[0] == 0x00)
		return FALSE;

	if(dirRw.DIR_Name[0] == (char)0xE5)
	{
		while(1)
		{
			if(retIndex >= recPerCluster)
			{
//				TRACE("%d\n" , curClu);
				curClu = _fats.at(curClu);
// 				if (curClu == 363)
// 					printf("1");
				if(curClu == EOC_STATUS32)
					return FALSE;
				retIndex = 0;
				retAddr = ClusterToRelatAddr(curClu);
			}

			SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
			ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
			retIndex++;
			nIndex++;


			if(dirRw.DIR_Name[0] == 0x00)
				return FALSE;
			if(dirRw.DIR_Name[0] != (char)0xE5)
				break;
		}			
	}
	
	//printf("GetDirectoryTabEx 3\n");
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
		RemoveTChar(lptLongName, _T(' '));
		DirInfoFromRwInfo(dir,  dirRw);
		dir.DIR_PosIndex = nIndex - 1;
	//	nIndex++;
		iEntryCount++;
		dir.DIR_EntryCount = iEntryCount;
		return TRUE;
	}
	
	//printf("GetDirectoryTabEx 4\n");
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
			if(curClu == EOC_STATUS32)
				return FALSE;
			retIndex = 0;
			retAddr = ClusterToRelatAddr(curClu);
		}
		SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
		ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
		if(dirRw.DIR_Name[0] == 0x00)
			return FALSE;
		if((IsFoulder(dirRw.DIR_Attr)||IsFile(dirRw.DIR_Attr)) && dirRw.DIR_Name[0]!= (char)0xE5)
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

				RemoveTChar(lptLongName , _T(' '));
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
	dir.DIR_EntryCount = iEntryCount;
	nIndex++;
	//printf("GetDirectoryTabEx 5\n");
	return TRUE;
}



BOOL DiskImgFile32::GetDirectoryTab(OUT Fat_Directory& dir, IN OUT INT& nIndex)
{
	Fat_DirectoryRW dirRw;
	DWORD nRead;
	int  ns = _curDirectory.size();
	UINT retAddr = _stOfRootAddr;
	UINT curClu;
	if(ns == 0) // 在根目录下
	{
		retAddr = _stOfRootAddr;
		curClu = _imgBpb.BPB_RootClus;
	}
	else
	{
		Fat_Directory& fat = _curDirectory.at(ns - 1);
		curClu = fat.DIR_FstClusHI;
		curClu = curClu<<16 | fat.DIR_FstClusLO;
		retAddr = ClusterToRelatAddr(curClu);
	}

	int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
	int nCluIndex = 0;  // 记录在该目录的第 几个簇下操作

//	WORD curClu = fat.DIR_FstClusLO;
	nCluIndex = nIndex / recPerCluster;
	int retIndex = 	nIndex % recPerCluster;	
	for(int  i = 0; i < nCluIndex; i++)
	{
		curClu = _fats.at(curClu);
		if (curClu == EOC_STATUS32) return FALSE;
		
		if (ns == 0 && curClu == 0xFFFFFFFF)  
			return FALSE;
		
		retAddr = ClusterToRelatAddr(curClu);
	}

	SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
	ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
	nIndex ++;	
	retIndex++;
	if(retIndex >= recPerCluster)
	{
		retIndex = 0;
		curClu = _fats.at(curClu);
		if(curClu == EOC_STATUS32)
			return FALSE;
		nCluIndex++;						
		retAddr = ClusterToRelatAddr(curClu);
	}
	if(dirRw.DIR_Name[0] == 0x00)
		return FALSE;
	
	while(dirRw.DIR_Name[0] == (char)0xE5)
	{
		SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
		ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);	
		nIndex ++;	
		retIndex++;
		if(retIndex >= recPerCluster)
		{
			retIndex = 0;
			curClu = _fats.at(curClu);
			if(curClu == EOC_STATUS32)
				return FALSE;
			nCluIndex++;						
			retAddr = ClusterToRelatAddr(curClu);
		}
		if(dirRw.DIR_Name[0] == 0x00)
			return FALSE;
	}

	while(IsLongDir(dirRw.DIR_Attr) && (dirRw.DIR_Name[0]!= (char)0xE5))
	{			
		SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
		ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
		nIndex ++;	
		retIndex++;
		if(retIndex >= recPerCluster)
		{
			retIndex = 0;
			curClu = _fats.at(curClu);
			if(curClu == EOC_STATUS32)
				return FALSE;
			nCluIndex++;						
			retAddr = ClusterToRelatAddr(curClu);
		}
		if(dirRw.DIR_Name[0] == 0x00)
			return FALSE;
	}

	DirInfoFromRwInfo(dir,  dirRw);
	dir.DIR_PosIndex = nIndex - 1;
	return TRUE;
}


BOOL DiskImgFile32::GetDirectoryTabEx(OUT Fat_Directory& dir, OUT LPTSTR lptLongName, IN OUT INT& nIndex)			// 返回当前目录下第nIndex个目录
{
	//printf("step0!\n");
	Fat_DirectoryRW dirRw;
	DWORD nRead;
	int  ns = _curDirectory.size();
	UINT curClu ;
	UINT retAddr; 
	//printf("_imgBpb.BPB_SecPerClus=%d\n", _imgBpb.BPB_SecPerClus);
	int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
	int iEntryCount = 0;
	dir.DIR_EntryCount = iEntryCount;

	if(ns == 0) // 在根目录下
	{
		//printf("ns == 0!\n");
		retAddr = _stOfRootAddr;
		curClu = this->_imgBpb.BPB_RootClus;
	}
	
	else
	{
		int nCluIndex = 0;  // 记录在该目录的第 几个簇下操作

		Fat_Directory& fat = _curDirectory.at(ns - 1);
		curClu = fat.DIR_FstClusHI;
		curClu = curClu<<16 | fat.DIR_FstClusLO;
		retAddr = ClusterToRelatAddr(curClu);
	}

	//printf("recPerCluster = %d, nIndex=%d, _imgBpb.BPB_SecPerClus=%d\n", recPerCluster, nIndex, _imgBpb.BPB_SecPerClus);
	int nCluIndex = nIndex / recPerCluster;
	int retIndex = 	nIndex % recPerCluster;	
	for(int  i = 0; i < nCluIndex; i++)
	{
		curClu = _fats.at(curClu);
		if (curClu == EOC_STATUS32) 
		{
			printf("curClu == EOC_STATUS32\n");
			return FALSE;
		}
		if (ns == 0 && curClu == 0xFFFFFFFF)
		{
			printf("ns == 0 && curClu == 0xFFFFFFFF\n");  
			return FALSE;
		}	
		
		retAddr = ClusterToRelatAddr(curClu);
	}

	//printf("step1!\n");
	while(1)
	{			
		SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
		//printf("SetDiskFilePointer\n");
		if (!ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL))
		{
			printf("!ReadDiskFile\n");
			return FALSE;
		}
		//printf("ReadDiskFile\n");
		//printf("dirRw.DIR_Name[0]=%d\n", dirRw.DIR_Name[0]);
		

		retIndex++;
		nIndex++;
		if(IsVolLabel(dirRw.DIR_Attr) && !IsLongDir(dirRw.DIR_Attr))
			continue;
		//printf("continue\n");

		if(dirRw.DIR_Name[0] == 0x00)
		{
			//printf("dirRw.DIR_Name[0] == 0x00  sss111111\n");
			return FALSE;
		}
		if(dirRw.DIR_Name[0] == (char)0xE5)
		{
			while(1)
			{
				if(retIndex >= recPerCluster)
				{
					retIndex = 0;
					curClu = _fats.at(curClu);
					if(curClu == EOC_STATUS32)
					{
printf("curClu == EOC_STATUS32\n");
						return FALSE;
					}
					nCluIndex++;						
					retAddr = ClusterToRelatAddr(curClu);
					//printf("ClusterToRelatAddr\n");
				}

				SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
				//printf("0xE5 SetDiskFilePointer\n");
				ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
				//printf("0xE5 ReadDiskFile\n");
				if(dirRw.DIR_Name[0] == 0x00)
				{
//printf("dirRw.DIR_Name[0] == 0x00 s2222222\n");
					return FALSE;
				}
				retIndex++;
				nIndex++;

				if(dirRw.DIR_Name[0] != (char)0xE5)
					break;					
			}			
		}	

		//printf("step2\n");		
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
			
			RemoveTChar(lptLongName, _T(' '));
			//printf("RemoveTChar\n");
			DirInfoFromRwInfo(dir,  dirRw);
			//printf("DirInfoFromRwInfo\n");
			dir.DIR_PosIndex = nIndex-1;
			iEntryCount++;
			dir.DIR_EntryCount= iEntryCount;
			return TRUE;
		} 
		
		BOOL bLongNameChkSumMatch = TRUE;
		BYTE* bTemp = (BYTE*)&dirRw;
		BYTE chksumbyte_l = *(bTemp+0x0D);

		while(IsLongDir(dirRw.DIR_Attr) && (dirRw.DIR_Name[0]!= ch))
		{
			Fat_LongDirectory longDir;
			LongDirInfoFromRwInfo(longDir, dirRw);
			//printf("LongDirInfoFromRwInfo\n");

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
				memset(pchar, 0, MAX_PATH);
				//printf("bbb WideCharToMultiByte\n");
// 				wchar_t wszTemp1[5]={0},wszTemp2[6]={0},wszTemp3[2]={0};
//				char* pName1 = (char*)longDir.LDIR_Name1;
//				char* pName2 = (char*)longDir.LDIR_Name2;
//				char* pName3 = (char*)longDir.LDIR_Name3;
 //				for (int i=0;i<5;i++)
//				{
 //					memcpy(wszTemp1+i, pName1+i*2,1);
 //				}
 //				for (int j=0;j<6;j++)
 //				{
 //					memcpy(wszTemp2+j, pName2+j*2,1);
 //				}
 //				for (int k=0;k<2;k++)
 //				{
 //					memcpy(wszTemp3+k, pName3+k*2,1);
 //				}

//				int nWidth = WideCharToMultiByte(m_CodePage, 0, wszTemp1, 5, pchar, MAX_PATH, "", FALSE);
//				nWidth = WideCharToMultiByte(m_CodePage, 0, wszTemp2, 6, pchar+5, MAX_PATH, "", FALSE);
//				nWidth = WideCharToMultiByte(m_CodePage, 0, wszTemp3, 2, pchar+11, MAX_PATH, "", FALSE);
				//solution 2
				int nWidth = WideCharToMultiByte(m_CodePage, 0, longDir.LDIR_Name1, 5, pchar, MAX_PATH, "", FALSE);
				nWidth = WideCharToMultiByte(m_CodePage, 0, longDir.LDIR_Name2, 6, pchar+5, MAX_PATH, "", FALSE);
				nWidth = WideCharToMultiByte(m_CodePage, 0, longDir.LDIR_Name3, 2, pchar+11, MAX_PATH, "", FALSE);

				//printf("WideCharToMultiByte\n");
				
				LPSTR lpTem = GetStrFromChArry(pchar, MAX_PATH);
				//printf("GetStrFromChArry\n");
				
				strcat(lpTem, lptLongName);
				strcpy(lptLongName, lpTem);
				//printf("******************lptLongName=%s\n", lptLongName);
				
				delete lpTem;			
#endif
			}
			iEntryCount++;

			if(retIndex >= recPerCluster)
			{
				retIndex = 0;
				curClu = _fats.at(curClu);
				if(curClu == EOC_STATUS32)
				{
printf("curClu == EOC_STATUS32\n");
					return FALSE;
				}
				nCluIndex++;						
				retAddr = ClusterToRelatAddr(curClu);
			}
			
			//printf("step3!\n");	
				
			SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
			//printf("SetDiskFilePointer\n");
			ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
			//printf("ReadDiskFile %d\n" , nRead);
			if(dirRw.DIR_Name[0] == 0x00)
			{
				printf("dirRw.DIR_Name[0] == 0x00 s33333333\n");
				return FALSE;
			}

			//printf("dirRw.DIR_Name[0] 444444\n"  );
			if(IsFile(dirRw.DIR_Attr) || IsFoulder(dirRw.DIR_Attr))
			{
				//printf("IsFile(dirRw.DIR_Attr) 1111\n");
				BYTE chksumbyte_s = ChkSum(dirRw.DIR_Name);
				//printf("ChkSum done\n");				
				if (chksumbyte_s!=chksumbyte_l || !bLongNameChkSumMatch)//long name check sum is not match the short name
				{
#ifdef _UNICODE
					//printf("IsFile(dirRw.DIR_Attr) 2222\n");
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

					RemoveTChar(lptLongName , _T(' '));
#else
					//printf("IsFile(dirRw.DIR_Attr) 33333\n");
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
				//printf("IsFile(dirRw.DIR_Attr) done\n");
				iEntryCount++;
				
				DirInfoFromRwInfo(dir,  dirRw);
				
				dir.DIR_PosIndex = nIndex;
				dir.DIR_EntryCount = iEntryCount;
				nIndex++;
				
				return TRUE;
			}
			retIndex++;
			nIndex++;
		}//while(IsLongDir(dirRw.DIR_Attr) && (dirRw.DIR_Name[0]!= ch))
//printf("while IsLongDir dirRw.DIR_Attr \n");
		return FALSE;
	}//while(1)

	//printf("bbb DirInfoFromRwInfo\n");
	DirInfoFromRwInfo(dir,  dirRw);
	//printf("DirInfoFromRwInfo\n");
	dir.DIR_PosIndex = nIndex - 1;
	return TRUE;
}



BOOL  DiskImgFile32::TrimDirEntrySpace()
{
	return TRUE;

	Fat_DirectoryRW dirRw;
	DWORD nRead;
	int  ns = _curDirectory.size();

	vector<Fat_DirectoryRW> dirs;
	dirs.resize(0);
	UINT retAddr;
	UINT curClu;
	UINT nIndex = 0;
	UINT tempCurclu;
	UINT maxCntPerClu = BytesPerSector()*SectorsPerCluster()/32;

	if(ns == 0) // 在根目录下
	{
		retAddr = _stOfRootAddr;
		curClu = this->_imgBpb.BPB_RootClus;
	}
	else
	{
		Fat_Directory& fat =  _curDirectory.at(ns - 1);//.DIR_FstClusLO;
		curClu = fat.DIR_FstClusHI;
		curClu = curClu<<16 | fat.DIR_FstClusLO;
		retAddr = ClusterToRelatAddr(curClu);
	}
	tempCurclu = curClu;

	UINT retIndex = 0;
	nIndex = 0;
	SetDiskFilePointer(_hFile, retAddr , NULL, FILE_BEGIN);

	while (1)
	{
		if(retIndex >= maxCntPerClu)
		{
			curClu = _fats.at(curClu);
			if(curClu == EOC_STATUS32)
				break;
			retAddr = ClusterToRelatAddr(curClu);
			SetDiskFilePointer(_hFile, retAddr, NULL, FILE_BEGIN);
		}

		ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);			
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
	curClu =  tempCurclu;
	retAddr =  ClusterToRelatAddr(curClu);
	SetDiskFilePointer(_hFile, retAddr, NULL, FILE_BEGIN);
	retIndex = 0;
	int i;
	for( i = 0; i < ds; i++)
	{
		if(retIndex >= maxCntPerClu)
		{
			curClu = _fats.at(curClu);
			if(curClu == EOC_STATUS32)
				break;
			retAddr = ClusterToRelatAddr(curClu);
			SetDiskFilePointer(_hFile, retAddr, NULL, FILE_BEGIN);
		}
		if (!WriteDiskFile(_hFile, &dirs.at(i), sizeof(Fat_DirectoryRW), &nRead, NULL))
		{
			return FALSE;
		}
		
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
			if(curClu == EOC_STATUS32)
				break;
			retAddr = ClusterToRelatAddr(curClu);
			SetDiskFilePointer(_hFile, retAddr, NULL, FILE_BEGIN);
		}		
		if (!WriteDiskFile(_hFile, blank, 32, &nRead, NULL))
		{
			return FALSE;
		}
		
		retIndex++;
	}

	curClu = _fats.at(endClu);

	SetClus(endClu, EOC_STATUS32);
	
	while(curClu != EOC_STATUS32)
	{
		UINT temp = _fats.at(curClu);
		SetClus(curClu, FREE_STATUS32);
		curClu = temp;
	}		

	return TRUE;	
}

BOOL DiskImgFile32::ImgCreateDirectory(IN LPCTSTR lptDirName)
{
	int ns = _curDirectory.size();
	int nNeed = this->CalcNeedTabCount(lptDirName); // 需要存放的结构的个数

	UINT len = _tcslen(lptDirName);
	LPWSTR pLongName = new WCHAR[len + 1];

#ifdef _UNICODE
	wcscpy(pLongName , lptDirName);
	int nl = _tcslen(lptDirName);
#else	
	memset(pLongName, 0x00, (len + 1) * 2);
	int nl = ::MultiByteToWideChar(m_CodePage, 0, lptDirName, len, pLongName, len+1);
#endif
	
	
	LPSTR pName = new char[MAX_PATH];
	memset(pName, 0x00, MAX_PATH);
	if(!GenerateShortName(lptDirName, pName))// new char[11];
	{
		delete pLongName;
		delete pName;
		printmsg("Generate short name faile!");
		return FALSE;
	}

	
	len = strlen(pName);
	ASSERT(len <= 12);


	DWORD nRead;
	DWORD nWrite;
	UINT dirAdd;
	UINT cClus;
	BYTE chkFlag = ChkSum(pName);
	UINT parentClus;
	if(ns == 0) // 在根目录下创建字目录
	{
		dirAdd = _stOfRootAddr;
		cClus = _imgBpb.BPB_RootClus;
		parentClus = cClus;
	}

	else  //在子目录下创建子目录
	{
		Fat_Directory curD = _curDirectory.at(_curDirectory.size() - 1);
		
		cClus = curD.DIR_FstClusHI;
		cClus = cClus<<16 | curD.DIR_FstClusLO;
		dirAdd = this->ClusterToRelatAddr(cClus);
		parentClus = cClus;
	}
	//////////////////////////////////////////////////////////////////////////
	// 建立新目录项
	Fat_Directory dir;
	Fat_DirectoryRW dirRw;
	INT nIndex = -1;

	memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
	memset(&dir, 0x00, sizeof(Fat_Directory));
	SetDiskFilePointer(_hFile, dirAdd, NULL, (DWORD)FILE_BEGIN);
	UINT maxDircnt = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;

	int  nDirPos = 0; // 记录目录的相对位置 放置在Fat_Directory.DIR_PosIndex中
		while(1)
		{
			for(int i = 0; i < maxDircnt; i++)
			{	
				ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
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

			DWORD dwClus = _fats.at(cClus);			
			if(dwClus == EOC_STATUS32)
				break;
			cClus = dwClus;
			dirAdd = ClusterToRelatAddr(cClus);
			SetDiskFilePointer(_hFile, dirAdd, NULL, (DWORD)FILE_BEGIN);
		}
		if(nIndex == -1)
		{
			UINT c = GetFirstFreeClusNum();
			SetClus(cClus, c);
			SetClus(c, EOC_STATUS32);
			InitializeClus(c);
			cClus = c;
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
			//int nl = _tcslen(lptDirName);
			int npos = (nNeed - 2) * 13;
			memset(longDir.LDIR_Name1, 0xff, 26);
			memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
			if((nl - npos) < 13)
				memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
			RwInfoFromLongDirInfo(dirRw, longDir);
			SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
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
				SetClus(c, EOC_STATUS32);
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
//			int nl = strlen(lpDirName);
			int npos = (i - 1) * 13;
			memset(longDir.LDIR_Name1, 0xff, 26);
			memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
			RwInfoFromLongDirInfo(dirRw, longDir);
			SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
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
				SetClus(c, EOC_STATUS32);
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
		dir.DIR_FstClusHI = clus>>16;
		dir.DIR_FstClusLO = clus;
		SetClus(clus, EOC_STATUS32);

		RwInfoFromDirInfo(dirRw, dir);
		SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
		//SetDiskFilePointer(_hFile, dirAdd + 32 * nIndex, NULL, FILE_BEGIN);
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
		dir.DIR_FstClusHI = clus>>16;
		dir.DIR_FstClusLO = clus;
		SetDiskFilePointer(_hFile, pos, NULL, (DWORD)FILE_BEGIN);			
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
		dir.DIR_FstClusHI = parentClus>>16;
		dir.DIR_FstClusLO = parentClus;
		::GetSystemTime(&dir.DIR_WrtDateTime);
		::GetSystemTime(&dir.DIR_CrtDateTime);
		::GetSystemTime(&dir.DIR_LstAcceDate);

	/*	dir.DIR_WrtDateTime = CTime::GetCurrentTime();//curD.DIR_WrtDateTime;
		dir.DIR_CrtDateTime = CTime::GetCurrentTime();//
		dir.DIR_LstAcceDate = CTime::GetCurrentTime();//curD.DIR_LstAcceDate;
*/
	    RwInfoFromDirInfo(dirRw, dir);
		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
		{
			delete pName;
			printmsg("Error In create SubDirectory -- write dir Struct");
			return FALSE;
		}
		SetDiskFilePointer(_hFile, 0, NULL, FILE_BEGIN);
		//
		//////////////////////////////////////////////////////////////////////////				
		delete pLongName;
	delete []pName;
	return TRUE;
	
}


BOOL  DiskImgFile32::ImgCreateFile(IN const LPCTSTR lptFileName, 
								  IN PBYTE  pBuffer,
								  IN const BYTE bFileAttr,
								  IN const UINT nSize,
								  HANDLE& hFile)// 创建文件函数
{
	int ns = _curDirectory.size();
	int nNeed = this->CalcNeedTabCount(lptFileName); // 需要存放的结构的个数

	UINT len = _tcslen(lptFileName);
	LPWSTR pLongName = new WCHAR[len + 1];

#ifdef _UNICODE
	wcscpy(pLongName , lptFileName);
	int nl = wcslen(pLongName);
#else	
	memset(pLongName, 0x00, (len + 1) * 2);
	int nl = ::MultiByteToWideChar(m_CodePage, 0, lptFileName, len, pLongName, len+1);
// printf("---------MultiByteToWideChar lptFileName=%s,pLongName=",lptFileName);
// char* p = (char*)pLongName;
// for(int i=0;i<(len+1)*2;i++)
// {
// printf("%c", p[i]);
// }
// printf("\n");
#endif


	LPSTR pName = new char[MAX_PATH];
	memset(pName, 0x00, MAX_PATH);
	if (!GenerateShortName(lptFileName, pName))
	{
//printf("!GenerateShortName!!!!!!!!lptFileName=%s, pName=%s\n", lptFileName,pName);
		delete pName;
		return FALSE;
	}
//printf("pName = %s\n", pName);
	
	

	DWORD nRead;
	DWORD nWrite;
	
	Fat_Directory dir;
	Fat_DirectoryRW dirRw;

	memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
	memset(&dir, 0x00, sizeof(Fat_Directory));

	UINT fstClus = 0;
	UINT dirAdd;
	
	BYTE chkFlag = ChkSum(pName);	
	UINT cClus;

	if(ns == 0) // 在根目录下创建文件
	{
		dirAdd = _stOfRootAddr;
		cClus = _imgBpb.BPB_RootClus;
			
	}
	else		// 在其他目录下建立文件
	{
		Fat_Directory curD = _curDirectory.at(_curDirectory.size() - 1);

		cClus = curD.DIR_FstClusHI;
		cClus = cClus<<16|curD.DIR_FstClusLO;
		
		dirAdd = this->ClusterToRelatAddr(cClus);
	}

	INT nIndex = -1;
	//////////////////////////////////////////////////////////////////////////
	// 开始建立目录项

	SetDiskFilePointer(_hFile, dirAdd, NULL, (DWORD)FILE_BEGIN);
	UINT maxDircnt = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;
	int  nDirPos = 0; // 记录目录的相对位置 放置在Fat_Directory.DIR_PosIndex中
	while(1)
	{
		for(int i = 0; i < maxDircnt; i++)
		{	
			ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
			if(nRead != sizeof(Fat_DirectoryRW))
			{
				delete pLongName;
				delete pName;
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
		UINT tmpClu = cClus;
		cClus = _fats.at(cClus);
		if(cClus == EOC_STATUS32)
		{
			cClus = tmpClu;
			break;
		}

		if (cClus == 0xFFFFFFFF && ns == 0)
		{
			cClus = tmpClu;
			break;
		}

		dirAdd = ClusterToRelatAddr(cClus);
		SetDiskFilePointer(_hFile, dirAdd, NULL, (DWORD)FILE_BEGIN);
	}
	if(nIndex == -1)
	{
		UINT c = GetFirstFreeClusNum();
		SetClus(cClus, c);
		SetClus(c, EOC_STATUS32);
		InitializeClus(c);
		nIndex = nDirPos;
		dirAdd = ClusterToRelatAddr(c);
	}	

	//////////////////////////////////////////////////////////////////////////
	// 处理长名目录项目
	int retIndex = nIndex%maxDircnt;
//printf("nnnnnnnnnnnnnnnnnnn here nNeed = %d\n", nNeed);
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
		
////////////////////////////////////////////
		//memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
		char* pTmp = (char*)pLongName;
		memcpy(longDir.LDIR_Name1, pTmp+npos*2+2, 10);
		memcpy(longDir.LDIR_Name2, pTmp+npos*2+12, 12);
		memcpy(longDir.LDIR_Name3, pTmp+npos*2+24, 4);
char *p = (char*)longDir.LDIR_Name1;
// printf("1111111111111111111  longDir.LDIR_Name1 =");
// for(int i=0;i<5*2;i++)
// {
// printf("%c", p[i]);
// }
// p = (char*)longDir.LDIR_Name2;
// printf("\nlongDir.LDIR_Name2 =");
// for(int j=0;j<6*2;j++)
// {
// printf("%c", p[j]);
// }
// p = (char*)longDir.LDIR_Name3;
// printf("\nlongDir.LDIR_Name3 =");
// for(int k=0;k<2*2;k++)
// {
// printf("%c", p[k]);
// }
// printf("\n");
////////////////////////////////////////////
		if((nl - npos) < 13)
			memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
		RwInfoFromLongDirInfo(dirRw, longDir);
		SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
		{
			delete pLongName;
			delete pName;
			printmsg("Error In create RootDirectory -- write dir Struct");
			return FALSE;
		}
		retIndex++;
		nIndex ++;
		if(retIndex >= maxDircnt)
		{
			UINT c = GetFirstFreeClusNum();
			SetClus(cClus, c);
			SetClus(c, EOC_STATUS32);
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
		//int nl = strlen(lpFileName);
		int npos = (i - 1) * 13;
		memset(longDir.LDIR_Name1, 0xff, 26);
////////////////////////////////////////////
		//memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
		char* pTmp = (char*)pLongName;
		memcpy(longDir.LDIR_Name1, pTmp+npos*2+2, 10);
		memcpy(longDir.LDIR_Name2, pTmp+npos*2+12, 12);
		memcpy(longDir.LDIR_Name3, pTmp+npos*2+24, 4);
char *p = (char*)longDir.LDIR_Name1;
// printf("2222222222222  longDir.LDIR_Name1 =");
// for(int i=0;i<5*2;i++)
// {
// printf("%c", p[i]);
// }
// p = (char*)longDir.LDIR_Name2;
// printf("\nlongDir.LDIR_Name2 =");
// for(int j=0;j<6*2;j++)
// {
// printf("%c", p[j]);
// }
// p = (char*)longDir.LDIR_Name3;
// printf("\nlongDir.LDIR_Name3 =");
// for(int k=0;k<2*2;k++)
// {
// printf("%c", p[k]);
// }
// printf("\n");
////////////////////////////////////////////
		RwInfoFromLongDirInfo(dirRw, longDir);
		SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
		{
			delete pLongName;
			delete pName;
			printmsg("Error In create RootDirectory -- write dir Struct");
			return FALSE;
		}
		
		nIndex ++;	
		retIndex++;
		if(retIndex >= maxDircnt)
		{
			UINT c = GetFirstFreeClusNum();
			SetClus(cClus, c);
			SetClus(c, EOC_STATUS32);
			InitializeClus(c);
			retIndex = 0;
			dirAdd = ClusterToRelatAddr(c);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	

	memcpy(&dir.DIR_Name, pName, 11);
	dir.DIR_Attr = bFileAttr;
//	dir.DIR_FileSize = nSize;
	dir.DIR_FileSize = 0;
/*	dir.DIR_WrtDateTime = CTime::GetCurrentTime();
	dir.DIR_LstAcceDate = CTime::GetCurrentTime();
	dir.DIR_CrtDateTime = CTime::GetCurrentTime();
*/
	::GetSystemTime(&dir.DIR_WrtDateTime);
	::GetSystemTime(&dir.DIR_CrtDateTime);
	::GetSystemTime(&dir.DIR_LstAcceDate);

	//fstClus = GetFirstFreeClusNum();
	//ASSERT(fstClus >= 2);
	fstClus = 0;
	dir.DIR_FstClusLO = fstClus;
	dir.DIR_FstClusHI = fstClus>>16;
	dir.DIR_PosIndex = nIndex;
	
	ImgFileHandle* fh = new ImgFileHandle;
	memcpy(&fh->_fileTab, &dir, sizeof(dir));
	fh->_exceedsize = 0;
	fh->_curRetpos = 0;
	fh->_tabStAddr = dirAdd + 32 * retIndex;
	UINT clu = dir.DIR_FstClusHI;
	clu =  clu<<16 | dir.DIR_FstClusLO;
	fh->_stAddr = ClusterToRelatAddr(clu);
	fh->_curPos =fh->_stAddr;

	hFile = (HANDLE)fh;


	RwInfoFromDirInfo(dirRw, dir);
//		SetDiskFilePointer(_hFile, dirAdd + 32 * nIndex, NULL, FILE_BEGIN);
	SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
	if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
	{
		delete pName;
		printmsg("Error In create RootFile -- write dir Struct");
		return FALSE;
	}
	

	// 计算文件占用簇的数目
	UINT bysPerClus = _imgBpb.BPB_BytsPerSec * _imgBpb.BPB_SecPerClus;
	//printf("bysPerClus = %d", bysPerClus);
	UINT nFileClus = (nSize +  bysPerClus - 1) / bysPerClus;
	if((nSize == 0) || (pBuffer == NULL))
	{
		delete pName;
		delete pLongName;
		SetClus(fstClus, EOC_STATUS32);
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
			SetDiskFilePointer(_hFile, reAdd, NULL, FILE_BEGIN);
			if(k < nFileClus -1)
			{
				if (!WriteDiskFile(_hFile, pBuffer + k * bysPerClus, bysPerClus, &nWrite, NULL))
				{
					return FALSE;
				}
				
			}
			else
			{
				if (!WriteDiskFile(_hFile, pBuffer + k * bysPerClus, nSize%bysPerClus, &nWrite, NULL))
				{
					return FALSE;
				}
				
			}
			k++;
			if(k >= nFileClus)
			{
				break;
			}
			SetClus(fstClus, EOC_STATUS32);
			fstClus = GetFirstFreeClusNum();
			SetClus(curClus, fstClus);			
		}		
		SetClus(fstClus, EOC_STATUS32);	
	}
	else
	{
		SetClus(fstClus, EOC_STATUS32);
	}
	delete pLongName;
	delete pName;
	return TRUE;
}

				   
BOOL  DiskImgFile32::ImgDeleteDirectory(IN LPCTSTR lptDirName)					// 删除目录函数
{
	int ns = _curDirectory.size();
	DWORD nWrite;
	LPTSTR lptLongName = new TCHAR[MAX_PATH];
	int nIndex = 0;
	Fat_Directory dir;
	UINT curC;
	while(1)
	{
		memset(lptLongName, 0x00, MAX_PATH * sizeof(TCHAR));
		if(!GetDirectoryTabEx(dir, lptLongName, nIndex))
		{
			delete lptLongName;
			return FALSE;
		}
		if(_tcsicmp(lptLongName, lptDirName) == 0)
		{
			break;
		}
	}

	UINT firstClus = dir.DIR_FstClusHI;
	firstClus = firstClus<<16|dir.DIR_FstClusLO;
	ASSERT(firstClus >=2);
	UINT curc = firstClus;
	if(_fats.at(curc)== EOC_STATUS32)
	{
		ImgDeleteDirectory(curc);
		SetClus(curc, FREE_STATUS32);
		InitializeClus(curc);
	//	return TRUE;
	}
	else
	{
		while(1)//_fats.at(curc) != EOC_STATUS32)
		{
			ImgDeleteDirectory(curc);
			UINT temp = _fats.at(curc);
			InitializeClus(curc);
			SetClus(curc, FREE_STATUS32);
			curc = temp;				
			if(curc == EOC_STATUS32)
				break;
		}
	}

	//int nNeed = this->CalcNeedTabCount(lpDirName);
	int nNeed = dir.DIR_EntryCount;
	if (nNeed == 0)	
		nNeed = 1;
	int nPos = dir.DIR_PosIndex;
	
	if(ns == 0 )	// 在根目录下删除目录
	{
		curC = _imgBpb.BPB_RootClus;
	}

	else			// 在子目录下删除目录
	{
		Fat_Directory& f = _curDirectory.at(ns - 1);
		curC = f.DIR_FstClusHI;
		curC = curC<<16|f.DIR_FstClusLO;
	}

	int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
	int nCluIndex = nPos /recPerCluster;  // 记录在该目录的第 几个簇下操作
	UINT retIndex = nPos%recPerCluster;

	UINT retAddr = ClusterToRelatAddr(curC);
	//comment out by joelee 2008.12.03
/*	int i;
	for( i = 0; i < nCluIndex; i++)
	{
		curC = _fats.at(curC);
		retAddr = ClusterToRelatAddr(curC);
	}

	if(retIndex < (nNeed -1))
	{
		retIndex = recPerCluster - (nNeed - 1 - retIndex);
		nCluIndex--;
		for(int i = 0; i < nCluIndex; i++)
		{
			curC = _fats.at(curC);
			retAddr = ClusterToRelatAddr(curC);
		}
	}
	
	BYTE emptyFlag[2] = {0xE5 , 0xFF};

	retIndex = retIndex - (nNeed - 1);*/

	//Add by joelee 2008.12.03
	BYTE emptyFlag[2] = {0xE5 , 0xFF};
	if(retIndex < (nNeed -1))
	{
		retIndex = recPerCluster - (nNeed  - retIndex - 1);		
		nCluIndex--;
	}
	else
		retIndex = retIndex - (nNeed - 1); 
	
	int i;
	for( i = 0; i < nCluIndex; i++)
	{
		curC = _fats.at(curC);
		if (curC == EOC_STATUS32) return FALSE;
		retAddr = ClusterToRelatAddr(curC);
	}
	for(i = 0; i < nNeed; i++)
	{
		SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
		if (!WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL))
		{
			return FALSE;
		}
		
		
		retIndex++;
		if(retIndex >= recPerCluster)
		{
			curC = _fats.at(curC);
			if (curC == EOC_STATUS32) return FALSE;
			retAddr = ClusterToRelatAddr(curC);
			retIndex = 0;
		}
	}
	
	delete lptLongName;

	return TRUE;
}


BOOL  DiskImgFile32::ImgDeleteFile(IN LPCTSTR lptFileName)						// 删除文件函数
{
	int ns = _curDirectory.size();
	DWORD nWrite;
	
	LPTSTR lptLongName = new TCHAR[MAX_PATH];
	
	int nIndex = 0;
	Fat_Directory dir;
	while(1)
	{
		memset(lptLongName, 0x00, MAX_PATH * sizeof(TCHAR));
		if(!GetDirectoryTabEx(dir, lptLongName, nIndex))
		{
			delete lptLongName;
			return FALSE;
		}
		if(_tcsicmp(lptLongName, lptFileName) == 0)
		{
			break;
		}
	}

	//int nNeed = CalcNeedTabCount(lpFileName);
	int nNeed = dir.DIR_EntryCount;
	if (nNeed == 0)	
		nNeed = 1;
	

	UINT curC ;
	UINT retAddr;
	if(ns == 0 )	// 在根目录下删除文件
	{
		curC = _imgBpb.BPB_RootClus;
		retAddr = _stOfRootAddr;
	}
	
	else			// 在子目录下删除文件
	{

		Fat_Directory& parDir = _curDirectory.at(ns - 1);
		curC = parDir.DIR_FstClusHI;
		curC = curC<<16|parDir.DIR_FstClusLO;
		retAddr = ClusterToRelatAddr(curC);		
	}
	int nPos = dir.DIR_PosIndex;
		
	BYTE emptyFlag[2] = {0xE5 , 0xFF};
	int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
	int nCluIndex = nPos /recPerCluster;  // 记录在该目录的第 几个簇下操作
	UINT retIndex = nPos%recPerCluster;
	
// 	for(int i = 0; i < nCluIndex; i++)
// 	{
// 		curC = _fats.at(curC);
// 		retAddr = ClusterToRelatAddr(curC);
// 	}

	if(retIndex < (nNeed -1))
	{
		retIndex = recPerCluster - (nNeed  - retIndex - 1);		
		nCluIndex--;
	}
	else
		retIndex = retIndex - (nNeed - 1); 

	int i;
	for( i = 0; i < nCluIndex; i++)
	{
		curC = _fats.at(curC);
		if (curC == EOC_STATUS32) return FALSE;
		retAddr = ClusterToRelatAddr(curC);
	}

	
	for(i = 0; i < nNeed; i++)
	{
		SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
		if (!WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL))
		{
			return	FALSE;
		}
			
		retIndex++;
		if(retIndex >= recPerCluster)
		{
			curC = _fats.at(curC);
			if (curC == EOC_STATUS32) /*return FALSE*/break;
			retAddr = ClusterToRelatAddr(curC);
			retIndex = 0;
		}
	}

	UINT firstClus = dir.DIR_FstClusHI;
	firstClus = firstClus<<16|dir.DIR_FstClusLO;
	
	//ASSERT(firstClus >=2);
	UINT curc = firstClus;
// 	if(_fats.at(curc) == EOC_STATUS32)
// 	{
// 		SetClus(curc, FREE_STATUS32);
// 		InitializeClus(curc);
// 		delete lpLongName;
// 		return TRUE;
// 	}
// 	while(_fats.at(curc) != EOC_STATUS32)
// 	{
// 		InitializeClus(curc);
// 		UINT temp = _fats.at(curc);
// 		SetClus(curc, FREE_STATUS32);
// 		curc = temp;				
// 	}
// 	InitializeClus(curc);
// 	SetClus(curc, FREE_STATUS32);	
	SetClusFreeStatus(curc);
	delete lptLongName;
	return TRUE;
}

BOOL  DiskImgFile32::ImgMoveFile(IN LPCSTR lpFileName, IN LPCSTR lpSrcDir, IN LPCSTR lpDesFileName, IN LPCSTR lpDesDir)		// 移动文件函数
{	
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
// 
// 	SetCurrentDirectory(lpSrcDir);
// 	
// 	Fat_Directory fileTab;
// 	DiskImgFile::GetDirectoryTab(fileTab, lpFileName);
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
// 		delete pLongName;
// 		delete pName;
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
// 	UINT dirAdd;
// 	UINT cClus ;
// 	if(ns == 0) // 在根目录下创建文件
// 	{
// 		dirAdd = _stOfRootAddr;
// 		cClus = _imgBpb.BPB_RootClus;
// 	}
// 	
// 	else  //在子目录下创建子目录
// 	{
// 
// 		//////////////////////////////////////////////////////////////////////////
// 		// 建立新目录项
// 		Fat_Directory curD = _curDirectory.at(_curDirectory.size() - 1);
// 		cClus = curD.DIR_FstClusHI;
// 		cClus = cClus<<16|curD.DIR_FstClusLO;
// 		dirAdd = this->ClusterToRelatAddr(cClus);
// 	}
// 
// 
// 	Fat_Directory dir;
// 	Fat_DirectoryRW dirRw;
// 	INT nIndex = -1;
// 
// 	memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
// 	memset(&dir, 0x00, sizeof(Fat_Directory));
// 	SetDiskFilePointer(_hFile, dirAdd, NULL, (DWORD)FILE_BEGIN);
// 	UINT maxDircnt = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;
// 
// 	int  nDirPos = 0; // 记录目录的相对位置 放置在Fat_Directory.DIR_PosIndex中
// 	while(1)
// 	{
// 		for(int i = 0; i < maxDircnt; i++)
// 		{	
// 			ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
// 			if(nRead != sizeof(Fat_DirectoryRW))
// 			{
// 				printmsg("Error In Create RootDirectroy --Read DirRW Error!");
// 				return FALSE;
// 			}
// 			
// 			if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed == 1))   // 判断是否为空项
// 			{
// 				nIndex = i;
// 				break;
// 			}	
// 			nDirPos++;
// 		}
// 		if(nIndex != -1)
// 			break;
// 		cClus = _fats.at(cClus);
// 		if(cClus == EOC_STATUS32)
// 			break;
// 		dirAdd = ClusterToRelatAddr(cClus);
// 		SetDiskFilePointer(_hFile, dirAdd, NULL, (DWORD)FILE_BEGIN);
// 	}
// 	if(nIndex == -1)
// 	{
// 		UINT c = GetFirstFreeClusNum();
// 		SetClus(cClus, c);
// 		SetClus(c, EOC_STATUS32);
// 		InitializeClus(c);
// 		nIndex = nDirPos;
// 		dirAdd = ClusterToRelatAddr(c);
// 	}	
// 	
// 
// 	//////////////////////////////////////////////////////////////////////////
// 	// 处理长名目录项目
// 	int retIndex = nIndex%maxDircnt;
// 	if(nNeed > 1)
// 	{
// 		Fat_LongDirectory longDir;
// 		memset(&longDir, 0x00, 32);
// 		longDir.LDIR_Attr = ATTR_LONG_NAME;
// 		longDir.LDIR_Chksum = chkFlag;
// 		longDir.LDIR_FstClusLO = 0;
// 		longDir.LDIR_Ord = LAST_LONG_ENTRY|(BYTE)(nNeed - 1);
// 		longDir.LDIR_Type = 0;
// 	//	int nl = strlen(lpDirName);
// 		int npos = (nNeed - 2) * 13;
// 		memset(longDir.LDIR_Name1, 0xff, 26);
// 		memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
// 		if((nl - npos) < 13)
// 			memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
// 		RwInfoFromLongDirInfo(dirRw, longDir);
// 		SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 		{
// 			
// 			printmsg("Error In create RootDirectory -- write dir Struct");
// 			return FALSE;
// 		}
// 		retIndex++;
// 		nIndex ++;	
// 		if(retIndex >= maxDircnt)
// 		{
// 			UINT c = GetFirstFreeClusNum();
// 			SetClus(cClus, c);
// 			SetClus(c, EOC_STATUS32);
// 			InitializeClus(c);
// 			retIndex = 0;
// 			dirAdd = ClusterToRelatAddr(c);
// 		}
// 	}
// 	int i;
// 	for(  i = nNeed - 2; i >0 ; i--)
// 	{
// 		Fat_LongDirectory longDir;
// 		memset(&longDir, 0x00, 32);
// 		longDir.LDIR_Attr = ATTR_LONG_NAME;
// 		longDir.LDIR_Chksum = chkFlag;
// 		longDir.LDIR_FstClusLO = 0;
// 		longDir.LDIR_Ord = (BYTE)i;
// 		longDir.LDIR_Type = 0;
// 		int nl = strlen(lpFileName);
// 		int npos = (i - 1) * 13;
// 		memset(longDir.LDIR_Name1, 0xff, 26);
// 		memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
// 		RwInfoFromLongDirInfo(dirRw, longDir);
// 		SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 		{
// 	
// 			printmsg("Error In create RootDirectory -- write dir Struct");
// 			return FALSE;
// 		}
// 		retIndex++;
// 		nIndex ++;	
// 		if(retIndex >= maxDircnt)
// 		{
// 			UINT c = GetFirstFreeClusNum();
// 			SetClus(cClus, c);
// 			SetClus(c, EOC_STATUS32);
// 			InitializeClus(c);
// 			retIndex = 0;
// 			dirAdd = ClusterToRelatAddr(c);
// 		}
// 	}
// 	//////////////////////////////////////////////////////////////////////////
// 	
// 	SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 	memcpy(&dir.DIR_Name, pName, 11);
// 	dir.DIR_Attr = fileTab.DIR_Attr;
// /*	dir.DIR_WrtDateTime = CTime::GetCurrentTime();
// 	dir.DIR_CrtDateTime = CTime::GetCurrentTime();
// 	dir.DIR_LstAcceDate = CTime::GetCurrentTime();
// */
// 	::GetSystemTime(&dir.DIR_WrtDateTime);
// 	::GetSystemTime(&dir.DIR_CrtDateTime);
// 	::GetSystemTime(&dir.DIR_LstAcceDate);
// 
// 	dir.DIR_FileSize = fileTab.DIR_FileSize;
// 
// 	UINT clus = fileTab.DIR_FstClusHI;
// 	clus = clus<<16|fileTab.DIR_FstClusLO;
// 	
// 	ASSERT(clus >= 2);
// 	dir.DIR_FstClusHI = clus<<16;
// 	dir.DIR_FstClusLO = clus;
// 
// 	RwInfoFromDirInfo(dirRw, dir);
// 	SetDiskFilePointer(_hFile, dirAdd + 32 * nIndex, NULL, FILE_BEGIN);
// 	if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 	{
// 		
// 		printmsg("Error In create RootDirectory -- write dir Struct");
// 		return FALSE;
// 	}
// 	//
// 	//////////////////////////////////////////////////////////////////////////
// 		
// 	UINT retAddr ;
// 	
// 	//////////////////////////////////////////////////////////////////////////
// 	// 取消原来位置的目录表格
// 
// 	ns = srcDirs.size();
// 	UINT curC;
// 	if(ns == 0)
// 	{
// 		retAddr = _stOfRootAddr;
// 		curC = _imgBpb.BPB_RootClus;		
// 	}
// 	else
// 	{
// 		UINT c = srcDirs.at(ns - 1).DIR_FstClusHI;
// 		c = c<<16|srcDirs.at(ns - 1).DIR_FstClusLO;
// 		retAddr = ClusterToRelatAddr(c);
// 
// 		Fat_Directory& f = srcDirs.at(ns - 1);
// 		
// 		curC = f.DIR_FstClusHI;
// 		curC = curC<<16|f.DIR_FstClusLO;
// 	}
// 
// 	int nPos = fileTab.DIR_PosIndex;
// 	int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
// 	int nCluIndex = nPos /recPerCluster;  // 记录在该目录的第 几个簇下操作
// 	retIndex = nPos%recPerCluster;
// 
// 	retAddr = ClusterToRelatAddr(curC);
// 
// 	for(i = 0; i < nCluIndex; i++)
// 	{
// 		curC = _fats.at(curC);
// 		retAddr = ClusterToRelatAddr(curC);
// 	}
// 
// 	if(retIndex < (nNeed -1))
// 	{
// 		retIndex = recPerCluster - (nNeed - 1 - retIndex);
// 		nCluIndex--;
// 		for(int i = 0; i < nCluIndex; i++)
// 		{
// 			curC = _fats.at(curC);
// 			retAddr = ClusterToRelatAddr(curC);
// 		}
// 	}
// 	
// 	BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 
// 	retIndex = retIndex - (nNeed - 1);
// 	for(i = 0; i < nNeed; i++)
// 	{
// 		SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 		WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
// 		retIndex++;
// 		if(retIndex >= recPerCluster)
// 		{
// 			curC = _fats.at(curC);
// 			retAddr = ClusterToRelatAddr(curC);
// 			retIndex = 0;
// 		}
// 	}
// 
// 	delete pName;
// 	delete pLongName;
// 	return TRUE;
	return TRUE;
}

BOOL  DiskImgFile32::ImgMoveDirectory(IN LPCSTR lpSrcDir, IN LPCSTR lpDesDir)	// 移动目录函数
{
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
// 		delete pName;
// 		delete pLongName;
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
// 
// 	UINT dirAdd ;
// 	UINT cClus;
// 	if(ns == 0)
// 	{
// 		dirAdd = _stOfRootAddr;
// 		cClus = _imgBpb.BPB_RootClus;
// 	}
// 
// 	else  //在子目录下创建子目录
// 	{
// 		Fat_Directory curD = _curDirectory.at(_curDirectory.size() - 1);
// 		cClus = curD.DIR_FstClusHI;
// 		cClus = cClus<<16|curD.DIR_FstClusLO;
// 		dirAdd = this->ClusterToRelatAddr(cClus);
// 	}
// 
// 	Fat_Directory dir;
// 	Fat_DirectoryRW dirRw;
// 	INT nIndex = -1;
// 	//////////////////////////////////////////////////////////////////////////
// 	// 建立新目录项
// 	
// 	memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
// 	memset(&dir, 0x00, sizeof(Fat_Directory));
// 	SetDiskFilePointer(_hFile, dirAdd, NULL, (DWORD)FILE_BEGIN);
// 	UINT maxDircnt = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;
// 	int  nDirPos = 0; // 记录目录的相对位置 放置在Fat_Directory.DIR_PosIndex中
// 	while(1)
// 	{
// 		for(int i = 0; i < maxDircnt; i++)
// 		{	
// 			ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
// 			if(nRead != sizeof(Fat_DirectoryRW))
// 			{
// 				printmsg("Error In Create RootDirectroy --Read DirRW Error!");
// 				return FALSE;
// 			}
// 				
// 			if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed == 1))   // 判断是否为空项
// 			{
// 				nIndex = i;
// 					break;
// 			}	
// 			nDirPos++;
// 		}
// 		if(nIndex != -1)
// 			break;
// 		cClus = _fats.at(cClus);
// 		if(cClus == EOC_STATUS32)
// 			break;
// 		dirAdd = ClusterToRelatAddr(cClus);
// 		SetDiskFilePointer(_hFile, dirAdd, NULL, (DWORD)FILE_BEGIN);
// 	}
// 	if(nIndex == -1)
// 	{
// 		UINT c = GetFirstFreeClusNum();
// 		SetClus(cClus, c);
// 		SetClus(c, EOC_STATUS32);
// 		InitializeClus(c);
// 		nIndex = nDirPos;
// 		dirAdd = ClusterToRelatAddr(c);
// 	}	
// 	
// 
// 	//////////////////////////////////////////////////////////////////////////
// 	// 处理长名目录项目
// 	int retIndex = nIndex%maxDircnt;
// 	if(nNeed > 1)
// 	{
// 		Fat_LongDirectory longDir;
// 		memset(&longDir, 0x00, 32);
// 		longDir.LDIR_Attr = ATTR_LONG_NAME;
// 		longDir.LDIR_Chksum = chkFlag;
// 		longDir.LDIR_FstClusLO = 0;
// 		longDir.LDIR_Ord = LAST_LONG_ENTRY|(BYTE)(nNeed - 1);
// 		longDir.LDIR_Type = 0;
// 	//	int nl = strlen(lpDirName);
// 		int npos = (nNeed - 2) * 13;
// 		memset(longDir.LDIR_Name1, 0xff, 26);
// 		memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
// 		if((nl - npos) < 13)
// 			memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
// 		RwInfoFromLongDirInfo(dirRw, longDir);
// 		SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 		{
// 			
// 			printmsg("Error In create RootDirectory -- write dir Struct");
// 			return FALSE;
// 		}
// 		retIndex++;
// 		nIndex ++;	
// 		if(retIndex >= maxDircnt)
// 		{
// 			UINT c = GetFirstFreeClusNum();
// 			SetClus(cClus, c);
// 			SetClus(c, EOC_STATUS32);
// 			InitializeClus(c);
// 			retIndex = 0;
// 			dirAdd = ClusterToRelatAddr(c);
// 		}
// 	}
// 	int i;
// 	for(  i = nNeed - 2; i >0 ; i--)
// 	{
// 		Fat_LongDirectory longDir;
// 		memset(&longDir, 0x00, 32);
// 		longDir.LDIR_Attr = ATTR_LONG_NAME;
// 		longDir.LDIR_Chksum = chkFlag;
// 		longDir.LDIR_FstClusLO = 0;
// 		longDir.LDIR_Ord = (BYTE)i;
// 		longDir.LDIR_Type = 0;
// 		int nl = strlen(lpDirName);
// 		int npos = (i - 1) * 13;
// 		memset(longDir.LDIR_Name1, 0xff, 26);
// 		memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
// 		RwInfoFromLongDirInfo(dirRw, longDir);
// 		SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 		{
// 	
// 			printmsg("Error In create RootDirectory -- write dir Struct");
// 			return FALSE;
// 		}
// 		retIndex++;
// 		nIndex ++;	
// 		if(retIndex >= maxDircnt)
// 		{
// 			UINT c = GetFirstFreeClusNum();
// 			SetClus(cClus, c);
// 			SetClus(c, EOC_STATUS32);
// 			InitializeClus(c);
// 			retIndex = 0;
// 			dirAdd = ClusterToRelatAddr(c);
// 		}
// 	}
// 	//////////////////////////////////////////////////////////////////////////
// 		
// 	SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 	memcpy(&dir.DIR_Name, pName, 11);
// 	dir.DIR_Attr = 0x10;
// /*	dir.DIR_WrtDateTime = CTime::GetCurrentTime();
// 	dir.DIR_CrtDateTime = CTime::GetCurrentTime();
// 	dir.DIR_LstAcceDate = CTime::GetCurrentTime();
// */
// 	::GetSystemTime(&dir.DIR_WrtDateTime);
// 	::GetSystemTime(&dir.DIR_CrtDateTime);
// 	::GetSystemTime(&dir.DIR_LstAcceDate);
// 
// 
// 	UINT clus = srcTab.DIR_FstClusHI;
// 	clus = clus<<16|srcTab.DIR_FstClusLO;
// 		
// 	ASSERT(clus >= 2);
// 	dir.DIR_FstClusHI = srcTab.DIR_FstClusHI;
// 	dir.DIR_FstClusLO = srcTab.DIR_FstClusLO;
// 	
// 	RwInfoFromDirInfo(dirRw, dir);
// 	//SetDiskFilePointer(_hFile, dirAdd + 32 * nIndex, NULL, FILE_BEGIN);
// 	if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 	{
// 			
// 		printmsg("Error In create RootDirectory -- write dir Struct");
// 		return FALSE;
// 	}
// 	//
// 	//////////////////////////////////////////////////////////////////////////
// 		
// 	//////////////////////////////////////////////////////////////////////////
// 	// 修改..目录的表格
// 	UINT retAddr = ClusterToRelatAddr(clus);
// 	SetDiskFilePointer(_hFile, retAddr +32, NULL, FILE_BEGIN);
// 	ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);
// 	if(nRead != sizeof(Fat_DirectoryRW))
// 	{
// 		printmsg("Error! ");
// 		return FALSE;
// 	}		
// 	DirInfoFromRwInfo(dir, dirRw);
// 	ns = _curDirectory.size();
// 	if(ns == 0)
// 	{
// 		dir.DIR_FstClusHI = 0;
// 		dir.DIR_FstClusLO = 0;
// 	}
// 	else
// 	{
// 		Fat_Directory& fd = _curDirectory.at(ns - 1);
// 		dir.DIR_FstClusHI = fd.DIR_FstClusHI;
// 		dir.DIR_FstClusLO = fd.DIR_FstClusLO;	
// 	}
// 	RwInfoFromDirInfo(dirRw, dir);
// 	SetDiskFilePointer(_hFile, retAddr +32, NULL, FILE_BEGIN);
// 	WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL);
// 	if(nWrite != sizeof(Fat_DirectoryRW))
// 	{
// 		printmsg("Error! ");
// 		return FALSE;
// 	}	
// 
// 	//////////////////////////////////////////////////////////////////////////
// 	// 取消原来位置的目录表格
// 
// 	ns = srcDirs.size();
// 	int nPos; 
// 	int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
// 	BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 	UINT curC;
// 	if(ns == 1)
// 	{
// 		retAddr = _stOfRootAddr;
// 		nPos = srcTab.DIR_PosIndex;
// 		curC = _imgBpb.BPB_RootClus;
// 	//	retAddr = retAddr + nPos*32;		
// 	}
// 	else
// 	{
// 		UINT tempClu = srcDirs.at(ns - 2).DIR_FstClusHI;
// 		tempClu = tempClu<<16|srcDirs.at(ns - 2).DIR_FstClusLO;
// 		retAddr = ClusterToRelatAddr(tempClu);
// 		Fat_Directory& f = srcDirs.at(ns - 2);
// 		nPos = srcTab.DIR_PosIndex;
// 		curC = tempClu;
// 	}
// 	int nCluIndex = nPos /recPerCluster;  // 记录在该目录的第 几个簇下操作
// 	retIndex = nPos%recPerCluster;
// 	retAddr = ClusterToRelatAddr(curC);
// 	for(i = 0; i < nCluIndex; i++)
// 	{
// 		curC = _fats.at(curC);
// 		retAddr = ClusterToRelatAddr(curC);
// 	}
// 	if(retIndex < (nNeed -1))
// 	{
// 		retIndex = recPerCluster - (nNeed - 1 - retIndex);
// 		nCluIndex--;
// 		for(int i = 0; i < nCluIndex; i++)
// 		{
// 			curC = _fats.at(curC);
// 			retAddr = ClusterToRelatAddr(curC);
// 		}
// 	}
// 			
// 	retIndex = retIndex - (nNeed - 1);
// 	for(i = 0; i < nNeed; i++)
// 	{
// 		SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 		WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
// 		retIndex++;
// 		if(retIndex >= recPerCluster)
// 		{
// 			curC = _fats.at(curC);
// 			retAddr = ClusterToRelatAddr(curC);
// 			retIndex = 0;
// 		}
// 	}			
// 	delete lpDirName;
// 	return TRUE;
	return TRUE;
}


BOOL  DiskImgFile32::CreateImageFile(IN LPCSTR lpszFileName, IN UINT fatType, IN LONGLONG diskSize)					// 创建镜像文件
{
#ifdef _WINDOWS
	//ASSERT(AfxIsValidString(lpszFileName));

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
	RwInfoCopyFormBpb32(bppRw, _imgBpb);
			
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
	
	int i;
	for( i = 1; i < _imgBpb.BPB_RsvdSecCnt; i++)
	{
		BYTE resv[512];
		memset(resv,0x00, 512);
		if (!WriteDiskFile((HANDLE)_hFile, resv, 512, &nWritten, NULL))
		{
			printmsg("Write bpb Error!");
			return FALSE;
		}
	}

	dwNew = SetDiskFilePointer(_hFile, _imgBpb.BPB_BytsPerSec*_imgBpb.BPB_FSInfo, NULL, (DWORD)FILE_BEGIN);
	Fat_FsInfo_RW fsRw;
	RWFsInfoFromFsInfo(fsRw, _fsInfo);

	if(!WriteDiskFile(_hFile, &fsRw, sizeof(fsRw), &nWritten,  NULL))
	{
		printmsg("Write FsInfo Error!");
		return FALSE;
	}
	SetDiskFilePointer(_hFile, dwNew, NULL, (DWORD)FILE_BEGIN);

//
//////////////////////////////////////////////////////////////////////////

	_stOfFATAddr = _imgBpb.BPB_RsvdSecCnt*_imgBpb.BPB_BytsPerSec;

//////////////////////////////////////////////////////////////////////////
// Write FATs

	int nsFat = _fats.size();
	_fats.at(0) = 0xFFFFFFF8;
	_fats.at(1) = 0xFFFFFFFF;
	DWORD* fs = new DWORD[nsFat];
	for(i = 0; i< nsFat; i++)
	{
		fs[i] = _fats.at(i);
	}
	if (!WriteDiskFile((HANDLE)_hFile, fs, nsFat*4, &nWritten, NULL))
	{
		printmsg("Write FAT1 Error!");
		return FALSE;
	}

	if (!WriteDiskFile((HANDLE)_hFile, fs, nsFat*4, &nWritten, NULL))
	{
		printmsg("Write FAT2 Error!");
		return FALSE;
	}
	delete fs;

//
//////////////////////////////////////////////////////////////////////////

	_stOfClusterAddr = _stOfFATAddr + nsFat * 4 * _imgBpb.BPB_NumFATs; 
	_stOfRootAddr = _stOfClusterAddr;

//////////////////////////////////////////////////////////////////////////
// Write RootDirectory
	this->InitializeClus(_imgBpb.BPB_RootClus);

	SetDiskFilePointer(_hFile, _stOfRootAddr, NULL, FILE_BEGIN);
	BYTE roorDir[512];
	memset(roorDir, 0x00, 512);

	Fat_Directory dir;
	Fat_DirectoryRW dirRw;
	dir.DIR_Attr = ATTR_VOLUME_ID;

	::GetSystemTime(&dir.DIR_WrtDateTime);
	::GetSystemTime(&dir.DIR_CrtDateTime);
	::GetSystemTime(&dir.DIR_LstAcceDate);

/*	dir.DIR_CrtDateTime = CTime::GetCurrentTime();
	dir.DIR_LstAcceDate = CTime::GetCurrentTime();
	
	dir.DIR_WrtDateTime = CTime::GetCurrentTime();
*/	dir.DIR_FstClusHI = 0;
	dir.DIR_FstClusLO = 0;
	dir.DIR_FileSize = 0;
	memcpy(dir.DIR_Name, "NO NAME    ", 11);
	RwInfoFromDirInfo(dirRw, dir);
	WriteDiskFile(_hFile, &dirRw, 32, &nWritten, NULL);

//
//////////////////////////////////////////////////////////////////////////
#endif
	return TRUE;
}


BOOL DiskImgFile32::FormatImgFile(LPCSTR lpVolLab, IN UINT fatType /* = FAT16_TYPE */, IN LONGLONG diskSize /* = 134217728 */)
{
	Iinitialize(lpVolLab, fatType, diskSize);//FAT16_TYPE, DISK_128M);

	BootSector_BPB_RW bppRw;
	RwInfoCopyFormBpb32(bppRw, _imgBpb);
		
	//_vdFile.SetVDFile(bppRw.BPB_SecPerClus, _vdFile.VDFileType());
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
	
	dwNew = SetDiskFilePointer(_hFile, _imgBpb.BPB_BytsPerSec*_imgBpb.BPB_FSInfo, NULL, (DWORD)FILE_BEGIN);
	Fat_FsInfo_RW fsRw;
	RWFsInfoFromFsInfo(fsRw, _fsInfo);

	if(!WriteDiskFile(_hFile, &fsRw, sizeof(fsRw), &nWritten,  NULL))
	{
		//printmsg(L"Write FsInfo Error!");
		return FALSE;
	}
	
//
//////////////////////////////////////////////////////////////////////////

	_stOfFATAddr = _imgBpb.BPB_RsvdSecCnt*_imgBpb.BPB_BytsPerSec;

	SetDiskFilePointer(_hFile, _stOfFATAddr , NULL, (DWORD)FILE_BEGIN);

//////////////////////////////////////////////////////////////////////////
// Write FATs

	int nsFat = _fats.size();
	_fats.at(0) = 0xFFFFFFF8;
	_fats.at(1) = 0xFFFFFFFF;
	_fats.at(2) = EOC_STATUS32;
	DWORD* fs = new DWORD[nsFat];
	for(i = 0; i< nsFat; i++)
	{
		fs[i] = _fats.at(i);
#ifdef _MAC_UNIX
		fs[i] = Swap32Host2Little(fs[i]);
#endif
	}
	if (!WriteDiskFile((HANDLE)_hFile, fs, nsFat*4, &nWritten, NULL))
	{
		//printmsg(L"Write FAT1 Error!");
		return FALSE;
	}

	if (!WriteDiskFile((HANDLE)_hFile, fs, nsFat*4, &nWritten, NULL))
	{
		//printmsg(L"Write FAT2 Error!");
		return FALSE;
	}
	delete fs;
//
//////////////////////////////////////////////////////////////////////////

	_stOfRootAddr = _stOfFATAddr + nsFat* 4 * _imgBpb.BPB_NumFATs;
	_stOfClusterAddr = _stOfRootAddr;

//////////////////////////////////////////////////////////////////////////
// Write RootDirectory
	SetClus(_imgBpb.BPB_RootClus, EOC_STATUS32);
	this->InitializeClus(_imgBpb.BPB_RootClus);
	Fat_Directory dir;
	memset(&dir , 0 , sizeof(Fat_Directory));
	Fat_DirectoryRW dirRw;
	dir.DIR_Attr = ATTR_VOLUME_ID;
	::GetSystemTime(&dir.DIR_WrtDateTime);
	::GetSystemTime(&dir.DIR_CrtDateTime);
	::GetSystemTime(&dir.DIR_LstAcceDate);
/*
	dir.DIR_CrtDateTime = CTime::GetCurrentTime();
	dir.DIR_LstAcceDate = CTime::GetCurrentTime();
	dir.DIR_WrtDateTime = CTime::GetCurrentTime();
*/	dir.DIR_FstClusHI = 0;
	dir.DIR_FstClusLO = 0;
	dir.DIR_FileSize = 0;
	SetDiskFilePointer(_hFile, _stOfClusterAddr, NULL, FILE_BEGIN);
	memcpy(dir.DIR_Name, lpVolLab, 11);
	RwInfoFromDirInfo(dirRw, dir);

	if (!WriteDiskFile(_hFile, &dirRw, 32, &nWritten, NULL))
	{
		return FALSE;
	}
	

//
//////////////////////////////////////////////////////////////////////////
	return TRUE;
}

BOOL DiskImgFile32::RefreshFatTable()
{
	//return TRUE;
	
	int nsFat = _fats.size();
	int i;
	DWORD nWritten = 0;
	
	DWORD* fs = new DWORD[nsFat];
	for(i = 0; i< nsFat; i++)
	{
		fs[i] = _fats.at(i);
#ifdef _MAC_UNIX
		fs[i] = Swap32Host2Little(fs[i]);
#endif
	}
	
	SetDiskFilePointer(_hFile ,  _stOfFATAddr , 0 , FILE_BEGIN);
	//printf("%d , %d\n" , nsFat , sizeof(DWORD));
	if (!WriteDiskFile((HANDLE)_hFile, fs, nsFat*4, &nWritten, NULL))
	{
		delete fs;		
		return FALSE;
	}
	
	if (!WriteDiskFile((HANDLE)_hFile, fs, nsFat*4, &nWritten, NULL))
	{
		delete fs;
		return FALSE;
	}
	
	
	delete fs;
	
	return TRUE;
	
	
// 	DWORD nRead;
// 	SetDiskFilePointer(_hFile, _stOfFATAddr, NULL, FILE_BEGIN);
// 	int nFats = _imgBpb.BPB_FATSz32* BYSPERSEC / 4;
// 	_fats.resize(nFats);
// 	DWORD* fs = new DWORD[_fats.size()];
// 	memset(fs, 0x00, _fats.size()*4);
// 
// 	ReadDiskFile(_hFile, fs, _fats.size(), &nRead, NULL);
// 
// 	for(int i = 0; i < nFats; i++)
// 	{
// 		_fats.at(i) = fs[i];
// 	}
// 
// 
// 	delete fs;
// 	return TRUE;
}

BOOL DiskImgFile32::OpenImgFile(IN LPCSTR lpFileName, IN LONGLONG diskSize )
{
	int ret = CreateDiskFile(lpFileName);
	if(ret == 0) 
		return FALSE;
	else if (ret == 2)
		FormatImgFile(DEFAULT_VOLUME_NAME,0,diskSize);

	SetDiskFilePointer(_hFile, 0, NULL, FILE_BEGIN);
	
	DWORD nRead;

	BootSector_BPB_RW bpbRw;
	ZeroMemory(&bpbRw, sizeof(BootSector_BPB_RW));
	
	ReadDiskFile(_hFile, &bpbRw, sizeof(BootSector_BPB_RW), &nRead, NULL);
	printf("bpbRw.BPB_BytsPerSec=%d, bpbRw.BPB_SecPerClus=%d, nRead=%d\n", bpbRw.BPB_BytsPerSec, bpbRw.BPB_SecPerClus, nRead);

	if(nRead != sizeof(BootSector_BPB_RW))
	{
// 		if(printmsg("This img file isn't formatted, \n would like fromat is now?", MB_YESNO) == IDYES)
// 		{
// 			CloseHandle(_hFile);
// 			::DeleteFile(lpFileName);						
// 			goto OpFlag;
// 			
// 		}
// 		printmsg("Error in  open Imgfile--- Read bpb!");z
		return FALSE;
	}

	Bpb32CobyFromRwInfo(_imgBpb, bpbRw);
	
	_stOfFATAddr = _imgBpb.BPB_RsvdSecCnt*_imgBpb.BPB_BytsPerSec;

	SetDiskFilePointer(_hFile, _stOfFATAddr, NULL, FILE_BEGIN);
	int nFats = _imgBpb.BPB_FATSz32* BYSPERSEC / 4;
	_fats.resize(nFats);
	DWORD* fs = new DWORD[_fats.size()];
	memset(fs, 0x00, _fats.size()*4);

	ReadDiskFile(_hFile, fs, _fats.size()*4, &nRead, NULL);

	for(int i = 0; i < nFats; i++)
	{
#ifdef _MAC_UNIX
		fs[i] = Swap32Little2Host(fs[i]);		
#endif
		_fats.at(i) = fs[i];		
	}


	delete fs;
	_stOfRootAddr = nFats * 4 * _imgBpb.BPB_NumFATs + _stOfFATAddr;
	_stOfClusterAddr = _stOfRootAddr;
	
	
	return TRUE;
}


void  DiskImgFile32::Iinitialize(LPCSTR lpVolab, IN UINT fatType, IN LONGLONG diskSize)											// 初始化
{
	_imgSpace = Disk32SizeType(diskSize);

	ZeroMemory(&_imgBpb, sizeof(_imgBpb));

	//------ fix param ---------
	_imgBpb.BPB_RsvdSecCnt = 32;	
	_imgBpb.BPB_NumFATs = 2;
	_imgBpb.BPB_RootEntCnt = 0;
	//--------------
	
	UINT SecPerClus = GenSecPerClus32(diskSize);
	UINT  totsec = diskSize/ BYSPERSEC;	

	ASSERT(_imgSpace <= DISK32_U32G && _imgSpace >= 0);


	
	//////////////////////////////////////////////////////////////////////////	
	//以下是按照FW的公式计算FATSz , FW的公式计算出来的结果跟白皮书的有点偏差,
	//但是FW的计算跟windows自己格式化后的结果一样，而白皮书的反而是错的
	//结论:白皮书描述的公式应该是对的，但是转化成代码时，代码有个地方写错了，暂时找不到写错哪里
 	UINT RootDirSectors =_imgBpb.BPB_RootEntCnt;  // FAT32 rootdirsecs must be zero;
//   	UINT TmpVal1 = totsec - (_imgBpb.BPB_RsvdSecCnt + RootDirSectors);
//   	UINT TmpVal2 = (256 * SecPerClus) + _imgBpb.BPB_NumFATs;// /*BPB_NumFATs*/2;
//   	TmpVal2 = TmpVal2 / 2;		// 根据FAT32规定需要
//   	UINT FATSz = (TmpVal1 + (TmpVal2 -1)) / TmpVal2;

	//code from xiaoma
	UINT TempVar1 = diskSize/BYSPERSEC - 32 + 2*DskTableFAT32[_imgSpace].SecPerClusVal;	
    UINT TempVar2 = 128*DskTableFAT32[_imgSpace].SecPerClusVal + 2;	
    UINT FATSz = TempVar1/TempVar2;	
    if(TempVar1%TempVar2)		
        FATSz++;
	

#ifdef _DEBUG
	//check if BPB value is right

	UINT DataSec = totsec - (_imgBpb.BPB_RsvdSecCnt + (_imgBpb.BPB_NumFATs * FATSz) + RootDirSectors);	
	UINT CountofClusters = DataSec / SecPerClus;
	
	//TRACE("CountofClusters : %d\n" , CountofClusters);
	ASSERT(CountofClusters >= 65525 );
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
	_imgBpb.BPB_FATSz16 = 0;
	_imgBpb.BPB_FATSz32 = FATSz;		

	//////////////////////////////////////////////////////////////////////////
	// Wanted Modified
	_imgBpb.BPB_SecPerTrk = 0;		
	_imgBpb.BPB_NumHeads = 0;		
	//////////////////////////////////////////////////////////////////////////

	_imgBpb.BPB_HiddSec = 0;		

	_imgBpb.PB_ExtFlags = 0;
	_imgBpb.BPB_FSVer = 0;
	_imgBpb.BPB_RootClus = 2;
	_imgBpb.BPB_FSInfo = 1;
	_imgBpb.BPB_BkBootSec = 6;
	memset(_imgBpb.BPB_Reserved, 0x00, 12);
	
	
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
	memcpy(_imgBpb.BS_FilSysType, "FAT32   ", 8);	

// Initilize bpb End
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Initilize FAT Begin
	_fats.resize(_imgBpb.BPB_FATSz32*512 / 4); // 计算得到FAT项目的数目
	for( i = 0; i < _fats.size(); i++)
	{
		_fats.at(i) = 0X00;
	}
// Initialize FAT End
//////////////////////////////////////////////////////////////////////////

	memset(&_fsInfo, 0x00, sizeof(_fsInfo));
	_curDirectory.resize(0);	
}

BOOL  DiskImgFile32::SetClusFreeStatus(IN UINT StartClusNum)
{
	if (StartClusNum == 0) return TRUE;
	
	DWORD firstClus , lastClus  , nextClus , tmp;
	
	firstClus = lastClus = nextClus = StartClusNum;		
	
	while(1)
	{			
		tmp = _fats.at(nextClus);			
		_fats.at(nextClus) = FREE_STATUS32;			
		
		if(tmp == EOC_STATUS32 || tmp == FREE_STATUS32)
		{
			break;
		}
		
		if (tmp < firstClus) firstClus = tmp;
		else if(tmp > lastClus) lastClus = tmp;	
		
		nextClus = tmp;				
	}
	
//	DWORD pdw;
	DWORD totallen = (lastClus - firstClus+1) * 4;
	BYTE* buf = new BYTE[totallen];
	BYTE* p = buf;
	
	for (int i=StartClusNum;i<=lastClus;i++)
	{
		tmp = _fats.at(i);
		memcpy(p , &tmp , 4);
		p+=4;
	}
	
	UINT retValue = _stOfFATAddr + StartClusNum * 4;
	//SetDiskFilePointer(NULL, retValue, NULL, FILE_BEGIN);
	//WriteDiskFile(_hFile, buf, totallen,&pdw , NULL);
	
	delete buf;
	return TRUE;	
}

BOOL  DiskImgFile32::SetClusEx(IN UINT StartClusNum, IN UINT nNeedMoreClus)// 设置簇链 , Add by Joelee
{
	if (nNeedMoreClus == 0) return TRUE;	
	
	if(StartClusNum < 2)
		return FALSE;
	
	int nsFat = _fats.size();
	
	DWORD firstClus , lastClus  , nextClus , tmp;
	
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
			//InitializeClus(tmp);
			_fats.at(nextClus) = tmp;
			nextClus = tmp;
			
			if (tmp < firstClus) firstClus = tmp;
			else if(tmp > lastClus) lastClus = tmp;	
		}				
	}	
	
	_fats.at(lastClus) = EOC_STATUS32;
	
	//if(StartClusNum <  firstClus) firstClus = StartClusNum;
	
//	DWORD pdw;
	DWORD totallen = (lastClus - firstClus+1) * 4;
	BYTE* buf = new BYTE[totallen];
	BYTE* p = buf;
	
	for (i=StartClusNum;i<=lastClus;i++)
	{
		tmp = _fats.at(i);
		memcpy(p , &tmp , 4);
		p+=4;
	}
	
	UINT retValue = _stOfFATAddr + StartClusNum * 4;
	//SetDiskFilePointer(NULL, retValue, NULL, FILE_BEGIN);
	//WriteDiskFile(_hFile, buf, totallen,&pdw , NULL);
	
	delete buf;
	return TRUE;
}

BOOL  DiskImgFile32::SetClus(IN UINT clusNum, IN UINT nValue)	// 设置簇链
{
//	DWORD nWrite;
	if(clusNum < 2)
		return FALSE;
	if(_fats.size() <= clusNum )
	{
		//TRACE0("InValidate ClusNumber;\n");
		return FALSE;
	}
	
	UINT retValue = _stOfFATAddr + clusNum * 4;
	_fats.at(clusNum) = nValue;
	//SetDiskFilePointer(_hFile, retValue, NULL, FILE_BEGIN);

	DWORD v = _fats.at(clusNum);
	//WriteDiskFile(_hFile, &v, 4, &nWrite, NULL);

	return TRUE;
	
}

void  DiskImgFile32::InitializeClus(IN UINT clusNum)
{
	UINT pos = ClusterToRelatAddr(clusNum);
	SetDiskFilePointer(_hFile, pos, NULL, (DWORD)FILE_BEGIN);
	DWORD nWrite;
	UINT  len = BYSPERSEC * SecPerClus();// _imgBpb.BPB_SecPerClus;

	BYTE* byts = new BYTE[len];
	memset(byts, 0x00, len);
	WriteDiskFile(_hFile, byts, len, &nWrite, NULL);
	SetDiskFilePointer(_hFile, 0, NULL, (DWORD)FILE_BEGIN);
	delete []byts;
}

BOOL DiskImgFile32::GetVolLabel(LPSTR lpLabel)
{
//	DWORD nRead;
	memset(lpLabel, 0x00, 12);
	memcpy(lpLabel, _imgBpb.BS_VolLab, 11);

	UINT retAddr = _stOfRootAddr;
	DWORD nRead;
	Fat_DirectoryRW dirRw;
	SetDiskFilePointer(_hFile, retAddr, NULL, FILE_BEGIN);
	UINT maxcnt = _imgBpb.BPB_BytsPerSec * _imgBpb.BPB_SecPerClus / 32;
	UINT curClus = _imgBpb.BPB_RootClus;
	BOOL sign = TRUE;
	while (sign)
	{
		for(int i = 0; i < maxcnt; i++)
		{
			if(!ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL))
			{
				printmsg("Error in SetVolLabel --- read dirEntry");
			}
			if(dirRw.DIR_Name[0] == (char)0xE5)
				continue;
			if(dirRw.DIR_Name[0] == 0X00)
				return FALSE;
			if(IsVolLabel(dirRw.DIR_Attr))
			{
				memcpy(lpLabel, dirRw.DIR_Name, 11);
				TrimString(lpLabel, FALSE);
				return TRUE;
			}
			curClus = _fats.at(curClus);
			if(curClus == EOC_STATUS32)
			{
				i = maxcnt - 1;
				sign = FALSE;
				break;
			}
			retAddr = ClusterToRelatAddr(curClus);
		}
	}

	TrimString(lpLabel, FALSE);
	return true;
}

BOOL DiskImgFile32::SetVolLabel(LPCSTR lpVolLabel)
{

	UINT retAddr = _stOfRootAddr;
	UINT curClus = _imgBpb.BPB_RootClus;
	DWORD nWrite, nRead;
	Fat_DirectoryRW dirRw;
	SetDiskFilePointer(_hFile, retAddr, NULL, FILE_BEGIN);
	UINT maxcnt = _imgBpb.BPB_BytsPerSec * _imgBpb.BPB_SecPerClus / 32;
	while (1)
	{
		for(int i = 0; i < maxcnt; i++)
		{
			if(!ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL))
			{				
				printmsg("Error in SetVolLabel --- read dirEntry");
				return FALSE;
			}
			if(dirRw.DIR_Name[0] == (char)0xE5)
				continue;
			if(dirRw.DIR_Name[0] == 0X00)
				return FALSE;
			if(IsVolLabel(dirRw.DIR_Attr))
			{
				memset(dirRw.DIR_Name, ' ', 11);
				int nLen = strlen(lpVolLabel);
				memcpy(dirRw.DIR_Name, lpVolLabel, nLen);
				SetDiskFilePointer(_hFile, _stOfRootAddr + i*32, NULL, FILE_BEGIN);
				WriteDiskFile(_hFile, dirRw.DIR_Name, 11, &nWrite, NULL);
				SetDiskFilePointer(_hFile, 71, NULL, FILE_BEGIN);	// 根据BPB的偏移量来确定
				WriteDiskFile(_hFile, dirRw.DIR_Name, 11, &nWrite, NULL);
				return TRUE;
			}
			curClus = _fats.at(curClus);
			if(curClus == EOC_STATUS32)
				break;
			retAddr = ClusterToRelatAddr(curClus);
		}
	}	
	return FALSE;

}




BOOL  DiskImgFile32::ImgDeleteDirectory(UINT clusNum)
{
	Fat_DirectoryRW  dirRw;
	Fat_Directory dir;
	DWORD nRead;

	memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
	UINT retAddr = this->ClusterToRelatAddr(clusNum);
	
	SetDiskFilePointer(_hFile, retAddr, NULL, (DWORD)FILE_BEGIN);

	UINT maxDircnt = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;

	for(int i = 0; i < maxDircnt; i++)
	{	
		SetDiskFilePointer(_hFile, retAddr + i*32, NULL, (DWORD)FILE_BEGIN);
		ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
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
			UINT firstClus = dir.DIR_FstClusHI;
			firstClus  =  firstClus << 16 | dir.DIR_FstClusLO;
			
			ASSERT(firstClus >=2);
			UINT curc = firstClus;
			if(_fats.at(curc)== EOC_STATUS32)
			{
				SetClus(curc, FREE_STATUS32);
				return TRUE;
			}
			while(_fats.at(curc) != EOC_STATUS32)
			{
				InitializeClus(curc);
				UINT temp = _fats.at(curc);
				SetClus(curc, FREE_STATUS32);
				curc = temp;				
			}
			InitializeClus(curc);
			SetClus(curc, FREE_STATUS32);
			memset(dir.DIR_Name, 0x00, 8);			
		}
		else if(IsFoulder(dirRw.DIR_Attr))
		{
		
			LPSTR thisDir = ".       ";
			LPSTR upDir = "..      ";
			//LPSTR pch = GetStrFromChArry(dirRw.DIR_Name, 8);
			//if((strcmp(pch, thisDir) == 0) || (strcmp(pch, upDir) == 0))
			if((memcpy(dirRw.DIR_Name, thisDir, 8) == 0) || (memcpy(dirRw.DIR_Name, upDir, 8) == 0))
			{
				//delete pch;
				continue;
			}
			//delete pch;
			DirInfoFromRwInfo(dir, dirRw);
			UINT c = dir.DIR_FstClusHI;
			c = c << 16 | dir.DIR_FstClusLO;//_fats.at(dir.DIR_FstClusLO);
			while(1)
			{
				ImgDeleteDirectory(c);
				InitializeClus(c);
				UINT temp = c;
				c = _fats.at(c);
				SetClus(temp, FREE_STATUS32);
				if(c == (WORD)EOC_STATUS32)
					break;
			}
		}
	}
	return TRUE;
}

DWORD DiskImgFile32::SetFilePointerEx(HANDLE hFile,  
								 LONG lDistanceToMove,  
								 PLONG lpDistanceToMoveHigh,  
								 DWORD dwMoveMethod )
{
	ImgFileHandle* fh = (ImgFileHandle*)hFile;
	int method = (int)dwMoveMethod;

	UINT firstClu = fh->_fileTab.DIR_FstClusHI;
	firstClu = firstClu << 16;
	firstClu +=	fh->_fileTab.DIR_FstClusLO;
	
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
			fh->_curPos = ClusterToRelatAddr(firstClu) + lDistanceToMove%maxSizePerClus;
			fh->_curRetpos = lDistanceToMove;
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
// 			while(_fats.at(firstClu) != EOC_STATUS32)
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
// 		 fh->_exceedsize = fh->_curRetpos - fh->_fileTab.DIR_FileSize;
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



BOOL DiskImgFile32::ReadFileEx(HANDLE hFile,   
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
	SetDiskFilePointer(_hFile, fh->_curPos, NULL, FILE_BEGIN);
	if(nBufferLen <= nFirstSize) // 不用跨簇读
	{
		if (ReadDiskFile(_hFile, pBuffer, nBufferLen, &nRead, NULL))
		{
			fh->_curRetpos = fh->_curRetpos + nRead;
			fh->_curPos = fh->_curPos + nRead;			
			*nImgRead = nRead;
			return TRUE;
		}
		else
			return FALSE;
		
		
	}
	if (!ReadDiskFile(_hFile, pBuffer, nFirstSize, &nRead, NULL)) // 先读第一块，不够一簇的那快)
	{
		return FALSE;
	}
	
	*nImgRead = *nImgRead + nRead;

	int k = 0;
	UINT tempValue = 0xffff;
	UINT leftSize = nBufferLen - nFirstSize;
	UINT nFileClus = (leftSize +  bysPerClus - 1) / bysPerClus;
	UINT curClus = RelatAddrToCluster(fh->_curPos);
	curClus = _fats.at(curClus);
	if (curClus == EOC_STATUS32) return FALSE;
	UINT retAddr = ClusterToRelatAddr(curClus);
	DWORD nw;

	while(k < nFileClus)
	{
		SetDiskFilePointer(_hFile, retAddr, NULL, FILE_BEGIN);
		if((k + 1)*bysPerClus > leftSize)
		{
			if (!ReadDiskFile(_hFile, pBuffer + nFirstSize + k * bysPerClus, leftSize%bysPerClus, &nRead, NULL))
			{
				return FALSE;
			}
			
			fh->_curRetpos = fh->_curRetpos + nBufferLen;
			fh->_curPos = retAddr + leftSize%bysPerClus;
			*nImgRead = *nImgRead + nRead;
			Fat_DirectoryRW dirRw;
			::GetSystemTime(&fh->_fileTab.DIR_LstAcceDate);
			//fh->_fileTab.DIR_LstAcceDate = CTime::GetCurrentTime();
			RwInfoFromDirInfo(dirRw, fh->_fileTab);
			SetDiskFilePointer(_hFile, fh->_tabStAddr, NULL, FILE_BEGIN);
			if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nw, NULL))
			{
				printmsg("Error in WriteFileEx() Function");
				return FALSE;
			}
			return TRUE;
		}	
		
		if (!ReadDiskFile(_hFile, pBuffer + nFirstSize + k * bysPerClus, bysPerClus, &nRead, NULL))
		{
			return FALSE;
		}
		
		*nImgRead = *nImgRead + nRead;

		if (curClus == EOC_STATUS32 || curClus == FREE_STATUS32)
		{
			return FALSE;
		}
		
		curClus = _fats.at(curClus);

		fh->_curPos = fh->_curPos + bysPerClus;
// 		if(curClus == EOC_STATUS32)
// 			return FALSE;
		retAddr = ClusterToRelatAddr(curClus);
		k++;
	}	
	fh->_curPos = retAddr + leftSize%bysPerClus;
	fh->_curRetpos = fh->_curRetpos + nBufferLen;
	Fat_DirectoryRW dirRw;
	::GetSystemTime(&fh->_fileTab.DIR_LstAcceDate);
//	fh->_fileTab.DIR_LstAcceDate = CTime::GetCurrentTime();
	RwInfoFromDirInfo(dirRw, fh->_fileTab);
	SetDiskFilePointer(_hFile, fh->_tabStAddr, NULL, FILE_BEGIN);
	if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nw, NULL))
	{
		printmsg("Error in WriteFileEx() Function");
		return FALSE;
	}

	return TRUE;
}

BOOL	DiskImgFile32::IniWrite(HANDLE hFile , DWORD dwWriteLen)
{
	ImgFileHandle* fh = (ImgFileHandle*)hFile;

	UINT maxSizePerClus = _imgBpb.BPB_BytsPerSec*_imgBpb.BPB_SecPerClus;

	UINT firstClu = fh->_fileTab.DIR_FstClusHI;
	firstClu = firstClu << 16;
	firstClu +=	fh->_fileTab.DIR_FstClusLO; 

	if ( firstClu == 0)
	{
		UINT fstClus = GetFirstFreeClusNum();	
		SetClus(fstClus, EOC_STATUS32);

		fh->_fileTab.DIR_FstClusLO = fstClus;
		fh->_fileTab.DIR_FstClusHI = fstClus>>16;

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
	int i;
	for (i=0;i<needclus-1 ;i++)
	{
		//TRACE("%d\n" , _fats.at(lastclu));
		if(_fats.at(lastclu) == EOC_STATUS32)
		{
			UINT freeclus = GetFirstFreeClusNum();
#ifndef DEV_IMG
			InitializeClus(freeclus);
#endif
			SetClus(lastclu , freeclus);				
			SetClus(freeclus , EOC_STATUS32);
		}
		
		lastclu = _fats.at(lastclu); 
	}		
	
	if (fh->_curRetpos > fh->_fileTab.DIR_FileSize)
		fh->_fileTab.DIR_FileSize = fh->_curRetpos ;


	//calc write pos
	UINT curclus = fh->_curRetpos / maxSizePerClus;
	UINT curbyte = fh->_curRetpos % maxSizePerClus;
	lastclu = firstClu;
	for ( i=0;i<curclus;i++)
	{
		if(_fats.at(lastclu) == EOC_STATUS32)
		{
			return FALSE;//
		}

		lastclu = _fats.at(lastclu); 
	}

	fh->_curPos = ClusterToRelatAddr(lastclu) + curbyte;

	return TRUE;
}

// BOOL  DiskImgFile32::CalcNewPos(HANDLE hFile)
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
// 		UINT firstClu = fh->_fileTab.DIR_FstClusHI;
// 		firstClu = firstClu << 16;
// 		firstClu +=	fh->_fileTab.DIR_FstClusLO; 
// 		
// 		UINT lastclu = firstClu;		
// 
// 		while(1)
// 		{
// 			if(_fats.at(lastclu) == EOC_STATUS32)
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
// 				if(_fats.at(lastclu) == EOC_STATUS32)
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

BOOL DiskImgFile32::WriteFileEx(HANDLE hFile ,
								IN PBYTE pBuffer, 
								IN DWORD nBufferLen,
								OUT PDWORD nWrite) 
{
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
	UINT retAddr = ClusterToRelatAddr(nCurClu);
	
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
		if(nCurClu == EOC_STATUS32)
			break;
		retAddr = ClusterToRelatAddr(nCurClu);
		SetDiskFilePointer(devHandle, retAddr, NULL, FILE_BEGIN);
		k++;
	}
	fh->_curRetpos = fh->_curRetpos + nBufferLen;
	fh->_curPos = retAddr + nLeftNeedWriteSize%bysPerClus;
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

BOOL  DiskImgFile32::RenameFileEx(LPCSTR lpSrcName, LPCSTR lpNewFileName)
{
	return TRUE;
	//////////////////////////////////////////////////////////////////////////
	// 获取当前的

// 	if(lpSrcName == NULL)
// 		return FALSE;
// 
// /*	LPSTR lpCurDir = GetLeftStr(lpSrcName, '\\', FALSE);
// 
// 	LPSTR lpName = new char[MAX_PATH];
// 	memset(lpName, 0x00, MAX_PATH);
// 		
// 	if(!GetRighStrByFind(lpName, lpSrcName, '\\', -1, ""))
// 	{
// 		delete lpName;
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
// 			delete lpName;
// 			return FALSE;
// 		}
// 	}
// 	*/
// 	//////////////////////////////////////////////////////////////////////////
// 	// 删除原来的FAT
// 
// 	int ns = _curDirectory.size();
// 	UINT ncu;
// 	UINT retAddr = 0;
// 	if(ns == 0)
// 	{
// 		retAddr = _stOfRootAddr;
// 		ncu = _imgBpb.BPB_RootClus;
// 	}
// 	else
// 	{
// 		Fat_Directory& tem = _curDirectory.at(ns - 1);
// 		retAddr = ClusterToRelatAddr(tem.DIR_FstClusLO);
// 		ncu = tem.DIR_FstClusHI;
// 		ncu = ncu<<16|tem.DIR_FstClusLO; 
// 	}
// 
// 	int nIndex = 0;
// 	LPSTR lpTemp = new char[MAX_PATH];
// 	Fat_Directory dir;
// 	while(1)
// 	{
// 		memset(lpTemp, 0x00, MAX_PATH);
// 
// 		if(!GetDirectoryTabEx(dir, lpTemp, nIndex))
// 		{
// 		//	delete lpName;
// 			delete lpTemp;
// //			delete lpCurDir;
// 			return FALSE;
// 		}
// 		if(strcmpnocase(lpTemp, lpSrcName) == 0)
// 			break;
// 	}
// 
// 	delete lpTemp;
// 	
// 	int nNeed = CalcNeedTabCount(lpSrcName);
// 	UINT curC ;
// //	UINT retAddr;
// 	if(ns == 0 )	// 在根目录下删除文件
// 	{
// 		curC = _imgBpb.BPB_RootClus;
// 		retAddr = _stOfRootAddr;
// 	}
// 	
// 	else			// 在子目录下删除文件
// 	{
// 
// 		Fat_Directory& parDir = _curDirectory.at(ns - 1);
// 		curC = parDir.DIR_FstClusHI;
// 		curC = curC<<16|parDir.DIR_FstClusLO;
// 		retAddr = ClusterToRelatAddr(curC);		
// 	}
// 	int nPos = dir.DIR_PosIndex;
// 		
// 	BYTE emptyFlag[2] = {0xE5 , 0xFF};
// 	int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;	//一簇包含最大的值
// 	int nCluIndex = nPos /recPerCluster;  // 记录在该目录的第 几个簇下操作
// 	UINT retIndex = nPos%recPerCluster;
// 	
// 	int i;
// 	for( i = 0; i < nCluIndex; i++)
// 	{
// 		curC = _fats.at(curC);
// 		retAddr = ClusterToRelatAddr(curC);
// 	}
// 
// 	if(retIndex < (nNeed -1))
// 	{
// 		retIndex = recPerCluster - (nNeed - 1 - retIndex);
// 		nCluIndex--;
// 		for(int i = 0; i < nCluIndex; i++)
// 		{
// 			curC = _fats.at(curC);
// 			retAddr = ClusterToRelatAddr(curC);
// 		}
// 	}
// 
// 	DWORD nWrite;
// 	retIndex = retIndex - (nNeed - 1); 
// 	for(i = 0; i < nNeed; i++)
// 	{
// 		SetDiskFilePointer(_hFile, retAddr + retIndex * 32, NULL, FILE_BEGIN);
// 		WriteDiskFile(_hFile, emptyFlag, 2, &nWrite, NULL);
// 		retIndex++;
// 		if(retIndex >= recPerCluster)
// 		{
// 			curC = _fats.at(curC);
// 			retAddr = ClusterToRelatAddr(curC);
// 			retIndex = 0;
// 		}
// 	}
// //	delete lpLongName;
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
// 		delete lpName;
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
// 	if (!GenerateShortName(lpNewFileName, pName))
// 	{
// 		delete pName;
// 		return FALSE;
// 	}
// 	//new char[11];
// 	
// 
// 	DWORD nRead;
// 
// 	Fat_DirectoryRW dirRw;
// 
// 	memset(&dirRw, 0x00, sizeof(Fat_DirectoryRW));
// //	memset(&dir, 0x00, sizeof(Fat_Directory));
// 
// 	ns = _curDirectory.size();
// 	
// 	BYTE chkFlag = ChkSum(pName);	
// 
// 	UINT cClus;
// 	UINT dirAdd;
// 
// 	if(ns == 0) // 在根目录下创建文件
// 	{
// 		dirAdd = _stOfRootAddr;
// 		cClus = _imgBpb.BPB_RootClus;
// 			
// 	}
// 	else		// 在其他目录下建立文件
// 	{
// 		Fat_Directory curD = _curDirectory.at(_curDirectory.size() - 1);
// 
// 		cClus = curD.DIR_FstClusHI;
// 		cClus = cClus<<16|curD.DIR_FstClusLO;
// 		
// 		dirAdd = this->ClusterToRelatAddr(cClus);
// 	}
// 
// 	 nIndex = -1;
// 	//////////////////////////////////////////////////////////////////////////
// 	// 开始建立目录项
// 
// 	SetDiskFilePointer(_hFile, dirAdd, NULL, (DWORD)FILE_BEGIN);
// 	UINT maxDircnt = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;
// 	int  nDirPos = 0; // 记录目录的相对位置 放置在Fat_Directory.DIR_PosIndex中
// 	while(1)
// 	{
// 		for(int i = 0; i < maxDircnt; i++)
// 		{	
// 			ReadDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nRead, NULL);		
// 			if(nRead != sizeof(Fat_DirectoryRW))
// 			{
// 				delete pLongName;
// 				delete pName;
// 				printmsg("Error In Create SubFile --Read DirRW Error!");
// 				return FALSE;
// 			}
// 			
// 			if(dirRw.DIR_Name[0] == 0x00 || ((dirRw.DIR_Name[0] == (char)0xE5) && nNeed == 1))   // 判断是否为空项
// 			{
// 				nIndex = i;
// 				break;
// 			}	
// 			nDirPos++;
// 		}
// 		if(nIndex != -1)
// 			break;
// 		UINT tmpClu = cClus;
// 		cClus = _fats.at(cClus);
// 		if(cClus == EOC_STATUS32)
// 		{
// 			cClus = tmpClu;
// 			break;
// 		}
// 		dirAdd = ClusterToRelatAddr(cClus);
// 		SetDiskFilePointer(_hFile, dirAdd, NULL, (DWORD)FILE_BEGIN);
// 	}
// 	if(nIndex == -1)
// 	{
// 		UINT c = GetFirstFreeClusNum();
// 		SetClus(cClus, c);
// 		SetClus(c, EOC_STATUS32);
// 		InitializeClus(c);
// 		nIndex = nDirPos;
// 		dirAdd = ClusterToRelatAddr(c);
// 	}	
// 
// 	//////////////////////////////////////////////////////////////////////////
// 	// 处理长名目录项目
// 	 retIndex = nIndex%maxDircnt;
// 	if(nNeed > 1)
// 	{
// 		Fat_LongDirectory longDir;
// 		memset(&longDir, 0x00, 32);
// 		longDir.LDIR_Attr = ATTR_LONG_NAME;
// 		longDir.LDIR_Chksum = chkFlag;
// 		longDir.LDIR_FstClusLO = 0;
// 		longDir.LDIR_Ord = LAST_LONG_ENTRY|(BYTE)(nNeed-1);
// 		longDir.LDIR_Type = 0;
// 		int npos = (nNeed - 2) * 13;
// 		memset(longDir.LDIR_Name1, 0xff, 26);
// 		memcpy(longDir.LDIR_Name1, pLongName + npos, (nl - npos)*2);		// 将剩余的名字全部拷贝进去
// 		if((nl - npos) < 13)
// 			memset(longDir.LDIR_Name1 + (nl - npos),  0x00, 2);
// 		RwInfoFromLongDirInfo(dirRw, longDir);
// 		SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 		{
// 			delete pLongName;
// 			delete pName;
// 			printmsg("Error In create RootDirectory -- write dir Struct");
// 			return FALSE;
// 		}
// 		retIndex++;
// 		nIndex ++;
// 		if(retIndex >= maxDircnt)
// 		{
// 			UINT c = GetFirstFreeClusNum();
// 			SetClus(cClus, c);
// 			SetClus(c, EOC_STATUS32);
// 			InitializeClus(c);
// 			retIndex = 0;
// 			dirAdd = ClusterToRelatAddr(c);
// 		}
// 	}
// 	for(i = nNeed - 2; i >0 ; i--)
// 	{
// 		Fat_LongDirectory longDir;
// 		memset(&longDir, 0x00, 32);
// 		longDir.LDIR_Attr = ATTR_LONG_NAME;
// 		longDir.LDIR_Chksum = chkFlag;
// 		longDir.LDIR_FstClusLO = 0;
// 		longDir.LDIR_Ord = (BYTE)i;
// 		longDir.LDIR_Type = 0;
// 		int nl = strlen(pName);
// 		int npos = (i - 1) * 13;
// 		memset(longDir.LDIR_Name1, 0xff, 26);
// 		memcpy(longDir.LDIR_Name1, pLongName + npos, 13*2);		// 将剩余的名字全部拷贝进去
// 		RwInfoFromLongDirInfo(dirRw, longDir);
// 		SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 		if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 		{
// 			delete pLongName;
// 			delete pName;
// 			printmsg("Error In create RootDirectory -- write dir Struct");
// 			return FALSE;
// 		}
// 		
// 		nIndex ++;	
// 		retIndex++;
// 		if(retIndex >= maxDircnt)
// 		{
// 			UINT c = GetFirstFreeClusNum();
// 			SetClus(cClus, c);
// 			SetClus(c, EOC_STATUS32);
// 			InitializeClus(c);
// 			retIndex = 0;
// 			dirAdd = ClusterToRelatAddr(c);
// 		}
// 	}
// 
// 
// 		//////////////////////////////////////////////////////////////////////////
// 			
// 		memcpy(&dir.DIR_Name, pName, 11);
// 		::GetSystemTime(&dir.DIR_WrtDateTime);
// 		::GetSystemTime(&dir.DIR_LstAcceDate);
// 			
// 		RwInfoFromDirInfo(dirRw, dir);
// 
// 
// 	SetDiskFilePointer(_hFile, dirAdd + 32 * retIndex, NULL, FILE_BEGIN);
// 	if(!WriteDiskFile(_hFile, &dirRw, sizeof(Fat_DirectoryRW), &nWrite, NULL))
// 	{
// 		delete pName;
// 		printmsg("Error In create RootFile -- write dir Struct");
// 		return FALSE;
// 	}
// 	
// 
// 	delete pLongName;
// 	delete pName;
// 	return TRUE;
}



LPTSTR DiskImgFile32::RootPath()
{
	return _T("\\");
}

DWORD DiskImgFile32::SectorsPerCluster()
{
	return _imgBpb.BPB_SecPerClus;
}

DWORD DiskImgFile32::BytesPerSector()
{
	return _imgBpb.BPB_BytsPerSec;
}

DWORD DiskImgFile32::NumberOfFreeClusters()
{
	UINT nFreeSum= 0;
	int ns = _fats.size();
	for(int i = 0; i < ns; i++)
	{
		if(_fats.at(i) == FREE_STATUS32)
			nFreeSum++;
	}
	return (DWORD)nFreeSum;
}


HANDLE DiskImgFile32::CreateFileEx(LPCTSTR lpFileName,     
									DWORD dwDesiredAccess,      
									DWORD dwShareMode,          
									LPSECURITY_ATTRIBUTES lpSecurityAttributes,	
									DWORD dwCreationDisposition, 
									DWORD dwFlagsAndAttributes,  
									HANDLE hTemplateFile         
									)
{
	//printf("eee CreateFileEx\n");
	if(lpFileName == NULL)
			return (HANDLE)-1;

	LPTSTR lpCurDir = GetLeftStr(lpFileName, _T('\\'), FALSE);

	LPTSTR lpName = new TCHAR[MAX_PATH];
	memset(lpName, 0x00, MAX_PATH * sizeof(TCHAR));
		
	//printf("bbb GetRighStrByFind\n");
	if(!GetRighStrByFind(lpName, lpFileName, _T('\\'), -1, _T("")))
	{
		printf("!GetRighStrByFind, return -1\n");
		delete lpName;
		return (HANDLE)-1;
	}

	if(lpCurDir == NULL)
	{
		//printf("bbb SetCurrentDirectory\n");
		this->SetCurrentDirectory(_T("\\"));
		//printf("SetCurrentDirectory\n");
		HANDLE handle;
		if (!this->ImgCreateFile(lpName, NULL, dwFlagsAndAttributes, 0, handle))
		{
			delete lpName;
			return (HANDLE)-1;
		}
		//printf("ImgCreateFile\n");

		ImgFileHandle* fh = (ImgFileHandle*)handle;
		if(fh)
		{
			fh->_shareMode = dwShareMode;
			fh->_accMode = dwDesiredAccess;
		}
		delete lpName;
		return handle;
	}



	//printf("bbb SetCurrentDirectory 1\n");
	if(!SetCurrentDirectory(lpCurDir))
	{
		//printf("!SetCurrentDirectory lpCurDir\n");
		if(dwCreationDisposition == OPEN_EXISTING)
		{
			//printmsg("No such file exite!");
			delete lpName;
			delete lpCurDir;
			return (HANDLE)-1;
		}

		this->CreateDirectoryEx(lpCurDir);
		//printf("CreateDirectoryEx\n");
		this->SetCurrentDirectory(lpCurDir);
		//printf("SetCurrentDirectory\n");

		HANDLE handle;
		if (!this->ImgCreateFile(lpName, NULL, dwFlagsAndAttributes, 0, handle))
		{
			delete lpName;
			return (HANDLE)-1;
		}
		//printf("ImgCreateFile\n");

		ImgFileHandle* fh = (ImgFileHandle*)handle;
		if(fh)
		{
			fh->_shareMode = dwShareMode;
			fh->_accMode = dwDesiredAccess;
		}
		delete lpName;
		delete lpCurDir;
		return handle;
	}


	//printf("bbb SetCurrentDirectory 2\n");
	
	int ns = _curDirectory.size();
	UINT ncu;
	UINT retAddr = 0;
	if(ns == 0)
	{
		retAddr = _stOfRootAddr;
		ncu = _imgBpb.BPB_RootClus;
	}
	else
	{
		Fat_Directory& tem = _curDirectory.at(ns - 1);
		retAddr = ClusterToRelatAddr(tem.DIR_FstClusLO);
		ncu = tem.DIR_FstClusHI;
		ncu = ncu<<16|tem.DIR_FstClusLO; 
	}
	//printf("ClusterToRelatAddr\n");

	int nIndex = 0;
	LPTSTR lpTemp = new TCHAR[MAX_PATH];
	Fat_Directory dir;
	while(1)
	{
		memset(lpTemp, 0x00, MAX_PATH * sizeof(TCHAR) );

// 		if (nIndex >= 124)
// 		{
// 			TRACE(L"1");
// 		}

		//printf("bbbb GetDirectoryTabEx\n");
		if(!GetDirectoryTabEx(dir, lpTemp, nIndex))
		{
			//printf("!GetDirectoryTabEx\n");
			if(dwCreationDisposition == OPEN_EXISTING)
			{
				delete lpTemp;
				delete lpName;
				delete lpCurDir;
				//printmsg("No such file exite!");
				return (HANDLE)-1;
			}

			HANDLE handle;
			if (!this->ImgCreateFile(lpName, NULL, dwFlagsAndAttributes, 0, handle))
			{
				delete lpName;
				return (HANDLE)-1;
			}	
			//printf("ImgCreateFile\n");
			ImgFileHandle* fh = (ImgFileHandle*)handle;
			if(fh)
			{
				fh->_shareMode = dwShareMode;
				fh->_accMode = dwDesiredAccess;
			}
			delete lpName;
			delete lpTemp;
			delete lpCurDir;
			return handle;
		}

//		printf("bbbb strcmpnocase %s , %s\n" , lpTemp , lpName);
		if(strcmpnocase(lpTemp, lpName) == 0)
			break;
		//printf("strcmpnocase\n");

// 		if (strcmpnocase(lpTemp , "&&&&&416.OGG") == 0)
// 		{
// 			printf("a");
// 		}
	}


	
	int recPerCluster = BYSPERSEC * _imgBpb.BPB_SecPerClus / 32;
	//printf("_imgBpb.BPB_SecPerClus = %d, recPerCluster=%d", _imgBpb.BPB_SecPerClus, recPerCluster);
	int nCluIndex = dir.DIR_PosIndex / recPerCluster;
	

	for(int i = 0; i < nCluIndex; i++)
	{
		ncu = _fats.at(ncu);
	}
	retAddr = ClusterToRelatAddr(ncu) + dir.DIR_PosIndex %recPerCluster * 32;

	ImgFileHandle* fh = new ImgFileHandle;
	fh->_curRetpos = 0;
	fh->_exceedsize = 0;
	UINT nc = dir.DIR_FstClusHI;
	nc = nc<<16|dir.DIR_FstClusLO;
	fh->_stAddr = ClusterToRelatAddr(nc);
	fh->_curPos = fh->_stAddr;

	memcpy(&fh->_fileTab, &dir, sizeof(dir));
	fh->_tabStAddr = retAddr;
	fh->_shareMode = dwShareMode;
	fh->_accMode = dwDesiredAccess;
	delete lpName;
	delete lpTemp;
	delete lpCurDir;
	return (HANDLE)fh;
}



HANDLE DiskImgFile32::FindFirstFile(LPCTSTR lptFileName,					// pointer to name of file to search for
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
		lptFileName = _T("*.*");	
	if(_tcsstr((LPCTSTR)lptFileName, (LPCTSTR)_T("*")) != NULL)
		fh->_findData.dwReserved0 = 1;			// 表示找相似like
	lstrcpy((LPTSTR)fh->_findData.cFileName, lptFileName);
	memset(fh->_findData.cAlternateFileName, 0x00, 14 * sizeof(TCHAR));
	lstrcpy((LPTSTR)fh->_findData.cAlternateFileName, lptFileName);
	_tcsupr(fh->_findData.cAlternateFileName);
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
BOOL DiskImgFile32::FindNextFile(
		HANDLE hFindFile,					// handle to search
		LPWIN32_FIND_DATA lpFindFileData	// pointer to structure for data on found file
		) 
{	
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
	//UINT retAddr = ClusterToRelatAddr(fh->_curCluster);
	Fat_Directory dir;
	LPTSTR longName = new TCHAR[MAX_PATH];
	LPTSTR cmpStr = new TCHAR[MAX_PATH];
	memset(cmpStr, 0x00, MAX_PATH * sizeof(TCHAR));
	memcpy(cmpStr, fh->_findData.cAlternateFileName, 14 * sizeof(TCHAR));
//	RemoveChar(cmpStr, '.');
	RemoveTChar(cmpStr, _T(' '));
	RemoveTChar(cmpStr, _T('*'));
	RemoveTChar(cmpStr, _T('?'));
	
	int cmpLen = _tcslen(cmpStr);
	int n = fh->_nRetIndex ;
	int k = 0;
	while (1)
	{
		k++;
		memset(longName, 0x00, MAX_PATH);
	
		if(!GetDirectoryTabEx(dir, fh->_curCluster,longName, n))
		{
			delete longName;
			delete cmpStr;
			return FALSE;
		}
		fh->_nRetIndex = n;
		memset(fh->_findData.cFileName, 0x00, MAX_PATH * sizeof(TCHAR));
		lstrcpy((LPTSTR)fh->_findData.cFileName ,longName);
		_tcsupr(longName);
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

	/*	comment out by joelee , 2008-08-21
		In fat32 , there is no limit  of dir counts in root folder,by in FAT16 , MAX dir count is 128
		if(fh->_nRetIndex >= maxdircnt)
		{
			if(fh->_curCluster == 0)
			{
				delete longName;
				delete cmpStr;
				return FALSE;
			}
			fh->_curCluster = _fats.at(fh->_curCluster);
			if(fh->_curCluster == EOC_STATUS32)
			{
				delete longName;
				delete cmpStr;
				return FALSE;
			}
			fh->_nRetIndex = 0;
			n = 0;
		}
		*/

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
	delete longName;
	delete cmpStr;
	return TRUE;	
}

BOOL DiskImgFile32::FindClose(HANDLE hFindFile)
{
	ImgFindHandle* fh =  (ImgFindHandle*)hFindFile;	
	delete fh;
	return TRUE;
}


UINT DiskImgFile32::ImgFileType()
{
	return FAT32_TYPE;
}

LPTSTR DiskImgFile32::GetMirrorFileName()
{
	return _lpImgFileName;
}

LONGLONG DiskImgFile32::TotoleSpace()
{
	return (LONGLONG)this->TolSec()* this->BytesPerSec();
}


BOOL DiskImgFile32::SetCurrentDirectory(IN LPCTSTR lptPathName)						// 设置当前路径	
{
	if(!ParaDirectoryFromStr(lptPathName, _curDirectory))
		return FALSE;
	
	return TRUE;
}

BOOL DiskImgFile32::ParaDirectoryFromStr(IN LPCTSTR lptDirName, OUT vector<Fat_Directory>& fatDir)
{
	int nLen = _tcslen(lptDirName);
	LPTSTR lptsz = NULL;
	//LPSTR lpFirstDir = NULL;
	
	BOOL isLast = FALSE;
	fatDir.resize(0);
	
	if(lptDirName == NULL)
		return FALSE;
	if(_tcscmp(lptDirName, _T("\\")) == 0)
		return TRUE;
	
	lptsz = new TCHAR[nLen + 1];
	memset(lptsz, 0x00, (nLen + 1) * sizeof(TCHAR));
	LPTSTR lpTem = lptsz;	
	_tcscpy(lptsz , lptDirName);
	lptsz[nLen] = '\0';
	UINT curClus = 0;
		
	lptsz = (TCHAR*)_tcsinc(lptsz);
	
	while(lptsz)
	{
		LPTSTR longName =  GetLeftStr(lptsz, _T('\\'), TRUE);
		if(longName == NULL)
			longName = lptsz;		
		
		lptsz = (TCHAR*)_tcsstr(lptsz, _T("\\"));		

		if(lptsz)
			lptsz = (TCHAR*)_tcsinc(lptsz);
		
		int ns = fatDir.size();
		if(ns == 0)
			curClus ;
		else
		{
			//curClus = fatDir.at(ns - 1).DIR_FstClusLO;

			Fat_Directory fat = fatDir.at(ns - 1);
			curClus = fat.DIR_FstClusHI;
			curClus = curClus << 16;
			curClus = curClus | fat.DIR_FstClusLO ;
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

BOOL DiskImgFile32::GetDirectoryTab(OUT Fat_Directory& dir, IN UINT clus, IN LPCTSTR longName)						 // 返回簇clus下目录下长名为longName的目录
{
	int nIndex = 0;
	TCHAR lptTemp[MAX_PATH];
	
	while(1)
	{
		memset(lptTemp, 0x00, MAX_PATH * sizeof(TCHAR));
		
		if(!GetDirectoryTabEx(dir, clus, lptTemp, nIndex))
			return FALSE;
		if(_tcsicmp(lptTemp, longName) == 0)		
			return TRUE;
	}
	
	return FALSE;
}

BOOL DiskImgFile32::CreateDirectoryEx(IN LPCTSTR lpFullDir)
{
	vector<DirPaths> paths;
	ParaPathFromStr(lpFullDir, paths);
	int ns = paths.size();

	LPTSTR lptPath = new TCHAR[MAX_PATH];
	memset(lptPath, 0x00, MAX_PATH * sizeof(TCHAR));
	BOOL ret = FALSE;
	for(int i = 0; i < ns; i++)
	{
		_tcscat(lptPath, _T("\\"));
		
		LPTSTR pck = GetStrFromChArry(paths.at(i).pName, _tcslen(paths.at(i).pName));
		_tcscat(lptPath, pck);

		if(SetCurrentDirectory(lptPath))
		{
			delete pck;
			continue;
		}
		
		ret = this->ImgCreateDirectory(pck);
		delete pck;	
		
		if (!ret)break;		
	}
	delete lptPath;
	return ret;
}

// BOOL  DiskImgFile32::IsFileExist(LPCSTR lpFileName)
// {
// 	return TRUE;
//}
