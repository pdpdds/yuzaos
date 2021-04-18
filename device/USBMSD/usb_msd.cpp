#include <stdio.h>
#include "usb_msd.h"
#include "usb_hc.h"
#include <systemcall_impl.h>
#include <string.h>

uint16_t htons(uint16_t v) {
	return (v >> 8) | (v << 8);
}
uint32_t htonl(uint32_t v) {
	return htons(v >> 16) | (htons((uint16_t)v) << 16);
}

#define ntohs(v) htons(v)
#define ntohl(v) htonl(v)


static const uint32_t CSWMagicNotOK = 0x01010101;
static const uint32_t CSWMagicOK    = 0x53425355; // USBS
static const uint32_t CBWMagic      = 0x43425355; // USBC


static uint32_t showResultsRequestSense(const void* addr);
static void     usb_bulkReset(usb_msd_t* msd);
static void     usb_resetRecoveryMSD(usb_msd_t* msd);
static void     testMSD(usb_msd_t* msd);
static uint32_t* g_pdataBuffer = 0;
static char* statusBuffer = 0;

void usb_setupMSD(usb_interface_t* interface)
{
	printf("\nUSB Setup MSD\n");
    usb_msd_t* msd = (usb_msd_t*)malloc(sizeof(usb_msd_t));
    g_pdataBuffer = (uint32_t*)malloc_aligned(4096, 4096);
    statusBuffer = (char*)malloc_aligned(4096, 4096);

    interface->data = msd;
    msd->interface = interface;

    // Get endpoints
    for (dlelement_t* el = interface->device->endpoints.head; el; el = el->next)
    {
        usb_endpoint_t* ep = (usb_endpoint_t*)el->data;
        if (ep->type == EP_BULK)
        {
            if (ep->dir == EP_OUT)
                msd->endpointOutMSD = ep;
            else if(ep->dir == EP_IN)
                msd->endpointInMSD = ep;
        }
    }
   
    // Disk
    interface->device->port->insertedDisk = &msd->disk;
    msd->disk.port            = interface->device->port;
    msd->disk.type            = &USB_MSD;
    msd->disk.sectorSize      = 512;
    msd->disk.data            = msd;
    msd->disk.accessRemaining = 0;
    msd->disk.BIOS_driveNum   = 0;
    msd->disk.headCount       = 0;

    bool UFI = interface->descriptor.interfaceSubclass == 0x04;
    if (UFI)
    {
        msd->disk.optAccSecCount  = 18;
        msd->disk.alignedAccess   = true;
    }
    else
    {
        msd->disk.optAccSecCount  = 64;
        msd->disk.alignedAccess   = false;
    }

    msd->disk.secPerTrack     = 0;
    memset(msd->disk.partition, 0, sizeof(partition_t*) * PARTITIONARRAYSIZE);
    deviceManager_attachDisk(&msd->disk);

    usb_getStringDescriptor(interface->device);

    uint8_t loop = 4;
    for (uint8_t i = 1; i<loop; i++) // fetch 3 strings
    {
        usb_getUnicodeStringDescriptor(interface->device, i);
    }
	printf("USB_Setup MSD End\n");
  
    testMSD(msd); // test with some SCSI commands
}

void usb_destroyMSD(usb_interface_t* interface)
{
    usb_msd_t* msd = (usb_msd_t*)interface->data;
    deviceManager_destructDisk(&msd->disk);
    interface->data = 0;
    free(msd);
}

