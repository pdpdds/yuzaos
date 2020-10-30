#include "common.h"
#include "Support.h"
//#include "ExtFsCommon.h"
//#include <mbstring.h>

// DskszToSecperClus DskTableFAT12 [] = {
// { 4096,1}
// };

DskszToSecperClus DskTableFAT16 [] = {
{ 8400, 1}, /* disks up to 4.1 MB, the 0 value for SecPerClusVal trips an error */
{ 32680, 2}, /* disks up to 16 MB, 1k cluster */
{ 262144, 4}, /* disks up to 128 MB, 2k cluster */
{ 524288, 8}, /* disks up to 256 MB, 4k cluster */
{ 1048576, 16}, /* disks up to 512 MB, 8k cluster */
/* The entries after this point are not used unless FAT16 is forced */
{ 2097152, 32}, /* disks up to 1 GB, 16k cluster */
{ 4194304, 64}, /* disks up to 2 GB, 32k cluster */
{ 0xFFFFFFFF, 0} /* any disk greater than 2GB, 0 value for SecPerClusVal trips an error */
};

DskszToSecperClus DskTableFAT32 [] = {
{ 66600, 1}, /* disks up to 32.5 MB, the 0 value for SecPerClusVal trips an error */
//{ 532480, 8}, /* disks up to 260 MB, .5k cluster */
{ 532480, 1}, /* disks up to 260 MB, .5k cluster */
{ 16777216, 8}, /* disks up to 8 GB, 4k cluster */
{ 33554432, 16}, /* disks up to 16 GB, 8k cluster */
{ 67108864, 32}, /* disks up to 32 GB, 16k cluster */
{ 0xFFFFFFFF, 64}/* disks greater than 32GB, 32k cluster */
};

UINT GenSecPerClus(LONGLONG disksize)
{
	UINT SecCount = disksize / BYSPERSEC;
	
	int iIndex = 0;
	
	do 
	{
		if (DskTableFAT16[iIndex].SecPerClusVal == 0) break;		
		
		if (SecCount <= DskTableFAT16[iIndex].DiskSize)
		{
			return DskTableFAT16[iIndex].SecPerClusVal; 
		}
		
		iIndex++;
		
	} while(1);//DskTableFAT16,总的Sector数和每Cluster含有Sector数
	
	
	return 0;
}

UINT GenSecPerClus32(LONGLONG disksize)
{
	UINT SecCount = disksize / BYSPERSEC;
	
	int iIndex = 0;
	
	do 
	{
		if (DskTableFAT32[iIndex].SecPerClusVal == 64) break;		
		
		if (SecCount <= DskTableFAT32[iIndex].DiskSize)
		{
			return DskTableFAT32[iIndex].SecPerClusVal;
		}
		
		iIndex++;
		
	} while(1);
	
	
	return 0;
}

void BpbCobyFromRwInfo(BootSector_BPB& bpb, const BootSector_BPB_RW& mbr)
{
	memcpy(bpb.BS_jmpBoot, mbr.BS_jmpBoot, 3);		//0

	memcpy(bpb.BS_OEMName, mbr.BS_OEMName,8);		//3

	memcpy(&bpb.BPB_BytsPerSec,mbr.BPB_BytsPerSec, 2) ;	//11

	bpb.BPB_SecPerClus = mbr.BPB_SecPerClus;	//13

	memcpy(&bpb.BPB_RsvdSecCnt, mbr.BPB_RsvdSecCnt, 2);	//14

	bpb.BPB_NumFATs = mbr.BPB_NumFATs;		//16

	memcpy(&bpb.BPB_RootEntCnt, mbr.BPB_RootEntCnt, 2);	//17

	memcpy(&bpb.BPB_TotSec16, mbr.BPB_TotSec16, 2);		//19

	bpb.BPB_Media = mbr.BPB_Media;			//21

	memcpy(&bpb.BPB_FATSz16 , mbr.BPB_FATSz16, 2);		//22
	memcpy(&bpb.BPB_SecPerTrk, mbr.BPB_SecPerTrk, 2) ;		//24
	memcpy(&bpb.BPB_NumHeads, mbr.BPB_NumHeads, 2) ;		//26
	memcpy(&bpb.BPB_HiddSec, mbr.BPB_HiddSec, 4);		//28
	memcpy(&bpb.BPB_TotSec32, mbr.BPB_TotSec32, 4);		//32

	bpb.BS_DrvNum = mbr.BPB_OTHERS[0];	//mbr.BS_DrvNum;			//36
	bpb.BS_Reserved1 = mbr.BPB_OTHERS[1];		//37
	bpb.BS_BootSig = mbr.BPB_OTHERS[2];		//38 
    memcpy(&bpb.BS_VolID, &mbr.BPB_OTHERS[3], 4);			//39  
	memcpy(bpb.BS_VolLab, &mbr.BPB_OTHERS[7], 11);	//43 
	memcpy(bpb.BS_FilSysType, &mbr.BPB_OTHERS[18], 8);	//54 

}

