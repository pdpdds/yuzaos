#pragma once
#include <minwindef.h>
#include <stdbool.h>
#include "key_def.h"
/*
 * API CALLs
 */

#pragma pack(1)
typedef struct _BITMAP_INFO {
	DWORD		width;
	DWORD		height;
	WORD		color_depth;
} BITMAP_INFO, * PBITMAP_INFO;

typedef struct _RGB_COLOR {
	union {
		struct {
			BYTE	b;
			BYTE	g;
			BYTE	r;
			BYTE	alpha;
		} element;

		DWORD		rgb;
	};
} RGB_COLOR, * PRGB_COLOR;
#pragma pack()

/* system api */
VOID API_ExitProgram(VOID);
DWORD API_GetTickCount(VOID);
VOID API_Delay(DWORD MilliSec); /* 1000 = 1sec */


/* screen associated */
VOID API_ClearScreen(VOID);
VOID API_PrintText(BYTE *pText);
VOID API_PrintTextXY(BYTE *pText, WORD x, WORD y);

BOOL API_GetKey(KBD_KEY_DATA* pKeyData);


HANDLE			API_LoadBitmap(char *pFilename);
BOOL			API_GetBitmapInfo(HANDLE hBitmap, BITMAP_INFO *pBitmapInfo);
BOOL			API_BitBlt(HANDLE hBitmap, WORD scrX, WORD scrY, DWORD startImgX, DWORD startImgY,
						   DWORD cxToBeDisplayed, DWORD cyToBeDisplayed, RGB_COLOR *pMaskColor); /* pMaskColor = optional */
VOID			API_CloseBitmapHandle(HANDLE hBitmap);