#pragma once
//�ָܼ��� �������� �׷��� ���� �������� ����
//�� ���� 1�� �����Ǹ� �Ʒ� ������ ��� ���õȴ�.
#define SKY_CONSOLE_MODE	0
//MINT64 32��Ʈ
//WindowsSys 24��Ʈ RGB24
//Haribote	8��Ʈ. WIN32���� SDL�� 8��Ʈ ó���� �ȵ� ���� �ִ�.
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