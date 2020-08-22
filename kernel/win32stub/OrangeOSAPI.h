#pragma once

#ifdef WIN32STUB
#define ORANGEOS_WIN32_DLL __declspec(dllexport)
#else
#define ORANGEOS_WIN32_DLL __declspec(dllimport)
#endif

extern "C"
{
	ORANGEOS_WIN32_DLL void* GetOrangeOSAPI(char * strAPIName);
	ORANGEOS_WIN32_DLL void* GetOrangeOSAPIByIndex(int index);
	ORANGEOS_WIN32_DLL void RegisterOrangeOSAPI(int APIIndex, char* strAPIName, void * ptrAPIFunction);
}