#include "flpydsk.h"
#include "cmos.h"
//#include "timer.h"
#include "irq.h"
#include "dma.h"
#include <systemcall_impl.h>
#include <string.h>

// detailed infos about FDC:
// http://www.isdaman.com/alsos/hardware/fdc/floppy.htm
// http://lowlevel.eu/wiki/FDC
// http://www.brokenthorn.com/Resources/OSDev20.html

const portType_t FDD = { .motorOff = &flpydsk_motorOff, .pollDisk = 0 };
const diskType_t FLOPPYDISK = { .readSectors = &flpydsk_readSectors, .writeSectors = &flpydsk_writeSectors };

#define MAX_FLOPPY                     2
#define MAX_ATTEMPTS_FLOPPY_DMA_BUFFER 5

// start of dma transfer buffer, end: 0x10000 (64 KiB border). It must be below 16 MiB = 0x1000000 and in identity mapped memory!
static void* const DMA_BUFFER = (void*)0x1000;
static DWORD DMA_mutex = 0;

static const uint8_t  FLPY_SECTORS_PER_TRACK      =  18; // sectors per track
static const uint32_t MOTOR_SPIN_UP_TURN_OFF_TIME = 300; // waiting time in milliseconds (motor spin up)

floppy_t* floppyDrive[MAX_FLOPPY];

static uint8_t flpydsk_version = 0; // Contains the result of the VERSION command of the FDC

typedef enum
{
    READ, WRITE
} RW_OPERATION;

// IO ports
enum
{
    FLPYDSK_DOR  = 0x3F2,
    FLPYDSK_MSR  = 0x3F4,
    FLPYDSK_FIFO = 0x3F5,
    FLPYDSK_CTRL = 0x3F7
};

// Bits 0-4 of command byte
enum
{
    FDC_CMD_READ_TRACK   =   2,
    FDC_CMD_SPECIFY      =   3,
    FDC_CMD_CHECK_STAT   =   4,
    FDC_CMD_WRITE_SECT   =   5,
    FDC_CMD_READ_SECT    =   6,
    FDC_CMD_CALIBRATE    =   7,
    FDC_CMD_CHECK_INT    =   8,
    FDC_CMD_FORMAT_TRACK = 0xD,
    FDC_CMD_DUMPREG      = 0xE,
    FDC_CMD_SEEK         = 0xF,
    FDC_CMD_VERSION      = 0x10,
    FDC_CMD_CONFIGURE    = 0x13,
    FDC_CMD_LOCK         = 0x94
};

// Additional command masks
enum
{
    FDC_CMD_EXT_SKIP       = 0x20,
    FDC_CMD_EXT_DENSITY    = 0x40,
    FDC_CMD_EXT_MULTITRACK = 0x80
};
#define BIT(n) (1U<<(n))
#define IS_BIT_SET(value, pos) ((value)&BIT(pos))

// Flags for CONFIGURE command
enum
{
    FDC_CONFIG_EIS_ON   = BIT(6),
    FDC_CONFIG_FIFO_OFF = BIT(5),
    FDC_CONFIG_POLL_OFF = BIT(4),
};

// Digital Output Register (DOR)
enum
{
    FLPYDSK_DOR_MASK_DRIVE0       =   0,
    FLPYDSK_DOR_MASK_DRIVE1       =   1,
    FLPYDSK_DOR_MASK_DRIVE2       =   2,
    FLPYDSK_DOR_MASK_DRIVE3       =   3,
    FLPYDSK_DOR_MASK_RESET        =   4,
    FLPYDSK_DOR_MASK_DMA          =   8,
    FLPYDSK_DOR_MASK_DRIVE0_MOTOR =  16,
    FLPYDSK_DOR_MASK_DRIVE1_MOTOR =  32,
    FLPYDSK_DOR_MASK_DRIVE2_MOTOR =  64,
    FLPYDSK_DOR_MASK_DRIVE3_MOTOR = 128
};

