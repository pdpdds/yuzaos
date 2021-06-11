#include "GUIConsole.h"
#include "memory.h"
#include "Mouse.h"
#include <stdio.h>
#include "stringdef.h"
#include "ConsoleShell.h"
#include <systemcall_impl.h>
#include <gdi32.h>

// Ŀ�ǵ� ���̺� ����
SHELLCOMMANDENTRY gs_vstCommandTable[] =
{
	{ "help", "Show Help", kHelp },
	{ "cls", "Clear Screen", kCls },
	{ "totalram", "Show Total RAM Size", kShowTotalRAMSize },
	{ "shutdown", "Shutdown And Reboot OS", kShutdown },
};

#define FONT_ENGLISHWIDTH   8
#define FONT_ENGLISHHEIGHT  16

// �ѱ� ��Ʈ�� �ʺ�� ����
#define FONT_HANGULWIDTH   16
#define FONT_HANGULHEIGHT  16

GUIConsole::GUIConsole() :
iCommandBufferIndex (0)
{
	
}


GUIConsole::~GUIConsole()
{
}

SHELLCOMMANDENTRY* GUIConsole::GetCommandArray()
{
	return gs_vstCommandTable;
}

bool GUIConsole::Initialize(int iX, int iY)
{
	memset(&m_ConsoleManager, 0, sizeof(CONSOLEMANAGER));

	memset(&m_vstScreenBuffer, 0, sizeof(m_vstScreenBuffer));
	memset(&m_vstKeyQueueBuffer, 0, sizeof(m_vstKeyQueueBuffer));

	m_ConsoleManager.pstScreenBuffer = (CHARACTER*)m_vstScreenBuffer;

	kInitializeQueue(&(m_ConsoleManager.stKeyQueueForGUI), m_vstKeyQueueBuffer,
		CONSOLE_GUIKEYQUEUE_MAXCOUNT, sizeof(KEYDATA));

	// Ŀ�� ��ġ ����
	SetCursor(iX, iY);
	
	m_commandCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);

	m_ConsoleManager.mutex = Syscall_CreateMutex("GUIConsole");

	return true;
}

/**
*  Ŀ���� ��ġ�� ����
*      ���ڸ� ����� ��ġ�� ���� ����
*/
void GUIConsole::SetCursor(int iX, int iY)
{
	int iLinearValue;
	int i;

	// Ŀ���� ��ġ�� ���
	iLinearValue = iY * CONSOLE_WIDTH + iX;


	// ������ Ŀ���� �ִ� ��ġ�� �״�� Ŀ���� ���������� Ŀ���� ����
	for (i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++)
	{
		// Ŀ���� ������ ����
		if ((m_ConsoleManager.pstScreenBuffer[i].bCharacter == '_') &&
			(m_ConsoleManager.pstScreenBuffer[i].bAttribute == 0x00))
		{
			m_ConsoleManager.pstScreenBuffer[i].bCharacter = ' ';
			m_ConsoleManager.pstScreenBuffer[i].bAttribute =
				CONSOLE_DEFAULTTEXTCOLOR;
			break;
		}
	}

	// ���ο� ��ġ�� Ŀ���� ���
	m_ConsoleManager.pstScreenBuffer[iLinearValue].bCharacter = '_';
	m_ConsoleManager.pstScreenBuffer[iLinearValue].bAttribute = 0x00;

	// ���ڸ� ����� ��ġ ������Ʈ
	m_ConsoleManager.iCurrentPrintOffset = iLinearValue;
}

/**
*  ���� Ŀ���� ��ġ�� ��ȯ
*/
void GUIConsole::GetCursor(int *piX, int *piY)
{
	*piX = m_ConsoleManager.iCurrentPrintOffset % CONSOLE_WIDTH;
	*piY = m_ConsoleManager.iCurrentPrintOffset / CONSOLE_WIDTH;
}

void GUIConsole::PrintPrompt()
{
	// ������Ʈ ��� �� Ŀ�ǵ� ���� �ʱ�ȭ
	char* newPath = ConsoleManager::MakePathName();
	Printf(newPath);
}

