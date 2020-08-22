#ifndef CMOS_H
#define CMOS_H

#include "windef.h"
#include "stdint.h"

#define CMOS_SECOND      0x00
#define CMOS_ALARMSECOND 0x01
#define CMOS_MINUTE      0x02
#define CMOS_ALARMMINUTE 0x03
#define CMOS_HOUR        0x04
#define CMOS_ALARMHOUR   0x05
#define CMOS_WEEKDAY     0x06
#define CMOS_DAYOFMONTH  0x07
#define CMOS_MONTH       0x08
#define CMOS_YEAR        0x09
#define CMOS_CENTURY     0x32 // BCD value

#define CMOS_STATUS_A    0x0A // read/write
#define CMOS_STATUS_B    0x0B // read/write
#define CMOS_STATUS_C    0x0C // read only
#define CMOS_STATUS_D    0x0D // read only
#define CMOS_STATUS_POST 0x0E // POST diagnosis
#define CMOS_STATUS_SD   0x0F // shutdown

#define CMOS_FLOPPYTYPE  0x10 // 2 nibbles: high=fd0, low=fd1 (4: 1.44 MB)
#define CMOS_HDDTYPE     0x12

#define CMOS_DEVICES     0x14 // 7-6: num of FDD,     5-4: display type,            3: display (y/n),
                              // 2:   keyboard (y/n), 1:   math co-processor (y/n), 0: FDD (y/n)

#define CMOS_CHECKSUM_HI 0x2E
#define CMOS_CHECKSUM_LO 0x2F

#define CMOS_NUM_CPU     0x5F // number of processors


uint8_t cmos_read(uint8_t offset); // Read byte from CMOS
void cmos_write(uint8_t offset, uint8_t val); // Write byte to CMOS


#endif
