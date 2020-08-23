#pragma once
#include "hanlib.h"

#ifdef UNICODE_FONT
#define UNICODEFONT_API __declspec(dllexport)
#else
#define UNICODEFONT_API __declspec(dllimport)
#endif


class UNICODEFONT_API UnicodeFont
{
public:
	UnicodeFont();
	~UnicodeFont();

	bool Initialize();
	int PutFonts(char *vram, int xsize, int x, int y, unsigned int c, unsigned char *s);

private:
	void PutFont8x16(char *vram, int xsize, int x, int y, unsigned int c, char *font);
	void PutFont16x16(char *vram, int xsize, int x, int y, unsigned int c, char *font);
};

