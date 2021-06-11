#include "orangeos.h"
#include <memory.h>
#include <intrinsic.h>
#include "SkyInputHandler.h"
#include <Hal.h>
#include <IDT.h>
#include "SkyGUISystem.h"

SkyInputHandler* SkyInputHandler::m_inputHandler = nullptr;
extern void SendEOI();

void SkyOSMouseHandler()
{
	SkyInputHandler::GetInstance()->ProcessMouseInput();
}

void SkyOSKeyboardHandler()
{
	SkyInputHandler::GetInstance()->ProcessKeyboardInput();
}

__declspec(naked) void kSkyOSMouseHandler()
{

	_asm {
		PUSHAD
		PUSHFD
		CLI
	}

	_asm
	{
		call SkyOSMouseHandler
		POPFD
		POPAD
	}

	SendEOI();

	_asm
	{
		IRETD
	}
}

__declspec(naked) void kSkyOSKeyboardHandler()
{

	_asm {
		PUSHAD
		PUSHFD
		CLI
	}

	_asm
	{
		call SkyOSKeyboardHandler
		POPFD
		POPAD
	}

	SendEOI();

	_asm
	{
		IRETD
	}
}


// ��ĵ �ڵ带 ASCII �ڵ�� ��ȯ�ϴ� ���̺�
static KEYMAPPINGENTRY gs_vstKeyMappingTable[KEY_MAPPINGTABLEMAXCOUNT] =
{
	/*  0   */{ KEY_NONE        ,   KEY_NONE },
	/*  1   */{ KEY_ESC         ,   KEY_ESC },
	/*  2   */{ '1'             ,   '!' },
	/*  3   */{ '2'             ,   '@' },
	/*  4   */{ '3'             ,   '#' },
	/*  5   */{ '4'             ,   '$' },
	/*  6   */{ '5'             ,   '%' },
	/*  7   */{ '6'             ,   '^' },
	/*  8   */{ '7'             ,   '&' },
	/*  9   */{ '8'             ,   '*' },
	/*  10  */{ '9'             ,   '(' },
	/*  11  */{ '0'             ,   ')' },
	/*  12  */{ '-'             ,   '_' },
	/*  13  */{ '='             ,   '+' },
	/*  14  */{ KEY_BACKSPACE   ,   KEY_BACKSPACE },
	/*  15  */{ KEY_TAB         ,   KEY_TAB },
	/*  16  */{ 'q'             ,   'Q' },
	/*  17  */{ 'w'             ,   'W' },
	/*  18  */{ 'e'             ,   'E' },
	/*  19  */{ 'r'             ,   'R' },
	/*  20  */{ 't'             ,   'T' },
	/*  21  */{ 'y'             ,   'Y' },
	/*  22  */{ 'u'             ,   'U' },
	/*  23  */{ 'i'             ,   'I' },
	/*  24  */{ 'o'             ,   'O' },
	/*  25  */{ 'p'             ,   'P' },
	/*  26  */{ '['             ,   '{' },
	/*  27  */{ ']'             ,   '}' },
	/*  28  */{ '\n'            ,   '\n' },
	/*  29  */{ KEY_CTRL        ,   KEY_CTRL },
	/*  30  */{ 'a'             ,   'A' },
	/*  31  */{ 's'             ,   'S' },
	/*  32  */{ 'd'             ,   'D' },
	/*  33  */{ 'f'             ,   'F' },
	/*  34  */{ 'g'             ,   'G' },
	/*  35  */{ 'h'             ,   'H' },
	/*  36  */{ 'j'             ,   'J' },
	/*  37  */{ 'k'             ,   'K' },
	/*  38  */{ 'l'             ,   'L' },
	/*  39  */{ ';'             ,   ':' },
	/*  40  */{ '\''            ,   '\"' },
	/*  41  */{ '`'             ,   '~' },
	/*  42  */{ KEY_LSHIFT      ,   KEY_LSHIFT },
	/*  43  */{ '\\'            ,   '|' },
	/*  44  */{ 'z'             ,   'Z' },
	/*  45  */{ 'x'             ,   'X' },
	/*  46  */{ 'c'             ,   'C' },
	/*  47  */{ 'v'             ,   'V' },
	/*  48  */{ 'b'             ,   'B' },
	/*  49  */{ 'n'             ,   'N' },
	/*  50  */{ 'm'             ,   'M' },
	/*  51  */{ ','             ,   '<' },
	/*  52  */{ '.'             ,   '>' },
	/*  53  */{ '/'             ,   '?' },
	/*  54  */{ KEY_RSHIFT      ,   KEY_RSHIFT },
	/*  55  */{ '*'             ,   '*' },
	/*  56  */{ KEY_LALT        ,   KEY_LALT },
	/*  57  */{ ' '             ,   ' ' },
	/*  58  */{ KEY_CAPSLOCK    ,   KEY_CAPSLOCK },
	/*  59  */{ KEY_F1          ,   KEY_F1 },
	/*  60  */{ KEY_F2          ,   KEY_F2 },
	/*  61  */{ KEY_F3          ,   KEY_F3 },
	/*  62  */{ KEY_F4          ,   KEY_F4 },
	/*  63  */{ KEY_F5          ,   KEY_F5 },
	/*  64  */{ KEY_F6          ,   KEY_F6 },
	/*  65  */{ KEY_F7          ,   KEY_F7 },
	/*  66  */{ KEY_F8          ,   KEY_F8 },
	/*  67  */{ KEY_F9          ,   KEY_F9 },
	/*  68  */{ KEY_F10         ,   KEY_F10 },
	/*  69  */{ KEY_NUMLOCK     ,   KEY_NUMLOCK },
	/*  70  */{ KEY_SCROLLLOCK  ,   KEY_SCROLLLOCK },

	/*  71  */{ KEY_HOME        ,   '7' },
	/*  72  */{ KEY_UP          ,   '8' },
	/*  73  */{ KEY_PAGEUP      ,   '9' },
	/*  74  */{ '-'             ,   '-' },
	/*  75  */{ KEY_LEFT        ,   '4' },
	/*  76  */{ KEY_CENTER      ,   '5' },
	/*  77  */{ KEY_RIGHT       ,   '6' },
	/*  78  */{ '+'             ,   '+' },
	/*  79  */{ KEY_END         ,   '1' },
	/*  80  */{ KEY_DOWN        ,   '2' },
	/*  81  */{ KEY_PAGEDOWN    ,   '3' },
	/*  82  */{ KEY_INS         ,   '0' },
	/*  83  */{ KEY_DEL         ,   '.' },
	/*  84  */{ KEY_NONE        ,   KEY_NONE },
	/*  85  */{ KEY_NONE        ,   KEY_NONE },
	/*  86  */{ KEY_NONE        ,   KEY_NONE },
	/*  87  */{ KEY_F11         ,   KEY_F11 },
	/*  88  */{ KEY_F12         ,   KEY_F12 }
};