void RWFsInfoFromFsInfo(OUT Fat_FsInfo_RW& rw, IN const Fat_FsInfo fs)
{
	memset(&rw, 0x00, sizeof(rw));
	memcpy(rw.FSI_LeadSig, &fs.FSI_LeadSig, 4);
	memcpy(rw.FSI_StrucSig, &fs.FSI_StrucSig, 4);
	memcpy(rw.FSI_Free_Count, &fs.FSI_Free_Count, 4);
	memcpy(rw.FSI_Nxt_Free, &fs.FSI_Nxt_Free,4);
	memcpy(rw.FSI_TrailSig, &fs.FSI_TrailSig, 4);
}

void FsInfoFromRWFsInfo(OUT Fat_FsInfo fs, IN const Fat_FsInfo_RW rw)
{	
	memcpy(&fs.FSI_LeadSig, rw.FSI_LeadSig, 4);
	memcpy(&fs.FSI_StrucSig, rw.FSI_StrucSig, 4);
	memcpy(&fs.FSI_Free_Count, rw.FSI_Free_Count, 4);
	memcpy(&fs.FSI_Nxt_Free, rw.FSI_Nxt_Free, 4);
	memcpy(&fs.FSI_TrailSig, rw.FSI_TrailSig, 4);
}


void RwInfoCopyFormBpb(OUT BootSector_BPB_RW& mbr, IN const BootSector_BPB& bpb)
{
	memcpy(mbr.BS_jmpBoot, bpb.BS_jmpBoot, 3);		//0

	memcpy(mbr.BS_OEMName, bpb.BS_OEMName, 8);		//3

	memcpy(mbr.BPB_BytsPerSec, &bpb.BPB_BytsPerSec,2) ;	//11

	mbr.BPB_SecPerClus = bpb.BPB_SecPerClus  ;	//13

	memcpy(mbr.BPB_RsvdSecCnt, &bpb.BPB_RsvdSecCnt, 2);	//14

	mbr.BPB_NumFATs = bpb.BPB_NumFATs ;		//16

	memcpy(mbr.BPB_RootEntCnt, &bpb.BPB_RootEntCnt, 2);	//17

	memcpy(mbr.BPB_TotSec16, &bpb.BPB_TotSec16, 2);		//19

	mbr.BPB_Media = bpb.BPB_Media;			//21

	memcpy(mbr.BPB_FATSz16, &bpb.BPB_FATSz16 , 2);		//22
	memcpy(mbr.BPB_SecPerTrk, &bpb.BPB_SecPerTrk, 2) ;		//24
	memcpy(mbr.BPB_NumHeads, &bpb.BPB_NumHeads, 2) ;		//26
	memcpy(mbr.BPB_HiddSec, &bpb.BPB_HiddSec, 4);		//28
	memcpy(mbr.BPB_TotSec32, &bpb.BPB_TotSec32, 4);		//32

	memset(mbr.BPB_OTHERS, 0x00, 474);
	mbr.BPB_OTHERS[0] = bpb.BS_DrvNum  ;			//36
	mbr.BPB_OTHERS[1] = bpb.BS_Reserved1  ;		//37
	mbr.BPB_OTHERS[2] = bpb.BS_BootSig ;		//38 
    memcpy(&mbr.BPB_OTHERS[3], &bpb.BS_VolID, 4);			//39  
	memcpy(&mbr.BPB_OTHERS[7], bpb.BS_VolLab, 11);	//43 
	memcpy(&mbr.BPB_OTHERS[18], bpb.BS_FilSysType, 8);	//54 
	mbr.BPB_EOC[0] = 0x55;
	mbr.BPB_EOC[1] = 0xAA;
}

void Bpb32CobyFromRwInfo(OUT BootSector_BPB32& bpb, IN const BootSector_BPB_RW& mbr)
{
	memcpy(bpb.BS_jmpBoot, mbr.BS_jmpBoot, 3);		//0

	memcpy(bpb.BS_OEMName, mbr.BS_OEMName,8);		//3

	memcpy(&bpb.BPB_BytsPerSec,mbr.BPB_BytsPerSec, 2) ;	//11

	bpb.BPB_SecPerClus = mbr.BPB_SecPerClus;	//13

	memcpy(&bpb.BPB_RsvdSecCnt, mbr.BPB_RsvdSecCnt, 2);	//14

	bpb.BPB_NumFATs = mbr.BPB_NumFATs;		//16

	memcpy(&bpb.BPB_RootEntCnt, mbr.BPB_RootEntCnt, 2);	//17

	memcpy(&bpb.BPB_TotSec16, mbr.BPB_TotSec16, 2);		//19

	bpb.BPB_Media = mbr.BPB_Media;			//21

	memcpy(&bpb.BPB_FATSz16 , mbr.BPB_FATSz16, 2);		//22
	memcpy(&bpb.BPB_SecPerTrk, mbr.BPB_SecPerTrk, 2) ;		//24
	memcpy(&bpb.BPB_NumHeads, mbr.BPB_NumHeads, 2) ;		//26
	memcpy(&bpb.BPB_HiddSec, mbr.BPB_HiddSec, 4);		//28
	memcpy(&bpb.BPB_TotSec32, mbr.BPB_TotSec32, 4);		//32


	memcpy(&bpb.BPB_FATSz32, &mbr.BPB_OTHERS[0], 4);		//36 4 
	memcpy(&bpb.PB_ExtFlags, &mbr.BPB_OTHERS[4], 2);		//40 2 
	memcpy(&bpb.BPB_FSVer, &mbr.BPB_OTHERS[6], 2);		//42 2 
	memcpy(&bpb.BPB_RootClus, &mbr.BPB_OTHERS[8], 4);		//44 4 
	memcpy(&bpb.BPB_FSInfo, &mbr.BPB_OTHERS[12], 2);		//48 2 
	memcpy(&bpb.BPB_BkBootSec, &mbr.BPB_OTHERS[14], 2);	//50 2 
	memcpy(bpb.BPB_Reserved, &mbr.BPB_OTHERS[16], 12); //52 12 
	
	bpb.BS_DrvNum = mbr.BPB_OTHERS[28];			//64 1 
	bpb.BS_Reserved1 = mbr.BPB_OTHERS[29];		//65 1 
	bpb.BS_BootSig = mbr.BPB_OTHERS[30];		//66 1  
	memcpy(&bpb.BS_VolID, &mbr.BPB_OTHERS[31], 4);			//67 4   
	memcpy(bpb.BS_VolLab, &mbr.BPB_OTHERS[35], 11);	//71 11  
	memcpy(bpb.BS_FilSysType, &mbr.BPB_OTHERS[46], 8);	//82 8  
	
}

