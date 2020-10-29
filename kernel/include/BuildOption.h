#pragma once
//콘솔모드로 시작할지 그래픽 모드로 시작할지 결정
//이값이 1로 설정되면 아래 값들은 모두 무시된다.
#define SKY_CONSOLE_MODE	0 
//MINT64 32비트
//WindowsSys 24비트 RGB24
//Haribote	8비트. WIN32하의 SDL은 8비트 처리가 안될 수도 있다.
#define SKY_WIDTH		1024
#define SKY_HEIGHT		768
#define SKY_BPP			32

#define KERNEL32_NAME		"yuza.exe"
#define KERNEL64_NAME		"yuza64.exe"
#define GRUB_095            "GNU GRUB 0.95"

#define SKY_EMULATOR 1 

#if SKY_EMULATOR
#define SKY_EMULATOR_DLL 1
#endif