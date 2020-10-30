#include <size_t.h>
#include <minwindef.h>
#include <minwinconst.h>
#include <sysfs.h>

typedef int(*PMain)(void*);
typedef CSysfs* (*PCreateDrive)(char* fileName);

int main(int argc, char* argv[]) 
{
	HMODULE libDriveHandle = LoadLibrary("libfat32.dll");
	
	PCreateDrive func = (PCreateDrive)GetProcAddress(libDriveHandle, "CreateDrive");
	CSysfs* pDrive = func("fat32.img");
	if (pDrive)
	{
		pDrive->CreateDirectory("\\juhang3", 0);
		HANDLE fp = pDrive->CreateFile(
            argv[1],
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if (fp == INVALID_HANDLE_VALUE)
        {
            printf("File 1 Open Error (%d) : %s\n", GetLastError(), argv[1]);
            return 1;
        }
	}
    return 0;
}