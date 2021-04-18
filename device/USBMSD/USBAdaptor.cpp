#include "USBAdaptor.h"
#include <minwindef.h>
#include <yuza_file_io.h>
#include <assert.h>
#include <SystemCall_Impl.h>
#include "devicemanager.h"
#include "pci.h"

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
	return new USBAdaptor("USBMSD");
}

static disk_t* g_disk = 0;
uint8_t* tempBuffer = 0;
#define READ_SECTOR_COUNT  1

extern "C" BYTE usb_disk_initialize()
{
	tempBuffer = (uint8_t * )malloc_aligned(512 * READ_SECTOR_COUNT, 4096);
	return 0;
}

extern "C" BYTE usb_disk_read(BYTE * buff, DWORD sector, UINT count)
{
	int readCount = 0;
	while (READ_SECTOR_COUNT <= count)
	{
		FS_ERROR result = SectorReads(sector + readCount * READ_SECTOR_COUNT, tempBuffer, READ_SECTOR_COUNT, g_disk);
		assert(result == CE_GOOD);

		memcpy(buff + readCount * g_disk->sectorSize * READ_SECTOR_COUNT, tempBuffer, g_disk->sectorSize * READ_SECTOR_COUNT);

		count -= READ_SECTOR_COUNT;
		readCount++;
	}

	if (count > 0)
	{
		FS_ERROR result = SectorReads(sector + readCount * READ_SECTOR_COUNT, tempBuffer, count, g_disk);
		assert(result == CE_GOOD);

		memcpy(buff + readCount * g_disk->sectorSize * READ_SECTOR_COUNT, tempBuffer, g_disk->sectorSize * count);
	}
	
	return 0;
}

extern "C" BYTE usb_disk_write(const BYTE * buff, DWORD sector, UINT count)
{
	Syscall_Printf("SCSI: write sector", count);
	int readCount = 0;
	while (READ_SECTOR_COUNT <= count)
	{
		memcpy(tempBuffer, buff + readCount * g_disk->sectorSize * READ_SECTOR_COUNT, g_disk->sectorSize * READ_SECTOR_COUNT);
		FS_ERROR result = SectorWrites(sector + readCount * READ_SECTOR_COUNT, tempBuffer, READ_SECTOR_COUNT, g_disk);
		assert(result == CE_GOOD);

		count -= READ_SECTOR_COUNT;
		readCount++;
	}

	if (count > 0)
	{
		memcpy(tempBuffer, buff + readCount * g_disk->sectorSize * READ_SECTOR_COUNT, g_disk->sectorSize * count);
		FS_ERROR result = SectorWrites(sector + readCount * READ_SECTOR_COUNT, tempBuffer, count, g_disk);
		assert(result == CE_GOOD);
	}

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

	if (g_disk == 0)
		return false;
		//Syscall_Panic("MSD Disk is NULL.\n");
	
	m_pFileSystem = pFileSystem;
	printf("g_disk %x %x\n", g_disk, g_disk->sectorSize);
	return m_pFileSystem->Initialize(&g_io_interface);
}

USBAdaptor::USBAdaptor(char* deviceName)
	: FileSysAdaptor(deviceName)
{
}

USBAdaptor::~USBAdaptor()
{
}