// Bulk-Only Mass Storage get maximum number of Logical Units
/*static uint8_t usb_getMaxLUN(usb_msd_t* msd) // only for bulk-only
{
  #ifdef _USB_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\nUSB2: usbTransferBulkOnlyGetMaxLUN, dev: %X interface: %u", device, numInterface);
    textColor(TEXT);
  #endif

    uint8_t maxLUN;

    usb_transfer_t transfer;
    usb_constructTransfer(msd->device, &transfer, USB_CONTROL, msd->device->endpoints.head->data);

    // bmRequestType bRequest  wValue wIndex    wLength   Data
    // 10100001b     11111110b 0000h  Interface 0001h     1 byte
    usb_setupTransaction(&transfer, 0xA1, 0xFE, 0, 0, msd->interface->interfaceNumber, 1);
    usb_inTransaction(&transfer, false, &maxLUN, 1);
    usb_outTransaction(&transfer, true, 0, 0); // handshake

    usb_scheduleTransfer(&transfer);
    usb_waitForTransfer(&transfer);
    usb_destructTransfer(&transfer);

    return maxLUN;
}*/

// Bulk-Only Mass Storage Reset
static void usb_bulkReset(usb_msd_t* msd)  // only for bulk-only
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
   // textColor(LIGHT_CYAN);
    printf("\nusbTransferBulkOnlyMassStorageReset");
    //textColor(TEXT);
  #endif

    // bmRequestType bRequest  wValue wIndex    wLength   Data
    // 00100001b     11111111b 0000h  Interface 0000h     none
    usb_controlSet(msd->interface->device, 0x21, 0xFF, 0, 0, msd->interface->descriptor.interfaceNumber);
}

static void formatSCSICommand(uint8_t SCSIcommand, struct usb_CommandBlockWrapper* cbw, uint32_t LBA, uint16_t TransferLength, usb_msd_t* msd)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    printf("\nSCSICommand Bulk-only: %u LBA %u TransferLength: %u", SCSIcommand, LBA, TransferLength);
  #endif

    memset(cbw, 0, sizeof(struct usb_CommandBlockWrapper));
    cbw->CBWSignature  = CBWMagic;                      // magic
    cbw->CBWTag        = 0x42424200 | SCSIcommand;      // device echoes this field in the CSWTag field of the associated CSW
    cbw->CBWDataTransferLength = TransferLength;        // Transfer length in bytes (only data)
    cbw->commandByte[0]        = SCSIcommand;           // Operation code
    switch (SCSIcommand)
    {
        case 0x00: // test unit ready(6)
            cbw->CBWFlags              = 0x00;          // Out: 0x00  In: 0x80
            cbw->CBWCBLength           = 6;             // only bits 4:0
            break;
        case 0x03: // Request Sense(6)
            cbw->CBWFlags              = 0x80;          // Out: 0x00  In: 0x80
            cbw->CBWCBLength           = 6;             // only bits 4:0
            cbw->commandByte[4]        = 18;            // Allocation length (max. bytes)
            break;
        case 0x12: // Inquiry(6)
            cbw->CBWFlags              = 0x80;          // Out: 0x00  In: 0x80
            cbw->CBWCBLength           = 6;             // only bits 4:0
            cbw->commandByte[4]        = 36;            // Allocation length (max. bytes)
            break;
        case 0x25: // read capacity(10)
            cbw->CBWFlags              = 0x80;          // Out: 0x00  In: 0x80
            cbw->CBWCBLength           = 10;            // only bits 4:0
            cbw->commandByte[2]        = BYTE4(LBA);    // LBA MSB
            cbw->commandByte[3]        = BYTE3(LBA);    // LBA
            cbw->commandByte[4]        = BYTE2(LBA);    // LBA
            cbw->commandByte[5]        = BYTE1(LBA);    // LBA LSB
            break;
        case 0x28: // read(10)
            cbw->CBWFlags               = 0x80;                  // Out: 0x00  In: 0x80
            cbw->CBWCBLength            = 10;                    // only bits 4:0
            cbw->commandByte[2]         = BYTE4(LBA);            // LBA MSB
            cbw->commandByte[3]         = BYTE3(LBA);            // LBA
            cbw->commandByte[4]         = BYTE2(LBA);            // LBA
            cbw->commandByte[5]         = BYTE1(LBA);            // LBA LSB
            cbw->commandByte[7]         = BYTE2(TransferLength/msd->disk.sectorSize); // MSB <--- blocks not byte!
            cbw->commandByte[8]         = BYTE1(TransferLength/msd->disk.sectorSize); // LSB
            break;
        case 0x2A: // write(10)
            cbw->CBWFlags               = 0x00;                  // Out: 0x00  In: 0x80
            cbw->CBWCBLength            = 10;                    // only bits 4:0
            cbw->commandByte[2]         = BYTE4(LBA);            // LBA MSB
            cbw->commandByte[3]         = BYTE3(LBA);            // LBA
            cbw->commandByte[4]         = BYTE2(LBA);            // LBA
            cbw->commandByte[5]         = BYTE1(LBA);            // LBA LSB
            cbw->commandByte[7]         = BYTE2(TransferLength/msd->disk.sectorSize); // MSB <--- blocks not byte!
            cbw->commandByte[8]         = BYTE1(TransferLength/msd->disk.sectorSize); // LSB
            break;
    }
}

