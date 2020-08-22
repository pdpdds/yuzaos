#include "FloppyDSKAdaptor.h"
#include <stringdef.h>
#include "platformapi.h"
#include <assert.h>
#include <string>
#include <systemcall_impl.h>
#include "flpydsk.h"
#include "../USB/devicemanager.h"
#include <yuza_file_io.h>

#ifdef OUT
#undef OUT
#endif

#define DMA_PICU1       0x0020
#define DMA_PICU2       0x00A0

__declspec(naked) void SendEOI()
{
	_asm
	{
		PUSH EBP
		MOV  EBP, ESP
		PUSH EAX

		; [EBP] < -EBP
		; [EBP + 4] < -RET Addr
		; [EBP + 8] < -IRQ 번호

		MOV AL, 20H; EOI 신호를 보낸다.
		OUT DMA_PICU1, AL

		CMP BYTE PTR[EBP + 8], 7
		JBE END_OF_EOI
		OUT DMA_PICU2, AL; Send to 2 also

		END_OF_EOI :
		POP EAX
			POP EBP
			RET
	}
}

/* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;

//플로피 디스크 인터럽트 핸들러
//플로피 디스크로부터 인터럽트가 발생했다는 것만 체크한다.

bool g_emulation = false;
WIN32_FILE_IO_INTERFACE g_win32_io =
{
	0,
	0,
	0,
	0,
	0,
};

volatile bool _floppyDiskIRQ = false;

__declspec(naked) void FloppyDiskHandler()
{
	//레지스터를 저장하고 인터럽트를 끈다.
	_asm
	{
		PUSHAD
		PUSHFD
		CLI
	}

	_floppyDiskIRQ = true;

	SendEOI();

	

	// 레지스터를 복원하고 원래 수행하던 곳으로 돌아간다.
	_asm
	{
		POPFD
		POPAD
		IRETD
	}
}

extern "C" __declspec(dllexport) FileSysAdaptor * CreateFileAdaptor()
{
	return new FloppyDSKAdaptor("Floppy");
}

FILE* g_virtualDisk = 0;
CRITICAL_SECTION g_cs;

extern "C" BYTE floppy_disk_initialize_win32()
{
	char buf[256];
	int count = Syscall_GetEnvironmentVariable("FLOPPY", buf, 256);

	assert(count != 0);

	g_virtualDisk = g_win32_io.sky_fopen(buf, "rb+");
	return RES_OK;
}

extern "C" BYTE floppy_disk_read_win32(BYTE * buff, DWORD sector, UINT count)
{
	Syscall_EnterCriticalSection(&g_cs);
	g_win32_io.sky_fseek((FILE*)g_virtualDisk, sector * 512, SEEK_SET);
	size_t readCnt = g_win32_io.sky_fread(buff, 1, count * 512, (FILE*)g_virtualDisk);

	Syscall_LeaveCriticalSection(&g_cs);

	if (readCnt != count * 512)
	{
		return RES_ERROR;
	}

	return RES_OK;
}

extern "C" BYTE floppy_disk_write_win32(const BYTE * buff, DWORD sector, UINT count)
{
	Syscall_EnterCriticalSection(&g_cs);

	g_win32_io.sky_fseek((FILE*)g_virtualDisk, sector * 512, SEEK_SET);
	size_t writeCnt = g_win32_io.sky_fwrite(buff, 1, count * 512, (FILE*)g_virtualDisk);

	Syscall_LeaveCriticalSection(&g_cs);


	if (writeCnt != count * 512)
	{
		return RES_ERROR;
	}

	return RES_OK;
}

extern "C" BYTE floppy_disk_status_win32()
{
	return RES_OK;
}

extern floppy_t* floppyDrive[2];

extern "C" DSTATUS floppy_disk_status()
{
	return RES_OK;
}

extern "C" BYTE floppy_disk_initialize()
{
	flpydsk_install();

	return 0;
}

extern "C" BYTE floppy_disk_read(BYTE * buff, DWORD sector, UINT count)
{
	floppy_t* floppy = floppyDrive[0];
	FS_ERROR result = flpydsk_readSectors(sector, (void*)buff, count, floppy->drive.insertedDisk);
	if (result != CE_GOOD)
		return RES_ERROR;

	return 0;
}

extern "C" BYTE floppy_disk_write(const BYTE * buff, DWORD sector, UINT count)
{
	floppy_t* floppy = floppyDrive[0];
	FS_ERROR result = flpydsk_writeSectors(sector, (void*)buff, count, floppy->drive.insertedDisk);
	if (result != CE_GOOD)
		return 1;

	return 0;
}


FILE_IO_INTERFACE g_io_interface =
{
	floppy_disk_initialize,
	floppy_disk_read,
	floppy_disk_write,
	floppy_disk_status,
};

FILE_IO_INTERFACE g_io_interface_win32 =
{
	floppy_disk_initialize_win32,
	floppy_disk_read_win32,
	floppy_disk_write_win32,
	floppy_disk_status_win32,
};

bool FloppyDSKAdaptor::Initialize(I_FileSystem* pFileSystem, void* arg)
{
	int irq = 38;
	Syscall_SetDriverInterruptVector(irq, FloppyDiskHandler);

	m_pFileSystem = pFileSystem;

	g_emulation = Syscall_IsEmulationMode();

	bool result = false;
	if (g_emulation)
	{
		result = InitializeWin32();
	}
	else result = m_pFileSystem->Initialize(&g_io_interface);

	return result;
}

FloppyDSKAdaptor::FloppyDSKAdaptor(char* deviceName)
	: FileSysAdaptor(deviceName)
{
	if (g_emulation)
		Syscall_InitializeCriticalSection(&g_cs);

	printf("Create Floopy\n");
}

FloppyDSKAdaptor::~FloppyDSKAdaptor()
{
	if (g_emulation)
	{
		Syscall_DeleteCriticalSection(&g_cs);
		g_win32_io.sky_fclose(g_virtualDisk);
	}
}

typedef size_t(*psky_fread)(void* ptr, size_t size, size_t nmemb, FILE* stream);
typedef FILE* (*psky_fopen)(const char* filename, const char* mode);
typedef size_t(*psky_fwrite)(const void* ptr, size_t size, size_t nmemb, FILE* stream);
typedef int (*psky_fseek)(FILE* stream, long int offset, int whence);
typedef int (*psky_fclose)(FILE* stream);

bool FloppyDSKAdaptor::InitializeWin32()
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