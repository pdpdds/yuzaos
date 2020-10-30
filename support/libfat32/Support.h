#pragma once
#include <string.h>

#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define lstrcpy(des,src) strcpy((des),(src))

#define _istspace   _ismbcspace
#define _PUC    unsigned char *
#define _CPUC   const unsigned char *
#define _PC     char *
#define _CPC    const char *
#define _UI     unsigned int

extern "C" int _ismbcspace(int c);
extern "C" unsigned char* _mbsinc(const unsigned char* current);
extern "C" char* _mbsupr(char* str);


__inline _PC _tcsinc(_CPC _s1) { return (_PC)_mbsinc((_CPUC)_s1); }
__inline _PC _tcsupr(_PC _s1) { return (_PC)_mbsupr((_PC)_s1); }
__inline _PC _tcsstr(_CPC _s1, _CPC _s2) { return (_PC)strstr((_CPC)_s1, (_CPC)_s2); }

//#define LOWORD(l)           ((WORD)(l))
//#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
//#define LOBYTE(w)           ((BYTE)(w))
//#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))