SkyInputHandler::SkyInputHandler()
{
	memset(&m_keyboardState, 0, sizeof(KEYBOARDSTATE));
	memset(&m_mouseState, 0, sizeof(MOUSESTATE));				
	m_owner = nullptr;
}

SkyInputHandler::~SkyInputHandler()
{
}

bool SkyInputHandler::Initialize(I_SkyInput* owner)
{
	m_owner = owner;
	SetInterruptVector(0x21, kSkyOSKeyboardHandler);
	SetInterruptVector(0x2c, kSkyOSMouseHandler);

	if (ActivateMouse() == true)
	{
		
		// ���콺 ���ͷ�Ʈ Ȱ��ȭ
		EnableMouseInterrupt();
		return ActivateKeyboard();
	}

	return false;
}

void SkyInputHandler::ProcessKeyboardInput()
{
	BYTE bTemp;

	// ��� ����(��Ʈ 0x60)�� ���ŵ� �����Ͱ� �ִ��� ���θ� Ȯ���Ͽ� ���� �����͸� 
	// Ű ť �Ǵ� ���콺 ť�� ����
	if (IsOutputBufferFull() == TRUE)
	{
		// ���콺 �����Ͱ� �ƴϸ� Ű ť�� ����
		if (IsMouseDataInOutputBuffer() == FALSE)
		{
			// ��� ����(��Ʈ 0x60)���� Ű ��ĵ �ڵ带 �д� �뵵�� �Լ����� Ű����� ���콺
			// �����ʹ� ��� ���۸� �������� ����ϹǷ� ���콺 �����͸� �дµ��� ��� ����
			bTemp = GetKeyboardScanCode();
			// Ű ť�� ����
			ConvertScanCodeAndPutQueue(bTemp);
		}
		// ���콺 �������̸� ���콺 ť�� ����
		else
		{
			// ��� ����(��Ʈ 0x60)���� Ű ��ĵ �ڵ带 �д� �뵵�� �Լ����� Ű����� ���콺
			// �����ʹ� ��� ���۸� �������� ����ϹǷ� ���콺 �����͸� �дµ��� ��� ����
			bTemp = GetKeyboardScanCode();
			// ���콺 ť�� ����
			AccumulateMouseDataAndPutQueue(bTemp);
		}
	}
}

