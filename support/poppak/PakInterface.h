#pragma once

#include <string>
#include <map>
#include <list>

#include <minwindef.h>
#include <minwinconst.h>
#include <winapi.h>

class PakCollection;

class PakRecord
{
public:
	PakCollection*			mCollection;
	std::string				mFileName;
	FILETIME				mFileTime;
	int						mStartPos;
	int						mSize;	
};

typedef std::map<std::string, PakRecord> PakRecordMap;

class PakCollection
{
public:
	HANDLE					mFileHandle;
	HANDLE					mMappingHandle;
	void*					mDataPtr;
};

typedef std::list<PakCollection> PakCollectionList;

struct _PFILE
{
	PakRecord*				mRecord;
	int						mPos;
	FILE*					mFP;
};

struct PFindData
{
	HANDLE					mWHandle;
	std::string				mLastFind;
	std::string				mFindCriteria;
};

class PakInterfaceBase
{
public:
	virtual _PFILE*			FOpen(const char* theFileName, const char* theAccess) = 0;
	virtual _PFILE*			FOpen(const wchar_t* theFileName, const wchar_t* theAccess) { return NULL; }
	virtual int				FClose(_PFILE* theFile) = 0;
	virtual int				FSeek(_PFILE* theFile, long theOffset, int theOrigin) = 0;
	virtual int				FTell(_PFILE* theFile) = 0;
	virtual size_t			FRead(void* thePtr, int theElemSize, int theCount, _PFILE* theFile) = 0;
	virtual int				FGetC(_PFILE* theFile) = 0;
	virtual int				UnGetC(int theChar, _PFILE* theFile) = 0;
	virtual char*			FGetS(char* thePtr, int theSize, _PFILE* theFile) = 0;
	virtual wchar_t*		FGetS(wchar_t* thePtr, int theSize, _PFILE* theFile) { return thePtr; }
	virtual int				FEof(_PFILE* theFile) = 0;

	virtual HANDLE			FindFirstFile(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData) = 0;	
	virtual BOOL			FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData) = 0;
	virtual BOOL			FindClose(HANDLE hFindFile) = 0;
};

class PakInterface : public PakInterfaceBase
{
public:
	PakCollectionList		mPakCollectionList;	
	PakRecordMap			mPakRecordMap;
	std::string				mDecryptPassword;
	std::string				mError;

public:
	bool					PFindNext(PFindData* theFindData, LPWIN32_FIND_DATA lpFindFileData);

public:
	PakInterface();
	~PakInterface();

	bool					AddPakFile(const std::string& theFileName);
	_PFILE*					FOpen(const char* theFileName, const char* theAccess);
	int						FClose(_PFILE* theFile);
	int						FSeek(_PFILE* theFile, long theOffset, int theOrigin);
	int						FTell(_PFILE* theFile);
	size_t					FRead(void* thePtr, int theElemSize, int theCount, _PFILE* theFile);
	int						FGetC(_PFILE* theFile);
	int						UnGetC(int theChar, _PFILE* theFile);
	char*					FGetS(char* thePtr, int theSize, _PFILE* theFile);
	int						FEof(_PFILE* theFile);

	HANDLE					FindFirstFile(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData);
	BOOL					FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData);
	BOOL					FindClose(HANDLE hFindFile);
};

extern PakInterface* gPakInterface;

static HANDLE gPakFileMapping = NULL;
static PakInterfaceBase** gPakInterfaceP = NULL;

static PakInterfaceBase* GetPakPtr()
{
	if (gPakFileMapping == NULL)
	{
		char aName[256];
		sprintf(aName, "gPakInterfaceP_%d", GetCurrentProcessId());
		gPakFileMapping = ::CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(PakInterface*), aName);
		gPakInterfaceP = (PakInterfaceBase**) MapViewOfFile(gPakFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(PakInterface*));		
	}
	return *gPakInterfaceP;
}

static _PFILE* p_fopen(const char* theFileName, const char* theAccess) 
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->FOpen(theFileName, theAccess);	
	FILE* aFP = fopen(theFileName, theAccess);
	if (aFP == NULL)
		return NULL;
	_PFILE* aPFile = new _PFILE();
	aPFile->mRecord = NULL;
	aPFile->mPos = 0;
	aPFile->mFP = aFP;
	return aPFile;
}

/*
static _PFILE* p_fopen(const wchar_t* theFileName, const wchar_t* theAccess) 
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->FOpen(theFileName, theAccess);	
	FILE* aFP = _wfopen(theFileName, theAccess);
	if (aFP == NULL)
		return NULL;
	_PFILE* aPFile = new _PFILE();
	aPFile->mRecord = NULL;
	aPFile->mPos = 0;
	aPFile->mFP = aFP;
	return aPFile;
}*/

static int p_fclose(_PFILE* theFile)
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->FClose(theFile);
	int aResult = fclose(theFile->mFP);
	delete theFile;
	return aResult;
}

static int p_fseek(_PFILE* theFile, long theOffset, int theOrigin)
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->FSeek(theFile, theOffset, theOrigin);
	return fseek(theFile->mFP, theOffset, theOrigin);
}

static int p_ftell(_PFILE* theFile)
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->FTell(theFile);
	return ftell(theFile->mFP);
}

static size_t p_fread(void* thePtr, int theSize, int theCount, _PFILE* theFile)
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->FRead(thePtr, theSize, theCount, theFile);
	return fread(thePtr, theSize, theCount, theFile->mFP);
}

static size_t p_fwrite(const void* thePtr, int theSize, int theCount, _PFILE* theFile)
{	
	if (theFile->mFP == NULL)
		return 0;
	return fwrite(thePtr, theSize, theCount, theFile->mFP);
}

static int p_fgetc(_PFILE* theFile)
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->FGetC(theFile);
	return fgetc(theFile->mFP);
}

static int p_ungetc(int theChar, _PFILE* theFile)
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->UnGetC(theChar, theFile);
	return ungetc(theChar, theFile->mFP);
}

static char* p_fgets(char* thePtr, int theSize, _PFILE* theFile)
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->FGetS(thePtr, theSize, theFile);
	return fgets(thePtr, theSize, theFile->mFP);
}

/*static wchar_t* p_fgets(wchar_t* thePtr, int theSize, _PFILE* theFile)
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->FGetS(thePtr, theSize, theFile);
	return fgetws(thePtr, theSize, theFile->mFP);
}*/

static int p_feof(_PFILE* theFile)
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->FEof(theFile);
	return feof(theFile->mFP);
}

static HANDLE p_FindFirstFile(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData)
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->FindFirstFile(lpFileName, lpFindFileData);
	return FindFirstFile(lpFileName, lpFindFileData);
}

static BOOL p_FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData)
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->FindNextFile(hFindFile, lpFindFileData);
	return FindNextFile(hFindFile, lpFindFileData);
}

static BOOL p_FindClose(HANDLE hFindFile)
{
	if (GetPakPtr() != NULL)
		return (*gPakInterfaceP)->FindClose(hFindFile);
	return FindClose(hFindFile);
}