// Main Status Register (MSR)
enum
{
    FLPYDSK_MSR_MASK_DRIVE1_POS_MODE =   1,
    FLPYDSK_MSR_MASK_DRIVE2_POS_MODE =   2,
    FLPYDSK_MSR_MASK_DRIVE3_POS_MODE =   4,
    FLPYDSK_MSR_MASK_DRIVE4_POS_MODE =   8,
    FLPYDSK_MSR_MASK_BUSY            =  16,
    FLPYDSK_MSR_MASK_DMA             =  32,
    FLPYDSK_MSR_MASK_DATAIO          =  64,
    FLPYDSK_MSR_MASK_DATAREG         = 128
};



// GAP 3 sizes
enum
{
    FLPYDSK_GAP3_LENGTH_STD        = 42,
    FLPYDSK_GAP3_LENGTH_5_14       = 32,
    FLPYDSK_GAP3_LENGTH_3_5        = 27,
    FLPYDSK_GAP3_LENGTH_3_5_FORMAT = 84
};

// Formula: 2^sector_number * 128
enum
{
    FLPYDSK_SECTOR_DTL_128  = 0,    
    FLPYDSK_SECTOR_DTL_256  = 1,
    FLPYDSK_SECTOR_DTL_512  = 2,
    FLPYDSK_SECTOR_DTL_1024 = 4
};
extern  volatile bool _floppyDiskIRQ;

void waitForIRQ(int irq, DWORD milliSecond)
{
    if (milliSecond == 0)
    {
        while (_floppyDiskIRQ == false)
        {
            Syscall_Sleep(1);
        }
    }
    else
    {
        int waitCount = milliSecond;
        while (_floppyDiskIRQ == false || waitCount == 0)
        {
            Syscall_Sleep(1);
            waitCount--;
        }
    }

    _floppyDiskIRQ = false;
}

static void flpydsk_reset(void);
static floppy_t* createFloppy(uint8_t ID)
{
	DMA_mutex = Syscall_CreateMutex("DMA_mutex");
    floppy_t* fdd        = (floppy_t *)malloc(sizeof(floppy_t));
    fdd->ID              = ID;
    fdd->motor           = false; // floppy motor is off
    fdd->accessRemaining = 0;
	
	fdd->RW_Lock = Syscall_CreateMutex("fdd->RW_Lock");
   
    fdd->drive.type         = &FDD;

    fdd->drive.data         = (void*)fdd;
    fdd->drive.insertedDisk = (disk_t*)malloc(sizeof(disk_t));

    fdd->drive.insertedDisk->type            = &FLOPPYDISK;
    fdd->drive.insertedDisk->data            = (void*)fdd;
    fdd->drive.insertedDisk->port            = &fdd->drive;
    fdd->drive.insertedDisk->size            = 1440*1024;
    fdd->drive.insertedDisk->secPerTrack     = FLPY_SECTORS_PER_TRACK;
    fdd->drive.insertedDisk->optAccSecCount  = FLPY_SECTORS_PER_TRACK;
    fdd->drive.insertedDisk->alignedAccess   = true;
    fdd->drive.insertedDisk->sectorSize      = 512;
    fdd->drive.insertedDisk->headCount       = 2;
    fdd->drive.insertedDisk->accessRemaining = 0;
    memset(fdd->drive.insertedDisk->partition, 0, PARTITIONARRAYSIZE * sizeof(partition_t*));

    //deviceManager_attachDisk(fdd->drive.insertedDisk); // disk == floppy disk
    //deviceManager_attachPort(&fdd->drive);

    flpydsk_reset();
    //deviceManager_analyzeDisk(fdd->drive.insertedDisk);

    return (fdd);
}

