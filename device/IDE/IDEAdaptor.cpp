#include "IDEAdaptor.h"
#include <stringdef.h>
#include "harddisk.h"
#include "platformapi.h"
#include <assert.h>
#include <systemcall_impl.h>
#include <yuza_file_io.h>

FILE* g_virtualDisk = 0;
CRITICAL_SECTION g_cs;
bool g_emulation = false;
WIN32_FILE_IO_INTERFACE g_win32_io =
{
	0,
	0,
	0,
	0,
	0,
};

extern "C" BYTE IDE_disk_initialize_win32()
{
	char buf[256];
	int count = Syscall_GetEnvironmentVariable("HARDDISK", buf, 256);

	assert(count != 0);

	g_virtualDisk = g_win32_io.sky_fopen(buf, "rb+");

	if (g_virtualDisk)
		return 0;

	return 1;
}

extern "C" BYTE IDE_disk_read_win32(BYTE * buff, DWORD sector, UINT count)
{
	Syscall_EnterCriticalSection(&g_cs);
	g_win32_io.sky_fseek((FILE*)g_virtualDisk, sector * 512, SEEK_SET);
	size_t readCnt = g_win32_io.sky_fread(buff, 1, count * 512, (FILE*)g_virtualDisk);

	if (readCnt != count * 512)
	{
		Syscall_LeaveCriticalSection(&g_cs);
		return 1;
	}

	Syscall_LeaveCriticalSection(&g_cs);

	return 0;
}

extern "C" BYTE IDE_disk_write_win32(const BYTE * buff, DWORD sector, UINT count)
{
	Syscall_EnterCriticalSection(&g_cs);

	g_win32_io.sky_fseek((FILE*)g_virtualDisk, sector * 512, SEEK_SET);
	size_t writeCnt = g_win32_io.sky_fwrite(buff, 1, count * 512, (FILE*)g_virtualDisk);

	if (writeCnt != count * 512)
	{
		Syscall_LeaveCriticalSection(&g_cs);
		return 1;
	}

	Syscall_LeaveCriticalSection(&g_cs);
	return 0;
}

extern "C" BYTE IDE_disk_status_win32()
{
	return 0;
}

HardDiskHandler* g_pHDDHandler = nullptr;

extern "C" BYTE IDE_disk_initialize()
{
	g_pHDDHandler = new HardDiskHandler();
	g_pHDDHandler->Initialize();

	return 0;
}


extern "C" BYTE IDE_disk_read(BYTE * buff, DWORD sector, UINT count)
{
	int error = g_pHDDHandler->ReadSectors((BYTE*)"H0", sector, count, buff);

	if (error != HDD_NO_ERROR)
		return 1;

	return 0;
}

extern "C" BYTE IDE_disk_write(const BYTE * buff, DWORD sector, UINT count)
{
	int error = g_pHDDHandler->WriteSectors((BYTE*)"H0", sector, count, (BYTE*)buff);

	if (error != HDD_NO_ERROR)
	{
		printf("IDE_disk_write Error!! %s %d %d %d\n", buff, sector, count, error);
		return 1;
	}

	return 0;
}

extern "C" BYTE IDE_disk_status()
{
	return 0;
}


IDEAdaptor::IDEAdaptor(char* deviceName)
	: FileSysAdaptor(deviceName)
{
	if (g_emulation)
		Syscall_InitializeCriticalSection(&g_cs);

	//printf("IDEAdaptor Created\n");
}

IDEAdaptor::~IDEAdaptor()
{
	if (g_emulation)
	{
		Syscall_DeleteCriticalSection(&g_cs);
		g_win32_io.sky_fclose(g_virtualDisk);
	}
}

FILE_IO_INTERFACE g_io_interface =
{
	IDE_disk_initialize,
	IDE_disk_read,
	IDE_disk_write,
	IDE_disk_status,
};

FILE_IO_INTERFACE g_io_interface_win32 =
{
	IDE_disk_initialize_win32,
	IDE_disk_read_win32,
	IDE_disk_write_win32,
	IDE_disk_status_win32,
};

