#pragma once
#include "stdint.h"
#include "usb.h"

#pragma pack(push, 1)
struct usb_CommandBlockWrapper
{
    uint32_t  CBWSignature;
    uint32_t  CBWTag;
    uint32_t  CBWDataTransferLength;
    uint8_t   CBWFlags;
    uint8_t   CBWLUN;           // only bits 3:0
    uint8_t   CBWCBLength;      // only bits 4:0
    uint8_t   commandByte[16];
} ;

struct usb_CommandBlockWrapperUFI
{
    uint8_t   commandByte[12];
} ;

typedef struct
{
    disk_t           disk;
    usb_interface_t* interface;

    usb_endpoint_t* endpointInMSD;
    usb_endpoint_t* endpointOutMSD;
} usb_msd_t;

#pragma pack(pop)

void     usb_setupMSD(usb_interface_t* interface);
void     usb_destroyMSD(usb_interface_t* interface);

FS_ERROR usb_readSectors (uint32_t sector, void* buffer, uint32_t count, disk_t* device);
FS_ERROR usb_writeSectors(uint32_t sector, const void* buffer, uint32_t count, disk_t* device);