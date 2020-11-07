#include <Windows.h>
#include "SkyInputHandlerWin32.h"
#include <SDL.h>

// 스캔 코드를 ASCII 코드로 변환하는 테이블
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
	/*  88  */{ KEY_F12         ,   KEY_F12 },
	/*  89  */ { '='         ,   '+' }
};

static KEYCODEASCIIMAPPINGENTRY gs_vstKeycodeAsciiMappingTable[KEY_MAPPINGTABLEMAXCOUNT] =
{
	{ SDLK_UNKNOWN ,KEY_NONE        ,   KEY_NONE },
	{ SDLK_ESCAPE ,KEY_ESC         ,   KEY_ESC },
	{ SDLK_1 ,'1'             ,   '!' },
	{ SDLK_2 ,'2'             ,   '@' },
	{ SDLK_3 ,'3'             ,   '#' },
	{ SDLK_4 ,'4'             ,   '$' },
	{ SDLK_5 ,'5'             ,   '%' },
	{ SDLK_6 ,'6'             ,   '^' },
	{ SDLK_7 ,'7'             ,   '&' },
	{ SDLK_8 ,'8'             ,   '*' },
	{ SDLK_9 ,'9'             ,   '(' },
	{ SDLK_0 ,'0'             ,   ')' },
	{ SDLK_MINUS ,'-'             ,   '_' },
	{ SDLK_PLUS ,'='             ,   '+' },
	{ SDLK_BACKSPACE ,KEY_BACKSPACE   ,   KEY_BACKSPACE },
	{ SDLK_TAB ,KEY_TAB         ,   KEY_TAB },
	{ SDLK_q ,'q'             ,   'Q' },
	{ SDLK_w ,'w'             ,   'W' },
	{ SDLK_e ,'e'             ,   'E' },
	{ SDLK_r ,'r'             ,   'R' },
	{ SDLK_t ,'t'             ,   'T' },
	{ SDLK_y ,'y'             ,   'Y' },
	{ SDLK_u ,'u'             ,   'U' },
	{ SDLK_i ,'i'             ,   'I' },
	{ SDLK_o ,'o'             ,   'O' },
	{ SDLK_p ,'p'             ,   'P' },
	{ SDLK_LEFTBRACKET ,'['             ,   '{' },
	{ SDLK_RIGHTBRACKET ,']'             ,   '}' },
	{ SDLK_RETURN ,'\n'            ,   '\n' },
	{ SDLK_LCTRL ,KEY_CTRL        ,   KEY_CTRL },
	{ SDLK_a ,'a'             ,   'A' },
	{ SDLK_s ,'s'             ,   'S' },
	{ SDLK_d ,'d'             ,   'D' },
	{ SDLK_f ,'f'             ,   'F' },
	{ SDLK_g ,'g'             ,   'G' },
	{ SDLK_h ,'h'             ,   'H' },
	{ SDLK_j ,'j'             ,   'J' },
	{ SDLK_k ,'k'             ,   'K' },
	{ SDLK_l ,'l'             ,   'L' },
	{ SDLK_SEMICOLON ,';'             ,   ':' },
	{ SDLK_QUOTE ,'\''            ,   '\"' },
	{ SDLK_BACKQUOTE ,'`'             ,   '~' },
	{ SDLK_LSHIFT ,KEY_LSHIFT      ,   KEY_LSHIFT },
	{ SDLK_BACKSLASH ,'\\'            ,   '|' },
	{ SDLK_z ,'z'             ,   'Z' },
	{ SDLK_x ,'x'             ,   'X' },
	{ SDLK_c ,'c'             ,   'C' },
	{ SDLK_v ,'v'             ,   'V' },
	{ SDLK_b ,'b'             ,   'B' },
	{ SDLK_n ,'n'             ,   'N' },
	{ SDLK_m ,'m'             ,   'M' },
	{ SDLK_COMMA,','             ,   '<' },
	{ SDLK_PERIOD,'.'             ,   '>' },
	{ SDLK_SLASH,'/'             ,   '?' },
	{ SDLK_RSHIFT,KEY_RSHIFT      ,   KEY_RSHIFT },
	{ SDLK_ASTERISK,'*'             ,   '*' },
	{ SDLK_LALT,KEY_LALT        ,   KEY_LALT },
	{ SDLK_SPACE,' '             ,   ' ' },
	{ SDLK_CAPSLOCK,KEY_CAPSLOCK    ,   KEY_CAPSLOCK },
	{ SDLK_F1,KEY_F1          ,   KEY_F1 },
	{ SDLK_F2,KEY_F2          ,   KEY_F2 },
	{ SDLK_F3,KEY_F3          ,   KEY_F3 },
	{ SDLK_F4,KEY_F4          ,   KEY_F4 },
	{ SDLK_F5,KEY_F5          ,   KEY_F5 },
	{ SDLK_F6,KEY_F6          ,   KEY_F6 },
	{ SDLK_F7,KEY_F7          ,   KEY_F7 },
	{ SDLK_F8,KEY_F8          ,   KEY_F8 },
	{ SDLK_F9,KEY_F9          ,   KEY_F9 },
	{ SDLK_F10,KEY_F10         ,   KEY_F10 },
	{ SDLK_NUMLOCKCLEAR,KEY_NUMLOCK     ,   KEY_NUMLOCK },
	{ SDLK_SCROLLLOCK,KEY_SCROLLLOCK  ,   KEY_SCROLLLOCK },

	{ SDLK_HOME,KEY_HOME        ,   '7' },
	{ SDLK_UP,KEY_UP          ,   '8' },
	{ SDLK_PAGEUP,KEY_PAGEUP      ,   '9' },
	{ SDLK_MINUS,'-'             ,   '-' },
	{ SDLK_LEFT,KEY_LEFT        ,   '4' },
	{ 0x97,KEY_CENTER      ,   '5' },
	{ SDLK_RIGHT,KEY_RIGHT       ,   '6' },
	{ SDLK_KP_PLUS,'+'             ,   '+' },
	{ SDLK_END,KEY_END         ,   '1' },
	{ SDLK_DOWN,KEY_DOWN        ,   '2' },
	{ SDLK_PAGEDOWN,KEY_PAGEDOWN    ,   '3' },
	{ SDLK_INSERT,KEY_INS         ,   '0' },
	{ SDLK_DELETE,KEY_DEL         ,   '.' },
	{ SDLK_UNKNOWN ,KEY_NONE        ,   KEY_NONE },
	{ SDLK_UNKNOWN ,KEY_NONE        ,   KEY_NONE },
	{ SDLK_UNKNOWN ,KEY_NONE        ,   KEY_NONE },
	{ SDLK_F11,KEY_F11         ,   KEY_F11 },
	{ SDLK_F12,KEY_F12         ,   KEY_F12 },
	{ SDLK_EQUALS,'='         ,   '=' }
};