/**
*  ��ü ȭ���� ����
*/
void GUIConsole::ClearScreen(void)
{
	CHARACTER* pstScreen;
	int i;

	// ȭ�� ���۸� ����
	pstScreen = m_ConsoleManager.pstScreenBuffer;

	// ȭ�� ��ü�� �������� ä���, Ŀ���� ��ġ�� 0, 0���� �ű�
	for (i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++)
	{
		pstScreen[i].bCharacter = ' ';
		pstScreen[i].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
	}

	// Ŀ���� ȭ�� ������� �̵�
	SetCursor(0, 0);
}

/**
*  ���ڿ��� ������ ������
*/
void GUIConsole::ReverseString(char* pcBuffer)
{
	int iLength;
	int i;
	char cTemp;


	// ���ڿ��� ����� �߽����� ��/�츦 �ٲ㼭 ������ ������
	iLength = strlen(pcBuffer);
	for (i = 0; i < iLength / 2; i++)
	{
		cTemp = pcBuffer[i];
		pcBuffer[i] = pcBuffer[iLength - 1 - i];
		pcBuffer[iLength - 1 - i] = cTemp;
	}
}

/**
*  16���� ���� ���ڿ��� ��ȯ
*/
QWORD GUIConsole::HexToString(QWORD qwValue, char* pcBuffer)
{
	QWORD i;
	QWORD qwCurrentValue;

	// 0�� ������ �ٷ� ó��
	if (qwValue == 0)
	{
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}

	// ���ۿ� 1�� �ڸ����� 16, 256, ...�� �ڸ� ������ ���� ����
	for (i = 0; qwValue > 0; i++)
	{
		qwCurrentValue = qwValue % 16;
		if (qwCurrentValue >= 10)
		{
			pcBuffer[i] = (char)('A' + (qwCurrentValue - 10));
		}
		else
		{
			pcBuffer[i] = (char)('0' + qwCurrentValue);
		}

		qwValue = qwValue / 16;
	}
	pcBuffer[i] = '\0';

	// ���ۿ� ����ִ� ���ڿ��� ����� ... 256, 16, 1�� �ڸ� ������ ����
	ReverseString(pcBuffer);
	return i;
}

/**
*  10���� ���� ���ڿ��� ��ȯ
*/
int GUIConsole::DecimalToString(QWORD lValue, char* pcBuffer)
{
	long i;

	// 0�� ������ �ٷ� ó��
	if (lValue == 0)
	{
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}

	// ���� �����̸� ��� ���ۿ� '-'�� �߰��ϰ� ����� ��ȯ
	if (lValue < 0)
	{
		i = 1;
		pcBuffer[0] = '-';
		lValue = -lValue;
	}
	else
	{
		i = 0;
	}

	// ���ۿ� 1�� �ڸ����� 10, 100, 1000 ...�� �ڸ� ������ ���� ����
	for (; lValue > 0; i++)
	{
		pcBuffer[i] = '0' + lValue % 10;
		lValue = lValue / 10;
	}
	pcBuffer[i] = '\0';

	// ���ۿ� ����ִ� ���ڿ��� ����� ... 1000, 100, 10, 1�� �ڸ� ������ ����
	if (pcBuffer[0] == '-')
	{
		// ������ ���� ��ȣ�� �����ϰ� ���ڿ��� ������
		ReverseString(&(pcBuffer[1]));
	}
	else
	{
		ReverseString(pcBuffer);
	}

	return i;
}

