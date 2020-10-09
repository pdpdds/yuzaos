#include "GUIConsole.h"
#include "SkyRenderer.h"
#include "memory.h"
#include "sprintf.h"
#include "string.h"
#include  "Keyboard.h"
#include "InputQueue.h"
#include <Mouse.h>
#include <systemcall_impl.h>
#include <ConsoleManager.h>
#include <UnicodeFont.h>
#include <HangulProcessor.h>
#include <string>

extern "C" __declspec(dllexport) I_GUIEngine* GetGUIEngine()
{
	return new GUIConsole();
}

#define TIMEOUT_PER_SECOND		1000
static bool m_bShowTSWatchdogClock = true;

void SampleFillRect(ULONG* lfb, int width, int x, int y, int w, int h, int col)
{
	for (int k = 0; k < h; k++)
		for (int j = 0; j < w; j++)
		{
			int index = ((j + x) + (k + y) * width);
			lfb[index] = col;
		}
}

DWORD WINAPI GUIWatchDogProc(LPVOID parameter)
{
	int pos = 0;

	GUIConsole* pConsole = (GUIConsole*)parameter;

	int colorStatus[] = { 0x00FF0000, 0x0000FF00, 0x0000FF };
	int first = Syscall_GetTickCount();

	//그래픽 버퍼 주소를 얻는다.
	ULONG* lfb = (ULONG*)pConsole->m_linearBufferInfo.pBuffer;

	//루프를 돌면서 오른쪽 상단에 사각형을 그린다.
	while (1)
	{
		int second = Syscall_GetTickCount();
		//1초 단위로 색상을 변경한다.
		if (second - first >= TIMEOUT_PER_SECOND)
		{
			if (++pos > 2)
				pos = 0;

			if (m_bShowTSWatchdogClock)
			{
				SampleFillRect(lfb, pConsole->GetWidth(), pConsole->GetWidth() - 20, 0, 20, 20, colorStatus[pos]);
			}

			first = Syscall_GetTickCount();
		}

		Syscall_Sleep(1000);
	}

	return 0;
}

#define COLOR(r,g,b) ((r<<16) | (g<<8) | b)
#define WHITE COLOR(255,255,255)
#define BLACK COLOR(0,0,0)
#define DARKGRAY COLOR(154,154,154)


extern QUEUE gs_stKeyQueue;
extern QUEUE gs_stMouseQueue;

#define PIVOT_X 0
#define PIVOT_Y 0
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16
#define CHAR_COLOR 0xff

GUIConsole::GUIConsole()
{
	m_width = 0;
	m_height = 0;
	m_bpp = 0;

	m_xPos = PIVOT_X;
	m_yPos = PIVOT_Y;

	m_pVideoRamPtr = nullptr;
	m_pUnicodeFont = nullptr;
}


GUIConsole::~GUIConsole()
{
}


void GUIConsole::SetLinearBuffer(LinearBufferInfo& linearBufferInfo)
{
	m_linearBufferInfo = linearBufferInfo;
}



bool GUIConsole::Initialize()
{		
	kInitializeKeyboard();
	kInitializeMouse();

	m_pUnicodeFont = new UnicodeFont();
	m_pUnicodeFont->Initialize();

	m_pHangulProcessor = new HangulProcessor();
		
	m_pVideoRamPtr = (ULONG*)m_linearBufferInfo.pBuffer;
	m_width = m_linearBufferInfo.width;
	m_height = m_linearBufferInfo.height;
	m_bpp = m_linearBufferInfo.depth;

	unsigned char buf[512];
	sprintf((char*)buf, "XRes : %d", m_width);
	Print2((char*)buf);

	sprintf((char*)buf, "YRes : %d", m_height);
	Print2((char*)buf);

	sprintf((char*)buf, "BitsPerPixel : %d", m_bpp);
	Print2((char*)buf);

	sprintf((char*)buf, "Ram Virtual Address : %x", (uint32_t)m_pVideoRamPtr);
	Print2((char*)buf);

	Print2("SkyOS에 오신걸 환영합니다!!");
	Print2("Welcome to SkyOS!!");
	Print2("歡迎來到SkyOS!!");
	Print2("SkyOSへようこそ!!");

	return true;
}