extern "C" __declspec(dllexport) FileSysAdaptor * CreateFileAdaptor()
{
	return new IDEAdaptor("IDE");
}

bool IDEAdaptor::Initialize(I_FileSystem* pFileSystem, void* arg)
{
	m_pFileSystem = pFileSystem;

	g_emulation = Syscall_IsEmulationMode();

	bool result = false;
	if (g_emulation)
	{
		result = InitializeWin32();
	}
	else result = m_pFileSystem->Initialize(&g_io_interface);

	printf("IDEAdaptor::Initialize %d %d \n", result, g_emulation);

	if (result == false)
		return false;


	if (!g_emulation)
	{
		printf("HardDisk Count : %d\n", g_pHDDHandler->GetTotalDevices());

		if (g_pHDDHandler->GetTotalDevices() == 0)
		{
			assert(0);
			return false;
		}
	}
	
	PrintHDDInfo();

	return true;
}


typedef size_t(*psky_fread)(void* ptr, size_t size, size_t nmemb, FILE* stream);
typedef FILE* (*psky_fopen)(const char* filename, const char* mode);
typedef size_t(*psky_fwrite)(const void* ptr, size_t size, size_t nmemb, FILE* stream);
typedef int (*psky_fseek)(FILE* stream, long int offset, int whence);
typedef int (*psky_fclose)(FILE* stream);

bool IDEAdaptor::InitializeWin32()
{
	HANDLE handle = (HANDLE)Syscall_LoadLibrary("Win32Stub.dll");
	if (handle == 0)
		return false;

	g_win32_io.sky_fopen = (psky_fopen)Syscall_GetProcAddress(handle, "sky_fopen");
	g_win32_io.sky_fread = (psky_fread)Syscall_GetProcAddress(handle, "sky_fread");
	g_win32_io.sky_fwrite = (psky_fwrite)Syscall_GetProcAddress(handle, "sky_fwrite");
	g_win32_io.sky_fseek = (psky_fseek)Syscall_GetProcAddress(handle, "sky_fseek");
	g_win32_io.sky_fclose = (psky_fclose)Syscall_GetProcAddress(handle, "sky_fclose");

	return m_pFileSystem->Initialize(&g_io_interface_win32);
}

void IDEAdaptor::PrintHDDInfo()
{
	if (!g_emulation)
	{
		int TotHDD = g_pHDDHandler->GetTotalDevices();
		HDDInfo* pHDDInfo;
		BYTE Key[3] = { 'H','0',0
		};

		for (BYTE i = 0; i < TotHDD; i++)
		{
			pHDDInfo = (HDDInfo*)g_pHDDHandler->GetHDDInfo(Key);
			if (pHDDInfo != NULL)
			{
				char buf[256];
				int index = 0;
				printf("\n");

				sprintf(buf, "%s Device ( %s ) :: ", pHDDInfo->DeviceNumber ? "Slave " : "Master", Key);
				index += strlen(buf);

				if (pHDDInfo->ModelNumber[0] == 0)
				{
					strcpy(buf + index, " N/A ");
					index += strlen(" N/A ");
				}
				else
				{
					for (BYTE j = 0; j < 20; j++)
					{
						buf[index] = pHDDInfo->ModelNumber[j];
						index++;
					}
				}

				strcpy(buf + index, " - ");
				index += strlen(" - ");

				if (pHDDInfo->SerialNumber[0] == 0)
				{
					strcpy(buf + index, " N/A ");
					index += strlen(" N/A ");
				}
				else
				{

					for (BYTE j = 0; j < 20; j++)
					{
						buf[index] = pHDDInfo->SerialNumber[j];
						index++;
					}
					buf[index] = 0;
					printf(buf);
				}
				printf("\n");
				printf("Cylinders %d Heads %d Sectors %d. LBA Sectors %d\n", pHDDInfo->CHSCylinderCount, pHDDInfo->CHSHeadCount, pHDDInfo->CHSSectorCount, pHDDInfo->LBACount);
			}
			Key[1]++;
		}
	}
}

