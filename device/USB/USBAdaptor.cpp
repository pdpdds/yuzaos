#include <minwindef.h>
#include "USBAdaptor.h"
#include <stringdef.h>
#include "platformapi.h"
#include <assert.h>
#include <string>
#include <../include/SystemCall_Impl.h>
#include "_pci.h"
#include "devicemanager.h"
#include "usb.h"
#include <yuza_file_io.h>

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

extern "C" __declspec(dllexport) FileSysAdaptor * CreateFileAdaptor()
{
	return new USBAdaptor("USB");
}

static disk_t* g_disk = 0;
uint8_t* tempBuffer = 0;

extern "C" BYTE usb_disk_initialize()
{
	tempBuffer = (uint8_t * )malloc(512 * 32);
	return 0;
}

extern "C" BYTE usb_disk_read(BYTE * buff, DWORD sector, UINT count)
{
	DWORD totalReadBytes = 512 * count;
	DWORD readCount = 0;
	DWORD remain = 0;
	
	if (g_disk->sectorSize > totalReadBytes)
		remain = totalReadBytes;
	else
	{
		readCount = totalReadBytes / g_disk->sectorSize;
		remain = totalReadBytes % g_disk->sectorSize;
	}

	/*if (count > 1)
	{
		printf("USB Read. SectorNum : %x, Count : %x MSD SectorSize : %x\n", sector, count, g_disk->sectorSize);
		
		FS_ERROR result = g_disk->type->readSectors(sector, tempBuffer, count, g_disk);
		assert(result == CE_GOOD);

		memcpy(buff, tempBuffer, count * 512);
		printf("passs\n");
		return 0;
	}*/
	

	int i = 0;
	for (i = 0; i < readCount; i++)
	{
		FS_ERROR result = singleSectorRead(sector + i, tempBuffer, g_disk);
		memcpy(buff + i * g_disk->sectorSize, tempBuffer, g_disk->sectorSize);
		assert(result == CE_GOOD);
	}

	if (remain > 0)
	{
		FS_ERROR result = singleSectorRead(sector + i, tempBuffer, g_disk);
		assert(result == CE_GOOD);
		memcpy(buff + i * g_disk->sectorSize, tempBuffer, remain);
		
	}

	//printf("USB Read. SectorNum : %x, Count : %x MSD SectorSize : %x %x %x\n", sector, count, g_disk->sectorSize, totalReadBytes, remain);
	
	return 0;
}

extern "C" BYTE usb_disk_write(const BYTE * buff, DWORD sector, UINT count)
{
	return 0;
}

extern  "C" BYTE usb_disk_status()
{
	return 0;
}

FILE_IO_INTERFACE g_io_interface =
{
	usb_disk_initialize,
	usb_disk_read,
	usb_disk_write,
	usb_disk_status,
};

bool USBAdaptor::Initialize(I_FileSystem* pFileSystem, void* arg)
{	
	pci_init();
	deviceManger_init();

	pci_scan();
	printf("ScanPCI Complete\n");

	pci_installDevices();

	g_disk = GetMSD();
	assert(g_disk != 0);

	m_pFileSystem = pFileSystem;
	printf("g_disk %x %x\n", g_disk, g_disk->sectorSize);
	return m_pFileSystem->Initialize(&g_io_interface);

	for (;;)
	{
		printf("USBAdaptor FileSystem Init\n");
		Syscall_Sleep(1000);
	}


	for (;;)
	{
		Syscall_Sleep(1000);
		deviceManager_showDiskList();
		usb_pollInterruptTransfers();
		//usb_hubWatchdog();
	}

	//deviceManager_showPortList();

	return true;
}

USBAdaptor::USBAdaptor(char* deviceName)
	: FileSysAdaptor(deviceName)
{
}

USBAdaptor::~USBAdaptor()
{
}