void SkyInputHandler::ProcessMouseInput()
{
	BYTE bTemp;

	

	// ��� ����(��Ʈ 0x60)�� ���ŵ� �����Ͱ� �ִ��� ���θ� Ȯ���Ͽ� ���� �����͸� 
	// Ű ť �Ǵ� ���콺 ť�� ����
	if (IsOutputBufferFull() == TRUE)
	{
		// ���콺 �����Ͱ� �ƴϸ� Ű ť�� ����
		if (IsMouseDataInOutputBuffer() == FALSE)
		{
			// ��� ����(��Ʈ 0x60)���� Ű ��ĵ �ڵ带 �д� �뵵�� �Լ����� Ű����� ���콺
			// �����ʹ� ��� ���۸� �������� ����ϹǷ� ���콺 �����͸� �дµ��� ��� ����
			bTemp = GetKeyboardScanCode();
			// Ű ť�� ����
			ConvertScanCodeAndPutQueue(bTemp);
		}
		// ���콺 �������̸� ���콺 ť�� ����
		else
		{
			// ��� ����(��Ʈ 0x60)���� Ű ��ĵ �ڵ带 �д� �뵵�� �Լ����� Ű����� ���콺
			// �����ʹ� ��� ���۸� �������� ����ϹǷ� ���콺 �����͸� �дµ��� ��� ����
			bTemp = GetKeyboardScanCode();
			// ���콺 ť�� ����
			AccumulateMouseDataAndPutQueue(bTemp);
		}
	}
}

bool SkyInputHandler::ActivateMouse(void)
{
	int i = 0;
	bool bResult;

	int flag = DisableInterrupts();

	// ��Ʈ�� ��������(��Ʈ 0x64)�� ���콺 Ȱ��ȭ Ŀ�ǵ�(0xA8)�� �����Ͽ� ���콺 ����̽� Ȱ��ȭ
	OutPortByte(0x64, 0xA8);

	// ��Ʈ�� ��������(��Ʈ 0x64)�� ���콺�� �����͸� �����ϴ� Ŀ�ǵ�(0xD4)�� �����Ͽ�
	// �Է� ����(��Ʈ 0x60)�� ���޵� �����͸� ���콺�� ����
	OutPortByte(0x64, 0xD4);

	// �Է� ����(��Ʈ 0x60)�� �� ������ ��ٷȴٰ� ���콺�� Ȱ��ȭ Ŀ�ǵ带 ����
	// 0xFFFF��ŭ ������ ������ �ð��̸� ����� Ŀ�ǵ尡 ���۵� �� ����
	// 0xFFFF ������ ������ ���Ŀ��� �Է� ����(��Ʈ 0x60)�� ���� ������ �����ϰ� ����
	for (i = 0; i < 0xFFFF; i++)
	{
		// �Է� ����(��Ʈ 0x60)�� ��������� Ű���� Ŀ�ǵ� ���� ����
		if (IsInputBufferFull() == FALSE)
		{
			break;
		}
	}

	// �Է� ����(��Ʈ 0x60)�� ���콺 Ȱ��ȭ(0xF4) Ŀ�ǵ带 �����Ͽ� ���콺�� ����
	OutPortByte(0x60, 0xF4);

	// ACK�� �� ������ �����
	bResult = WaitForACKAndPutOtherScanCode();

	RestoreInterrupts(flag);

	return bResult;
}