static void formatSCSICommandUFI(uint8_t SCSIcommand, struct usb_CommandBlockWrapperUFI* cbw, uint32_t LBA, uint16_t TransferLength, usb_msd_t* msd)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    printf("\nSCSICommand UFI: %u LBA %u TransferLength: %u", SCSIcommand, LBA, TransferLength);
  #endif

    memset(cbw, 0, sizeof(struct usb_CommandBlockWrapperUFI));
    cbw->commandByte[0]        = SCSIcommand;           // Operation code
    switch (SCSIcommand)
    {
        case 0x00: // test unit ready(6)
            break;
        case 0x03: // Request Sense(6)
            cbw->commandByte[4]        = 18;            // Allocation length (max. bytes)
            break;
        case 0x12: // Inquiry(6)
            cbw->commandByte[4]        = 36;            // Allocation length (max. bytes)
            break;
        case 0x25: // read capacity(10)
            cbw->commandByte[2]        = BYTE4(LBA);    // LBA MSB
            cbw->commandByte[3]        = BYTE3(LBA);    // LBA
            cbw->commandByte[4]        = BYTE2(LBA);    // LBA
            cbw->commandByte[5]        = BYTE1(LBA);    // LBA LSB
            break;
        case 0x28: // read(10)
            cbw->commandByte[2]         = BYTE4(LBA);            // LBA MSB
            cbw->commandByte[3]         = BYTE3(LBA);            // LBA
            cbw->commandByte[4]         = BYTE2(LBA);            // LBA
            cbw->commandByte[5]         = BYTE1(LBA);            // LBA LSB
            cbw->commandByte[7]         = BYTE2(TransferLength/msd->disk.sectorSize); // MSB <--- blocks not byte!
            cbw->commandByte[8]         = BYTE1(TransferLength/msd->disk.sectorSize); // LSB
            break;
        case 0x2A: // write(10)
            cbw->commandByte[2]         = BYTE4(LBA);            // LBA MSB
            cbw->commandByte[3]         = BYTE3(LBA);            // LBA
            cbw->commandByte[4]         = BYTE2(LBA);            // LBA
            cbw->commandByte[5]         = BYTE1(LBA);            // LBA LSB
            cbw->commandByte[7]         = BYTE2(TransferLength/msd->disk.sectorSize); // MSB <--- blocks not byte!
            cbw->commandByte[8]         = BYTE1(TransferLength/msd->disk.sectorSize); // LSB
            break;
    }
}

void memshow(const void* start, size_t count, bool alpha)
{
	for (size_t i = 0; i < count; i++)
	{
		if (alpha)
		{
			printf("%c", ((const char*)start)[i]);
		}
		else
		{
			if (i % 16 == 0)
				printf("\n");
			printf("%x ", ((const uint8_t*)start)[i]);
		}
	}
}

