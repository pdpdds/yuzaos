#include "svgamgr.h"
#include <InputQueue.h>

unsigned long color[50];
QUEUE stEventQueue;
unsigned long* g_frameBuffer;
EVENT svga_event;

char savechar = '\0';

void __cdecl error(char*)
{

}

unsigned long* getlfb()
{
	return g_frameBuffer;
}

static void Define32BitColor(void)
{
	/* Define the extra colors */
	BLACK = 1;
	DARKGREY = 2;
	GREY = 3;
	LIGHTGREY = 4;
	WHITE = 5;
	DARKRED = 6;
	RED = 7;
	DARKBLUE = 8;
	BLUE = 9;
	LIGHTBLUE = 10;
	DARKGREEN = 11;
	GREEN = 12;
	DARKYELLOW = 13;
	YELLOW = 14;

	/* redefine the colorscheme */
	BACKGROUND = 0;
	WIN_BACK = 15;
	TITLE_BACK = DARKGREY;
	TITLE_FORE = GREY;
	ACTIVE_TITLE_BACK = DARKBLUE;
	ACTIVE_TITLE_FORE = WHITE;
	BUTTON_BACK = GREY;
	BUTTON_FORE = BLACK;
	SLIDER_BACK = LIGHTGREY;
	SLIDER_FORE = LIGHTGREY;
	TEXT_EMBOSS = DARKGREY;
	TEXT_NORMAL = BLACK;
	INPUT_BACK = GREY;
	INPUT_FORE = BLACK;
	ACTIVE_INPUT_BACK = LIGHTGREY;
	BROWSER_BACK = WHITE;
	BROWSER_FORE = BLACK;
	INFO_BACK = 16;
	INFO_FORE = BLACK;
	CHOICE_BACK = WHITE;
	CHOICE_FORE = BLACK;
	LISTBOX_BACK = WHITE;
	LISTBOX_FORE = BLACK;
	NUMBER_FORE = 17;

	color[BACKGROUND] = 0x608189;
	color[WIN_BACK] = 0xC0C0C0;
	color[INFO_BACK] = 0xDCDCA8;
	color[NUMBER_FORE] = 0x4880B8;

	color[BLACK] = 0x000000;
	color[DARKGREY] = 0x909090;
	color[GREY] = 0xC0C0C0;
	color[LIGHTGREY] = 0xD8D8D8;
	color[WHITE] = 0xFFFFFF;
	color[DARKRED] = 0x780000;
	color[RED] = 0xE80000;
	color[DARKBLUE] = 0x808080;
	color[BLUE] = 0x0000C0;
	color[LIGHTBLUE] = 0x0080F0;
	color[DARKGREEN] = 0x007800;
	color[GREEN] = 0x54FF54;
	color[DARKYELLOW] = 0xFFBC00;
	color[YELLOW] = 0xFFFF54;
}


void UpdateSheet()
{
	int length, line, x;
	char* src;
	unsigned long* dst32;
	unsigned long* buffer = getlfb();

	src = guiscreen.data + guiscreen.x_min + guiscreen.y_min * guiscreen.width;
	length = guiscreen.x_max - guiscreen.x_min + 1;
	dst32 = buffer + guiscreen.x_min + guiscreen.y_min * guiscreen.width;

	int maxline = guiscreen.y_max - guiscreen.y_min + 1 ;
	for(line = 0; line < maxline; line++) {
		for (x = 0; x < length; x++)
			*(dst32++) = *(color + *src++);
		dst32 += guiscreen.width - length;
		src += guiscreen.width - length;
	}

	RECT rect;
	rect.left = guiscreen.x_min;
	rect.top = guiscreen.y_min;
	rect.right = guiscreen.x_max  + 1;
	rect.bottom = guiscreen.y_max + 1;

	if (rect.bottom >= guiscreen.height - WINDOW_TITLEBAR_HEIGHT)
		rect.bottom = guiscreen.height- WINDOW_TITLEBAR_HEIGHT;
	if (rect.right >= guiscreen.width)
		rect.right = guiscreen.width;
	Syscall_BitBlt(&parentScreenId, &rect, buffer, guiscreen.width, guiscreen.height);	 
}


bool CreateSheet(QWORD windowId, int width, int height, int colors)
{
	g_frameBuffer = (unsigned long*)malloc(width * height * sizeof(unsigned long));
	memset(g_frameBuffer, 0, sizeof(unsigned long) * width * height);

	Define32BitColor();

	if (guiscreen.data)
		free(guiscreen.data);

	guiscreen.data = (char*)malloc(width * height);
	if (guiscreen.data == NULL)
		error("Cannot allocate memory for screen operations in open_screen().");
	memset(guiscreen.data, 0, width * height);

	guiscreen.type = SVGALIB;

	EVENT* pstEventBuffer;
	// 이벤트 큐에서 사용할 이벤트 자료구조 풀을 생성
	pstEventBuffer = (EVENT*) new char[sizeof(EVENT) * EVENTQUEUE_WNIDOWMANAGERMAXCOUNT];
	
	// 이벤트 큐를 초기화
	kInitializeQueue(&stEventQueue, pstEventBuffer, EVENTQUEUE_WNIDOWMANAGERMAXCOUNT, sizeof(EVENT));

	return true;
}