bool SkyInputHandler::IsInputBufferFull(void)
{
	// ���� ��������(��Ʈ 0x64)���� ���� ���� �Է� ���� ���� ��Ʈ(��Ʈ 1)��
	// 1�� �����Ǿ� ������ ���� Ű���尡 �����͸� �������� �ʾ���
	if (InPortByte(0x64) & 0x02)
	{
		return TRUE;
	}
	return FALSE;
}

bool SkyInputHandler::IsOutputBufferFull(void)
{
	// ���� ��������(��Ʈ 0x64)���� ���� ���� ��� ���� ���� ��Ʈ(��Ʈ 0)��
	// 1�� �����Ǿ� ������ ��� ���ۿ� Ű���尡 ������ �����Ͱ� ������
	if (InPortByte(0x64) & 0x01)
	{
		return TRUE;
	}
	return FALSE;
}

void SkyInputHandler::EnableMouseInterrupt(void)
{
	BYTE bOutputPortData;
	int i;

	// Ŀ�ǵ� ����Ʈ �б�
	// ��Ʈ�� ��������(��Ʈ 0x64)�� Ű���� ��Ʈ�ѷ��� Ŀ�ǵ� ����Ʈ�� �д� Ŀ�ǵ�(0x20) ����
	OutPortByte(0x64, 0x20);

	// ��� ��Ʈ�� �����͸� ��ٷȴٰ� ����
	for (i = 0; i < 0xFFFF; i++)
	{
		// ��� ����(��Ʈ 0x60)�� �������� �����͸� ���� �� ����
		if (IsOutputBufferFull() == TRUE)
		{
			break;
		}
	}
	// ��� ��Ʈ(��Ʈ 0x60)�� ���ŵ� Ŀ�ǵ� ����Ʈ ���� ����
	bOutputPortData = InPortByte(0x60);


	// ���콺 ���ͷ�Ʈ ��Ʈ Ȱ��ȭ�� �� Ŀ�ǵ� ����Ʈ ����
	// ���콺 ���ͷ�Ʈ ��Ʈ(��Ʈ 1) ����
	bOutputPortData |= 0x02;

	// Ŀ�ǵ� ��������(0x64)�� Ŀ�ǵ� ����Ʈ�� ���� Ŀ�ǵ�(0x60)�� ����
	OutPortByte(0x64, 0x60);

	// �Է� ����(��Ʈ 0x60)�� �����Ͱ� ��������� ��� ��Ʈ�� ���� ���� Ŀ�ǵ�� Ŀ�ǵ� ����Ʈ ����
	for (i = 0; i < 0xFFFF; i++)
	{
		// �Է� ����(��Ʈ 0x60)�� ������� Ŀ�ǵ� ���� ����
		if (IsInputBufferFull() == FALSE)
		{
			break;
		}
	}

	// �Է� ����(0x60)�� ���콺 ���ͷ�Ʈ ��Ʈ�� 1�� ������ ���� ����
	OutPortByte(0x60, bOutputPortData);
}

/**
*  ���콺 �����Ͱ� ��� ���ۿ� �ִ����� ��ȯ
*/
bool SkyInputHandler::IsMouseDataInOutputBuffer(void)
{
	// ��� ����(��Ʈ 0x60�� �б� ���� ���� ���� ��������(��Ʈ 0x64)�� �о
	// ���콺 �������ΰ��� Ȯ��, ���콺 �����ʹ� AUXB ��Ʈ(��Ʈ 5)�� 1�� ������
	if (InPortByte(0x64) & 0x20)
	{
		return TRUE;
	}

	return false;
}