void RwInfoCopyFormBpb32(OUT BootSector_BPB_RW& mbr, IN const BootSector_BPB32& bpb)
{
	memcpy(mbr.BS_jmpBoot, bpb.BS_jmpBoot, 3);		//0

	memcpy(mbr.BS_OEMName, bpb.BS_OEMName, 8);		//3

	memcpy(mbr.BPB_BytsPerSec, &bpb.BPB_BytsPerSec,2) ;	//11

	mbr.BPB_SecPerClus = bpb.BPB_SecPerClus  ;	//13

	memcpy(mbr.BPB_RsvdSecCnt, &bpb.BPB_RsvdSecCnt, 2);	//14

	mbr.BPB_NumFATs = bpb.BPB_NumFATs ;		//16

	memcpy(mbr.BPB_RootEntCnt, &bpb.BPB_RootEntCnt, 2);	//17

	memcpy(mbr.BPB_TotSec16, &bpb.BPB_TotSec16, 2);		//19

	mbr.BPB_Media = bpb.BPB_Media;			//21

	memcpy(mbr.BPB_FATSz16, &bpb.BPB_FATSz16 , 2);		//22
	memcpy(mbr.BPB_SecPerTrk, &bpb.BPB_SecPerTrk, 2) ;		//24
	memcpy(mbr.BPB_NumHeads, &bpb.BPB_NumHeads, 2) ;		//26
	memcpy(mbr.BPB_HiddSec, &bpb.BPB_HiddSec, 4);		//28
	memcpy(mbr.BPB_TotSec32, &bpb.BPB_TotSec32, 4);		//32

	memset(mbr.BPB_OTHERS, 0x00, 474);

	memcpy(&mbr.BPB_OTHERS[0], &bpb.BPB_FATSz32, 4);		//36 4 
	memcpy(&mbr.BPB_OTHERS[4], &bpb.PB_ExtFlags, 2);		//40 2 
	memcpy(&mbr.BPB_OTHERS[6], &bpb.BPB_FSVer, 2);		//42 2 
	memcpy(&mbr.BPB_OTHERS[8], &bpb.BPB_RootClus, 4);		//44 4 
	memcpy(&mbr.BPB_OTHERS[12], &bpb.BPB_FSInfo, 2);		//48 2 
	memcpy(&mbr.BPB_OTHERS[14], &bpb.BPB_BkBootSec, 2);	//50 2 
	memcpy(&mbr.BPB_OTHERS[16], bpb.BPB_Reserved, 12); //52 12 
	
	mbr.BPB_OTHERS[28] = bpb.BS_DrvNum ;			//64 1 
	mbr.BPB_OTHERS[29] = bpb.BS_Reserved1;		//65 1 
	mbr.BPB_OTHERS[30] = bpb.BS_BootSig;		//66 1  
	memcpy(&mbr.BPB_OTHERS[31], &bpb.BS_VolID, 4);			//67 4   
	memcpy(&mbr.BPB_OTHERS[35], bpb.BS_VolLab, 11);	//71 11  
	memcpy(&mbr.BPB_OTHERS[46], bpb.BS_FilSysType, 8);	//82 8  

	mbr.BPB_EOC[0] = 0x55;
	mbr.BPB_EOC[1] = 0xAA;
}



