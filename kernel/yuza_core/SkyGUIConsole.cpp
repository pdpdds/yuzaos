#include "SkyGUIConsole.h"
#include <SkyRenderer32.h>
#include <sprintf.h>
#include <BootParams.h>

extern BootParams g_bootParams;
namespace SkyGUIConsole
{

#define PIVOT_X 8
#define PIVOT_Y 16
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16
#define CHAR_COLOR 0xff

	unsigned char charColor = 0xff;
	ULONG* m_pVideoRamPtr;
	int m_width;
	int m_height;
	int m_bpp;

	SkyRenderer32* m_renderer;
	int m_yPos;
	int m_xPos;

	void GetNewLine()
	{
		int x, y;
		ULONG* buf = m_pVideoRamPtr;
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

	void Initialize()
	{
		LoadFontFromMemory();

		m_yPos = PIVOT_Y;
		m_xPos = PIVOT_X;

		m_width = 0;
		m_height = 0;
		m_bpp = 0;

		m_yPos = 0;
		m_xPos = 0;

		m_renderer = new SkyRenderer32();
		m_pVideoRamPtr = (ULONG*)g_bootParams.framebuffer_addr;
		m_width = g_bootParams.framebuffer_width;
		m_height = g_bootParams.framebuffer_height;
		m_bpp = g_bootParams.framebuffer_bpp;

		unsigned char buf[512];
		sprintf((char*)buf, "XRes : %d", m_width);
		
		m_renderer->PutFonts_ASC((char*)m_pVideoRamPtr, m_width, m_xPos, m_yPos, (char)charColor, buf);
		GetNewLine();

		sprintf((char*)buf, "YRes : %d", m_height);
		m_renderer->PutFonts_ASC((char*)m_pVideoRamPtr, m_width, m_xPos, m_yPos, (char)charColor, buf);
		GetNewLine();

		sprintf((char*)buf, "BitsPerPixel : %d", m_bpp);
		m_renderer->PutFonts_ASC((char*)m_pVideoRamPtr, m_width, m_xPos, m_yPos, (char)charColor, buf);
		GetNewLine();

		sprintf((char*)buf, "Ram Virtual Address : %x", (unsigned int)m_pVideoRamPtr);
		m_renderer->PutFonts_ASC((char*)m_pVideoRamPtr, m_width, m_xPos, m_yPos, (char)charColor, buf);
		GetNewLine();
	}

	void Print(char* szString)
	{
		m_renderer->PutFonts_ASC((char*)m_pVideoRamPtr, m_width, m_xPos, m_yPos, (char)charColor, (unsigned char*)szString);
		GetNewLine();
	}
}