static int checkSCSICommand(void* MSDStatus, usb_msd_t* msd, uint16_t TransferLength, uint8_t SCSIOpcode)
{
    // CSW Status
  #ifdef _USB_DIAGNOSIS_
    printf("\n");
    memshow(MSDStatus,13, false);
	printf("\n");
  #endif

    int error = 0;

    // check signature 0x53425355 // DWORD 0 (byte 0:3)
    uint32_t CSWsignature = *(uint32_t*)MSDStatus; // DWORD 0
    if (CSWsignature == CSWMagicOK)
    {
      #ifdef _USB_DIAGNOSIS_
        //textColor(SUCCESS);
        printf("\nCSW signature OK    ");
        //textColor(TEXT);
      #endif
    }
    else if (CSWsignature == CSWMagicNotOK)
    {
        printf("\nCSW signature wrong (not processed)");
        return -1;
    }
    else
    {
        printf("\nCSW signature wrong (processed, but wrong value)");
        error = -2;
    }

    // check matching tag
    uint32_t CSWtag = *(((uint32_t*)MSDStatus)+1); // DWORD 1 (byte 4:7)

    if ((BYTE1(CSWtag) == SCSIOpcode) && (BYTE2(CSWtag) == 0x42) && (BYTE3(CSWtag) == 0x42) && (BYTE4(CSWtag) == 0x42))
    {
      #ifdef _USB_DIAGNOSIS_
        //textColor(SUCCESS);
        printf("CSW tag %xh OK    ",BYTE1(CSWtag));
        //textColor(TEXT);
      #endif
    }
    else
    {
        printf("\nError: CSW tag wrong");
        error = -3;
    }

    // check CSWDataResidue
    uint32_t CSWDataResidue = *(((uint32_t*)MSDStatus)+2); // DWORD 2 (byte 8:11)
    if (CSWDataResidue == 0)
    {
      #ifdef _USB_DIAGNOSIS_
        //textColor(SUCCESS);
        printf("\tCSW data residue OK    ");
        //textColor(TEXT);
      #endif
    }
    else
    {
        //textColor(BROWN);
        printf("\nCSW data residue: %u", CSWDataResidue);
        //textColor(TEXT);
    }

    // check status byte // DWORD 3 (byte 12)
    uint8_t CSWstatusByte = *(((uint8_t*)MSDStatus)+12); // byte 12 (last byte of 13 bytes)

    switch (CSWstatusByte)
    {
        case 0x00:
          #ifdef _USB_DIAGNOSIS_
            //textColor(SUCCESS);
            printf("\tCSW status OK");
            //textColor(TEXT);
          #endif
            break;
        case 0x01:
            printf("\nCommand %xh failed.", SCSIOpcode);
            return -4;
        case 0x02:
            //textColor(ERROR);
            printf("\nPhase Error. ");
            //textColor(IMPORTANT);
            printf("Reset recovery is needed.");
            usb_resetRecoveryMSD(msd);
            //textColor(TEXT);
            return -5;
        default:
            printf("\nCSW status byte: undefined value (error)., %d\n", CSWstatusByte);
			for (;;);
			return -6;
    }

    return error;
}

