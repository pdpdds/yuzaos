#pragma once
#include <windef.h>
#include <skyoswindow.h>
#include <I_SkyInput.h>

#if  defined(DLL_YUZA_API_EXPORT)
#define YUZA_API __declspec(dllexport) 
#else
#define YUZA_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

	YUZA_API bool IsInRectangle(const RECT* pstArea, int iX, int iY);
	YUZA_API int GetRectangleWidth(const RECT* pstArea);
	YUZA_API int GetRectangleHeight(const RECT* pstArea);
	YUZA_API bool GetOverlappedRectangle(const RECT* pstArea1, const RECT* pstArea2,
		RECT* pstIntersection);
	YUZA_API bool ConvertPointScreenToClient(QWORD qwWindowID, const POINT* pstXY,
		POINT* pstXYInWindow);
	YUZA_API bool ConvertPointClientToScreen(QWORD qwWindowID, const POINT* pstXY,
		POINT* pstXYInScreen);
	YUZA_API bool ConvertRectScreenToClient(QWORD qwWindowID, const RECT* pstArea,
		RECT* pstAreaInWindow);
	YUZA_API bool ConvertRectClientToScreen(QWORD qwWindowID, const RECT* pstArea,
		RECT* pstAreaInScreen);
	YUZA_API void SetRectangleData(int iX1, int iY1, int iX2, int iY2, RECT* pstRect);
	YUZA_API bool SetMouseEvent(QWORD qwWindowID, QWORD qwEventType, int iMouseX, int iMouseY,
		BYTE bButtonStatus, EVENT* pstEvent);
	YUZA_API bool SetWindowEvent(QWORD qwWindowID, QWORD qwEventType, EVENT* pstEvent);
	YUZA_API void SetKeyEvent(QWORD qwWindow, const KEYDATA* pstKeyData, EVENT* pstEvent);

#ifdef __cplusplus
}
#endif