void DirInfoFromRwInfo(Fat_Directory& dir, const Fat_DirectoryRW& rw) 
{
	memcpy(dir.DIR_Name, rw.DIR_Name, 11);
//	memcpy(dir.DIR_Ext, rw.DIR_Ext, 3);
	dir.DIR_Attr = rw.DIR_Attr;
	dir.DIR_Resrv = rw.DIR_Resrv;
	dir.DIR_CrtTimeTenth = rw.DIR_CrtTimeTenth;
	
	int y,m,d,h, minute, sec;
	WORD date;
	WORD dtm;
	
	memcpy(&date,rw.DIR_CrtDate, 2);
	memcpy(&dtm, rw.DIR_CrtTime, 2);
	y = (date&0xFE00)>>9;
	y = y + 1980;
	m = (date&0x01E0)>>5;
	d = date&0x001f;
	h = (dtm&0xf800)>>11;
	minute = (dtm&0x07e0)>>5;
	sec = dtm&0x001f;

	if(y < 1900)
		y = 1971;
	if(m <= 0)
		m = 1;
	if(d <= 0)
		d = 1;
	if(h < 0)
		h = 1;	
	dir.DIR_CrtDateTime.wYear = y;
	dir.DIR_CrtDateTime.wMonth  = m;
	dir.DIR_CrtDateTime.wDay  = d;
	dir.DIR_CrtDateTime.wHour = h;
	dir.DIR_CrtDateTime.wMinute = minute;
	dir.DIR_CrtDateTime.wSecond = sec;
	dir.DIR_CrtDateTime.wMilliseconds = 0;
	//= CTime(y, m, d, h, minute, sec);

	memcpy(&date,rw.DIR_LstAccDate, 2);
	y = (date&0xFE00)>>9;
	y = y + 1980;
	m = (date&0x01E0)>>5;
	d = date&0x001f;
	
	if(y < 1900)
		y = 1971;
	if(m <= 0)
		m = 1;
	if(d <= 0)
		d = 1;
	
	dir.DIR_LstAcceDate.wYear = y;
	dir.DIR_LstAcceDate.wMonth  = m;
	dir.DIR_LstAcceDate.wDay  = d;
	dir.DIR_LstAcceDate.wHour = h;
	dir.DIR_LstAcceDate.wMinute = minute;
	dir.DIR_LstAcceDate.wSecond = sec;
	dir.DIR_LstAcceDate.wMilliseconds = 0;
//	dir.DIR_LstAcceDate = CTime(y, m, d, h, minute, sec);
	
	
	memcpy(&date,rw.DIR_WrtDate, 2);
	memcpy(&dtm, rw.DIR_WrtTime, 2);
		y = (date&0xFE00)>>9;
	y = y + 1980;
	m = (date&0x01E0)>>5;
	d = date&0x001f;
	h = (dtm&0xf800)>>11;
	minute = (dtm&0x07e0)>>5;
	sec = dtm&0x001f;

	if(y < 1900)
		y = 1971;
	if(m <= 0)
		m = 1;
	if(d <= 0)
		d = 1;
	if(h < 0)
		h = 1;	
	
	dir.DIR_WrtDateTime.wYear = y;
	dir.DIR_WrtDateTime.wMonth  = m;
	dir.DIR_WrtDateTime.wDay  = d;
	dir.DIR_WrtDateTime.wHour = h;
	dir.DIR_WrtDateTime.wMinute = minute;
	dir.DIR_WrtDateTime.wSecond = sec;
	dir.DIR_WrtDateTime.wMilliseconds = 0;
//	dir.DIR_WrtDateTime = CTime(y, m, d, h, minute, sec);

	memcpy(&dir.DIR_FstClusHI, rw.DIR_FstClusHI, 2);
	memcpy(&dir.DIR_FstClusLO, rw.DIR_FstClusLO, 2);
	memcpy(&dir.DIR_FileSize, rw.DIR_FileSize, 4);
}

void RwInfoFromDirInfo(Fat_DirectoryRW& rw, const Fat_Directory& dir) 
{
	memcpy(rw.DIR_Name, dir.DIR_Name, 11);
	rw.DIR_Attr = dir.DIR_Attr ;

	rw.DIR_Resrv = dir.DIR_Resrv;
	rw.DIR_CrtTimeTenth = dir.DIR_CrtTimeTenth;

	int y,m,d,h, minute, sec;
	
	WORD tem;
	
	WORD date;
	WORD dtm;

	
	y = dir.DIR_CrtDateTime.wYear - 1980;// .GetYear() - 1980;
	
	m = dir.DIR_CrtDateTime.wMonth;// .GetMonth();
	//rw.DIR_WrtDate&0x00f0)>>4;
	d = dir.DIR_CrtDateTime.wDay;// .GetDay();
	//rw.DIR_WrtDate&0x000f;
	h = dir.DIR_CrtDateTime.wHour;// .GetHour();
	//rw.DIR_WrtTime&0xf800)>>8;
	minute = dir.DIR_CrtDateTime.wMinute;// .GetMinute();
	//rw.DIR_WrtTime&0x07e0)>>5;
	sec = dir.DIR_CrtDateTime.wSecond;// .GetSecond();
	//rw.DIR_WrtTime&0x001f;

	date = d;
	tem = m;
	tem = tem<<5;
	date = date | tem;
	
	tem = y;
	tem = tem<<9;
	date =date | tem;

	dtm = sec;
	tem = minute;
	tem = tem<<5;
	dtm = dtm | tem;

	tem = h;
	tem  = tem<<11;
	dtm = dtm | tem;

	memcpy(rw.DIR_CrtDate, &date, 2);
	memcpy(rw.DIR_CrtTime, &dtm, 2);


	y = dir.DIR_LstAcceDate.wYear - 1980;//;  .GetYear() - 1980;
	//rw.DIR_WrtDate&0xff00)>>8 + 1980;
	m = dir.DIR_LstAcceDate.wMonth;// .GetMonth();
	//rw.DIR_WrtDate&0x00f0)>>4;
	d = dir.DIR_LstAcceDate.wDay;// .GetDay();
	//rw.DIR_WrtDate&0x000f;
	date = d;
	tem = m;
	tem = tem<<5;
	date = date | tem;
	
	tem = y;
	tem = tem<<9;
	date =date | tem;
	memcpy(rw.DIR_LstAccDate, &date, 2);

	
	y = dir.DIR_WrtDateTime.wYear - 1980;// .GetYear() - 1980;
	//rw.DIR_WrtDate&0xff00)>>8 + 1980;
	m = dir.DIR_WrtDateTime.wMonth;// .GetMonth();
	//rw.DIR_WrtDate&0x00f0)>>4;
	d = dir.DIR_WrtDateTime.wDay;// .GetDay();
	//rw.DIR_WrtDate&0x000f;
	h = dir.DIR_WrtDateTime.wHour;// .GetHour();
	//rw.DIR_WrtTime&0xf800)>>8;
	minute = dir.DIR_WrtDateTime.wMinute;// .GetMinute();
	//rw.DIR_WrtTime&0x07e0)>>5;
	sec = dir.DIR_WrtDateTime.wSecond;// .GetSecond();
	//rw.DIR_WrtTime&0x001f;

	
	date = d;
	tem = m;
	tem = tem<<5;
	date = date | tem;
	
	tem = y;
	tem = tem<<9;
	date =date | tem;

	dtm = sec;
	tem = minute;
	tem = tem<<5;
	dtm = dtm | tem;

	tem = h;
	tem  = tem<<11;
	dtm = dtm | tem;

	memcpy(rw.DIR_WrtDate, &date, 2);
	memcpy(rw.DIR_WrtTime, &dtm, 2);

	memcpy(rw.DIR_FstClusHI, &dir.DIR_FstClusHI, 2);
	memcpy(rw.DIR_FstClusLO, &dir.DIR_FstClusLO, 2);
	memcpy(rw.DIR_FileSize, &dir.DIR_FileSize, 4);
}