/// cf. http://www.beyondlogic.org/usbnutshell/usb4.htm#Bulk
static bool usb_sendSCSICommand(usb_msd_t* msd, uint8_t SCSIcommand, uint32_t LBA, uint16_t TransferLength, void* dataBuffer, void* statusBuffer)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    printf("\nsendSCSICMD OUT part");
  #endif

	usb_transfer_t transfer;

	struct usb_CommandBlockWrapper cbw;
	formatSCSICommand(SCSIcommand, &cbw, LBA, TransferLength, msd);

	usb_constructTransfer(msd->interface->device, &transfer, USB_BULK, msd->endpointOutMSD);
	usb_outTransaction(&transfer, false, &cbw, 31);
	usb_scheduleTransfer(&transfer);
	usb_waitForTransfer(&transfer);
	usb_destructTransfer(&transfer);

    if (!transfer.success)
        return false;

  /**************************************************************************************************************************************/

  #ifdef _USB_FUNCTION_DIAGNOSIS_
    printf("\nsendSCSICMD IN part");
  #endif

    memset(statusBuffer, 0, 13);
 
    usb_constructTransfer(msd->interface->device, &transfer, USB_BULK, msd->endpointInMSD);
    if (TransferLength > 0)
    {
        usb_inTransaction(&transfer, false, dataBuffer, TransferLength);
    }
    
    usb_inTransaction(&transfer, false, statusBuffer, 13);
    usb_scheduleTransfer(&transfer);
    usb_waitForTransfer(&transfer);
    usb_destructTransfer(&transfer);
	if (!transfer.success)
	{
		return false;
	}
   
  #ifdef _USB_DIAGNOSIS_
    if (TransferLength) // byte
    {
        printf("\n");
        //memshow(dataBuffer, TransferLength, false);
		printf("\n");

        if ((TransferLength==512) || (TransferLength==36)) // data block (512 byte), inquiry feedback (36 byte)
        {
            //memshow(dataBuffer, TransferLength, true); // alphanumeric
			printf("\n");
        }
        if (TransferLength > 512)
        {
            printf("TransferLength %x\n", TransferLength);
        }
    }
  #endif
   
	if (checkSCSICommand(statusBuffer, msd, TransferLength, SCSIcommand) != 0)
	{
		return false;
	}
    
    return true;
}

static bool usb_sendSCSICommand_out(usb_msd_t* msd, uint8_t SCSIcommand, uint32_t LBA, uint16_t TransferLength, const void* dataBuffer, void* statusBuffer)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    printf("\nsendSCSICMDout OUT part");
  #endif

	usb_transfer_t transfer;

	struct usb_CommandBlockWrapper cbw;
	formatSCSICommand(SCSIcommand, &cbw, LBA, TransferLength, msd);

	usb_constructTransfer(msd->interface->device, &transfer, USB_BULK, msd->endpointOutMSD);
	usb_outTransaction(&transfer, false, &cbw, 31);
	usb_outTransaction(&transfer, false, dataBuffer, TransferLength);

	usb_scheduleTransfer(&transfer);
	usb_waitForTransfer(&transfer);
	usb_destructTransfer(&transfer);
	if (!transfer.success)
		return false;
    

  /**************************************************************************************************************************************/
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    printf("\nsend SCSICMDout IN part");
  #endif

    char tempStatusBuffer[13];
    if (statusBuffer == 0)
        statusBuffer = tempStatusBuffer;

    usb_constructTransfer(msd->interface->device, &transfer, USB_BULK, msd->endpointInMSD);
    usb_inTransaction(&transfer, false, statusBuffer, 13);
    usb_scheduleTransfer(&transfer);
    usb_waitForTransfer(&transfer);
    usb_destructTransfer(&transfer);
    return transfer.success;
}

static uint8_t testDeviceReady(usb_msd_t* msd)
{
    uint8_t maxTest = 50;

    bool UFI = msd->interface->descriptor.interfaceSubclass == 0x04;
    if (UFI)
        maxTest = 2;

    uint32_t timeout = maxTest;
    uint8_t statusByte = 0;
    while (timeout != 0)
    {
        timeout--;
        if (!UFI)
        {
          #ifdef _USB_FUNCTION_DIAGNOSIS_
            //textColor(LIGHT_BLUE);
            printf("\n\nSCSI: test unit ready");
           // textColor(TEXT);
          #endif

            bool success = usb_sendSCSICommand(msd, 0x00, 0, 0, g_pdataBuffer, statusBuffer); // dev, cmd, LBA, transfer length

            uint8_t statusByteTestReady = success ? statusBuffer[12] : 0xFF;

            if (timeout >= maxTest/2 && statusByteTestReady != 0)
            {
                //sleepMilliSeconds(50);
				Syscall_Sleep(50);
				continue;
            }
        }

      #ifdef _USB_FUNCTION_DIAGNOSIS_
        //textColor(LIGHT_BLUE);
        printf("\n\nSCSI: request sense");
        //textColor(TEXT);
      #endif
      
        memset(g_pdataBuffer, 0, 18);
        if (usb_sendSCSICommand(msd, 0x03, 0, 18, g_pdataBuffer, statusBuffer)) // dev, cmd, LBA, transfer length
        {
            if (!UFI)
                statusByte = statusBuffer[12];

            uint32_t sense = showResultsRequestSense(g_pdataBuffer);
            if (sense == 0 || sense == 6)
            {
                break;
            }
        }
       
        //sleepMilliSeconds(50);
		Syscall_Sleep(50);
    }
  #ifdef _USB_FUNCTION_DIAGNOSIS_
    //waitForKeyStroke();
  #endif

    return statusByte;
}

