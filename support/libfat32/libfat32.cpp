#include "windef.h"
#include "PlatformAPI.h"
#include "Sysfs32.h"
#include <map>
#include "AnsiMemFile.h"
#include <string>

std::map<std::string, CAnsiMemFile*>* g_MapName2File;//map filename to file object
std::map<HANDLE, std::string>*  g_MapHandle2Name;   // map file handle to filename

extern "C" __declspec(dllexport) CSysfs * CreateDrive(char* fileName)
{
	g_MapName2File = new std::map<std::string, CAnsiMemFile*>();
	g_MapHandle2Name = new std::map<HANDLE, std::string>();
	CSysfs32* pfat32 = new CSysfs32();
	FILE* fp = fopen(fileName, "rb");
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fclose(fp);
	pfat32->Mount(fileName, "C", size);
	return pfat32;
}

void printmsg(char* msg)
{
	printf(msg);
	printf("\n");
}