void LongDirInfoFromRwInfo(Fat_LongDirectory& dir, const Fat_DirectoryRW& rw) 
{
	Fat_LongDirectory_RW longRw;
	memcpy(&longRw, &rw, sizeof(Fat_LongDirectory_RW));

	dir.LDIR_Ord = longRw.LDIR_Ord;

	memcpy(dir.LDIR_Name1, longRw.LDIR_Name1, 10);
	dir.LDIR_Attr = longRw.LDIR_Attr;
	dir.LDIR_Type = longRw.LDIR_Type;
	dir.LDIR_Chksum = longRw.LDIR_Chksum;
	memcpy(dir.LDIR_Name2, longRw.LDIR_Name2, 12);

	memcpy(&dir.LDIR_FstClusLO, longRw.LDIR_FstClusLO, 2);

	memcpy(dir.LDIR_Name3, longRw.LDIR_Name3, 4);
}

void RwInfoFromLongDirInfo(Fat_DirectoryRW& rw, const Fat_LongDirectory& dir) 
{
	Fat_LongDirectory_RW longRw;

	longRw.LDIR_Ord = dir.LDIR_Ord;

	memcpy(longRw.LDIR_Name1, dir.LDIR_Name1, 10);
	longRw.LDIR_Attr = dir.LDIR_Attr;
	longRw.LDIR_Type = dir.LDIR_Type;
	longRw.LDIR_Chksum= dir.LDIR_Chksum ;
	memcpy(longRw.LDIR_Name2, dir.LDIR_Name2, 12);

	memcpy(longRw.LDIR_FstClusLO, &dir.LDIR_FstClusLO, 2);

	memcpy(longRw.LDIR_Name3, dir.LDIR_Name3, 4);

	memcpy(&rw, &longRw, sizeof(Fat_DirectoryRW));
}

void RemoveCharA(LPSTR lpSrc, CHAR ch)   // 清除字符串中的ch 字符
{
	int nLen = strlen(lpSrc);
	LPSTR pch =  lpSrc;//new char[nLen + 1];

	LPSTR pstrSource = pch;
	LPSTR pstrDest = pch;	
	LPSTR pstrEnd = pch + nLen;

	while (pstrSource < pstrEnd)
	{
		if (*pstrSource != ch)
		{
			*pstrDest = *pstrSource;						
			pstrDest = (char*)_mbsinc((unsigned char*)pstrDest);
		}
		pstrSource = (char*)_mbsinc((unsigned char*)pstrSource);
	}
	*pstrDest = '\0';
}

void RemoveTChar(LPTSTR lptSrc, char ch)   // 清除字符串中的ch 字符
{
	int nLen = strlen(lptSrc);
	LPTSTR pch =  lptSrc;//new char[nLen + 1];

	LPTSTR pstrSource = pch;
	LPTSTR pstrDest = pch;	
	LPTSTR pstrEnd = pch + nLen;

	while (pstrSource < pstrEnd)
	{
		if (*pstrSource != ch)
		{
			*pstrDest = *pstrSource;			
			pstrDest = (char*)_tcsinc(pstrDest);
		}
		pstrSource = (char*)_tcsinc(pstrSource);
	}
	*pstrDest = '\0';
	
}


