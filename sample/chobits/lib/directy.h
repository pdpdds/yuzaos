#ifndef _DIRECT_Y_H_
#define _DIRECT_Y_H_

/**********************************************************************************************************
 *                                                INCLUDE FILES                                           *
 **********************************************************************************************************/
#include "minwindef.h"
#include "chobits_api.h"

/**********************************************************************************************************
 *                                               EXPORTED FUNCTIONS                                       *
 **********************************************************************************************************/

VOID			DyPutPixel(WORD scrX, WORD scrY, RGB_COLOR color);
VOID			DyDrawLine(WORD startX, WORD startY, WORD endX, WORD endY, RGB_COLOR color);
VOID			DyDrawRect(WORD left, WORD top, WORD right, WORD bottom, RGB_COLOR lineColor);
VOID			DyFillRect(WORD left, WORD top, WORD right, WORD bottom, RGB_COLOR fillColor);

HANDLE		DyLoadBitmap(char *pFilename);
HANDLE		DyGetPaletteHandle(HANDLE hBitmap);
BOOL			DyGetBitmapInfo(HANDLE hBitmap, BITMAP_INFO *pBitmapInfo);
BOOL			DyBitBlt(HANDLE hBitmap, WORD scrX, WORD scrY, DWORD startImgX, DWORD startImgY,
					 DWORD cxToBeDisplayed, DWORD cyToBeDisplayed, RGB_COLOR *pMaskColor); /* pMaskColor = optional */
VOID			DyCloseBitmapHandle(HANDLE hBitmap);


#endif // #ifndef _DIRECT_Y_H_