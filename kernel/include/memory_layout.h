#pragma once

#define IO_AREA_PAGE_COUNT 2000

const unsigned int kKernelBase = 0x80000000;
#if !SKY_EMULATOR
const unsigned int kHeapBase = 0xc0000000;
const unsigned int kIOAreaBase = 0xe4000000;
#endif
const unsigned int kIOAreaTop = 0xe7ffffff;
const unsigned int kKernelStackTop = 0xffff0000;
const unsigned int kKernelTop = 0xffffffff;
const unsigned int kKernelStackSize = 0x040000 * 4;

const unsigned int kUserStackBase = 0x7ff0000;
const unsigned int kUserStackTop = 0x8000000;
const unsigned int kUserBase = 0x08000000;
const unsigned int kUserHeapBase = 0x60000000;
const unsigned int kUserHeapTop = 0x70000000;
const unsigned int kUserTop = 0x80000000;
const unsigned int kUserStackSize = 0x40000;

const unsigned int kAddressSpaceTop = 0xffffffff;