/**
*  ACK�� ��ٸ�
*      ACK�� �ƴ� �ٸ� �ڵ�� Ű���� �����Ϳ� ���콺 �����͸� �����Ͽ� ť�� ����
*/
bool SkyInputHandler::WaitForACKAndPutOtherScanCode(void)
{
	int i, j;
	BYTE bData;
	bool bResult = FALSE;
	bool bMouseData;

	// ACK�� ���� ���� Ű���� ��� ����(��Ʈ 0x60)�� Ű �����Ͱ� ����Ǿ� ���� �� �����Ƿ�
	// Ű���忡�� ���޵� �����͸� �ִ� 100������ �����Ͽ� ACK�� Ȯ��
	for (j = 0; j < 100; j++)
	{
		// 0xFFFF��ŭ ������ ������ �ð��̸� ����� Ŀ�ǵ��� ������ �� �� ����
		// 0xFFFF ������ ������ ���Ŀ��� ��� ����(��Ʈ 0x60)�� �� ���� ������ �����ϰ� ����
		for (i = 0; i < 0xFFFF; i++)
		{
			// ��� ����(��Ʈ 0x60)�� �������� �����͸� ���� �� ����
			if (IsOutputBufferFull() == TRUE)
			{
				break;
			}
		}

		// ��� ����(��Ʈ 0x60�� �б� ���� ���� ���� ��������(��Ʈ 0x64)�� �о
		// ���콺 ������������ Ȯ��
		if (IsMouseDataInOutputBuffer() == TRUE)
		{
			bMouseData = TRUE;
		}
		else
		{
			bMouseData = FALSE;
		}

		// ��� ����(��Ʈ 0x60)���� ���� �����Ͱ� ACK(0xFA)�̸� ����
		bData = InPortByte(0x60);
		if (bData == 0xFA)
		{
			bResult = TRUE;
			break;
		}
		// ACK(0xFA)�� �ƴϸ� �����Ͱ� ���ŵ� ����̽��� ���� Ű���� ť�� ���콺 ť�� ����
		else
		{
			if (bMouseData == FALSE)
			{
				ConvertScanCodeAndPutQueue(bData);
			}
			else
			{
				AccumulateMouseDataAndPutQueue(bData);
			}
		}
	}
	return bResult;
}

/**
*  ���콺 �����͸� ��Ƽ� ť�� ����
*/
bool SkyInputHandler::AccumulateMouseDataAndPutQueue(BYTE bMouseData)
{

	// ���ŵ� ����Ʈ ���� ���� ���콺 �����͸� ����
	switch (m_mouseState.iByteCount)
	{
		// ����Ʈ 1�� ������ ����
	case 0:
		m_mouseState.stCurrentData.bButtonStatusAndFlag = bMouseData;
		m_mouseState.iByteCount++;
		break;

		// ����Ʈ 2�� ������ ����
	case 1:
		m_mouseState.stCurrentData.bXMovement = bMouseData;
		m_mouseState.iByteCount++;
		break;

		// ����Ʈ 3�� ������ ����
	case 2:
		m_mouseState.stCurrentData.bYMovement = bMouseData;
		m_mouseState.iByteCount++;
		break;

		// �� ���� ���� ���ŵ� ����Ʈ �� �ʱ�ȭ
	default:
		m_mouseState.iByteCount = 0;
		break;
	}

	// 3����Ʈ�� ��� ���ŵǾ����� ���콺 ť�� �����ϰ� ���ŵ� Ƚ���� �ʱ�ȭ
	if (m_mouseState.iByteCount >= 3)
	{
		int flag = DisableInterrupts();
		
		// ���콺 ť�� ���콺 ������ ���� 		
		if (m_owner)
		{
			m_owner->PutMouseQueue(&m_mouseState.stCurrentData);
		}
		else
		{
			SkyGUISystem::GetInstance()->PutMouseQueue(&m_mouseState.stCurrentData);
		}						

		RestoreInterrupts(flag);
		
		// ���ŵ� ����Ʈ �� �ʱ�ȭ
		m_mouseState.iByteCount = 0;
	}
	return true;
}


///////
//Ű����
/**
*  ��� ����(��Ʈ 0x60)���� Ű�� ����
*/
BYTE SkyInputHandler::GetKeyboardScanCode(void)
{
	// ��� ����(��Ʈ 0x60)�� �����Ͱ� ���� ������ ���
	while (IsOutputBufferFull() == FALSE)
	{
		;
	}
	return InPortByte(0x60);
}

