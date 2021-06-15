#pragma once
#include "I_SkyInput.h"

typedef struct tag_WIN32_STUB
{
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

typedef struct tag_YUZAOS_MODULE
{
	char _name[256];
	unsigned int _startAddress;
	unsigned int _endAddress;	
}YUZAOS_MODULE;

typedef struct tag_SKYOS_MODULE_LIST
{
	unsigned int _moduleCount;
	YUZAOS_MODULE* _module;
}YUZAOS_MODULE_LIST;

#ifdef WIN32STUB
#define YUZAOS_DLL __declspec(dllexport)
#else
#define YUZAOS_DLL __declspec(dllimport)
#endif

extern "C"
{
	YUZAOS_DLL WIN32_STUB* GetWin32Stub();
	YUZAOS_DLL WIN32_VIDEO* GetFrameBufferInfo();
	YUZAOS_DLL void LoopWin32(I_SkyInput* pVirtualIO, unsigned int& tickCount);
	YUZAOS_DLL bool StartWin32StubTimer(I_SkyInput* pVirtualIO, unsigned int& tickCount);
	YUZAOS_DLL cdecl YUZAOS_MODULE_LIST* InitYuzaOSModule(char** moduleList, int size);
	
	/*YUZAOS_DLL bool SKY_VirtualProtect(void* address, int size, int attribute, unsigned int* dwOld);
	YUZAOS_DLL bool SKY_VirtualProtect(void* address, int size, int attribute, unsigned int* dwOld);
	YUZAOS_DLL bool SKY_VirtualFree(void* lpAddress, unsigned int dwSize, unsigned int  dwFreeType);
	YUZAOS_DLL void* SKY_VirtualAlloc(void* lpAddress, unsigned int dwSize, unsigned int  flAllocationType, unsigned int  flProtect);
	*/

	YUZAOS_DLL size_t sky_fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
	YUZAOS_DLL FILE * sky_fopen(const char* filename, const char* mode);
	YUZAOS_DLL size_t sky_fwrite(const void* ptr, size_t size, size_t nmemb, FILE * stream);
	YUZAOS_DLL int sky_fseek(FILE * stream, long int offset, int whence);
	YUZAOS_DLL int sky_fclose(FILE * stream);
}