SkyInputHandlerWin32* SkyInputHandlerWin32::m_inputHandler = nullptr;


SkyInputHandlerWin32::SkyInputHandlerWin32()
{
	memset(&m_keyboardState, 0, sizeof(KEYBOARDSTATE));
	memset(&m_mouseState, 0, sizeof(MOUSESTATE));			
	m_pGUIEngine = nullptr;	
	m_owner = nullptr;
}

SkyInputHandlerWin32::~SkyInputHandlerWin32()
{
}

bool SkyInputHandlerWin32::Initialize(I_SkyInput* owner)
{
	m_owner = owner;
	
	return true;
}

bool SkyInputHandlerWin32::ConvertScanCodeAndPutQueue(BYTE bScanCode)
{
	KEYDATA stData;
	bool bResult = FALSE;

	stData.bScanCode = bScanCode;
	if (ConvertScanCodeToASCIICode(bScanCode, &(stData.bASCIICode), &(stData.bFlags)) == TRUE)
	{
		
		if(m_pGUIEngine)
			bResult = m_pGUIEngine->PutKeyboardQueue(&stData);
		else if (m_owner)
		{
			m_owner->PutKeyboardQueue(&stData);
		}
	}
	
	return bResult;
}

void SkyInputHandlerWin32::Print(char* str)
{	
	
	if (m_owner)
	{
		m_owner->Print(0, str);
	}
}