/**
*  ��ĵ �ڵ带 ���������� ����ϴ� Ű �����ͷ� �ٲ� �� Ű ť�� ����
*/
bool SkyInputHandler::ConvertScanCodeAndPutQueue(BYTE bScanCode)
{
	KEYDATA stData = { 0, };
	bool bResult = FALSE;

	stData.bScanCode = bScanCode;
	if (ConvertScanCodeToASCIICode(bScanCode, &(stData.bASCIICode), &(stData.bFlags)) == TRUE)
	{
		int flag = DisableInterrupts();
		
		if (m_owner)
		{
			m_owner->PutKeyboardQueue(&stData);
		}
		else
			bResult = SkyGUISystem::GetInstance()->PutKeyboardQueue(&stData);
		
		RestoreInterrupts(flag);
	}
	return bResult;
}

/**
*  ��ĵ �ڵ带 ASCII �ڵ�� ��ȯ
*/
bool SkyInputHandler::ConvertScanCodeToASCIICode(BYTE bScanCode, BYTE* pbASCIICode, unsigned char* pbFlags)
{
	bool bUseCombinedKey = false;

	// ������ Pause Ű�� ���ŵǾ��ٸ�, Pause�� ���� ��ĵ �ڵ带 ����
	if (m_keyboardState.iSkipCountForPause > 0)
	{
		m_keyboardState.iSkipCountForPause--;
		return FALSE;
	}

	// Pause Ű�� Ư���� ó��
	if (bScanCode == 0xE1)
	{
		*pbASCIICode = KEY_PAUSE;
		*pbFlags = KEY_FLAGS_DOWN;
		m_keyboardState.iSkipCountForPause = KEY_SKIPCOUNTFORPAUSE;
		return TRUE;
	}
	// Ȯ�� Ű �ڵ尡 ������ ��, ���� Ű ���� ������ �����Ƿ� �÷��� ������ �ϰ� ����
	else if (bScanCode == 0xE0)
	{
		m_keyboardState.bExtendedCodeIn = TRUE;
		return FALSE;
	}

	// ���յ� Ű�� ��ȯ�ؾ� �ϴ°�?
	bUseCombinedKey = IsUseCombinedCode(bScanCode);

	// Ű �� ����
	if (bUseCombinedKey == TRUE)
	{		
		*pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bCombinedCode;
	}
	else
	{
		*pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bNormalCode;
	}

	// Ȯ�� Ű ���� ����
	if (m_keyboardState.bExtendedCodeIn == TRUE)
	{
//20191208 21:00
		//*pbFlags = KEY_FLAGS_EXTENDEDKEY;
		m_keyboardState.bExtendedCodeIn = FALSE;
	}
	else
	{
		*pbFlags = 0;
	}

	// ������ �Ǵ� ������ ���� ����
	if ((bScanCode & 0x80) == 0)
	{
		*pbFlags |= KEY_FLAGS_DOWN;
	}

	// ���� Ű ���� �Ǵ� ������ ���¸� ����
	UpdateCombinationKeyStatusAndLED(bScanCode);
	return TRUE;
}

/**
*  ���� Ű�� ���¸� �����ϰ� LED ���µ� ����ȭ ��
*/
void SkyInputHandler::UpdateCombinationKeyStatusAndLED(BYTE bScanCode)
{
	bool bDown = false;
	BYTE bDownScanCode = 0;
	bool bLEDStatusChanged = false;

	// ���� �Ǵ� ������ ����ó��, �ֻ��� ��Ʈ(��Ʈ 7)�� 1�̸� Ű�� ���������� �ǹ��ϰ�
	// 0�̸� ���������� �ǹ���
	if (bScanCode & 0x80)
	{
		bDown = false;
		bDownScanCode = bScanCode & 0x7F;
	}
	else
	{
		bDown = true;
		bDownScanCode = bScanCode;
	}

	// ���� Ű �˻�
	// Shift Ű�� ��ĵ �ڵ�(42 or 54)�̸� Shift Ű�� ���� ����
	if ((bDownScanCode == 42) || (bDownScanCode == 54))
	{
		m_keyboardState.bShiftDown = bDown;
	}
	// Caps Lock Ű�� ��ĵ �ڵ�(58)�̸� Caps Lock�� ���� �����ϰ� LED ���� ����
	else if ((bDownScanCode == 58) && (bDown == TRUE))
	{		
		m_keyboardState.bCapsLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	}
	// Num Lock Ű�� ��ĵ �ڵ�(69)�̸� Num Lock�� ���¸� �����ϰ� LED ���� ����
	else if ((bDownScanCode == 69) && (bDown == TRUE))
	{
		m_keyboardState.bNumLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	}
	// Scroll Lock Ű�� ��ĵ �ڵ�(70)�̸� Scroll Lock�� ���¸� �����ϰ� LED ���� ����
	else if ((bDownScanCode == 70) && (bDown == TRUE))
	{
		m_keyboardState.bScrollLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	}

	// LED ���°� �������� Ű����� Ŀ�ǵ带 �����Ͽ� LED�� ����
	if (bLEDStatusChanged == TRUE)
	{
		ChangeKeyboardLED(m_keyboardState.bCapsLockOn,
			m_keyboardState.bNumLockOn, m_keyboardState.bScrollLockOn);
	}
}