static uint8_t flpydsk_readVersion(void);
// Looks for floppy drives and installs them
void flpydsk_install(void)
{
    printf("   => Floppy Disk (1.44 MB): ");
    uint8_t cmos = cmos_read(CMOS_FLOPPYTYPE);
    if ((cmos >> 4) == 4) // 1st floppy 1,44 MB: 0100....b
    {
        printf("first ");

        bool second = (cmos & 0xF) == 4; // 2nd floppy 1,44 MB: ....0100b
        if (second)
            printf("and second ");
        printf("drive found.");

        flpydsk_version = flpydsk_readVersion();

        floppyDrive[0] = createFloppy(0);
        strcpy(floppyDrive[0]->drive.name, "Floppy Dev 1");
        floppyDrive[0]->drive.name[12]=0; // terminate string

        if (second)
        {
            floppyDrive[1] = createFloppy(1);
            strcpy(floppyDrive[1]->drive.name, "Floppy Dev 2");
            floppyDrive[1]->drive.name[12]=0; // terminate string
        }
        else
        {
            floppyDrive[1] = 0;
        }

      #ifdef _FLOPPY_DIAGNOSIS_
        printf("     => FDC version: ");
        textColor(TEXT);
        printf("%yh\n",flpydsk_version);
        textColor(LIGHT_GRAY);
      #endif
    }
    else
    {
        printf("Not Present.");
        floppyDrive[0] = 0;
    }
}


/// Basic Controller In/Out Routines

// return fdc status
static uint8_t flpydsk_readStatus(void)
{
    return Syscall_InPortByte(FLPYDSK_MSR); // just return main status register
}

// write to the fdc dor
static void flpydsk_writeDOR(uint8_t val)
{
	Syscall_OutPortByte(FLPYDSK_DOR, val); // write the digital output register
}

// send command byte to fdc
static void flpydsk_sendCommand(uint8_t cmd)
{
    // wait until data register is ready. We send commands to the data register
    for (uint16_t i = 0; i < 500; i++)
    {
        uint8_t status = flpydsk_readStatus();
        if ((status & FLPYDSK_MSR_MASK_DATAREG) && !(status & FLPYDSK_MSR_MASK_DATAIO))
        {
			Syscall_OutPortByte(FLPYDSK_FIFO, cmd);
            return;
        }
    }
}

// get data from fdc
static uint8_t flpydsk_readData(void)
{
    // same as above function but returns data register for reading
    for (uint16_t i = 0; i < 500; i++)
    {
        uint8_t status = flpydsk_readStatus();
        if ((status & FLPYDSK_MSR_MASK_DATAREG) && (status & FLPYDSK_MSR_MASK_DATAIO))
        {
            return Syscall_InPortByte(FLPYDSK_FIFO);
        }
    }
    return (0);
}

// write to the configuation control register
static void flpydsk_writeCCR(uint8_t val)
{
	Syscall_OutPortByte(FLPYDSK_CTRL, val); // write the configuation control
}


/// Controller Command Routines

// check interrupt status command
static void flpydsk_checkInt(uint8_t* st0, uint8_t* cyl)
{
    flpydsk_sendCommand(FDC_CMD_CHECK_INT);
    *st0 = flpydsk_readData();
    *cyl = flpydsk_readData();
}

// turns the current floppy drives motor on
void flpydsk_motorOn(port_t* port)
{
    if (port == 0) return;

    floppy_t* fdrive = (floppy_t*)port->data;

  #ifdef _FLOPPY_DIAGNOSIS_
    if (fdrive->motor == false)
    {
        textColor(IMPORTANT);
        printf("\nflpydsk_motorOn drive: %u", fdrive->ID);
        textColor(TEXT);
    }
  #endif

    if (fdrive->motor == true) return;

    uint32_t motor = 0;
    switch (fdrive->ID) // select the correct mask based on current drive
    {
        case 0: motor = FLPYDSK_DOR_MASK_DRIVE0_MOTOR; break;
        case 1: motor = FLPYDSK_DOR_MASK_DRIVE1_MOTOR; break;
        case 2: motor = FLPYDSK_DOR_MASK_DRIVE2_MOTOR; break;
        case 3: motor = FLPYDSK_DOR_MASK_DRIVE3_MOTOR; break;
    }
    fdrive->motor = true;
    flpydsk_writeDOR(fdrive->ID | motor | FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA); // motor on
    Syscall_Sleep(MOTOR_SPIN_UP_TURN_OFF_TIME); // wait for the motor to spin up/turn off
}