void SkyInputHandlerWin32::SoftwareInterrupt()
{
	m_owner->SoftwareInterrupt();
}

BYTE SkyInputHandlerWin32::ConvertKeycodeToScancode(unsigned int keycode)
{
	for (int i = 0; i < KEY_MAPPINGTABLEMAXCOUNT; i++)
	{
		if (gs_vstKeycodeAsciiMappingTable[i].sdlKeycode == keycode)
		{
			return i;
		}
	}

	return 0;
}

/**
*  스캔 코드를 ASCII 코드로 변환
*/
bool SkyInputHandlerWin32::ConvertScanCodeToASCIICode(BYTE bScanCode, BYTE* pbASCIICode, unsigned char* pbFlags)
{
	bool bUseCombinedKey = false;

	// 이전에 Pause 키가 수신되었다면, Pause의 남은 스캔 코드를 무시
	if (m_keyboardState.iSkipCountForPause > 0)
	{
		m_keyboardState.iSkipCountForPause--;
		return FALSE;
	}

	// Pause 키는 특별히 처리
	if (bScanCode == 0xE1)
	{
		*pbASCIICode = KEY_PAUSE;
		*pbFlags = KEY_FLAGS_DOWN;
		m_keyboardState.iSkipCountForPause = KEY_SKIPCOUNTFORPAUSE;
		return TRUE;
	}
	// 확장 키 코드가 들어왔을 때, 실제 키 값은 다음에 들어오므로 플래그 설정만 하고 종료
	else if (bScanCode == 0xE0)
	{
		m_keyboardState.bExtendedCodeIn = TRUE;
		return FALSE;
	}

	// 조합된 키를 반환해야 하는가?
	bUseCombinedKey = IsUseCombinedCode(bScanCode);

	// 키 값 설정
	if (bUseCombinedKey == TRUE)
	{		
		*pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bCombinedCode;
	}
	else
	{
		*pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bNormalCode;
	}

	// 확장 키 유무 설정
	if (m_keyboardState.bExtendedCodeIn == TRUE)
	{
		*pbFlags = KEY_FLAGS_EXTENDEDKEY;
		m_keyboardState.bExtendedCodeIn = FALSE;
	}
	else
	{
		*pbFlags = 0;
	}

	// 눌러짐 또는 떨어짐 유무 설정
	if ((bScanCode & 0x80) == 0)
	{
		*pbFlags |= KEY_FLAGS_DOWN;
	}

	// 조합 키 눌림 또는 떨어짐 상태를 갱신
	UpdateCombinationKeyStatusAndLED(bScanCode);
	return TRUE;
}

/**
*  조합 키의 상태를 갱신하고 LED 상태도 동기화 함
*/
void SkyInputHandlerWin32::UpdateCombinationKeyStatusAndLED(BYTE bScanCode)
{
	bool bDown = false;
	BYTE bDownScanCode = 0;
	bool bLEDStatusChanged = false;

	// 눌림 또는 떨어짐 상태처리, 최상위 비트(비트 7)가 1이면 키가 떨어졌음을 의미하고
	// 0이면 떨어졌음을 의미함
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

	// 조합 키 검색
	// Shift 키의 스캔 코드(42 or 54)이면 Shift 키의 상태 갱신
	if ((bDownScanCode == 42) || (bDownScanCode == 54))
	{
		m_keyboardState.bShiftDown = bDown;
	}
	// Caps Lock 키의 스캔 코드(58)이면 Caps Lock의 상태 갱신하고 LED 상태 변경
	else if ((bDownScanCode == 58) && (bDown == TRUE))
	{		
		m_keyboardState.bCapsLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	}
	// Num Lock 키의 스캔 코드(69)이면 Num Lock의 상태를 갱신하고 LED 상태 변경
	else if ((bDownScanCode == 69) && (bDown == TRUE))
	{
		m_keyboardState.bNumLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	}
	// Scroll Lock 키의 스캔 코드(70)이면 Scroll Lock의 상태를 갱신하고 LED 상태 변경
	else if ((bDownScanCode == 70) && (bDown == TRUE))
	{
		m_keyboardState.bScrollLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	}
}