// http://en.wikipedia.org/wiki/SCSI_Inquiry_Command
static void analyzeInquiry(const void* addr)
{
    // cf. Jan Axelson, USB Mass Storage, page 140
    uint8_t PeripheralDeviceType = getField(addr, 0, 0, 5); // byte, shift, len
 // uint8_t PeripheralQualifier  = getField(addr, 0, 5, 3);
 // uint8_t DeviceTypeModifier   = getField(addr, 1, 0, 7);
    uint8_t RMB                  = getField(addr, 1, 7, 1);
  
    uint8_t ANSIapprovedVersion  = getField(addr, 2, 0, 3);
 // uint8_t ECMAversion          = getField(addr, 2, 3, 3);
 // uint8_t ISOversion           = getField(addr, 2, 6, 2);
    uint8_t ResponseDataFormat   = getField(addr, 3, 0, 4);
    uint8_t HISUP                = getField(addr, 3, 4, 1);
    uint8_t NORMACA              = getField(addr, 3, 5, 1);
 // uint8_t AdditionalLength     = getField(addr, 4, 0, 8);
    uint8_t CmdQue               = getField(addr, 7, 1, 1);
    uint8_t Linked               = getField(addr, 7, 3, 1);

    char vendorID[9] = { 0, };
    memcpy(vendorID, (char*)addr+8, 8);
    vendorID[8]=0;

    char productID[17];
    memcpy(productID, (char*)addr+16, 16);
    productID[16]=0;

    char productRevisionLevel[5];
    memcpy(productRevisionLevel, (char*)addr+32, 4);
    productRevisionLevel[4]=0;

    if(strlen(vendorID) == 0)
        printf("Vendor ID: Unknown\n");
    else
        printf("Vendor ID: %s\n", vendorID);
    
    printf("Product ID: %s\n", productID);
    printf("Revision:   %s\n", productRevisionLevel);

    // Book of Jan Axelson, "USB Mass Storage", page 140:
    //printf("\nVersion ANSI: %u  ECMA: %u  ISO: %u", ANSIapprovedVersion, ECMAversion, ISOversion);
    printf("Version: %d (4: SPC-2, 5: SPC-3)\n", (int)ANSIapprovedVersion);

    // Jan Axelson, USB Mass Storage, page 140
    if (ResponseDataFormat == 2)
    {
        printf("Response Data Format OK\n");
    }
    else
    {
        printf("Response Data Format is not OK: %u (should be 2)\n", ResponseDataFormat);
    }

    printf("Removable device type:            %s\n", RMB     ? "yes" : "no");
    printf("Supports hierarch. addr. support: %s\n", HISUP   ? "yes" : "no");
    printf("Supports normal ACA bit support:  %s\n", NORMACA ? "yes" : "no");
    printf("Supports linked commands:         %s\n", Linked  ? "yes" : "no");
    printf("Supports tagged command queuing:  %s\n", CmdQue  ? "yes" : "no");

    static const char* const PeripheralDeviceTypes[] =
    {
        "direct-access device (e.g., magnetic disk)",
        "sequential-access device (e.g., magnetic tape)",
        "printer device",
        "processor device",
        "write-once device",
        "CD/DVD device",
        "scanner device",
        "optical memory device (non-CD optical disk)",
        "medium Changer (e.g. jukeboxes)",
        "communications device",
        "defined by ASC IT8 (Graphic arts pre-press devices)",
        "defined by ASC IT8 (Graphic arts pre-press devices)",
        "Storage array controller device (e.g., RAID)",
        "Enclosure services device",
        "Simplified direct-access device (e.g., magnetic disk)",
        "Optical card reader/writer device",
        "Reserved for bridging expanders",
        "Object-based Storage Device",
        "Automation/Drive Interface",
        "Reserved",
        "Reserved",
        "Reduced block command (RBC) direct-access device",
        "Unknown or no device type"
    };
    printf("%s\n", PeripheralDeviceTypes[PeripheralDeviceType]);
}

