#pragma once
#include <stdint.h>

#define PARTITIONARRAYSIZE 4

typedef enum
{
	CE_GOOD = 0,                    // No error
	CE_ERASE_FAIL,                  // An erase failed
	CE_NOT_PRESENT,                 // No device was present
	CE_NOT_FORMATTED,               // The disk is of an unsupported format
	CE_BAD_PARTITION,               // The boot record is bad

	CE_UNSUPPORTED_FS,              // The file system type is unsupported
	CE_BAD_FORMAT_PARAM,            // Trying to format a disk with bad parameters
	CE_NOT_INIT,                    // An operation was performed on an uninitialized device
	CE_BAD_SECTOR_READ,             // A bad read of a sector occured
	CE_WRITE_ERROR,                 // Could not write to a sector

	CE_INVALID_CLUSTER,             // Invalid cluster value > maxcls
	CE_FILE_NOT_FOUND,              // Could not find the file on the device
	CE_DIR_NOT_FOUND,               // Could not find the directory
	CE_BAD_FILE,                    // File is corrupted
	CE_TIMEOUT,                     // Timout while trying to access

	CE_COULD_NOT_GET_CLUSTER,       // Could not load/allocate next cluster in file
	CE_FILENAME_2_LONG,             // A specified file name is too long to use
	CE_FILENAME_EXISTS,             // A specified filename already exists on the device
	CE_INVALID_FILENAME,            // Invalid file name
	CE_DELETE_DIR,                  // The user tried to delete a directory with FSremove

	CE_DIR_FULL,                    // All root dir entry are taken
	CE_DISK_FULL,                   // All clusters in partition are taken
	CE_DIR_NOT_EMPTY,               // This directory is not empty yet, remove files before deleting
	CE_NONSUPPORTED_SIZE,           // The disk is too big to format as FAT16
	CE_WRITE_PROTECTED,             // Card is write protected

	CE_FILENOTOPENED,               // File not opened for the write
	CE_SEEK_ERROR,                  // File location could not be changed successfully
	CE_BADCACHEREAD,                // Bad cache read
	CE_UNSUPPORTED_FUNCTION,        // Driver does not support this operation
	CE_READONLY,                    // The file is read-only

	CE_WRITEONLY,                   // The file is write-only
	CE_INVALID_ARGUMENT,            // Invalid argument
	CE_FOPEN_ON_DIR,                // Attempted to call fopen() on a directory
	CE_UNSUPPORTED_SECTOR_SIZE,     // Unsupported sector size

	CE_FAT_EOF = 60,                // Read try beyond FAT's EOF
	CE_EOF                          // EOF
} FS_ERROR;

typedef struct partition
{
	//const fileSystem_t* type;  // Type of the partition. 0 = unformated
	uint32_t       subtype;    // Example: To detect wether its a FAT12, FAT16 or FAT32 device although it uses the same driver
	void* data;       // data specific to partition-type
	struct folder* rootFolder; // Root of the file tree

	struct disk* disk;    // The disk containing the partition
	uint32_t      serial;  // serial for identification
	char* name;    // volume label

	uint32_t      start;   // First sector
	uint32_t      size;    // Total size of partition (in sectors)
	bool          mount;   // false: not mounted
} partition_t;

typedef struct
{
	void (*motorOff)(struct port*);
	void (*pollDisk)(struct port*);
} portType_t;

typedef struct diskType
{
	// Parameters: LBA, buffer, sector count, disk
	// Implementation note: The devmgr will take care about disk_t::optAccSecCount
	//                      and always request optAccSecCount aligned request with
	//                      a maximum of optAccSecCount sectors at once, if
	//                      disk_t::alignedAccess is set.
	FS_ERROR(*readSectors) (uint32_t, void*, uint32_t, struct disk*);
	FS_ERROR(*writeSectors)(uint32_t, const void*, uint32_t, struct disk*);
} diskType_t;

typedef struct disk
{
	const diskType_t* type;
	partition_t* partition[PARTITIONARRAYSIZE]; // 0 if partition is not used
	uint64_t     size;                          // size of disk in bytes
	void* data;                          // Contains additional information depending on disk-type
	uint32_t     accessRemaining;               // Used to control motor
	struct port* port;

	// Technical data of the disk
	uint32_t sectorSize;     // Bytes per sector
	uint16_t secPerTrack;    // Number of sectors per track (if the disk is separated into tracks)
	uint16_t headCount;      // Number of heads (if the disk is separated into heads)
	uint16_t optAccSecCount; // Number of sectors that should be accessed at once
	bool     alignedAccess;  // Accesses will be aligned by optAccSecCount
	uint8_t  BIOS_driveNum;  // Number of this disk given by BIOS
} disk_t;

typedef struct port
{
	const portType_t* type;
	disk_t* insertedDisk; // 0 if no disk is inserted
	void* data;         // Contains additional information depending on its type
	char        name[15];
} port_t;

#pragma pack(push, 1)
// 16-byte partition record // http://en.wikipedia.org/wiki/Master_boot_record
typedef struct
{
    uint8_t  bootflag;     // Status: 0x80 = bootable (active), 0x00 = non-bootable, other = invalid
    uint8_t  startCHS[3];  // CHS address of first absolute sector in partition
    uint8_t  type;         // http://en.wikipedia.org/wiki/Partition_type
    uint8_t  endCHS[3];    // CHS address of last absolute sector in partition
    uint32_t startLBA;     // LBA of first absolute sector in the partition
    uint32_t sizeLBA;      // Number of sectors in partition
}  partitionEntry_t;

typedef struct
{
	char     jumpBoot[3];
	char     SysName[8];
	uint16_t bytesPerSector;
	uint8_t  SectorsPerCluster;
	uint16_t ReservedSectors;
	uint8_t  FATcount;
	uint16_t MaxRootEntries;
	uint16_t TotalSectors16;
	uint8_t  MediaDescriptor;
	uint16_t FATsize16;
	uint16_t SectorsPerTrack;
	uint16_t HeadCount;
	uint32_t HiddenSectors;
	uint32_t TotalSectors32;
} BPBbase_t;
#pragma pack(pop)


void deviceManager_install(void);
void deviceManager_checkDrives(void);
void deviceManager_attachPort(port_t* port);
void deviceManager_destructPort(port_t* port);
void deviceManager_attachDisk(disk_t* disk);
void deviceManager_destructDisk(disk_t* disk);
void deviceManager_showPortList(void);
void deviceManager_showDiskList(void);
void deviceManger_init();

FS_ERROR deviceManager_analyzeDisk(disk_t* disk);
disk_t* GetMSD();

FS_ERROR SectorReads       (uint32_t sector, uint8_t* buffer, int count, disk_t* disk);
FS_ERROR SectorWrites      (uint32_t sector, const uint8_t* buffer, int count, disk_t* disk);

void deviceManager_fillCache(uint32_t sector, disk_t* disk, const uint8_t* buffer, uint32_t count, bool write);
void deviceManager_flushCaches(disk_t* disk);
void deviceManager_clearCaches(disk_t* disk);

extern const portType_t FDD, USB_UHCI, USB_EHCI, USB_XHCI;
extern const diskType_t FLOPPYDISK, USB_MSD;