bool GUIConsole::Print2(char* pMsg)
{
	FillRect(m_xPos, m_yPos, CHAR_WIDTH, CHAR_HEIGHT, 0x00);
	
	m_pUnicodeFont->PutFonts((char*)m_pVideoRamPtr, m_width, m_xPos, m_yPos, 0xffffffff, (unsigned char*)pMsg);
	GetNewLine();

	//PutCursor();

	return true;
}

bool GUIConsole::Run()
{
	ConsoleManager manager;

	Syscall_CreateThread(GUIWatchDogProc,"guiwatchdog", this, 16, 0);
	int bufferLen = (m_width / CHAR_WIDTH) - 15;
	char* commandBuffer = new char[bufferLen];

	while (1)
	{
		Print(0, "C:\\>");
		memset(commandBuffer, 0, bufferLen);

		m_pHangulProcessor->Reset();
		
		GetCommandForGUI(commandBuffer, MAXPATH - 2);
		
		if (manager.RunCommand(commandBuffer) == true)
			break;
	}

	return false;
}

void GUIConsole::Update(float deltaTime)
{

}

bool GUIConsole::PutKeyboardQueue(KEYDATA* pData)
{
	return kPutQueue(&gs_stKeyQueue, pData);
}

bool GUIConsole::PutMouseQueue(MOUSEDATA* pData)
{
	return kPutQueue(&gs_stMouseQueue, pData);
}


VOID GUIConsole::GetNewLine()
{
	int x, y;
	ULONG *buf = m_pVideoRamPtr;	
	int bxsize = m_width;
	if ((m_yPos + PIVOT_Y + CHAR_HEIGHT) < m_height) 
	{
		m_yPos += CHAR_HEIGHT; //커서를 다음행으로 옮긴다.
	}
	else
	{
		//화면을 스크롤한다.
		for (y = PIVOT_Y; y < (m_height - CHAR_HEIGHT - PIVOT_Y); y++)
		{
			for (x = PIVOT_X; x < m_width; x++) 
			{
				buf[x + y * bxsize] = buf[x + (y + CHAR_HEIGHT) * bxsize];
			}
		}
		for (y = m_height - CHAR_HEIGHT - PIVOT_Y; y < (m_height - PIVOT_Y); y++) 
		{
			for (x = PIVOT_X; x < m_width; x++) 
			{
				buf[x + y * bxsize] = 0x00000000;
			}
		}
	}

	m_xPos = PIVOT_X;
}
bool GUIConsole::Clear()
{
	m_xPos = PIVOT_X;
	m_yPos = PIVOT_Y;

	memset(m_pVideoRamPtr, BLACK, (m_width * m_height) * sizeof(ULONG));

	return true;
}

ULONG GUIConsole::GetBPP()
{
	return m_bpp;
}

void GUIConsole::PutPixel(ULONG x, ULONG y, ULONG col)
{
	m_pVideoRamPtr[(y * m_width) + x] = col;
}

ULONG GUIConsole::GetPixel(ULONG i) {
	return m_pVideoRamPtr[i];
}

void GUIConsole::PutPixel(ULONG i, ULONG col) {
	m_pVideoRamPtr[i] = col;
}

void GUIConsole::FillRect(int x, int y, int w, int h, int col)
{
	unsigned* lfb = (unsigned*)m_pVideoRamPtr;
	for (int k = 0; k < h; k++)
		for (int j = 0; j < w; j++)
		{
			int index = ((j + x) + (k + y) * m_width);
			lfb[index] = col;
		}
}

