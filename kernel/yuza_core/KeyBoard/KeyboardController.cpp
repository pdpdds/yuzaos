#include "KeyboardController.h"
#include "SystemAPI.h"
#include "Hal.h"
#include "intrinsic.h"
#include "SkyConsole.h"
#include "platformapi.h"
#include <IDT.h>

extern void SendEOI();

//Ư��Ű ����
bool shift = false;	
bool ctrl = false;
bool alt = false;
bool caps = false;
bool num = false;
bool bExtensionKey = false;

unsigned char leds = 0; //LED ����ũ
const unsigned int KEYBUFFSIZE = 129;	//Ű ���� ������

Func_Key FKey[10] =		//���Ű�� �Լ��� �������Ѽ� Ư�� �Լ��� ������ �� �ְ� �Ѵ�.
{						
        {false, 0},
        {false, 0},
        {false, 0},
        {false, 0},
        {false, 0},
        {false, 0},
        {false, 0},
        {false, 0},
        {false, 0},
        {false, 0}
};

unsigned char normal[] = {					//Ű���� ĳ���� ��
	0x00,0x1B,'1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
	'q','w','e','r','t','y','u','i','o','p','[',']',0x0D,0x80,
	'a','s','d','f','g','h','j','k','l',';',047,0140,0x80,
	0134,'z','x','c','v','b','n','m',',','.','/',0x80,
	'*',0x80,' ',0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,'0',0177
};
//����ƮŰ�� ������ ���
unsigned char shifted[] = {
	0,033,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t',
	'Q','W','E','R','T','Y','U','I','O','P','{','}',015,0x80,
	'A','S','D','F','G','H','J','K','L',':',042,'~',0x80,
	'|','Z','X','C','V','B','N','M','<','>','?',0x80,
	'*',0x80,' ',0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,'7','8','9',0x80,'4','5','6',0x80,
	'1','2','3','0',177
};

//Caps LockŰ�� Ȱ��ȭ�� ���
unsigned char capsNormal[] = {
	0x00,0x1B,'1','2','3','4','5','6','7','8','9','0','-','=','\b','\t',
	'Q','W','E','R','T','Y','U','I','O','P','[',']',0x0D,0x80,
	'A','S','D','F','G','H','J','K','L',';',047,0140,0x80,
	'|','Z','X','C','V','B','N','M',',','.','/',0x80,
	'*',0x80,' ',0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,'0',0177
};

//����ƮŰ Caps LockŰ �Ѵ� �������
unsigned char capsShifted[] = {
	0,033,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t',
	'q','w','e','r','t','y','u','i','o','p','{','}',015,0x80,
	'a','s','d','f','g','h','j','k','l',':',042,'~',0x80,
	0134,'z','x','c','v','b','n','m','<','>','?',0x80,
	'*',0x80,' ',0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x80,'7','8','9',0x80,'4','5','6',0x80,
	'1','2','3','0',177
};

unsigned char buffer[KEYBUFFSIZE];	//Ű ����
int  buffend = 0;		//���ۿ� ����� ������ Ű���� ����Ų��.
unsigned char scanCode;	//Ű����κ��� ���� ��ĵ�ڵ尪


KeyboardController::KeyboardController()
{
}

KeyboardController::~KeyboardController()
{
}

