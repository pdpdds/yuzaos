#include "idt.h"
#include <stringdef.h>
#include "memory.h"

//���ͷ�Ʈ ��ũ���� ���̺�
static idt_descriptor	_IDT[I86_MAX_INTERRUPTS];

//CPU�� IDTR �������͸� �ʱ�ȭ�ϴµ� ������ �ִ� IDTR ����ü
static idtr				_IDTR;


//IDTR �������Ϳ� IDT�� �ּҰ��� �ִ´�.
static void IDTInstall() 
{
	_asm lidt [_IDTR]
}

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

extern bool InitIDT(uint16_t codeSel);


//i��° ���ͷ�Ʈ ��ũ��Ʈ�� ���´�.
idt_descriptor* GetInterruptDescriptor(uint32_t i) {

	if (i>I86_MAX_INTERRUPTS)
		return 0;

	return &_IDT[i];
}

//���ͷ�Ʈ �ڵ鷯 ��ġ
bool InstallInterrputHandler(uint32_t i, uint16_t flags, uint16_t sel, I86_IRQ_HANDLER irq) {

	if (i>I86_MAX_INTERRUPTS)
		return false;

	if (!irq)
		return false;

	//���ͷ�Ʈ�� ���̽� �ּҸ� ���´�.
	uint64_t		uiBase = (uint64_t)&(*irq);

	idt_descriptor* pIDT = GetInterruptDescriptor(i);
	
	if ((flags & 0x0500) == 0x0500) {
		pIDT->sel = sel;
		pIDT->flags = uint8_t(flags);
	}
	else
	{
		//���˿� �°� ���ͷ�Ʈ �ڵ鷯�� �÷��� ���� ��ũ���Ϳ� �����Ѵ�.
		pIDT->baseLo = uint16_t(uiBase & 0xffff);
		pIDT->baseHi = uint16_t((uiBase >> 16) & 0xffff);
		pIDT->reserved = 0;
		pIDT->flags = uint8_t(flags);
		pIDT->sel = sel;
	}

	return	true;
}

//IDT�� �ʱ�ȭ�ϰ� ����Ʈ �ڵ鷯�� ����Ѵ�
bool InitIDT(uint16_t codeSel) 
{

	//IDTR �������Ϳ� �ε�� ����ü �ʱ�ȭ
	_IDTR.limit = sizeof(idt_descriptor) * I86_MAX_INTERRUPTS - 1;
	_IDTR.base = (uint32_t)&_IDT[0];

	//NULL ��ũ����
	memset((void*)&_IDT[0], 0, sizeof(idt_descriptor) * I86_MAX_INTERRUPTS - 1);

	//����Ʈ �ڵ鷯 ���
	for (int i = 0; i<I86_MAX_INTERRUPTS; i++)
		InstallInterrputHandler(i, I86_IDT_DESC_PRESENT | I86_IDT_DESC_BIT32,
			codeSel, (I86_IRQ_HANDLER)InterrputDefaultHandler);

	//IDTR �������͸� �¾��Ѵ�
	IDTInstall();

	return true;
}

//���ͷ�Ʈ ���� ����
void SetInterruptVector(int intno, void(&vect) (), int flags) {

	//! install interrupt handler! This overwrites prev interrupt descriptor
	InstallInterrputHandler(intno, (uint16_t)(I86_IDT_DESC_PRESENT | I86_IDT_DESC_BIT32 | flags), 0x8, vect);
}