/**
*  Ű���� LED�� ON/OFF�� ����
*/
bool SkyInputHandler::ChangeKeyboardLED(bool bCapsLockOn, bool bNumLockOn, bool bScrollLockOn)
{
	int i = 0;
	
	bool bResult;

	int flag = DisableInterrupts();

	// Ű���忡 LED ���� Ŀ�ǵ� �����ϰ� Ŀ�ǵ尡 ó���� ������ ���
	for (i = 0; i < 0xFFFF; i++)
	{
		// ��� ����(��Ʈ 0x60)�� ������� Ŀ�ǵ� ���� ����
		if (IsInputBufferFull() == FALSE)
		{
			break;
		}
	}

	// ��� ����(��Ʈ 0x60)�� LED ���� ���� Ŀ�ǵ�(0xED) ����
	OutPortByte(0x60, 0xED);
	for (i = 0; i < 0xFFFF; i++)
	{
		// �Է� ����(��Ʈ 0x60)�� ��������� Ű���尡 Ŀ�ǵ带 ������ ����
		if (IsInputBufferFull() == FALSE)
		{
			break;
		}
	}

	// ACK�� �ö����� �����
	bResult = WaitForACKAndPutOtherScanCode();

	if (bResult == FALSE)
	{
		RestoreInterrupts(flag);
		return FALSE;
	}

	// LED ���� ���� Ű����� �����ϰ� �����Ͱ� ó���� �Ϸ�� ������ ���
	OutPortByte(0x60, ((bCapsLockOn << 2) | (bNumLockOn << 1) | (bScrollLockOn ? 1 : 0) ));
	for (i = 0; i < 0xFFFF; i++)
	{
		// �Է� ����(��Ʈ 0x60)�� ��������� Ű���尡 LED �����͸� ������ ����
		if (IsInputBufferFull() == FALSE)
		{
			break;
		}
	}

	// ACK�� �� ������ �����
	bResult = WaitForACKAndPutOtherScanCode();

	RestoreInterrupts(flag);
	return bResult;
}

/**
*  ���յ� Ű ���� ����ؾ� �ϴ��� ���θ� ��ȯ
*/