void SVGA_SetEvent(const EVENT& stReceivedEvent)
{
	kPutQueue(&stEventQueue, (void*)&stReceivedEvent);
}

extern "C" int GetMessageEvent()
{
	if (kGetQueue(&stEventQueue, (void*)&svga_event))
	{
		switch (svga_event.qwType)
		{
			// 마우스 이벤트 처리
		case EVENT_MOUSE_MOVE:
		case EVENT_MOUSE_LBUTTONUP:
		case EVENT_MOUSE_LBUTTONDOWN:
		case EVENT_MOUSE_RBUTTONDOWN:
		case EVENT_MOUSE_RBUTTONUP:
		case EVENT_MOUSE_MBUTTONDOWN:
		case EVENT_MOUSE_MBUTTONUP:
		{
			mouse.x = svga_event.stMouseEvent.stPoint.iX;
			mouse.y = svga_event.stMouseEvent.stPoint.iY - WINDOW_TITLEBAR_HEIGHT;

			if (svga_event.qwType == EVENT_MOUSE_LBUTTONUP)
				svga_event.qwType = 0;

			return GuiMouseEvent;
		}
			// 키 이벤트 처리
		/*case EVENT_KEY_DOWN:
		case EVENT_KEY_UP:
			// 여기에 키보드 이벤트 처리 코드 넣기
			//pstKeyEvent = &(stReceivedEvent.stKeyEvent);
			savechar = svga_event.stKeyEvent.bASCIICode;
			return GuiKeyboardEvent;*/
		}
	}

	return 0;
}

extern "C" int mouse_getbutton() {

	switch (svga_event.qwType)
	{
	case EVENT_MOUSE_LBUTTONUP:
		return 0;
	case EVENT_MOUSE_LBUTTONDOWN:
		return GuiMouseLeftButton;
	
	case EVENT_MOUSE_RBUTTONUP:
	case EVENT_MOUSE_RBUTTONDOWN:
		return GuiMouseRightButton;
	
	case EVENT_MOUSE_MBUTTONUP:
	case EVENT_MOUSE_MBUTTONDOWN:
		return GuiMouseMiddleButton;

	case EVENT_MOUSE_MOVE:
		{
		if (svga_event.stMouseEvent.bButtonStatus == 1)
			return GuiMouseLeftButton;
		}	
	}



	return 0;
}

#include <ff.h>
const char ScanDir_fileid[] = "Hatari scandir.c : " __DATE__ " " __TIME__;
int alphasort(const void* d1, const void* d2)
{
	return strcmp((*(FILINFO* const*)d1)->fname,
		(*(FILINFO* const*)d2)->fname);
}


/*-----------------------------------------------------------------------*/
/**
 * Scan a directory for all its entries
 * Return -1 on error, number of entries on success
 */
size_t get_dir_item_count(const char* dirname, int (*sdfilter)(dirent*))
{
	DIR* dirp = 0;
	int count = 0;
	
	dirent* info;
	if ((dirp = opendir(dirname)) == 0)
		return 0;
	bool result = true;
	while (result)
	{
		info = readdir(dirp);
		if (info == 0)
		{
			result = false;
			break;
		}

		if (sdfilter != NULL && !(*sdfilter)(info))
			continue;       /* just selected names */

		count++;

		
	}

	closedir(dirp);

	if (dirp)
	{
		free(dirp);
	}

	return count;
}

#include <dirent.h>
int scandir(const char* dirname, dirent*** namelist, int (*sdfilter)(dirent*), int (*dcomp)(const void*, const void*))
{
	dirent* p = NULL, ** names = NULL;
	dirent* info;
	size_t nitems = 0;

	size_t arraysz = get_dir_item_count(dirname, sdfilter);

	if (arraysz == 0)
		return 0; 

	DIR* dirp;
	int result = 0;

	dirp = opendir(dirname);
	if (dirp == 0)
		goto error_out;

	names = (dirent**)malloc(arraysz * sizeof(dirent*));
	if (names == NULL)
		goto error_out;

	
	while (true)
	{
		
		info = readdir(dirp);
		if (info == 0)
		{
			break;
		}

		if (sdfilter != NULL && !(*sdfilter)(info))
			continue;       /* just selected names */


		p = (dirent*)malloc(sizeof(dirent));
		if (p == NULL)
			goto error_out;


		memcpy(p, info, sizeof(dirent));

		names[nitems++] = p;

		p = NULL;

		
		
	}

	closedir(dirp);

	free(dirp);

	if (nitems && dcomp != NULL)
		qsort(names, nitems, sizeof(FILINFO*), dcomp);

	*namelist = names;

	return nitems;

error_out:
	if (names)
	{
		int i;
		for (i = 0; i < nitems; i++)
			free(names[i]);
		free(names);
	}
	
	closedir(dirp);
	return -1;
}