LPSTR ReplaceChar(LPCSTR lpSrc, char srcCh, char desCh)   // 清除字符串中的srcch 字符 为desCh
{
	int nLen = strlen(lpSrc);
	LPSTR pch =  new char[nLen + 1];
	memset(pch, 0x00, nLen + 1);
	memcpy(pch, lpSrc, nLen);
	pch[nLen] = '\0';

	LPSTR pstrSource = pch;
	LPSTR pstrDest = pch;	
	LPSTR pstrEnd = pch + nLen;

	while (pstrSource < pstrEnd)
	{
		if (*pstrSource != srcCh)
		{
			*pstrDest = *pstrSource;
			pstrDest = (char*)_mbsinc((const unsigned char*)pstrDest);
		}
		else
		{
			*pstrDest = desCh;
			pstrDest = (char*)_mbsinc((const unsigned char*)pstrDest);
		}
		pstrSource = (char*)_mbsinc((const unsigned char*)pstrSource);
	}
	*pstrDest = '\0';
	return pch;

}

BOOL GetRighStrByFindA(LPSTR lpReturn, LPCSTR lpSrc, CHAR ch, int len, LPCSTR defLps)
{
		int nLen = strlen(lpSrc);
	LPSTR pch =  new char[MAX_PATH];
	memset(pch, 0x00, MAX_PATH );
	memcpy(pch, lpSrc, nLen);
	pch[nLen] = '\0';

	LPSTR pstrSource = pch;
	//LPTSTR pstrEnd = pch + nLen;
	char* pstrEnd = pch + nLen;//modify by joelee 2008-03-11

	while (pstrEnd >= pstrSource)
	{
		if (*pstrEnd != ch)
		{
			if(pstrEnd == pstrSource)
				break;
			pstrEnd = pstrEnd -1;				
		}
		else
		{
			//pstrEnd = _tcsinc(pstrEnd);
			pstrEnd = (char*)_mbsinc((unsigned char*)pstrEnd);//modify by joelee 2008-03-11
			break;
		}		
	}

	if(pstrEnd == pstrSource)
	{
		delete pch;
		return FALSE;
	}

//	pstrEnd = _tcsinc(pstrEnd);
	nLen = strlen(pstrEnd);
	if(len == -1)
	{
		memcpy(lpReturn, pstrEnd, strlen(pstrEnd));
		delete pch;
		return TRUE;
	}
	
	if(nLen > len)
		pstrEnd[len] = '\0';
	else
	{
		for(int i = 0; i < len - nLen; i++)
		{
			strcat(pstrEnd, defLps);
		}
	}	
	strcpy(lpReturn, pstrEnd);
	delete pch;
	return TRUE;
}

BOOL GetRighStrByFind(LPTSTR lptReturn, LPCTSTR lptSrc, char ch, int len, LPCTSTR defLps)
{
	int nLen = strlen(lptSrc);
	LPTSTR pch =  new char[MAX_PATH];
	memset(pch, 0x00, MAX_PATH * sizeof(char));
	memcpy(pch, lptSrc, nLen * sizeof(char));
	pch[nLen] = '\0';

	LPTSTR pstrSource = pch;
	LPTSTR pstrEnd = pch + nLen;

	while (pstrEnd >= pstrSource)
	{
		if (*pstrEnd != ch)
		{
			if(pstrEnd == pstrSource)
				break;
			pstrEnd = pstrEnd -1;				
		}
		else
		{
			pstrEnd = (char*)_tcsinc(pstrEnd);
			break;
		}		
	}

	if(pstrEnd == pstrSource)
	{
		delete pch;
		return FALSE;
	}

//	pstrEnd = _mbsinc(pstrEnd);
	nLen = strlen(pstrEnd);
	if(len == -1)
	{
		memcpy(lptReturn, pstrEnd, strlen(pstrEnd) * sizeof(char));
		delete pch;
		return TRUE;
	}
	
	if(nLen > len)
		pstrEnd[len] = '\0';
	else
	{
		for(int i = 0; i < len - nLen; i++)
		{
			strcat(pstrEnd, defLps);
		}
	}	
	strcpy(lptReturn, pstrEnd);
	delete pch;
	return TRUE;
}

LPSTR GetLeftStrA(LPCSTR lpSrc, CHAR ch, BOOL isFromLeft)
{
	int nLen = strlen(lpSrc);
	LPSTR pch =  new CHAR[MAX_PATH];
	memset(pch, 0x00, MAX_PATH * sizeof(CHAR));
	memcpy(pch, lpSrc, nLen * sizeof(CHAR));
	pch[nLen] = '\0';

	LPSTR pstrSource = pch;
	LPSTR pstrEnd = pch + nLen;
	LPSTR pcursor = pstrSource;
	if(isFromLeft)
	{
		while (pcursor < pstrEnd)
		{
			if (*pcursor == ch)
			{
				*pcursor = '\0';
				break;
			}
			pcursor = pcursor + 1;		
		}
	}
	else
	{
		pcursor = pstrEnd;
		while (pcursor > pstrSource)
		{
			if (*pcursor == ch)
			{
				*pcursor = '\0';
				break;
			}
			pcursor = pcursor  -1 ;		
		}
	}


	if(pcursor == pstrSource)
	{
	
		if(*pcursor == ch)
		{
			if(strlen(pstrSource) > 1)
			{
				pcursor++;
				*pcursor = '\0';
				return pstrSource;
			}
		}
		delete pch;
		return NULL;
	}
	
	return pstrSource;
}

