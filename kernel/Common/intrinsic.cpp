#include "intrinsic.h"
#include <stringdef.h>


extern "C" int _outp(unsigned short, int);
extern "C" unsigned long _outpd(unsigned int, int);
extern "C" unsigned short _outpw(unsigned short, unsigned short);
extern "C" int _inp(unsigned short);
extern "C" unsigned short _inpw(unsigned short);
extern "C" unsigned long _inpd(unsigned int shor);

extern "C" void kLock();
extern "C" void kUnlock();

#define DMA_PICU1       0x0020
#define DMA_PICU2       0x00A0

#ifdef OUT
#undef OUT
#endif

__declspec(naked) void SendEOI()
{
	_asm
	{
		PUSH EBP
		MOV  EBP, ESP
		PUSH EAX

		; [EBP] < -EBP
		; [EBP + 4] < -RET Addr
		; [EBP + 8] < -IRQ 번호

		MOV AL, 20H; EOI 신호를 보낸다.
		OUT DMA_PICU1, AL

		CMP BYTE PTR[EBP + 8], 7
		JBE END_OF_EOI
		OUT DMA_PICU2, AL; Send to 2 also

		END_OF_EOI :
		POP EAX
			POP EBP
			RET
	}
}

#ifdef __cplusplus
extern "C" {
#endif
	void OutPortByte(ushort port, uchar value)
	{
		_outp(port, value);
	}

	void OutPortWord(ushort port, ushort value)
	{
		_outpw(port, value);
	}

	void OutPortDWord(ushort port, unsigned int value)
	{
		_outpd(port, value);
	}

	long InPortDWord(unsigned int port)
	{
		return _inpd(port);
	}

	uchar InPortByte(ushort port)
	{

		return (uchar)_inp(port);
	}

	ushort InPortWord(ushort port)
	{
		return _inpw(port);
	}

	void kEnterCriticalSection(void* lpCriticalSection);
	void kLeaveCriticalSection(void* lpCriticalSection);

	int DisableInterrupts()
	{
		int fl = 0;
#if SKY_EMULATOR
		kLock();
#else
		__asm	PUSHFD
		__asm	POP fl
		__asm	CLI
#endif
		return fl;
	}

	void RestoreInterrupts(const int flags)
	{
#if SKY_EMULATOR
		kUnlock();
#else
		__asm	PUSH	flags
		__asm	POPFD
		//asm volatile("pushl %0; popfl\n" : : "g" (flags));
#endif
	}

	void EnableInterrupts()
	{
		__asm sti
	}

	const int kMasterIcw1 = 0x20;
	const int kMasterIcw2 = 0x21;
	const int kSlaveIcw1 = 0xa0;
	const int kSlaveIcw2 = 0xa1;
	const int kUserCs = 0x1b;

	void EnableIrq(int irq)
	{
		if (irq < 8)
			OutPortByte(kMasterIcw2, InPortByte(kMasterIcw2) & static_cast<unsigned char>(~(1 << irq)));
		else
			OutPortByte(kSlaveIcw2, InPortByte(kSlaveIcw2) & static_cast<unsigned char>(~(1 << (irq - 8))));
	}

	void DisableIrq(int irq)
	{
		if (irq < 8)
			OutPortByte(kMasterIcw2, InPortByte(kMasterIcw2) | (1 << irq));
		else
			OutPortByte(kSlaveIcw2, InPortByte(kSlaveIcw2) | (1 << (irq - 8)));
	}
#ifdef __cplusplus
}
#endif

#define OUT