// turns the current floppy drives motor on
void flpydsk_motorOff(port_t* port)
{
    if (port == 0) return;

    floppy_t* fdrive = (floppy_t*)port->data;

  #ifdef _FLOPPY_DIAGNOSIS_
    if (fdrive->motor == true)
    {
        textColor(IMPORTANT);
        printf("\nflpydsk_motorOff drive: %u", fdrive->ID);
        textColor(TEXT);
    }
    writeInfo(0, "Floppy motor: Global-Access-Counter: %u   Internal counter: %u   Motor on: %u", fdrive->drive.insertedDisk->accessRemaining, fdrive->accessRemaining, fdrive->motor);
  #endif

    if (fdrive->motor == false) return; // everything is already fine

    //deviceManager_flushCaches(port->insertedDisk); // Flush caches before turning of motor
    
	Syscall_LockMutex(fdrive->RW_Lock);
    if (fdrive->accessRemaining == 0)
    {
        flpydsk_writeDOR(FLPYDSK_DOR_MASK_RESET); // motor off
        fdrive->motor = false;
    }
    Syscall_UnlockMutex(fdrive->RW_Lock);
}

// configure drive
static void flpydsk_driveData(uint32_t stepr, uint32_t loadt, uint32_t unloadt, bool dma)
{
    flpydsk_sendCommand(FDC_CMD_SPECIFY);
    flpydsk_sendCommand(((stepr & 0xF) << 4) | (unloadt & 0xF));
    flpydsk_sendCommand((loadt << 1)         | (dma ? 0 : 1));
}

static uint8_t flpydsk_readVersion(void)
{
    flpydsk_sendCommand(FDC_CMD_VERSION);
    return (flpydsk_readData());
}

static bool flpydsk_lock(void)
{
    flpydsk_sendCommand(FDC_CMD_LOCK);
    return (flpydsk_readData() & BIT(4));
}

static void flpydsk_getDump(void)
{
  #ifdef _FLOPPY_DIAGNOSIS_
    textColor(HEADLINE);
    printf("\n\nFDC dump:");
    textColor(TEXT);
    flpydsk_sendCommand(FDC_CMD_DUMPREG);
    uint8_t temp = flpydsk_readData();
    printf("\n0. Cylinder: %u", temp);
    temp = flpydsk_readData();
    printf("     1. Cylinder: %u", temp);
    temp = flpydsk_readData();
    printf("     2. Cylinder: %u", temp);
    temp = flpydsk_readData();
    printf("     3. Cylinder: %u", temp);
    temp = flpydsk_readData();
    printf("\nSteprate: %u      Head unload time: %u", temp>>4, temp&0xF);
    temp = flpydsk_readData();
    printf("      Head load time: %u      DMA: %u", temp>>1, IS_BIT_SET(temp, 0));
    temp = flpydsk_readData();
    printf("\nSector/End of track: %u", temp);
    temp = flpydsk_readData();
    printf("      Lock: %u      D0: %u   D1: %u   D2: %u   D3: %u", IS_BIT_SET(temp, 7), IS_BIT_SET(temp, 2), IS_BIT_SET(temp, 3), IS_BIT_SET(temp, 4), IS_BIT_SET(temp, 5));
    printf("\nGAP: %u      WGATE: %u", IS_BIT_SET(temp, 1), IS_BIT_SET(temp, 0));
    temp = flpydsk_readData();
    printf("      Implied seek: %u      FIFO: %u      Polling: %u", IS_BIT_SET(temp, 6), !IS_BIT_SET(temp, 5), !IS_BIT_SET(temp, 4));
    printf("\nFIFO threshold: %u", temp&0xF);
    temp = flpydsk_readData();
    printf("      Precompensation track number: %u\n", temp);
  #endif
}

static void flpydsk_configure(void)
{
    if (flpydsk_version == 0x90) // Enhanced FDC
    {
        flpydsk_getDump();
        flpydsk_sendCommand(FDC_CMD_CONFIGURE);
        flpydsk_sendCommand(0);
        flpydsk_sendCommand(FDC_CONFIG_EIS_ON | FDC_CONFIG_POLL_OFF | 7); // Implied Seek, FIFO, no Polling, Threshold: 8 (7=8-1)
        flpydsk_sendCommand(0);
        flpydsk_lock(); // Lock the configuration. Then we do not have to repeat configure after every reset
        flpydsk_getDump();
    }
}