LPTSTR GetLeftStr(LPCTSTR lptSrc, char ch, BOOL isFromLeft)
{
	int nLen = strlen(lptSrc);
	LPTSTR pch =  new char[MAX_PATH];
	memset(pch, 0x00, MAX_PATH * sizeof(char));
	memcpy(pch, lptSrc, nLen * sizeof(char));
	pch[nLen] = '\0';

	LPTSTR pstrSource = pch;
	LPTSTR pstrEnd = pch + nLen;
	LPTSTR pcursor = pstrSource;
	if(isFromLeft)
	{
		while (pcursor < pstrEnd)
		{
			if (*pcursor == ch)
			{
				*pcursor = '\0';
				break;
			}
			pcursor = pcursor + 1;		
		}
	}
	else
	{
		pcursor = pstrEnd;
		while (pcursor > pstrSource)
		{
			if (*pcursor == ch)
			{
				*pcursor = '\0';
				break;
			}
			pcursor = pcursor  -1 ;		
		}
	}


	if(pcursor == pstrSource)
	{
	
		if(*pcursor == ch)
		{
			if(strlen(pstrSource) > 1)
			{
				pcursor++;
				*pcursor = '\0';
				return pstrSource;
			}
		}
		delete pch;
		return NULL;
	}
	
	return pstrSource;
}

LPTSTR GetStrFromChArry(char* pch, INT len)
{
	LPTSTR lptStr = new char[len + 1];
	memcpy(lptStr, pch, len * sizeof(char));
	lptStr[len] = '\0';
	return lptStr;
}

BOOL  IsNeedLongEntry(LPCSTR lpstr) // 大小写一致，全是字母 如果是文件必须要满足8.3的格式形式
{
/*	int nLen ;
	LPSTR lpName = GetLeftStr(lpstr, '.', TRUE);
	if(lpName)				// 有.的文件名
	{
		int i = 1;
		nLen = strlen(lpName);
		if(nLen > 8)
			return TRUE;
		BOOL isUp = FALSE;
		if(lpName[0] >='a' && lpName[0] <='z')
			isUp =FALSE;
		else if(lpName[0] >='A' && lpName[0] <='Z')
			isUp = TRUE;
		else
			return TRUE;

		while(i < nLen)
		{
			char ch = lpName[i];
			if(ch > 'z' || ch == (char)0x60 || ch < 'A')
				return TRUE;
			if(ch >='a' && ch <='z')
			{
				if(isUp)
					return TRUE;
			}
			else
			{
				if(!isUp)
					return TRUE;
			}			
			i++;
		}
		LPSTR lpExt = new charGetRighStrByFind(lpstr, '.', -1, "");
		if(lpExt)
		{
			nLen = strlen(lpExt);
			if(nLen > 3)
				return TRUE;

		    i = 1;
			isUp = FALSE;
			if(lpExt[0] >='a' && lpExt[0] <='z')
				isUp =FALSE;
			else if(lpExt[0] >='A' && lpExt[0] <='Z')
				isUp = TRUE;
			else
				return TRUE;

			while(i < nLen)
			{
				char ch = lpExt[i];
				if(ch > 'z' || ch == (char)0x60 || ch < 'A')
					return TRUE;
				if(ch >='a' && ch <='z')
				{
					if(isUp)
						return TRUE;
				}
				else
				{
					if(!isUp)
						return TRUE;
				}			
				i++;
			}
		}
	}
	else
	{
		nLen = strlen(lpstr);
		if(nLen > 8)
			return TRUE;

		BOOL isUp = FALSE;
		if(lpstr[0] >='a' && lpstr[0] <='z')
			isUp =FALSE;
		else if(lpstr[0] >='A' && lpstr[0] <='Z')
			isUp = TRUE;
		else
				return TRUE;

		int i = 1;
		while(i < nLen)
		{
			char ch = lpstr[i];
			if(ch >'z' || ch < 'A' || ch == (char)0x60)
			{
				return TRUE;
			}			
			if(ch >='a' && ch <='z')
			{
				if(isUp)
					return TRUE;
			}
			else
			{
				if(!isUp)
					return TRUE;
			}			
			i++;
		}
	}	*/
	return FALSE;	
}

UINT DiskSizeType(LONGLONG disksize)
{
	if(disksize <= 4194304) // 4M
	{
		return DISK_4M;
	}
	else if(disksize <= 16777216)
	{
		return DISK_16M;
	}
	else if(disksize <= 134217728)
	{
		return DISK_128M;
	}
	else if(disksize <= 268435456)
	{
		return DISK_256M;
	}
	else if(disksize <= 536870912)
	{
		return DISK_512M;
	}
	else if(disksize <= 1073741824)
	{
		return DISK_1G;
	}
	else if(disksize <= 2147483648)
	{
		return DISK_2G;
	}
	else
		return DISK_U2G;
}


UINT Disk32SizeType(LONGLONG disksize)
{
	if(disksize <= 34099200) // 4M
	{
		return DISK32_32M;
	}
	else if(disksize <= 272629760)
	{
		return DISK32_260M;
	}
	else if(disksize <= 8589934592)
	{
		return DISK32_8G;
	}
	else if(disksize <= 17179869184)
	{
		return DISK32_16G;
	}
	else if(disksize <= 34359738368)
	{
		return DISK32_32G;
	}
	else
		return DISK32_U32G;
}

