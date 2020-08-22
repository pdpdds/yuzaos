#ifndef FLPYDSK_H
#define FLPYDSK_H

#include "minwindef.h"
#include <stdio.h>
#include <stdint.h>
#include "../USB/devicemanager.h"

typedef struct
{
    port_t   drive;
    DWORD  RW_Lock;
    uint32_t accessRemaining;
    uint8_t  ID;
    bool     motor;
} floppy_t;


void flpydsk_install(void);
void flpydsk_motorOn (port_t* port);
void flpydsk_motorOff(port_t* port);
void flpydsk_refreshVolumeName(disk_t* disk);
FS_ERROR flpydsk_readSectors(uint32_t sector, void* buffer, uint32_t count, disk_t* device);
FS_ERROR flpydsk_writeSectors(uint32_t sector, const void* buffer, uint32_t count, disk_t* device);


#endif
