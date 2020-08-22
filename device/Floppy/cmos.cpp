#include "cmos.h"
#include <minwindef.h>
#include <systemcall_impl.h>

#define BIT(n) (1U<<(n))
#define IS_BIT_SET(value, pos) ((value)&BIT(pos))

#define CMOS_ADDRESS  0x70
#define CMOS_DATA     0x71

#define BIT_6_TO_0    0x7F

static DWORD mutex = 0;

uint8_t cmos_read(uint8_t offset) // Read byte from CMOS
{
    Syscall_LockMutex(mutex);
    uint8_t tmp = Syscall_InPortByte(CMOS_ADDRESS) & BIT(7);
	Syscall_OutPortByte(CMOS_ADDRESS, (tmp | (offset & BIT_6_TO_0))); // do not change bit7 (NMI mask)
    uint8_t retVal = Syscall_InPortByte(CMOS_DATA);
    Syscall_UnlockMutex(mutex);
    return retVal;
}

void cmos_write(uint8_t offset, uint8_t val) // Write byte to CMOS
{
	Syscall_LockMutex(mutex);
    uint8_t tmp = Syscall_InPortByte(CMOS_ADDRESS) & BIT(7);
	Syscall_OutPortByte(CMOS_ADDRESS, (tmp | (offset & BIT_6_TO_0))); // do not change bit7 (NMI mask)
	Syscall_OutPortByte(CMOS_DATA, val);
	Syscall_UnlockMutex(mutex);
}