// disable controller
static void flpydsk_disableController(void)
{
    flpydsk_writeDOR(0);
    for (uint8_t i = 0; i < MAX_FLOPPY; i++)
        if (floppyDrive[i])
            floppyDrive[i]->motor = false; // Attention! The motor had been turned off, although flpydsk_control_motor was not called
}

// enable controller
static void flpydsk_enableController(void)
{
    //irq_resetCounter(IRQ_FLOPPY);
    flpydsk_writeDOR(FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA);
    for (uint8_t i = 0; i < MAX_FLOPPY; i++)
        if (floppyDrive[i])
            floppyDrive[i]->motor = false; // Attention! The motor had been turned off, although flpydsk_control_motor was not called
	waitForIRQ(IRQ_FLOPPY, 2000);
}

// reset controller
static void flpydsk_reset(void)
{
    flpydsk_disableController();
    flpydsk_enableController();

    flpydsk_writeCCR(0); // transfer speed 500 kb/s. Although the specification says, there should be an interrupt after setting the CCR, there is no.

    if (flpydsk_version == 0x90)
    {
        uint8_t st0, cyl;
        // send CHECK_INT/SENSE INTERRUPT command to all drives
        for (uint8_t i=0; i<4; i++)
        {
            flpydsk_checkInt(&st0,&cyl);
        }
    }

    flpydsk_configure();

    flpydsk_driveData((16-4), 15, 0xF, true); // pass mechanical drive info: steprate=4ms, load time=30ms, unload time=240ms (0xF bei 500K)
}

/*
http://en.wikipedia.org/wiki/CHS_conversion#From_LBA_to_CHS
CYL = LBA / (HPC * SPT)
TEMP = LBA % (HPC * SPT)
HEAD = TEMP / SPT
SECT = TEMP % SPT + 1
*/

// convert LBA to CHS
static void flpydsk_LBAtoCHS(uint32_t lba, uint8_t* head, uint8_t* track, uint8_t* sector)
{
    *track  =  lba / (FLPY_SECTORS_PER_TRACK * 2);
    *head   = (lba % (FLPY_SECTORS_PER_TRACK * 2)) / FLPY_SECTORS_PER_TRACK;
    *sector = (lba % (FLPY_SECTORS_PER_TRACK * 2)) % FLPY_SECTORS_PER_TRACK + 1;
}

// calibrate the drive
static bool flpydsk_calibrate(floppy_t* drive)
{
    flpydsk_motorOn(&drive->drive);

    uint8_t st0, cyl, timeout = 10;
    // Send CHECK_INT command before "real" execution, because real hardware rejects command at first attempt otherwise
    flpydsk_checkInt(&st0, &cyl);

    do
    {
        //irq_resetCounter(IRQ_FLOPPY);

        flpydsk_sendCommand(FDC_CMD_CALIBRATE);
        flpydsk_sendCommand(drive->ID);
        waitForIRQ(IRQ_FLOPPY, 500);
        flpydsk_checkInt(&st0, &cyl);

        timeout--;
        if (timeout == 0)
        {
            drive->accessRemaining--;
            return false;
        }
    } while (!IS_BIT_SET(st0, 5));

    drive->accessRemaining--;
    return true;
}

// seek to given track/cylinder
static int32_t flpydsk_seek(floppy_t* drive, uint8_t cyl, uint8_t head)
{
    drive->accessRemaining++;

    if (!flpydsk_calibrate(drive)) // calibrate the disk ==> cyl. 0
    {
        drive->accessRemaining--;
        return (-2);
    }

    flpydsk_motorOn(&drive->drive);

    uint8_t st0, cyl0, timeout = 10;
    do
    {
        //irq_resetCounter(IRQ_FLOPPY);

        flpydsk_sendCommand(FDC_CMD_SEEK);
        flpydsk_sendCommand((head) << 2 | drive->ID);
        flpydsk_sendCommand(cyl);

        waitForIRQ(IRQ_FLOPPY, 2000);
        flpydsk_checkInt(&st0,&cyl0);

        timeout--;
        if (timeout == 0)
        {
            drive->accessRemaining--;
            return (-1);
        }
    } while (!IS_BIT_SET(st0, 5));

    drive->accessRemaining--;
    return (0);
}

