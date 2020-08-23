#pragma once
#include <minwindef.h>

#ifdef HANGUL_PROCESSOR
#define HANGUL_PROCESSOR_API __declspec(dllexport)
#else
#define HANGUL_PROCESSOR_API __declspec(dllimport)
#endif

class HANGUL_PROCESSOR_API HangulProcessor
{
public:
	HangulProcessor();
	~HangulProcessor();

	bool SwitchMode();
	bool InputAscii(unsigned char letter);
	int GetString(char* buffer);
	void Reset();
	int DrawText(int iX1, int iY1, int iX2, int iY2, DWORD* pstMemoryAddress, int iX, int iY,
		DWORD stTextColor, DWORD stBackgroundColor, const char* pcString, int iLength);

protected:
	bool m_bHangulMode;
};