bool SkyInputHandler::IsUseCombinedCode(BYTE bScanCode)
{
	BYTE bDownScanCode = 0;
	bool bUseCombinedKey = false;

	bDownScanCode = bScanCode & 0x7F;

	// ���ĺ� Ű��� Shift Ű�� Caps Lock�� ������ ����
	if (IsAlphabetScanCode(bDownScanCode) == true)
	{		
		// ���� Shift Ű�� Caps Lock Ű �߿� �ϳ��� ������������ ���յ� Ű�� �ǵ��� ��
		if (m_keyboardState.bShiftDown ^ m_keyboardState.bCapsLockOn)
		{
			
			bUseCombinedKey = TRUE;
		}
		else
		{
			bUseCombinedKey = FALSE;
		}
	}
	// ���ڿ� ��ȣ Ű��� Shift Ű�� ������ ����
	else if (IsNumberOrSymbolScanCode(bDownScanCode) == TRUE)
	{
		// Shift Ű�� ������������ ���յ� Ű�� �ǵ��� ��
		if (m_keyboardState.bShiftDown == TRUE)
		{
			bUseCombinedKey = TRUE;
		}
		else
		{
			bUseCombinedKey = FALSE;
		}
	}
	// ���� �е� Ű��� Num Lock Ű�� ������ ����
	// 0xE0�� �����ϸ� Ȯ�� Ű �ڵ�� ���� �е��� �ڵ尡 ��ġ�Ƿ�,
	// Ȯ�� Ű �ڵ尡 ���ŵ��� �ʾ��� ����ó�� ���յ� �ڵ� ���
	else if ((IsNumberPadScanCode(bDownScanCode) == TRUE) &&
		(m_keyboardState.bExtendedCodeIn == FALSE))
	{
		// Num Lock Ű�� ������������, ���յ� Ű�� �ǵ��� ��
		if (m_keyboardState.bNumLockOn == TRUE)
		{
			bUseCombinedKey = TRUE;
		}
		else
		{
			bUseCombinedKey = FALSE;
		}
	}

	return bUseCombinedKey;
}

/**
*  ��ĵ �ڵ尡 ���ĺ� �������� ���θ� ��ȯ
*/
bool SkyInputHandler::IsAlphabetScanCode(BYTE bScanCode)
{
	// ��ȯ ���̺��� ���� ���� �о ���ĺ� �������� Ȯ��
	if (('a' <= gs_vstKeyMappingTable[bScanCode].bNormalCode) &&
		(gs_vstKeyMappingTable[bScanCode].bNormalCode <= 'z'))
	{
		return true;
	}
	return false;
}

/**
*  ���� �Ǵ� ��ȣ �������� ���θ� ��ȯ
*/
bool SkyInputHandler::IsNumberOrSymbolScanCode(BYTE bScanCode)
{
	// ���� �е峪 Ȯ�� Ű ������ ������ ����(��ĵ �ڵ� 2~53)���� �����ڰ� �ƴϸ�
	// ���� �Ǵ� ��ȣ��
	if ((2 <= bScanCode) && (bScanCode <= 53) &&
		(IsAlphabetScanCode(bScanCode) == FALSE))
	{
		return TRUE;
	}

	return FALSE;
}

/**
* ���� �е� �������� ���θ� ��ȯ
*/
bool SkyInputHandler::IsNumberPadScanCode(BYTE bScanCode)
{
	// ���� �е�� ��ĵ �ڵ��� 71~83�� ����
	if ((71 <= bScanCode) && (bScanCode <= 83))
	{
		return TRUE;
	}

	return FALSE;
}

/**
*  Ű���带 Ȱ��ȭ ��
*/
bool SkyInputHandler::ActivateKeyboard(void)
{
	int i = 0;
	bool bResult;

	int flag = DisableInterrupts();

	// ��Ʈ�� ��������(��Ʈ 0x64)�� Ű���� Ȱ��ȭ Ŀ�ǵ�(0xAE)�� �����Ͽ� Ű���� ����̽� Ȱ��ȭ
	OutPortByte(0x64, 0xAE);

	// �Է� ����(��Ʈ 0x60)�� �� ������ ��ٷȴٰ� Ű���忡 Ȱ��ȭ Ŀ�ǵ带 ����
	// 0xFFFF��ŭ ������ ������ �ð��̸� ����� Ŀ�ǵ尡 ���۵� �� ����
	// 0xFFFF ������ ������ ���Ŀ��� �Է� ����(��Ʈ 0x60)�� ���� ������ �����ϰ� ����
	for (i = 0; i < 0xFFFF; i++)
	{
		// �Է� ����(��Ʈ 0x60)�� ��������� Ű���� Ŀ�ǵ� ���� ����
		if (IsInputBufferFull() == FALSE)
		{
			break;
		}
	}

	// �Է� ����(��Ʈ 0x60)�� Ű���� Ȱ��ȭ(0xF4) Ŀ�ǵ带 �����Ͽ� Ű����� ����
	OutPortByte(0x60, 0xF4);

	// ACK�� �� ������ �����
	bResult = WaitForACKAndPutOtherScanCode();

	RestoreInterrupts(flag);
	return bResult;
}