// read or write a sector
static bool flpydsk_transferSector(floppy_t* drive, uint8_t head, uint8_t track, uint8_t sector, uint8_t numberOfSectors, RW_OPERATION operation)
{
    flpydsk_motorOn(&drive->drive);

    //irq_resetCounter(IRQ_FLOPPY);

    // Prepare DMA and send command
    if (operation == READ)
    {
        dma_read(DMA_BUFFER, numberOfSectors*512, &dma_channel[2], DMA_SINGLE);
        flpydsk_sendCommand(FDC_CMD_READ_SECT | FDC_CMD_EXT_MULTITRACK | FDC_CMD_EXT_DENSITY);
    }
    else if (operation == WRITE)
    {
        dma_write(DMA_BUFFER, numberOfSectors*512, &dma_channel[2], DMA_SINGLE);
        flpydsk_sendCommand(FDC_CMD_WRITE_SECT | FDC_CMD_EXT_DENSITY);
    }

    flpydsk_sendCommand(head << 2 | drive->ID);
    flpydsk_sendCommand(track);
    flpydsk_sendCommand(head);
    flpydsk_sendCommand(sector);
    flpydsk_sendCommand(FLPYDSK_SECTOR_DTL_512);
    flpydsk_sendCommand(sector+numberOfSectors-1); // Last sector to be transfered
    flpydsk_sendCommand(FLPYDSK_GAP3_LENGTH_3_5);
    flpydsk_sendCommand(0xFF);
    waitForIRQ(IRQ_FLOPPY, 2000);
    uint8_t val;
    for (uint8_t j=0; j<7; ++j)
    {
        val = flpydsk_readData(); // read status info: ST0 ST1 ST2 C H S Size(2: 512 Byte)
    }

    uint8_t st0, cyl;
    flpydsk_checkInt(&st0,&cyl); // inform FDC that we handled interrupt

    drive->accessRemaining--;

    return val == 2; // value 2 means 512 Byte
}

static FS_ERROR flpydsk_read(floppy_t* drive, uint32_t sectorLBA, uint8_t numberOfSectors)
{
    uint8_t head = 0, track = 0, sector = 1;
    flpydsk_LBAtoCHS(sectorLBA, &head, &track, &sector);

    drive->accessRemaining+=2;
    if (flpydsk_seek(drive, track, head) != 0)
    {
        printf("\nseek error");
        drive->accessRemaining--;
        return CE_SEEK_ERROR;
    }

    uint32_t timeout = 5; // limit
    while (!flpydsk_transferSector(drive, head, track, sector, numberOfSectors, READ))
    {
        timeout--;
        if (timeout == 0)
        {
            printf("\nread_sector timeout: read error!");
            return CE_TIMEOUT;
        }
        drive->accessRemaining++;
    }

    return CE_GOOD;
}

// write a sector
static FS_ERROR flpydsk_write(floppy_t* drive, uint32_t sectorLBA, uint8_t numberOfSectors)
{
    // convert LBA sector to CHS
    uint8_t head = 0, track = 0, sector = 1;
    flpydsk_LBAtoCHS(sectorLBA, &head, &track, &sector);

    drive->accessRemaining+=2;

    if (flpydsk_seek(drive, track, head) != 0)
    {
        printf("flpydsk_seek not ok. sector not written.\n");
        drive->accessRemaining--;
        drive->drive.insertedDisk->accessRemaining--;
        return CE_SEEK_ERROR;
    }

    if (!flpydsk_transferSector(drive, head, track, sector, numberOfSectors, WRITE))
    {
        drive->drive.insertedDisk->accessRemaining--;
        return CE_WRITE_ERROR;
    }
    drive->drive.insertedDisk->accessRemaining--;
    return CE_GOOD;
}


