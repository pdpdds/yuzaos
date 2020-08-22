#pragma once
#include "I_SkyInput.h"

typedef struct tag_WIN32_STUB
{
	void* _allocInterface;
	void* _printInterface;
	void* _processInterface;
	unsigned int _virtualAddress;
	unsigned int _virtualAddressSize;
	unsigned int _kernelSize;
}WIN32_STUB;

typedef struct tag_WIN32_VIDEO
{
	unsigned int _frameBuffer;
	unsigned int _width;
	unsigned int _height;
	unsigned int _bpp;
}WIN32_VIDEO;

typedef struct tag_SKYOS_MODULE
{
	char _name[256];
	unsigned int _startAddress;
	unsigned int _endAddress;	
}SKYOS_MODULE;

typedef struct tag_SKYOS_MODULE_LIST
{
	unsigned int _moduleCount;
	SKYOS_MODULE* _module;
}SKYOS_MODULE_LIST;

#ifdef WIN32STUB
#define ORANGEOS_WIN32_DLL __declspec(dllexport)
#else
#define ORANGEOS_WIN32_DLL __declspec(dllimport)
#endif

extern "C"
{
	ORANGEOS_WIN32_DLL WIN32_STUB* GetWin32Stub();
	ORANGEOS_WIN32_DLL tag_WIN32_VIDEO* InitWin32System(int width, int height, int bpp);
	ORANGEOS_WIN32_DLL void LoopWin32(I_SkyInput* pVirtualIO, unsigned int& tickCount);
	ORANGEOS_WIN32_DLL SKYOS_MODULE_LIST* InitSkyOSModule();
	
	ORANGEOS_WIN32_DLL bool SKY_VirtualProtect(void* address, int size, int attribute, unsigned int* dwOld);
	ORANGEOS_WIN32_DLL bool SKY_VirtualProtect(void* address, int size, int attribute, unsigned int* dwOld);
	ORANGEOS_WIN32_DLL bool SKY_VirtualFree(void* lpAddress, unsigned int dwSize, unsigned int  dwFreeType);
	ORANGEOS_WIN32_DLL void* SKY_VirtualAlloc(void* lpAddress, unsigned int dwSize, unsigned int  flAllocationType, unsigned int  flProtect);

	ORANGEOS_WIN32_DLL size_t sky_fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
	ORANGEOS_WIN32_DLL FILE * sky_fopen(const char* filename, const char* mode);
	ORANGEOS_WIN32_DLL size_t sky_fwrite(const void* ptr, size_t size, size_t nmemb, FILE * stream);
	ORANGEOS_WIN32_DLL int sky_fseek(FILE * stream, long int offset, int whence);
	ORANGEOS_WIN32_DLL int sky_fclose(FILE * stream);
}