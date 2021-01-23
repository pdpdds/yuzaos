#pragma once
#include <minwindef.h>
#include "LiteZip.h"
#include "LiteUnzip.h"

class File;
class Unzip;


class CFilePack
{
public:
	CFilePack(void);
	virtual ~CFilePack(void);


	bool OpenFile(const char *fname);
	File* LoadFileMem(File *fp);
	bool  Store(const char *fname);
	bool OpenFileFromZip(const char *fname);

protected:

private:
	Unzip* m_Unzip;
	File* m_pFile;
};

namespace Orange
{

	class CFilePackSys
	{
		
	public:
		CFilePackSys(void);
		virtual ~CFilePackSys(void);

		BOOL Initialize();
		BOOL Finally();

		void show_errmsg(void);
		
		bool PackMemory(TCHAR* pZipFileName, TCHAR* pFileName, unsigned char* pBuffer, int BufferSize);
		bool UnPackMemory( TCHAR* pZipFileName, TCHAR* pFileName );
		
		bool PackMemoryRaw(TCHAR* pZipFileName, unsigned char* pBuffer, int UnCompressedSize, int& CompressedSize);
		unsigned char* UnPackMemoryRaw( TCHAR* pZipFileName, unsigned char* pBuffer, int BufferSize );


////////////////////////////////////////////////////////////////////////////////////////////
//PackDir : 선택한 디렉토리 하위 구조까지 압축한다. 유니코드 함수는 제대로 동작하지 않는 듯 함
////////////////////////////////////////////////////////////////////////////////////////////
		bool PackFile( TCHAR* pZipFileName, TCHAR* pTargetFileName, bool bNewZipFile = true);

////////////////////////////////////////////////////////////////////////////////////////////
//PackDir : 선택한 디렉토리 하위 구조까지 압축한다. 유니코드 함수는 제대로 동작하지 않는 듯 함
////////////////////////////////////////////////////////////////////////////////////////////
		bool PackDir(TCHAR* pFileName, TCHAR* pDirFullPath, int DirFullPathLen );

////////////////////////////////////////////////////////////////////////////////////////////
//UnPackRes : 실행파일의 리소스에 있는 zip파일에서 데이터를 얻어 온다. 이름을 지정하면 하나의 파일만 끄집어낼 수 있다.
////////////////////////////////////////////////////////////////////////////////////////////
		bool UnPackRes(TCHAR* pExtractedFileName = NULL);
		unsigned char* UnPackResMem( TCHAR* pExtractedFileName, int& BufferSize);
		bool UnPackResMemAll( TCHAR* pResourceName);

////////////////////////////////////////////////////////////////////////////////////////////
//UnPackFile : Zip 파일의 압축을 풀어서 파일로 저장한다.
//pExtractedFileName 이름을 지정하면 압축파일에서 하나의 파일만 끄집어 내어 저장할 수 있다.
////////////////////////////////////////////////////////////////////////////////////////////
		bool UnPackFile(TCHAR* pZipFileName, TCHAR* pExtractedFileName = NULL);

	protected:

	private:
		// Where we store the pointers to LiteZip.dll's functions that we call
		ZipCreateFilePtr		*lpZipCreateFile;
		ZipAddDirPtr			*lpZipAddDir;
		ZipClosePtr				*lpZipClose;
		ZipFormatMessagePtr		*lpZipFormatMessage;
		ZipAddFilePtr			*lpZipAddFile;
		ZipAddBufferPtr			*lpZipAddBuffer;
		ZipCreateBufferPtr		*lpZipCreateBuffer;
		ZipAddBufferRawPtr		*lpZipAddBufferRaw;
		ZipGetMemoryPtr			*lpZipGetMemory;

		UnzipOpenFilePtr		*lpUnzipOpenFile;
		UnzipGetItemPtr			*lpUnzipGetItem;
		UnzipItemToBufferPtr	*lpUnzipItemToBuffer;
		UnzipClosePtr			*lpUnzipClose;
		UnzipFormatMessagePtr	*lpUnzipFormatMessage;		
		UnzipOpenBufferPtr		*lpUnzipOpenBuffer;				
		UnzipItemToFilePtr		*lpUnzipItemToFile;		
		UnzipFindItemPtr		*lpUnzipFindItem;		
		UnzipOpenFileRawPtr		*lpUnzipOpenFileRaw;
		

		HMODULE		m_zipDll;
		HMODULE		m_unzipDll;
	};
}