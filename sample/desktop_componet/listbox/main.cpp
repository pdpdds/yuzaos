#include "WidgetManager.h"
#include <skyoswindow.h>
#include <string.h>
#include <systemcall_impl.h>

QWORD CreateWindow(int windowWidth, int windowHeight, char* windowName, int flags)
{
	RECT rect;
	int iMouseX, iMouseY;
	Syscall_GetCursorPosition(&iMouseX, &iMouseY);

	rect.left = iMouseX - 10;
	rect.top = iMouseY - WINDOW_TITLEBAR_HEIGHT / 2;
	rect.right = rect.left + windowWidth;
	rect.bottom = rect.top + windowHeight;

	QWORD qwWindowID;
	Syscall_CreateWindow(&rect, windowName, flags, &qwWindowID);
	return qwWindowID;
}

static void CallbackClickFromListBox(GuiObject* obj, int data)
{
	char buf[MAXPATH];
	sprintf(buf, "Listbox Click. Text : %s, Index : %d\n", obj->label, data);
	WidgetManager::GetInstance()->MessageDialog(buf, DIA_INFO);
}

static void CallbackDoubleClickFromListBox(GuiObject* obj, int data)
{
	char buf[MAXPATH];
	sprintf(buf, "Listbox Double Click. Text : %s, Index : %d\n", obj->label, data);

	WidgetManager::GetInstance()->MessageDialog(buf, DIA_INFO);
}

static void CallbackFileMenu(GuiObject* obj, int data)
{
	if (data == 1)
	{
		char mask[MAXPATH];
		sprintf(mask, "C files|*.c|Object files|*.o|All files|*|");
		WidgetManager::GetInstance()->FileDialog("File dialog", obj->label, mask);
	}	
	else if (data == 2)
	{
		exit(0);
	}
}

int main(int argc, char** argv)
{
	char* szTitle = "Widget ListBox";
	int width = 640, height = 480;
	QWORD qwWindowID = CreateWindow(width, height, szTitle, WINDOW_FLAGS_DEFAULT | WINDOW_FLAGS_RESIZABLE);

	if (qwWindowID == WINDOW_INVALIDID)
	{
		return 0;
	}

	WidgetManager::GetInstance()->Initialize(qwWindowID, width, height, szTitle);

	//첫번째 리스트박스 생성
	RECTINFO windowRect = { 1, 51, 200, 105 };
	RECTINFO listBoxRect = { 5, 25, 70, 2 };	
	WindowWidget* pWindow = WidgetManager::GetInstance()->AddWindow("ListBox1", windowRect, NORMAL_WINDOW);
	ListBoxWidget* pListBox = pWindow->CreateListBox(listBoxRect);
	pListBox->AddElement("ListBox1 1", CallbackClickFromListBox, 0);
	
	//두번째 리스트박스 생성
	RECTINFO windowRect2 = { 100, 100, 200, 105 };
	RECTINFO listBoxRect2 = { 5, 25, 70, 2 };
	pWindow = WidgetManager::GetInstance()->AddWindow("ListBox2", windowRect2, NORMAL_WINDOW);
	pListBox = pWindow->CreateListBox(listBoxRect2);
	pListBox->AddElement("ListBox2 1", 0, CallbackDoubleClickFromListBox);
	pListBox->AddElement("ListBox2 2", 0, CallbackDoubleClickFromListBox);
	
	//메뉴 File을 생성하고 후속 아이템 추가
	RECTINFO windowRect3 = { 0, 0, width, 21 };
	pWindow = WidgetManager::GetInstance()->AddWindow("", windowRect3, NO_TITLE_WINDOW);
	PullDownWidget* pPullDown = pWindow->CreatePullDown("File");
	pPullDown->AddElement("Open", CallbackFileMenu);
	pPullDown->AddElement("Close", CallbackFileMenu);

	pPullDown = pWindow->CreatePullDown("Edit");

	WidgetManager::GetInstance()->Activate();
	WidgetManager::GetInstance()->Run();

	return 0;
}