static void testMSD(usb_msd_t* msd)
{
    // start with correct endpoint toggles and reset interface
    msd->endpointOutMSD->toggle = msd->endpointInMSD->toggle = false;

    bool UFI =  msd->interface->descriptor.interfaceSubclass == 0x04;
    if (!UFI)
        usb_bulkReset(msd); // Reset Interface

    // send SCSI command "inquiry (opcode: 0x12)"
    printf("SCSI: inquiry\n");

    if (usb_sendSCSICommand(msd, 0x12 /*SCSI opcode*/, 0 /*LBA*/, 36 /*Bytes In*/, g_pdataBuffer, statusBuffer))
       analyzeInquiry(g_pdataBuffer);
	else
	{
		printf("Inquiry failed.\n");
	}
    // send SCSI command "test unit ready(6)"
    testDeviceReady(msd);
   
    // send SCSI command "read capacity(10)"
    printf("\nSCSI: read capacity\n");
 
	if (usb_sendSCSICommand(msd, 0x25 /*SCSI opcode*/, 0 /*LBA*/, 8 /*Bytes In*/, g_pdataBuffer, statusBuffer))
    {
        // MSB ... LSB
        g_pdataBuffer[0] = htonl(g_pdataBuffer[0]);
        g_pdataBuffer[1] = htonl(g_pdataBuffer[1]);

        msd->disk.size = (((uint64_t)g_pdataBuffer[0]) + 1) * ((uint64_t)g_pdataBuffer[1]);
        msd->disk.sectorSize = g_pdataBuffer[1];

        printf("Last LBA: %d, block size: %u\n", g_pdataBuffer[0], msd->disk.sectorSize);
        deviceManager_analyzeDisk(&msd->disk);
       
    }
    else
        printf("Read Capacity failed.\n");

	printf("Test End\n");  
}

FS_ERROR usb_readSectors(uint32_t sector, void* buffer, uint32_t count, disk_t* device)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
   // textColor(LIGHT_BLUE);
    printf("\n\n>SCSI: read sector: %u", sector);
    //textColor(TEXT);
  #endif

    usb_msd_t* msd = (usb_msd_t*)device->data;
    memset(statusBuffer, 0, 13);
    if (usb_sendSCSICommand(msd, 0x28 /*SCSI opcode*/, sector /*LBA*/, count*device->sectorSize /*Bytes In*/, buffer, statusBuffer))
        return CE_GOOD;
    return CE_BAD_SECTOR_READ;
}

FS_ERROR usb_writeSectors(uint32_t sector, const void* buffer, uint32_t count, disk_t* device)
{
  #ifdef _USB_FUNCTION_DIAGNOSIS_
  //  textColor(IMPORTANT);
    printf("\n\n>>> SCSI: write sector: %u", sector);
  //  textColor(TEXT);
  #endif
    printf("SCSI: write sector: %d", sector);

    usb_msd_t* msd = (usb_msd_t *)device->data;

    if (usb_sendSCSICommand_out(msd, 0x2A /*SCSI opcode*/, sector /*LBA*/, count * device->sectorSize /*Bytes Out*/, buffer, statusBuffer))
        return CE_GOOD;
    return CE_WRITE_ERROR;
}

