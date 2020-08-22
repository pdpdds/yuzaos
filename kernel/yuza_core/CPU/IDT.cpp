#include "idt.h"
#include <stringdef.h>
#include "memory.h"

idt_descriptor* g_pIDT = 0;
extern void SendEOI();
//다룰수 있는 핸들러가 존재하지 않을때 호출되는 기본 핸들러
__declspec(naked) void InterrputDefaultHandler () 
{
	
	//레지스터를 저장하고 인터럽트를 끈다.
	_asm
	{
		PUSHAD
		PUSHFD
		CLI
	}

	SendEOI();

	// 레지스터를 복원하고 원래 수행하던 곳으로 돌아간다.
	_asm
	{
		POPFD
		POPAD
		IRETD
	}
}

//인터럽트 핸들러 설치
bool InstallInterrputHandler(uint32_t i, uint16_t flags, uint16_t sel, I86_IRQ_HANDLER irq) {

	if (i>I86_MAX_INTERRUPTS)
		return false;

	if (!irq)
		return false;

	//인터럽트의 베이스 주소를 얻어온다.
	uint64_t		uiBase = (uint64_t)&(*irq);
	
	if ((flags & 0x0500) == 0x0500) {
		g_pIDT[i].sel = sel;
		g_pIDT[i].flags = uint8_t(flags);
	}
	else
	{
		//포맷에 맞게 인터럽트 핸들러와 플래그 값을 디스크립터에 세팅한다.
		g_pIDT[i].baseLo = uint16_t(uiBase & 0xffff);
		g_pIDT[i].baseHi = uint16_t((uiBase >> 16) & 0xffff);
		g_pIDT[i].reserved = 0;
		g_pIDT[i].flags = uint8_t(flags);
		g_pIDT[i].sel = sel;
	}

	return	true;
}


//인터럽트 벡터 설정
void SetInterruptVector(int intno, void(&vect) (), int flags) {

	//! install interrupt handler! This overwrites prev interrupt descriptor
	InstallInterrputHandler(intno, (uint16_t)(I86_IDT_DESC_PRESENT | I86_IDT_DESC_BIT32 | flags), 0x8, vect);
}