/// Functions accessed from outside the floppy driver
FS_ERROR flpydsk_readSectors(uint32_t sector, void* buffer, uint32_t count, disk_t* device)
{
    floppy_t* drive = (floppy_t*)device->data;

    FS_ERROR retVal = CE_GOOD;

	Syscall_LockMutex(drive->RW_Lock);
	Syscall_LockMutex(DMA_mutex);

    memset((void*)DMA_BUFFER, 0x41, 4*sizeof(uint32_t)); // 0x41 is in ASCII the 'A'. Used to detect problems while reading.
    for (uint8_t n = 0; n < MAX_ATTEMPTS_FLOPPY_DMA_BUFFER; n++)
    {
        ((uint32_t*)DMA_BUFFER)[5] = n;
        retVal = flpydsk_read(drive, sector, count); // Read the whole track.

        if (retVal != CE_GOOD)
        {
            printf("\nread error: %d\n", retVal);
        }
        else if (((uint32_t*)DMA_BUFFER)[4] == n &&
                 ((uint32_t*)DMA_BUFFER)[0] == 0x41414141 && ((uint32_t*)DMA_BUFFER)[1] == 0x41414141 &&
                 ((uint32_t*)DMA_BUFFER)[2] == 0x41414141 && ((uint32_t*)DMA_BUFFER)[3] == 0x41414141)
        {
            #ifdef _FLOPPY_DIAGNOSIS_
            printfe("\nDMA attempt no. %d failed.", n+1);
            #endif
            if (n >= MAX_ATTEMPTS_FLOPPY_DMA_BUFFER-1)
            {
                printf("\nDMA error.");
				Syscall_UnlockMutex(DMA_mutex);
				Syscall_UnlockMutex(drive->RW_Lock);
                drive->drive.insertedDisk->accessRemaining--;
                return (CE_NOT_PRESENT); // We assume, that this means, that no disk is in the slot
            }
        }
        else
        {
            break; // Everything is fine
        }
    }

    if (retVal == CE_SEEK_ERROR) // We assume, that this means, that no disk is in the slot
    {
		Syscall_UnlockMutex(DMA_mutex);
		Syscall_UnlockMutex(drive->RW_Lock);
        drive->drive.insertedDisk->accessRemaining--;
        return (CE_NOT_PRESENT);
    }

    memcpy(buffer, DMA_BUFFER, count * 512); // Copy the requested sectors from the DMA_BUFFER to the destination buffer
	Syscall_UnlockMutex(DMA_mutex);
	Syscall_UnlockMutex(drive->RW_Lock);

    drive->drive.insertedDisk->accessRemaining--;
    return (retVal);
}

FS_ERROR flpydsk_writeSectors(uint32_t sector, const void* buffer, uint32_t count, disk_t* device)
{
    floppy_t* drive = (floppy_t*)device->data;

	Syscall_LockMutex(drive->RW_Lock);
	Syscall_LockMutex(DMA_mutex);
    memcpy(DMA_BUFFER, buffer, 512 * count);

    uint32_t timeout = 2; // limit
    FS_ERROR retVal = CE_GOOD;

    while ((retVal = flpydsk_write(drive, sector, count)) != 0)
    {
        timeout--;
        printf("write error: attempts left: %u\n", timeout);
        if (timeout == 0)
        {
            printf("timeout\n");
            break;
        }
        drive->drive.insertedDisk->accessRemaining++;
    }
	Syscall_UnlockMutex(DMA_mutex);
	Syscall_UnlockMutex(drive->RW_Lock);

    if (retVal == CE_SEEK_ERROR)
        return (CE_NOT_PRESENT); // We assume, that this means, that no disk is in the slot
    return retVal;
}

void flpydsk_refreshVolumeName(disk_t* disk)
{
    if (!disk->partition[0] || !disk->partition[0]->name)
        return;

    char buffer[512];

    disk->accessRemaining++;
    if (flpydsk_readSectors(19, buffer, 1, disk) == CE_GOOD) // start at 0x2600: root directory (14 sectors)
    {
        //deviceManager_fillCache(19, disk, (uint8_t*)buffer, 1, false);
        strncpy(disk->partition[0]->name, buffer, 11);
        disk->partition[0]->name[11] = 0; // end of string
    }
}

/*
* Copyright (c) 2009-2017 The PrettyOS Project. All rights reserved.
*
* http://www.prettyos.de
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