static void usb_resetRecoveryMSD(usb_msd_t* msd)
{
    // Reset Interface
    usb_bulkReset(msd);

    // TEST ////////////////////////////////////
    //usbSetFeatureHALT(device, device->numEndpointInMSD);
    //usbSetFeatureHALT(device, device->numEndpointOutMSD);

    // Clear Feature HALT to the Bulk-In  endpoint
    printf("\nGetStatus: %u", usb_getStatus(msd->interface->device, msd->endpointInMSD));
    usb_clearFeatureHALT(msd->interface->device, msd->endpointInMSD);
    printf("\nGetStatus: %u", usb_getStatus(msd->interface->device, msd->endpointInMSD));

    // Clear Feature HALT to the Bulk-Out endpoint
    printf("\nGetStatus: %u", usb_getStatus(msd->interface->device, msd->endpointOutMSD));
    usb_clearFeatureHALT(msd->interface->device, msd->endpointOutMSD);
    printf("\nGetStatus: %u", usb_getStatus(msd->interface->device, msd->endpointOutMSD));

    // set configuration to 1 and endpoint IN/OUT toggles to 0
    usb_setConfiguration(msd->interface->device, 1); // set first configuration
    uint8_t config = usb_getConfiguration(msd->interface->device);
    if (config != 1)
    {
        printf("\tconfiguration: %u (to be: 1)", config);
    }

    // start with correct endpoint toggles and reset interface
    msd->endpointOutMSD->toggle = false;
    msd->endpointInMSD->toggle = false;
    usb_bulkReset(msd); // Reset Interface
}

static uint32_t showResultsRequestSense(const void* addr)
{
    uint32_t Valid        = getField(addr, 0, 7, 1); // byte 0, bit 7
    uint32_t ResponseCode = getField(addr, 0, 0, 7); // byte 0, bit 6:0
    uint32_t SenseKey     = getField(addr, 2, 0, 4); // byte 2, bit 3:0

    printf("\nResults of \"request sense\":\n");
    if (ResponseCode >= 0x70 && ResponseCode <= 0x73)
    {
        printf("Valid: ");
        if (Valid == 0)
        {
            printf("Sense data are not SCSI compliant\n");
        }
        else
        {
            printf("Sense data are SCSI compliant\n");
        }
        printf("Response Code: ");
        switch (ResponseCode)
        {
            case 0x70:
                printf("Current errors, fixed format\n");
                break;
            case 0x71:
                printf("Deferred errors, fixed format\n");
                break;
            case 0x72:
                printf("Current error, descriptor format\n");
                break;
            case 0x73:
                printf("Deferred error, descriptor format\n");
                break;
            default:
                printf("No valid response code!\n");
                break;
        }

        static const char* const SenseKeys[] =
        {
            "No Sense",
            "Recovered Error - last command completed with some recovery action",
            "Not Ready - logical unit addressed cannot be accessed",
            "Medium Error - command terminated with a non-recovered error condition",
            "Hardware Error",
            "Illegal Request - illegal parameter in the command descriptor block",
            "Unit Attention - disc drive may have been reset.",
            "Data Protect - command read/write on a protected block",
            "Undefined",
            "Firmware Error",
            "Undefined",
            "Aborted Command - disc drive aborted the command",
            "Equal - SEARCH DATA command has satisfied an equal comparison",
            "Volume Overflow - buffered peripheral device has reached the end of medium partition",
            "Miscompare - source data did not match the data read from the medium",
            "Undefined"
        };
        printf("Sense Key: %s\n", SenseKeys[SenseKey]);
        return SenseKey;
    }

    printf("No vaild response code!\n");
    return 0xFFFFFFFF;
}
