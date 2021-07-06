#include <BuildOption.h>
#include "MultbootUtil.h"
#include <SkyConsole.h>
#include <memory.h>
#include <string.h>
#include <Kernel32.h>
#include <Kernel64.h>

extern "C" void kmain(unsigned long magic, unsigned long addr);
extern "C" void InitializeConstructors();

_declspec(naked) void multiboot_entry(void)
{
	__asm {
		align MULTIBOOT_HEADER_ALIGN

		multiboot_header:
		//멀티부트 헤더 사이즈 : 0X30
		dd(MULTIBOOT_HEADER_MAGIC); //멀티부트 헤더 매직값

#if SKY_CONSOLE_MODE == 0
		dd(MULTIBOOT_HEADER_FLAGS_GUI); //플래그
		dd(CHECKSUM_GUI); checksum
#else
		dd(MULTIBOOT_HEADER_FLAGS); //플래그
		dd(CHECKSUM); //체크섬
#endif		
		dd(HEADER_ADRESS); //헤더 주소 KERNEL_LOAD_ADDRESS+ALIGN(0x100400)
		dd(GRUB_KERNEL_LOAD_ADDRESS); //커널이 로드된 가상주소 공간
		dd(00); //사용되지 않음
		dd(00); //사용되지 않음
		dd(HEADER_ADRESS + 0x30); //커널 시작 주소 : 멀티부트 헤더 주소 + 0x30, kernel_entry

		dd(SKY_CONSOLE_MODE);
		dd(SKY_WIDTH);
		dd(SKY_HEIGHT);
		dd(SKY_BPP)

		kernel_entry:
		MOV     ESP, 0x40000; //스택 설정

		PUSH    0; //플래그 레지스터 초기화
		POPF

		//GRUB에 의해 담겨 있는 정보값을 스택에 푸쉬한다.
		PUSH    EBX; //멀티부트 구조체 포인터
		PUSH    EAX; //부트로더 매직값

		//위의 두 파라메터와 함께 kmain 함수를 호출한다.
		CALL    kmain; //C++ 메인 함수 호출

	//루프를 돈다. kmain이 리턴되지 않으면 아래 코드는 수행되지 않는다.
	halt:
		jmp halt;
	}
}

int sum(int a, int b)
{
	return a + b;
}

void SampleFillRect(ULONG* lfb0, int x, int y, int w, int h, int col)
{
	for (int k = 0; k < h; k++)
		for (int j = 0; j < w; j++)
		{
			int index = ((j + x) + (k + y) * 1024);
			lfb0[index] = col;
		}
}

extern "C" void kmain(unsigned long magic, unsigned long addr)
{
	multiboot_info_t* mb_info = (multiboot_info_t*)addr;
	
	InitializeConstructors(); 
	SkyConsole::Initialize();	
	SkyConsole::Print("32Bit Kernel Loader Entered..\n");
	
	int result = sum(5, 4);
	
	SkyConsole::Print("Kernel Name : %s, BootLoader : %s\n", mb_info->cmdline, mb_info->boot_loader_name);
	// 부트로더 이름이 "GNU GRUB 0.95" 이라면 커널이름 앞에 /을 붙인다. 
	if (0 == strcmp(mb_info->boot_loader_name, GRUB_095))
	{
		strcpy(mb_info->cmdline, "/yuza.exe");
	}
	 
	if (IsKernel64(mb_info, mb_info->cmdline))
		Boot64BitMode(mb_info, mb_info->cmdline);
	else
		Boot32BitMode(magic, mb_info, mb_info->cmdline);

	//not reached
	/*int trickCode = 0;
	if (trickCode)
		multiboot_entry();*/

	for (;;);
}