bool SkyInputHandlerWin32::IsUseCombinedCode(BYTE bScanCode)
{
	BYTE bDownScanCode = 0;
	bool bUseCombinedKey = false;

	bDownScanCode = bScanCode & 0x7F;

	// 알파벳 키라면 Shift 키와 Caps Lock의 영향을 받음
	if (IsAlphabetScanCode(bDownScanCode) == true)
	{		
		// 만약 Shift 키와 Caps Lock 키 중에 하나만 눌러져있으면 조합된 키를 되돌려 줌
		if (m_keyboardState.bShiftDown ^ m_keyboardState.bCapsLockOn)
		{
			
			bUseCombinedKey = TRUE;
		}
		else
		{
			bUseCombinedKey = FALSE;
		}
	}
	// 숫자와 기호 키라면 Shift 키의 영향을 받음
	else if (IsNumberOrSymbolScanCode(bDownScanCode) == TRUE)
	{
		// Shift 키가 눌러져있으면 조합된 키를 되돌려 줌
		if (m_keyboardState.bShiftDown == TRUE)
		{
			bUseCombinedKey = TRUE;
		}
		else
		{
			bUseCombinedKey = FALSE;
		}
	}
	// 숫자 패드 키라면 Num Lock 키의 영향을 받음
	// 0xE0만 제외하면 확장 키 코드와 숫자 패드의 코드가 겹치므로,
	// 확장 키 코드가 수신되지 않았을 때만처리 조합된 코드 사용
	else if ((IsNumberPadScanCode(bDownScanCode) == TRUE) &&
		(m_keyboardState.bExtendedCodeIn == FALSE))
	{
		// Num Lock 키가 눌러져있으면, 조합된 키를 되돌려 줌
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
*  스캔 코드가 알파벳 범위인지 여부를 반환
*/
bool SkyInputHandlerWin32::IsAlphabetScanCode(BYTE bScanCode)
{
	// 변환 테이블을 값을 직접 읽어서 알파벳 범위인지 확인
	if (('a' <= gs_vstKeyMappingTable[bScanCode].bNormalCode) &&
		(gs_vstKeyMappingTable[bScanCode].bNormalCode <= 'z'))
	{
		return true;
	}
	return false;
}

/**
*  숫자 또는 기호 범위인지 여부를 반환
*/
bool SkyInputHandlerWin32::IsNumberOrSymbolScanCode(BYTE bScanCode)
{
	// 숫자 패드나 확장 키 범위를 제외한 범위(스캔 코드 2~53)에서 영문자가 아니면
	// 숫자 또는 기호임
	if ((2 <= bScanCode) && (bScanCode <= 53) &&
		(IsAlphabetScanCode(bScanCode) == FALSE))
	{
		return TRUE;
	}

	if (bScanCode == 89)
		return TRUE;

	return FALSE;
}

/**
* 숫자 패드 범위인지 여부를 반환
*/
bool SkyInputHandlerWin32::IsNumberPadScanCode(BYTE bScanCode)
{
	// 숫자 패드는 스캔 코드의 71~83에 있음
	if ((71 <= bScanCode) && (bScanCode <= 83))
	{
		return TRUE;
	}

	return FALSE;
}