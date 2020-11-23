#pragma once
#define MEGA_BYTES 1048576
#define KILO_BYTES 1024

#define PAGE_ALIGN_DOWN(value)				((value) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_UP(value)				(((value) & (PAGE_SIZE - 1)) ? (PAGE_ALIGN_DOWN((value)) + PAGE_SIZE) : (value))

enum
{
	eHandleDivideByZero = 0,
	eHandleSingleStepTrap = 1,
	eHandleNMITrap = 2,
	eHandleBreakPointTrap = 3,
	eHandleOverflowTrap = 4,
	eHandleBoundsCheckFault = 5,
	eHandleInvalidOpcodeFault = 6,
	eHandleNoDeviceFault = 7,
	eHandleDoubleFaultAbort = 8,
	ekHandleInvalidTSSFault = 10,
	eHandleSegmentFault = 11,
	eHandleStackFault = 12,
	eHandleGeneralProtectionFault = 13,
	eHandlePageFault = 14,
	eHandlefpu_fault = 16,
	eHandleAlignedCheckFault = 17,
	eHandleMachineCheckAbort = 18,
	eHandleSIMDFPUFault = 19,
};