void TrimString(LPSTR lpSrc, BOOL isLeft)   // 整理字符串中的空格字符
{
	int nLen = strlen(lpSrc);
	LPSTR pch =  lpSrc;//new char[nLen + 1];
	LPSTR pstrSource = pch;
	LPSTR pstrDest = pch;	
	LPSTR pstrEnd = pch + nLen;
	if(isLeft)
	{
		while (pstrSource < pstrEnd)
		{
			if (*pstrSource != ' ')
			{
				strcpy(pstrDest, pstrSource);
				break;
			}
			pstrSource = (char*)_mbsinc((const unsigned char*)pstrSource);
		}
	}
	else
	{
		pstrEnd--;
		while(pstrEnd > pstrSource)
		{
			if(*pstrEnd != ' ')
			{
				break;
			}
			*pstrEnd = '\0';
			pstrEnd--;
		}
	}

}
UINT GetImgType(LPCSTR lpFileName)
{
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle =  0;
	// map creation flags
	// attempt file creation
	HANDLE hFile = CreateFile(lpFileName,  GENERIC_READ, FILE_SHARE_READ, &sa,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	
	DWORD nRead;
	
// 	ext2_super_block  extSupBlock;
// 	::SetFilePointer(hFile, 1024, NULL, FILE_BEGIN);
// 	::ReadFile(hFile, &extSupBlock, sizeof(ext2_super_block), &nRead, NULL);
// 	
// 	if(nRead == sizeof(ext2_super_block) && extSupBlock.s_magic == EXT_SUPER_MAGIC)
// 	{
// 		if(extSupBlock.s_feature_compat & EXT_FEATURE_COMPAT_HAS_JOURNAL)
// 		{
// 			if (extSupBlock.s_feature_incompat & EXT4_FEATURE_INCOMPAT_USEE_EXTENTS)
// 			{
// 				::CloseHandle(hFile);
// 				return EXT4_TYPE;
// 			} 
// 			else
// 			{
// 				::CloseHandle(hFile);
// 				return EXT3_TYPE;
// 			}
// 			
// 		}
// 		else
// 		{
// 			if (extSupBlock.s_feature_incompat & EXT4_FEATURE_INCOMPAT_USEE_EXTENTS)
// 			{
// 				::CloseHandle(hFile);
// 				return EXT4_TYPE;
// 			} 
// 			else
// 			{
// 				::CloseHandle(hFile);
// 				return EXT2_TYPE;
// 			}
// 			
// 		}
// 	}
	
	BootSector_BPB_RW bpbRw;
	::SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	::ReadFile(hFile, &bpbRw, sizeof(BootSector_BPB_RW), &nRead, NULL);
	::CloseHandle(hFile);
	
	if(nRead != sizeof(BootSector_BPB_RW))
	{		
		return -1;
	}
	
	BootSector_BPB bpb16;	
	BpbCobyFromRwInfo(bpb16, bpbRw);
	
	BootSector_BPB32 bpb32;
	Bpb32CobyFromRwInfo(bpb32, bpbRw);
	
	DWORD FATSz = 0;
	DWORD TotSec = 0;
	if(bpb16.BPB_FATSz16 != 0)
		FATSz = bpb16.BPB_FATSz16;
	else			
		FATSz = bpb32.BPB_FATSz32;
	
	
	if(bpb16.BPB_TotSec16 != 0)
		TotSec = bpb16.BPB_TotSec16;
	else
		TotSec = bpb16.BPB_TotSec32;
	
	UINT DataSec = TotSec - (bpb16.BPB_RsvdSecCnt + (bpb16.BPB_NumFATs * FATSz) + bpb16.BPB_RootEntCnt);	
	UINT CountofClusters = DataSec / bpb16.BPB_SecPerClus;
	
	if(CountofClusters < 4085) {
		/* Volume is FAT12 */
		return FAT12_TYPE;
	} else if(CountofClusters < 65525) {
		/* Volume is FAT16 */
		return FAT16_TYPE;
	} else {
		/* Volume is FAT32 */
		return FAT32_TYPE;
	}	
	
	return -1;

}

ULONGLONG GetPosition(HANDLE hFile)
{
	DWORD dwHighPos = 0;
	DWORD dwLowPos = ::SetFilePointer(hFile, 0, (LONG*) &dwHighPos, FILE_CURRENT);
	if (dwLowPos  == (DWORD)-1)
		//wbt mod
		//CFileException::ThrowOsError((LONG)::GetLastError());
		return false;
	
	return MakeUnsignedInt64(dwHighPos, dwLowPos);
}

ULONGLONG MakeUnsignedInt64( DWORD nHigh, DWORD nLow )
{
	return ((((ULONGLONG) nHigh) << 32) | nLow);
}

void SplitUnsignedInt64( const ULONGLONG& nBigInt, DWORD& nHigh, DWORD& nLow )
{
	nHigh = (DWORD) ((nBigInt & 0xFFFFFFFF00000000) >> 32);
	nLow = (DWORD) (nBigInt & 0x00000000FFFFFFFF);
}

ULONGLONG GetLength(HANDLE hFile)
{
	DWORD dwHighLength;
	DWORD dwLowLength = GetFileSize(hFile, &dwHighLength);
	if (dwLowLength  == (DWORD)-1)
		//wbt mod
		//CFileException::ThrowOsError((LONG)::GetLastError());
		return false;
	
	return MakeUnsignedInt64(dwHighLength, dwLowLength);
}

