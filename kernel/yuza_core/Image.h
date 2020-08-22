#pragma once
#include "windef.h"
#include "PEImage.h"
#include <stdio.h>

class Team;

class Image {
public:
	Image();
	virtual ~Image();
	int Open(const char path[]);
	int Load(Team&);
	unsigned int GetEntryAddress() const;
	const char* GetPath() const;
	void *fBaseAddress;

private:
	int ReadHeader();
	void PrintSections() const;
	void PrintSymbols() const;

	HANDLE fFileHandle;
	
	char *fPath;
	IMAGE_DOS_HEADER *fDosHeader;
	IMAGE_NT_HEADERS *fNTHeader;		
};