void KeyboardController::UpdateLeds(unsigned char led)	//Ű���� LED�� ������Ʈ�Ѵ�.
{
	if(led == 0)
	{
		leds = 0;
	}
	else
	{
		if (leds == (leds|led))	//LED�� ���� �ִٸ�
		{
			leds = leds^led;	//LED�� ����
		}
		else
		{
			leds = leds | led;	// LED�� �Ҵ�
		}
	}

	//Ŀ�ǵ� ����Ʈ�� ������ ����� ����������
	//������ ����
	OutPortByte(0x64, 0xED);
	while ((InPortByte(0x64) % 2) == 2)
		;
	//LED ���¸� ������Ʈ �Ѵ�.
	OutPortByte(0x60, leds);
	
}

	
//Ű���� ���ͷ�Ʈ �ڵ鷯
__declspec(naked) void KeyboardHandler()
{
	//�������͸� �����ϰ� ���ͷ�Ʈ�� ����.
	_asm
	{
		PUSHAD
		PUSHFD
		CLI
	}

	// ���û��°� ����Ǵ� ���� ���� ���� �Լ��� ȣ���Ѵ�. 
	_asm call KeyboardController::HandleKeyboardInterrupt

	SendEOI();

	// �������͸� �����ϰ� ���� �����ϴ� ������ ���ư���.
	_asm
	{
		POPFD
		POPAD
		IRETD
	}
}

int KeyboardController::SpecialKey(unsigned char key)
{
	static bool specKeyUp = true;	
	switch (key)
	{
	case 0x36: //R-Shift down
	case 0x2A: //L-Shift down
		shift = true;
		break;
	case 0xB6: //R-Shift up
	case 0xAA: //L-Shift up
		shift = false;
		break;
	case 0x1D: //Control down
		ctrl = true;
		break;
	case 0x9D: //Control up
		ctrl = false;
		break;
	case 0x38: //Alt down
		alt = true;
		break;
	case 0xB8: //Alt up
		alt = false;
		break;
	case 0x3A: //Caps down
		if (specKeyUp == true)
		{
			caps = !caps;
			UpdateLeds(CapsLock);
			specKeyUp = false;
		}
		break;
	case 0x45: //Num down
		if (specKeyUp == true)
		{
			num = !num;
			UpdateLeds(NumLock);
			specKeyUp = false;
		}
		break;
	case 0x46: //Scroll down
		if (specKeyUp == true)
		{
			num = !num;
			UpdateLeds(ScrollLock);
			specKeyUp = false;
		}
		break;
	case 0x3B: //F1 Down
		if (specKeyUp && FKey[0].enabled)
		{
			FKey[0].func();
			specKeyUp = false;
		}
		break;
	case 0x3C: //F2 Down
		if (specKeyUp && FKey[1].enabled)
		{
			FKey[1].func();
			specKeyUp = false;
		}
		break;
	case 0x3D: //F3 Down
		if (specKeyUp && FKey[2].enabled)
		{
			FKey[2].func();
			specKeyUp = false;
		}
		break;
	case 0x3E: //F4 Down
		if (specKeyUp && FKey[3].enabled)
		{
			FKey[3].func();
			specKeyUp = false;
		}
		break;
	case 0x3F: //F5 Down
		if (specKeyUp && FKey[4].enabled)
		{
			FKey[4].func();
			specKeyUp = false;
		}
		break;
	case 0x40: //F6 Down
		if (specKeyUp && FKey[5].enabled)
		{
			FKey[5].func();
			specKeyUp = false;
		}
		break;
	case 0x41: //F7 Down
		if (specKeyUp && FKey[6].enabled)
		{
			FKey[6].func();
			specKeyUp = false;
		}
		break;
	case 0x42: //F8 Down
		if (specKeyUp && FKey[7].enabled)
		{
			FKey[7].func();
			specKeyUp = false;
		}
		break;
	case 0x43: //F9 Down
		if (specKeyUp && FKey[8].enabled)
		{
			FKey[8].func();
			specKeyUp = false;
		}
		break;
	case 0x44: //F10 Down
		if (specKeyUp && FKey[9].enabled)
		{
			FKey[9].func();
			specKeyUp = false;
		}
		break;
	case 0xBA: //Caps Up
	case 0xBB: //F1 Up
	case 0xBC: //F2 Up
	case 0xBD: //F3 Up
	case 0xBE: //F4 Up
	case 0xBF: //F5 Up
	case 0xC0: //F6 Up
	case 0xC1: //F7 Up
	case 0xC2: //F8 Up
	case 0xC3: //F9 Up
	case 0xC4: //F10 Up
	case 0xC5: //Num Up
	case 0xC6: //Scroll Up
		specKeyUp = true;
		break;
	case 0xE0:
		break;
	default:
		return(0);
	}
	return (1);
}

