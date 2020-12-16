#include "chobits_api.h"
#include <systemcall_impl.h>
#include <stdlib.h>
#include <skyoswindow.h>
/**********************************************************************************************************
 *                                            SYSTEM API CALLS                                            *
 **********************************************************************************************************/
VOID API_ExitProgram(VOID)
{
	exit(0);
}

DWORD API_GetTickCount(VOID)
{
	return Syscall_GetTickCount();
}

VOID API_Delay(DWORD MilliSec)
{
	Syscall_Sleep(MilliSec);
}


/**********************************************************************************************************
 *                                             SCREEN API CALLS                                           *
 **********************************************************************************************************/
VOID API_ClearScreen(VOID)
{

}

VOID API_PrintText(BYTE *pText)
{

}

VOID API_PrintTextXY(BYTE *pText, WORD x, WORD y)
{
	TEXTCOLOR textColor;
	textColor.textColor = RGB(255, 255, 255);
	textColor.backgroundColor = RGB(55, 215, 47);
	POINT point;
	point.iX = x;
	point.iY = y;
	Syscall_DrawText(&qwWindowID, &point, &textColor, pText, strlen(pText));
}

BOOL API_GetKey(KBD_KEY_DATA *pKeyData)
{
	int result;
	pKeyData->key = (BYTE)(result & 0x000000ff);
	pKeyData->type = (BYTE)((result & 0x0000ff00) >> 8);

	return TRUE;
}



HANDLE API_LoadBitmap(char *pFilename)
{
	/*HANDLE result;
	SYSCALL_MSG syscall;

	syscall.syscall_type = SYSCALL_LOAD_BITMAP;
	syscall.parameters.LOAD_BITMAP.pt_filename = pFilename;
	result = (HANDLE)internel_syscall(&syscall);*/

	return 0;
}

BOOL API_GetBitmapInfo(HANDLE hBitmap, BITMAP_INFO *pBitmapInfo)
{	
	return 0;
}

BOOL API_BitBlt(HANDLE hBitmap, WORD scrX, WORD scrY, DWORD startImgX, DWORD startImgY,
				DWORD cxToBeDisplayed, DWORD cyToBeDisplayed, RGB_COLOR *pMaskColor) /* pMaskColor = optional */
{
	

	return 0;
}

VOID API_CloseBitmapHandle(HANDLE hBitmap)
{
	
}