bool GUIConsole::Print(QWORD taskId, char* pMsg)
{	
	//백스페이스
	if (strlen(pMsg) == 1 && pMsg[0] == 0x08) 
	{
		if (m_xPos > 9 * 8)
		{
			FillRect(m_xPos, m_yPos, CHAR_WIDTH, CHAR_HEIGHT, 0x00);
			m_xPos -= 1 * 8;
			FillRect(m_xPos, m_yPos, CHAR_WIDTH, CHAR_HEIGHT, 0x00);

			PutCursor();
		}
		
		return true;
	}

	FillRect(m_xPos, m_yPos, CHAR_WIDTH, CHAR_HEIGHT, 0x00);
	
	unsigned char *s = (unsigned char*)pMsg;
	for (; *s != 0x00; s++)
	{
		if (*s == '\n')
		{
			GetNewLine();
			continue;
		}

		char buf[2] = { 0, };
		buf[0] = *s;
		int width = m_pUnicodeFont->PutFonts((char*)m_pVideoRamPtr, m_width, m_xPos, m_yPos, 0xffffffff, (unsigned char*)buf);
		
		m_xPos += width;
	}	
	
	PutCursor();
	
	return true;
}

void GUIConsole::PutCursor()
{
	FillRect(m_xPos, m_yPos + (CHAR_HEIGHT - 4), CHAR_WIDTH, 4, WHITE);
}

void GUIConsole::PutPixel(ULONG i, unsigned char r, unsigned char g, unsigned char b) {
	m_pVideoRamPtr[i] = (r << 16) | (g << 8) | b;
}

void GUIConsole::GetCommandForGUI(char* commandBuffer, int bufSize)
{
	char c = 0;
	bool	BufChar;

	//! get command string
	int i = 0;
	while (i < bufSize) {

		//! buffer the next char
		BufChar = true;
		

		KEYDATA keyData;

		if (kGetKeyFromKeyQueue(&keyData) == false)
		{
			Syscall_Sleep(0);
			continue;
		}

		if ((keyData.bFlags & KEY_FLAGS_DOWN) == false)
			continue;

		c = keyData.bASCIICode;

		//return
		if (c == 0x0d || c =='\n')
		{
			Print(0, "\n");
			break;
		}
		//backspace
		if (c == 0x08) {

			//! dont buffer this char
			BufChar = false;

			if (i > 0) {

				m_pHangulProcessor->InputAscii(c);
				i = m_pHangulProcessor->GetString(commandBuffer);

				FillRect(0, m_yPos, m_width, CHAR_HEIGHT, 0x00);

				m_xPos = m_pUnicodeFont->PutFonts((char*)m_pVideoRamPtr, m_width, PIVOT_X, m_yPos, 0xffffffff, (unsigned char*)"C:\\>");

				int width = 0;
				if(i > 0)
					width = m_pUnicodeFont->PutFonts((char*)m_pVideoRamPtr, m_width, m_xPos, m_yPos, 0xffffffff, (unsigned char*)commandBuffer);
				else
					memset(commandBuffer, 0, bufSize);

				m_xPos += width;
				PutCursor();

			}
		}

		if (c == 0x85) {

			//! dont buffer this char
			BufChar = false;

			m_pHangulProcessor->InputAscii(c);
		}

		//! only add the char if it is to be buffered
		if (BufChar) 
		{	
			//if (c != 0 && KEY_SPACE != c) { //insure its an ascii char
			if (c != 0)
			{ 

				FillRect(m_xPos, m_yPos, CHAR_WIDTH, CHAR_HEIGHT, 0x00);
				unsigned char buf[512];
				sprintf((char*)buf, "%c\n", c);
				unsigned char charColor = 0xff;

				m_pHangulProcessor->InputAscii(c);
				i = m_pHangulProcessor->GetString(commandBuffer);
				
				FillRect(0, m_yPos, m_width, CHAR_HEIGHT, 0x00);
				
				m_xPos = m_pUnicodeFont->PutFonts((char*)m_pVideoRamPtr, m_width, PIVOT_X, m_yPos, 0xffffffff, (unsigned char*)"C:\\>");
				int width = m_pUnicodeFont->PutFonts((char*)m_pVideoRamPtr, m_width, m_xPos, m_yPos, 0xffffffff, (unsigned char*)commandBuffer);
				
				m_xPos += width;
				PutCursor();
			}
		}

		//! wait for next key. You may need to adjust this to suite your needs
		//msleep(10);
	}

	//! null terminate the string
	commandBuffer[i] = 0;
}

