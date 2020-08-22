#pragma once
#include "I_GUIEngine.h"
////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// Pause 키를 처리하기 위해 무시해야 하는 나머지 스캔 코드의 수
#define KEY_SKIPCOUNTFORPAUSE       2

// 키 상태에 대한 플래그
#define KEY_FLAGS_UP             0x00
#define KEY_FLAGS_DOWN           0x01
#define KEY_FLAGS_EXTENDEDKEY    0x02

// 스캔 코드 매핑 테이블에 대한 매크로
#define KEY_MAPPINGTABLEMAXCOUNT    90

#define KEY_NONE        0x00
#define KEY_ENTER       '\n'
#define KEY_TAB         '\t'
#define KEY_ESC         0x1B
#define KEY_BACKSPACE   0x08

#define KEY_CTRL        0x81
#define KEY_LSHIFT      0x82
#define KEY_RSHIFT      0x83
#define KEY_PRINTSCREEN 0x84
#define KEY_LALT        0x85
#define KEY_CAPSLOCK    0x86
#define KEY_F1          0x87
#define KEY_F2          0x88
#define KEY_F3          0x89
#define KEY_F4          0x8A
#define KEY_F5          0x8B
#define KEY_F6          0x8C
#define KEY_F7          0x8D
#define KEY_F8          0x8E
#define KEY_F9          0x8F
#define KEY_F10         0x90
#define KEY_NUMLOCK     0x91
#define KEY_SCROLLLOCK  0x92
#define KEY_HOME        0x93
#define KEY_UP          0x94
#define KEY_PAGEUP      0x95
#define KEY_LEFT        0x96
#define KEY_CENTER      0x97
#define KEY_RIGHT       0x98
#define KEY_END         0x99
#define KEY_DOWN        0x9A
#define KEY_PAGEDOWN    0x9B
#define KEY_INS         0x9C
#define KEY_DEL         0x9D
#define KEY_F11         0x9E
#define KEY_F12         0x9F
#define KEY_PAUSE       0xA0


class SkyInputHandlerWin32
{
public:
	SkyInputHandlerWin32();
	~SkyInputHandlerWin32();

	bool Initialize(I_SkyInput* owner);

	static SkyInputHandlerWin32* GetInstance()
	{
		if (m_inputHandler == nullptr)
			m_inputHandler = new SkyInputHandlerWin32();

		return m_inputHandler;	
	}

	void SoftwareInterrupt();
	void Print(char* str);
	BYTE ConvertKeycodeToScancode(unsigned int keycode);
	bool ConvertScanCodeAndPutQueue(BYTE bScanCode);
	void SetCallback(I_GUIEngine* pCallback) { m_pGUIEngine = pCallback; }

	bool ConvertScanCodeToASCIICode(BYTE bScanCode, BYTE* pbASCIICode, unsigned char* pbFlags);
protected:	
	
	void UpdateCombinationKeyStatusAndLED(BYTE bScanCode);
	bool IsUseCombinedCode(BYTE bScanCode);
	bool IsAlphabetScanCode(BYTE bScanCode);
	bool IsNumberOrSymbolScanCode(BYTE bScanCode);
	bool IsNumberPadScanCode(BYTE bScanCode);

private:
	static SkyInputHandlerWin32* m_inputHandler;
	
	KEYBOARDSTATE	m_keyboardState;
	MOUSESTATE		m_mouseState;
	I_GUIEngine* m_pGUIEngine;
	I_SkyInput*  m_owner;
};