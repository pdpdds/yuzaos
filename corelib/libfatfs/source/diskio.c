/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

/* Definitions of physical drive number for each drive */
#define DEV_IDE		0	
#define DEV_RAM		1	/* Example: Map Ramdisk to physical drive 1 */
#define DEV_MMC		2	/* Example: Map MMC/SD card to physical drive 2 */
#define DEV_USB		3	/* Example: Map USB MSD to physical drive 3 */


extern DRESULT fat_disk_read(BYTE *buff, DWORD sector, UINT count);
extern DRESULT fat_disk_write(const BYTE *buff, DWORD sector, UINT count);
extern DSTATUS fat_disk_status();
extern DSTATUS fat_disk_initialize();

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;	
	stat = fat_disk_status();
	return stat;
	//switch (pdrv) {
	/*case DEV_RAM :
		stat = RAM_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		stat = MMC_disk_status();

		// translate the reslut code here

		return stat;
		*/
	/*case DEV_USB :
		stat = USB_disk_status();

		// translate the reslut code here

		return stat;
	case DEV_IDE:
		stat = IDE_disk_status();

		// translate the reslut code here

		return stat;
	}*/
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;	
	stat = fat_disk_initialize();
	return stat;
	//switch (pdrv) {
	/*case DEV_RAM :
		result = RAM_disk_initialize();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		result = MMC_disk_initialize();

		// translate the reslut code here

		return stat;
		*/
	/*case DEV_USB :
		stat = USB_disk_initialize();

		// translate the reslut code here

		return stat;
	case DEV_IDE:
		stat = IDE_disk_initialize();

		// translate the reslut code here

		return stat;
	}*/
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;	
	res = fat_disk_read(buff, sector, count);
	return res;
	//switch (pdrv) {
	/*case DEV_RAM :
		// translate the arguments here

		result = RAM_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		result = MMC_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
		*/
	/*case DEV_USB :
		// translate the arguments here

		res = USB_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
	case DEV_IDE:
		// translate the arguments here

		res = IDE_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
	}*/

	//return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;	
	res = fat_disk_write(buff, sector, count);
	return res;
	//switch (pdrv) {
	/*case DEV_RAM :
		// translate the arguments here

		result = RAM_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case DEV_MMC :
		// translate the arguments here

		result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
*/
	/*case DEV_USB :
		// translate the arguments here

		res = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	case DEV_IDE:
		// translate the arguments here

		res = IDE_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	}*/

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = 0;	
	return res;
	//switch (pdrv) {
	/*case DEV_RAM :

		// Process of the command for the RAM drive

		return res;

	case DEV_MMC :

		// Process of the command for the MMC/SD card

		return res;

	case DEV_USB :

		// Process of the command the USB drive

		return res;*/
	/*case DEV_IDE:
		// Process of the command the USB drive
		return res;
	}*/

	return RES_PARERR;
}