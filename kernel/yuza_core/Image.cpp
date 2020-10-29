#include "Image.h"
#include "ktypes.h"
#include <stringdef.h>
#include "SystemAPI.h"
#include "memory.h"
#include "SystemCall.h"
#include <AddressSpace.h>
#include <Team.h>
#include <Page.h>
#include <physicalmap.h>
#include "PageCache.h"
#include <FileBackingStore.h>

Image::Image()
	: fFileHandle(0),
	fBaseAddress(0),
	fPath(0),
	fDosHeader(0),
	fNTHeader(0)	
{
}

Image::~Image()
{
	if (fFileHandle > 0)
	{
		kCloseHandle(fFileHandle);
		fclose((FILE*)fFileHandle);
	}
	
	delete[] fPath;
	delete fDosHeader;
	delete fNTHeader;	
}

int Image::Open(const char path[])
{
	int error = E_NO_ERROR;
#ifdef PAGE_SYSTEM
	fFileHandle = open(path, 0);
#else
	fFileHandle = (HANDLE)fopen(path, "rb");
#endif

	if (fFileHandle == 0)
	{
		kprintf("exe open fail %s\n", path);
		//assert(0);
		return -1;
	}

	fPath = new char[strlen(path) + 1];
	strcpy(fPath, path);


	error = ReadHeader();
	if (error < 0)
		return error;

	return 0;
}

#include <Area.h>

extern uint32_t alignUp(uint32_t val, uint32_t alignment);
int Image::Load(Team &team)
{
	const char *filename = fPath;
	for (const char *c = fPath; *c; c++)
		if (*c == '/')
			filename = c + 1;

	char areaName[OS_NAME_LENGTH];
	snprintf(areaName, OS_NAME_LENGTH, "%s", filename);

	int size = ((fNTHeader->OptionalHeader.SizeOfImage + PAGE_SIZE - 1) / PAGE_SIZE)* PAGE_SIZE;
	fBaseAddress = reinterpret_cast<void*>(fNTHeader->OptionalHeader.ImageBase & ~(PAGE_SIZE - 1));
	//int size = alignUp(fNTHeader->OptionalHeader.SizeOfImage, PAGE_SIZE);
	//fBaseAddress = (void*)fNTHeader->OptionalHeader.ImageBase;

#if 1
	fseek((FILE*)fFileHandle, SEEK_SET, 0);
	BackingStore* pObject = new FileBackingStore(fFileHandle);

	Area *area = team.GetAddressSpace()->CreateArea(filename, size, AREA_NOT_WIRED,
		USER_READ | USER_WRITE | SYSTEM_READ | SYSTEM_WRITE, new PageCache(pObject), 0, (unsigned int)fBaseAddress, 0);

	/*area->GetProtection();
	kprintf("protection %x %x %d\n", area, fBaseAddress, area->GetProtection());

	Area *area2 = static_cast<Area*>(team.GetAddressSpace()->fAreas.Find((unsigned int)fBaseAddress));
	kprintf("protection %x %x %d\n", area2, fBaseAddress, area->GetProtection());*/
#else
	Area *area = team.GetAddressSpace()->CreateArea(filename, size, AREA_WIRED,
		USER_READ | USER_WRITE | SYSTEM_READ | SYSTEM_WRITE, new PageCache, 0, (unsigned int)fBaseAddress, 0);

	fseek((FILE*)fFileHandle, 0, SEEK_SET);
	//파일을 메모리로 로드한다.
	int fileRest = 0;
	if ((size % 512) != 0)
		fileRest = 1;

	char szBuffer[512];

	int readCount = (size / 512) + fileRest;

	
	FILE* file = reinterpret_cast<FILE*>(fFileHandle);
	
	for (int i = 0; i < readCount; i++)
	{
		if (feof(file) != 0)
			break;

		int readCnt = fread(szBuffer, 1, 512, file);
		
		memcpy((void*)((char*)fBaseAddress + 512 * i), szBuffer, readCnt);
	}

#endif 
	
	return 0;
}

unsigned int Image::GetEntryAddress() const
{
	return (DWORD)fBaseAddress + fNTHeader->OptionalHeader.AddressOfEntryPoint;
}

const char* Image::GetPath() const
{
	return fPath;
}

int Image::ReadHeader()
{
	unsigned char buf[512];

	fDosHeader = new IMAGE_DOS_HEADER;

	if (fDosHeader == 0)
		return E_NO_MEMORY;

#ifdef PAGE_SYSTEM
	if (read_pos(fFileHandle, 0, buf, 512) != 512) {
		kprintf("incomplete header\n");
		return E_NOT_IMAGE;
	}
#else
	if (fread(buf, 1, 512, (FILE*)fFileHandle) != 512) {
		kprintf("incomplete header\n");
		return E_NOT_IMAGE;
	}
#endif
	
	//유효하지 않은 PE파일이면 파일 핸들을 닫고 종료한다.
	if (!ValidatePEImage(buf))
	{
		kprintf("Invalid PE Format!! %s\n", fPath);
		return -1;
	}
	
	memcpy(fDosHeader, buf, sizeof(IMAGE_DOS_HEADER));
	fNTHeader = new IMAGE_NT_HEADERS;
	memcpy(fNTHeader, (void*)(fDosHeader->e_lfanew + (uint32_t)buf), sizeof(IMAGE_NT_HEADERS));	

	return 0;
}

void Image::PrintSections() const
{
	
}

void Image::PrintSymbols() const
{
	
}