bool GUIConsole::Start()
{
	BYTE bKey;

	CONSOLEMANAGER* pstConsoleManager;

	// �ܼ��� �����ϴ� �ڷᱸ���� ��ȯ
	pstConsoleManager = GetConsoleManager();


	// ������Ʈ ���

	PrintPrompt();
	//pConsole->Printf( CONSOLESHELL_PROMPTMESSAGE );

	// �ܼ� �� ���� �÷��װ� TRUE�� �� ������ �ݺ�
	while (pstConsoleManager->bExit == FALSE)
	{
		QWORD topWindowId = 0;
		Syscall_GetTopWindowID(&topWindowId);
		if (GetWindowId() != topWindowId)
		{
			Syscall_Sleep(1);
			continue;
		}

		bKey = GetCh();

		// �ܼ� �� ���� �÷��װ� ������ ��� ������ ����
		if (pstConsoleManager->bExit == TRUE)
		{
			break;
		}

		if (bKey == KEY_BACKSPACE)
		{
			if (iCommandBufferIndex > 0)
			{
				// ���� Ŀ�� ��ġ�� �� �� ���� ������ �̵��� ���� ������ ����ϰ� 
				// Ŀ�ǵ� ���ۿ��� ������ ���� ����
				GetCursor(&iCursorX, &iCursorY);
				PrintStringXY(iCursorX - 1, iCursorY, " ");
				SetCursor(iCursorX - 1, iCursorY);
				iCommandBufferIndex--;
			}
		}
		else if (bKey == KEY_ENTER || bKey == 0x0d)
		{
			Printf("\n");

			if (iCommandBufferIndex > 0)
			{
				// Ŀ�ǵ� ���ۿ� �ִ� ����� ����
				vcCommandBuffer[iCommandBufferIndex] = '\0';
				ExecuteCommand(vcCommandBuffer);
			}
			else
			{
				PrintPrompt();
			}


			//pConsole->Printf( "%s", CONSOLESHELL_PROMPTMESSAGE );
			memset(vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
			iCommandBufferIndex = 0;
		}
		// ����Ʈ Ű, CAPS Lock, NUM Lock, Scroll Lock�� ����
		else if ((bKey == KEY_LSHIFT) || (bKey == KEY_RSHIFT) ||
			(bKey == KEY_CAPSLOCK) || (bKey == KEY_NUMLOCK) ||
			(bKey == KEY_SCROLLLOCK))
		{
			;
		}
		else if (bKey < 128)
		{
			// TAB�� �������� ��ȯ
			if (bKey == KEY_TAB)
			{
				bKey = ' ';
			}

			// ���۰� �������� ���� ����
			if (iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT)
			{
				vcCommandBuffer[iCommandBufferIndex++] = (char)bKey;
				Printf("%c", (char)bKey);
			}
		}

		Syscall_Sleep(1);
	}
	return false;
}

QWORD GUIConsole::IToA(QWORD lValue, char* pcBuffer, int iRadix)
{
	QWORD iReturn;

	switch (iRadix)
	{
		// 16����
	case 16:
		iReturn = HexToString(lValue, pcBuffer);
		break;

		// 10���� �Ǵ� ��Ÿ
	case 10:
	default:
		iReturn = DecimalToString(lValue, pcBuffer);
		break;
	}

	return iReturn;
}

/**
*  vsprintf() �Լ��� ���� ����
*      ���ۿ� ���� ���ڿ��� ���� �����͸� ����
*/
int GUIConsole::VSPrintf(char* pcBuffer, const char* pcFormatString, va_list ap)
{
	QWORD i, k;
	int iBufferIndex = 0;
	int iFormatLength, iCopyLength;
	char* pcCopyString;
	QWORD qwValue;
	int iValue;
	double dValue;

	// ���� ���ڿ��� ���̸� �о ���ڿ��� ���̸�ŭ �����͸� ��� ���ۿ� ���
	iFormatLength = strlen(pcFormatString);
	for (i = 0; i < iFormatLength; i++)
	{
		// %�� �����ϸ� ������ Ÿ�� ���ڷ� ó��
		if (pcFormatString[i] == '%')
		{
			// % ������ ���ڷ� �̵�
			i++;
			switch (pcFormatString[i])
			{
				// ���ڿ� ���  
			case 's':
				// ���� ���ڿ� ����ִ� �Ķ���͸� ���ڿ� Ÿ������ ��ȯ
				pcCopyString = (char*)(va_arg(ap, char*));
				iCopyLength = strlen(pcCopyString);
				// ���ڿ��� ���̸�ŭ�� ��� ���۷� �����ϰ� ����� ���̸�ŭ 
				// ������ �ε����� �̵�
				memcpy(pcBuffer + iBufferIndex, pcCopyString, iCopyLength);
				iBufferIndex += iCopyLength;
				break;

				// ���� ���
			case 'c':
				// ���� ���ڿ� ����ִ� �Ķ���͸� ���� Ÿ������ ��ȯ�Ͽ� 
				// ��� ���ۿ� �����ϰ� ������ �ε����� 1��ŭ �̵�
				pcBuffer[iBufferIndex] = (char)(va_arg(ap, int));
				iBufferIndex++;
				break;

				// ���� ���
			case 'd':
			case 'i':
				// ���� ���ڿ� ����ִ� �Ķ���͸� ���� Ÿ������ ��ȯ�Ͽ�
				// ��� ���ۿ� �����ϰ� ����� ���̸�ŭ ������ �ε����� �̵�
				iValue = (int)(va_arg(ap, int));
				iBufferIndex += (long)IToA(iValue, pcBuffer + iBufferIndex, 10);
				break;

				// 4����Ʈ Hex ���
			case 'x':
			case 'X':
				// ���� ���ڿ� ����ִ� �Ķ���͸� DWORD Ÿ������ ��ȯ�Ͽ�
				// ��� ���ۿ� �����ϰ� ����� ���̸�ŭ ������ �ε����� �̵�
				qwValue = (DWORD)(va_arg(ap, DWORD)) & 0xFFFFFFFF;
				iBufferIndex += (long)IToA(qwValue, pcBuffer + iBufferIndex, 16);
				break;

				// 8����Ʈ Hex ���
			case 'q':
			case 'Q':
			case 'p':
				// ���� ���ڿ� ����ִ� �Ķ���͸� QWORD Ÿ������ ��ȯ�Ͽ�
				// ��� ���ۿ� �����ϰ� ����� ���̸�ŭ ������ �ε����� �̵�
				qwValue = (QWORD)(va_arg(ap, QWORD));
				iBufferIndex += (long)IToA(qwValue, pcBuffer + iBufferIndex, 16);
				break;

				// �Ҽ��� ��° �ڸ����� �Ǽ��� ���
			case 'f':
				dValue = (double)(va_arg(ap, double));
				// ��° �ڸ����� �ݿø� ó��
				dValue += 0.005;
				// �Ҽ��� ��° �ڸ����� ���ʷ� �����Ͽ� ���۸� ������
				pcBuffer[iBufferIndex] = '0' + (QWORD)(dValue * 100) % 10;
				pcBuffer[iBufferIndex + 1] = '0' + (QWORD)(dValue * 10) % 10;
				pcBuffer[iBufferIndex + 2] = '.';
				for (k = 0; ; k++)
				{
					// ���� �κ��� 0�̸� ����
					if (((QWORD)dValue == 0) && (k != 0))
					{
						break;
					}
					pcBuffer[iBufferIndex + 3 + k] = '0' + ((QWORD)dValue % 10);
					dValue = dValue / 10;
				}
				pcBuffer[iBufferIndex + 3 + k] = '\0';
				// ���� ����� ���̸�ŭ ������ ���̸� ������Ŵ
				ReverseString(pcBuffer + iBufferIndex);
				iBufferIndex += (int)(3 + k);
				break;

				// ���� �ش����� ������ ���ڸ� �״�� ����ϰ� ������ �ε�����
				// 1��ŭ �̵�
			default:
				pcBuffer[iBufferIndex] = pcFormatString[i];
				iBufferIndex++;
				break;
			}
		}
		// �Ϲ� ���ڿ� ó��
		else
		{
			// ���ڸ� �״�� ����ϰ� ������ �ε����� 1��ŭ �̵�
			pcBuffer[iBufferIndex] = pcFormatString[i];
			iBufferIndex++;
		}
	}

	// NULL�� �߰��Ͽ� ������ ���ڿ��� ����� ����� ������ ���̸� ��ȯ
	pcBuffer[iBufferIndex] = '\0';
	return iBufferIndex;
}

void GUIConsole::Printf(const char* pcFormatString, ...)
{
	va_list ap;
	char vcBuffer[1024];
	int iNextPrintOffset;

	// ���� ���� ����Ʈ�� ����ؼ� vsprintf()�� ó��
	va_start(ap, pcFormatString);
	VSPrintf(vcBuffer, pcFormatString, ap);
	va_end(ap);

	// ���� ���ڿ��� ȭ�鿡 ���
	iNextPrintOffset = ConsolePrintString(vcBuffer);

	// Ŀ���� ��ġ�� ������Ʈ
	SetCursor(iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH);
}

/**
*  ���ڿ��� X, Y ��ġ�� ���
*/
void GUIConsole::PrintStringXY(int iX, int iY, const char* pcString)
{
	CHARACTER* pstScreen;
	int i;

	// ȭ�� ���۸� ����
	pstScreen = m_ConsoleManager.pstScreenBuffer;

	// ���� ����� ��ġ�� ���
	pstScreen += (iY * CONSOLE_WIDTH) + iX;
	// ���ڿ��� ���̸�ŭ ������ ���鼭 ���ڿ� �Ӽ��� ����
	for (i = 0; pcString[i] != 0; i++)
	{
		pstScreen[i].bCharacter = pcString[i];
		pstScreen[i].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
	}
}

/**
*  getch() �Լ��� ����
*/
BYTE GUIConsole::GetCh(void)
{
	KEYDATA stData;

	// Ű�� ������ ������ �����
	while (1)
	{

		while (GetKeyFromGUIKeyQueue(&stData) == FALSE)
		{
			// �׷��� ��忡�� �����ϴ� �߿� �� �½�ũ�� �����ؾߵ� ��� ������ ����
			if (m_ConsoleManager.bExit == TRUE)
			{
				return 0xFF;
			}
			//kSchedule();

			Syscall_Sleep(1);
		}

		// Ű�� ���ȴٴ� �����Ͱ� ���ŵǸ� ASCII �ڵ带 ��ȯ
		if (stData.bFlags & KEY_FLAGS_DOWN)
		{
			return stData.bASCIICode;
		}
	}
}

/**
*  �ܼ��� �����ϴ� �ڷᱸ���� ��ȯ
*/
CONSOLEMANAGER* GUIConsole::GetConsoleManager(void)
{
	return &m_ConsoleManager;
}

/**
*  �׷��� ���� Ű ť���� Ű �����͸� ����
*/
bool GUIConsole::GetKeyFromGUIKeyQueue(KEYDATA* pstData)
{
	bool bResult;

	// ť�� �����Ͱ� ������ ����
	if (kIsQueueEmpty(&(m_ConsoleManager.stKeyQueueForGUI)) == TRUE)
	{
		return FALSE;
	}
	
	Syscall_LockMutex(m_ConsoleManager.mutex);
	bResult = kGetQueue(&(m_ConsoleManager.stKeyQueueForGUI), pstData);
	Syscall_UnlockMutex(m_ConsoleManager.mutex);
	
	return bResult;
}

/**
*  �׷��� ���� Ű ť�� �����͸� ����
*/
bool GUIConsole::PutKeyToGUIKeyQueue(KEYDATA* pstData)
{
	bool bResult;

	// ť�� �����Ͱ� ���� á���� ����
	if (kIsQueueFull(&(m_ConsoleManager.stKeyQueueForGUI)) == TRUE)
	{
		return FALSE;
	}

	
	// ť�� �����͸� ����
	Syscall_LockMutex(m_ConsoleManager.mutex);
	bResult = kPutQueue(&(m_ConsoleManager.stKeyQueueForGUI), pstData);
	Syscall_UnlockMutex(m_ConsoleManager.mutex);
	
	return bResult;
}

/**
*  �ܼ� �� �½�ũ ���� �÷��׸� ����
*/
void GUIConsole::SetConsoleShellExitFlag(bool bFlag)
{
	m_ConsoleManager.bExit = bFlag;
}

//
//  ȭ�� ������ ����� ������ GUI �ܼ� �� ������ ȭ�鿡 ������Ʈ
//
void GUIConsole::ProcessConsoleBuffer(QWORD qwWindowID, CHARACTER* vstPreviousScreenBuffer)
{
	int i;
	int j;
	CONSOLEMANAGER* pstManager;
	CHARACTER* pstScreenBuffer;
	CHARACTER* pstPreviousScreenBuffer;
	RECT stLineArea;
	bool bChanged;
	static DWORD dwLastTickCount = 0;
	bool bFullRedraw;

	// �ܼ��� �����ϴ� �ڷᱸ���� ��ȯ �޾� ȭ�� ������ ��巹���� �����ϰ� 
	// ���� ȭ�� ������ ��巹���� ����
	pstManager = GetConsoleManager();
	pstScreenBuffer = pstManager->pstScreenBuffer;
	pstPreviousScreenBuffer = vstPreviousScreenBuffer;

	// ȭ���� ��ü�� ������Ʈ �� �� 5�ʰ� �������� ������ ȭ�� ��ü�� �ٽ� �׸�
	if (Syscall_GetTickCount() - dwLastTickCount > 5000)
	{
		dwLastTickCount = Syscall_GetTickCount();
		bFullRedraw = TRUE;
	}
	else
	{
		bFullRedraw = FALSE;
	}
	//bFullRedraw = TRUE;

	// ȭ�� ������ ���̸�ŭ �ݺ�
	for (j = 0; j < CONSOLE_HEIGHT; j++)
	{
		// ó������ ������� ���� ������ �÷��� ����
		bChanged = FALSE;

		// ���� ���ο� ��ȭ�� �ִ��� ���Ͽ� ���� ���� �÷��׸� ó��
		for (i = 0; i < CONSOLE_WIDTH; i++)
		{
			// ���ڸ� ���Ͽ� �ٸ��ų� ��ü�� ���� �׷��� �ϸ� ���� ȭ�� ���ۿ�
			// ������Ʈ�ϰ� ���� ���� �÷��׸� ����
			if ((pstScreenBuffer->bCharacter != pstPreviousScreenBuffer->bCharacter) ||
				(bFullRedraw == TRUE))
			{
				// ���ڸ� ȭ�鿡 ���
				POINT loc;
				loc.iX = i * FONT_ENGLISHWIDTH + 2;
				loc.iY = j * FONT_ENGLISHHEIGHT + WINDOW_TITLEBAR_HEIGHT;

				TEXTCOLOR textColor;
				textColor.textColor = RGB(0, 255, 0);
				textColor.backgroundColor = RGB(0, 0, 0);
				Syscall_DrawText(&qwWindowID, &loc, &textColor, (const char*)&(pstScreenBuffer->bCharacter), 1);

				// ���� ȭ�� ���۷� ���� �Ű� ���� ���� ���ο� ������
				// �ٸ� �����Ͱ� ������ ǥ��
				memcpy(pstPreviousScreenBuffer, pstScreenBuffer, sizeof(CHARACTER));
				bChanged = TRUE;
			}

			pstScreenBuffer++;
			pstPreviousScreenBuffer++;
		}

		// ���� ���ο��� ����� �����Ͱ� �ִٸ� ���� ���θ� ȭ�鿡 ������Ʈ
		if (bChanged == TRUE)
		{
			// ���� ������ ������ ���
			SetRectangleData(2, j * FONT_ENGLISHHEIGHT + WINDOW_TITLEBAR_HEIGHT,
				5 + FONT_ENGLISHWIDTH * CONSOLE_WIDTH, (j + 1) * FONT_ENGLISHHEIGHT +
				WINDOW_TITLEBAR_HEIGHT - 1, &stLineArea);
			// �������� �Ϻθ� ȭ�� ������Ʈ
			Syscall_UpdateScreenByWindowArea(&qwWindowID, &stLineArea);
		}
	}
}

/**
*  \n, \t�� ���� ���ڰ� ���Ե� ���ڿ��� ����� ��, ȭ����� ���� ����� ��ġ��
*  ��ȯ
*/
int GUIConsole::ConsolePrintString(const char* pcBuffer)
{
	CHARACTER* pstScreen;
	int i, j;
	int iLength;
	int iPrintOffset;

	// ȭ�� ���۸� ����
	pstScreen = m_ConsoleManager.pstScreenBuffer;

	// ���ڿ��� ����� ��ġ�� ����
	iPrintOffset = m_ConsoleManager.iCurrentPrintOffset;

	// ���ڿ��� ���̸�ŭ ȭ�鿡 ���
	iLength = strlen(pcBuffer);
	for (i = 0; i < iLength; i++)
	{
		// ���� ó��
		if (pcBuffer[i] == '\n')
		{
			// ����� ��ġ�� 80�� ��� �÷����� �ű�
			// ���� ������ ���� ���ڿ��� ����ŭ ���ؼ� ���� �������� ��ġ��Ŵ
			iPrintOffset += (CONSOLE_WIDTH - (iPrintOffset % CONSOLE_WIDTH));
		}
		// �� ó��
		else if (pcBuffer[i] == '\t')
		{
			// ����� ��ġ�� 8�� ��� �÷����� �ű�
			iPrintOffset += (8 - (iPrintOffset % 8));
		}
		// �Ϲ� ���ڿ� ���
		else
		{
			// ���� �޸𸮿� ���ڿ� �Ӽ��� �����Ͽ� ���ڸ� ����ϰ�
			// ����� ��ġ�� �������� �̵�
			pstScreen[iPrintOffset].bCharacter = pcBuffer[i];
			pstScreen[iPrintOffset].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
			iPrintOffset++;
		}

		// ����� ��ġ�� ȭ���� �ִ�(80 * 25)�� ������� ��ũ�� ó��
		if (iPrintOffset >= (CONSOLE_HEIGHT * CONSOLE_WIDTH))
		{
			// ���� ������ ������ �������� �� �� ���� ����
			memcpy(pstScreen, pstScreen + CONSOLE_WIDTH,
				(CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH * sizeof(CHARACTER));

			// ���� ������ ������ �������� ä��
			for (j = (CONSOLE_HEIGHT - 1) * (CONSOLE_WIDTH);
				j < (CONSOLE_HEIGHT * CONSOLE_WIDTH); j++)
			{
				// ���� ���
				pstScreen[j].bCharacter = ' ';
				pstScreen[j].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
			}

			// ����� ��ġ�� ���� �Ʒ��� ������ ó������ ����
			iPrintOffset = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
		}
	}
	return iPrintOffset;
}

bool GUIConsole::ProcessKey(BYTE bKey)
{
	if (bKey == KEY_BACKSPACE)
	{
		if (iCommandBufferIndex > 0)
		{
			// ���� Ŀ�� ��ġ�� �� �� ���� ������ �̵��� ���� ������ ����ϰ� 
			// Ŀ�ǵ� ���ۿ��� ������ ���� ����
			GetCursor(&iCursorX, &iCursorY);
			PrintStringXY(iCursorX - 1, iCursorY, " ");
			SetCursor(iCursorX - 1, iCursorY);
			iCommandBufferIndex--;
		}
	}
	else if (bKey == KEY_ENTER || bKey == 0x0d)
	{
		Printf("\n");
		memset(vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
		iCommandBufferIndex = 0;
	}
	// ����Ʈ Ű, CAPS Lock, NUM Lock, Scroll Lock�� ����
	else if ((bKey == KEY_LSHIFT) || (bKey == KEY_RSHIFT) ||
		(bKey == KEY_CAPSLOCK) || (bKey == KEY_NUMLOCK) ||
		(bKey == KEY_SCROLLLOCK))
	{
		return false;
	}
	else if (bKey < 128)
	{
		// TAB�� �������� ��ȯ
		if (bKey == KEY_TAB)
		{
			bKey = ' ';
		}

		// ���۰� �������� ���� ����
		if (iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT)
		{
			vcCommandBuffer[iCommandBufferIndex++] = (char)bKey;
			Printf("%c", (char)bKey);
		}
	}

	return true;
}

/*
*  Ŀ�ǵ� ���ۿ� �ִ� Ŀ�ǵ带 ���Ͽ� �ش� Ŀ�ǵ带 ó���ϴ� �Լ��� ����
*/
void GUIConsole::ExecuteCommand(const char* pcCommandBuffer)
{
	manager.RunCommand(pcCommandBuffer);

	EVENT stEvent;
	stEvent.qwType = EVENT_CONSOLE_COMMAND_END;

	stEvent.stWindowEvent.qwWindowID = m_windowId;
	Syscall_SendEventToWindow(&m_windowId, &stEvent);

	/*int i, iSpaceIndex;
	int iCommandBufferLength, iCommandLength;	

	// �������� ���е� Ŀ�ǵ带 ����
	iCommandBufferLength = strlen(pcCommandBuffer);
	for (iSpaceIndex = 0; iSpaceIndex < iCommandBufferLength; iSpaceIndex++)
	{
		if (pcCommandBuffer[iSpaceIndex] == ' ')
		{
			break;
		}
	}
	
	for (i = 0; i < m_commandCount; i++)
	{
		iCommandLength = strlen(gs_vstCommandTable[i].pcCommand);
		// Ŀ�ǵ��� ���̿� ������ ������ ��ġ�ϴ��� �˻�
		if ((iCommandLength == iSpaceIndex) &&
			(memcmp(gs_vstCommandTable[i].pcCommand, pcCommandBuffer,
				iSpaceIndex) == 0))
		{
			gs_vstCommandTable[i].pfFunction(this, pcCommandBuffer + iSpaceIndex + 1);
			break;
		}
	}

	// ����Ʈ���� ã�� �� ���ٸ� ���� ���
	if (i >= m_commandCount)
	{
		int result = Syscall_CreateProcess(pcCommandBuffer, nullptr, 16);

		if(result == 0)
			Printf("'%s' is not found.\n", pcCommandBuffer);
		else
			Printf("result %d\n", result);
	}*/
}

