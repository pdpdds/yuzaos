#pragma once
#include <windef.h>
#include "dirent.h"

typedef struct tag_WIN_RECT
{
	int x;
	int y;
	int width;
	int height;
}WIN_RECT;

typedef struct tag_WIN_CONF
{
	int type;
	int kill_button;
	int always_on_top;
	char title[256];
}WIN_CONF;

void (*function) (HWND hwnd, int parameter);

#ifdef __cplusplus
extern "C" {
#endif
	HWND CreateSheet(void);
	HWND AddWindow(HWND parent, WIN_CONF conf, WIN_RECT rect);
	HWND AddButton(HWND window, WIN_RECT rect, int option, char* label);
	HWND SetWindowName(HWND window, int x, int y, char* windowName, int option);
	HWND AddListBox(HWND window, int x, int y, char* windowName, int option);
	HWND AddObjectCallback(HWND object, void (*function) (HWND hwnd, int parameter));
	bool CreateWindow(HWND window);
#ifdef __cplusplus
}
#endif
