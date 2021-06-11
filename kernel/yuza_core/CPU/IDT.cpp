#include "idt.h"
#include <stringdef.h>
#include "memory.h"

idt_descriptor* g_pIDT = 0;
extern void SendEOI();
//�ٷ�� �ִ� �ڵ鷯�� �������� ������ ȣ��Ǵ� �⺻ �ڵ鷯
__declspec(naked) void InterrputDefaultHandler () 
{
	
	//�������͸� �����ϰ� ���ͷ�Ʈ�� ����.
	_asm
	{
		PUSHAD
		PUSHFD
		CLI
	}

	_asm
	{
		POPFD
		POPAD
	}

	SendEOI();

	// �������͸� �����ϰ� ���� �����ϴ� ������ ���ư���.
	_asm
	{
		IRETD
	}
}

//���ͷ�Ʈ �ڵ鷯 ��ġ
bool InstallInterrputHandler(uint32_t i, uint16_t flags, uint16_t sel, I86_IRQ_HANDLER irq) {

	if (i>I86_MAX_INTERRUPTS)
		return false;

	if (!irq)
		return false;

	//���ͷ�Ʈ�� ���̽� �ּҸ� ���´�.
	uint64_t		uiBase = (uint64_t)&(*irq);
	
	if ((flags & 0x0500) == 0x0500) {
		g_pIDT[i].sel = sel;
		g_pIDT[i].flags = uint8_t(flags);
	}
	else
	{
		//���˿� �°� ���ͷ�Ʈ �ڵ鷯�� �÷��� ���� ��ũ���Ϳ� �����Ѵ�.
		g_pIDT[i].baseLo = uint16_t(uiBase & 0xffff);
		g_pIDT[i].baseHi = uint16_t((uiBase >> 16) & 0xffff);
		g_pIDT[i].reserved = 0;
		g_pIDT[i].flags = uint8_t(flags);
		g_pIDT[i].sel = sel;
	}

	return	true;
}


//���ͷ�Ʈ ���� ����
void SetInterruptVector(int intno, void(&vect) (), int flags) {

	//! install interrupt handler! This overwrites prev interrupt descriptor
	InstallInterrputHandler(intno, (uint16_t)(I86_IDT_DESC_PRESENT | I86_IDT_DESC_BIT32 | flags), 0x8, vect);
}