void KeyboardController::FlushBuffers()
{
	unsigned char c = 0;
	while ((c = InPortByte(0x60)) != InPortByte(0x60))
		;
}

void KeyboardController::SetupInterrupts()
{
	FlushBuffers();
	SetInterruptVector(33, KeyboardHandler);
}

void KeyboardController::SetLEDs(bool scroll, bool number, bool capslk)
{
	//Bit 1 : ��ũ�� �� LED�� �Ҵ�.
	unsigned char status = scroll ? 1 : 0;

	if (number)	//Bit 2 : Num Lock
		status |= 2;
	if (capslk)//Bit 3:	Caps Lock
		status |= 4;
	//Ŀ�ǵ� ���۰� ������� ���� ����Ѵ�.
	while ((InPortByte(0x64) & 2) == 2)
		;
	//Ŀ�ǵ带 ������ ����� �ö����� ���
	OutPortByte(0x64, 0xED);
	while ((InPortByte(0x64) % 2) == 2)
		;
	//LED ���� ������Ʈ�� ��û�Ѵ�.
	OutPortByte(0x60, status);
}

char KeyboardController::GetInput()		//Ű���� �����͸� �ܺο� �ֱ����� �����Ǵ� �޼ҵ�
{
	int i = 0;
	while (buffend == 0) //Ű���� �����Ͱ� ���ö����� ����Ѵ�.
	{		
		kSleep(1);
	}

	int flag = DisableInterrupts();

	for (; i < buffend; i++)
	{
		buffer[i] = buffer[i + 1];
	}
	buffend--;

	RestoreInterrupts(flag);

	return buffer[0];
}

void KeyboardController::HandleKeyboardInterrupt()
{
	unsigned char asciiCode;

	scanCode = InPortByte(0x60);	//Ű ��ĵ�ڵ带 ��´�.

	if (bExtensionKey == true)
	{
		bExtensionKey = false;

		// ������ �Ǵ� ������ ���� ����
		if ((scanCode & 0x80) != 0)
		{
			return;
		}

		//Ű���ۿ� �ƽ�Ű���� ����Ѵ�.
		if (buffend != (KEYBUFFSIZE - 1))
		{
			buffend++;
		}
		buffer[buffend] = scanCode & 0x7F;

	}

	if (!(SpecialKey(scanCode) || (scanCode >= 0x80))) //�ƽ�Ű�ڵ���
	{
		scanCode = scanCode & 0x7F;
		if (shift)		//����ƮŰ�� Caps Lock ���¿� ���� ������ �ƽ�Ű���� ���´�.
		{
			if (!caps)
			{
				asciiCode = shifted[scanCode];
			}
			else
			{
				asciiCode = capsShifted[scanCode];
			}
		}
		else
		{
			if (!caps)
			{
				asciiCode = normal[scanCode];
			}
			else
			{
				asciiCode = capsNormal[scanCode];
			}
		}

		//Ű���ۿ� �ƽ�Ű���� ����Ѵ�.
		if (buffend != (KEYBUFFSIZE - 1))
		{
			buffend++;
		}
		buffer[buffend] = asciiCode;
	}
	else if (scanCode == 0x38) //LEFT ALT
	{
		//Ű���ۿ� �ƽ�Ű���� ����Ѵ�.
		if (buffend != (KEYBUFFSIZE - 1))
		{
			buffend++;
		}
		buffer[buffend] = 0x85;
	}
	else if (scanCode == 0xE0)
	{
		bExtensionKey = true;
		//Ű���ۿ� �ƽ�Ű���� ����Ѵ�.
		if (buffend != (KEYBUFFSIZE - 1))
		{
			buffend++;
		}
		buffer[buffend] = scanCode;
	}
}