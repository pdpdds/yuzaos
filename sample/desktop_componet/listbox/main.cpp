#include "WidgetManager.h"
#include <skyoswindow.h>
#include <string.h>
#include <systemcall_impl.h>
#include "svgagui.h"

static void CallbackClickFromListBox(GuiObject* obj, int data)
{
	printf("Listbox : %s\n", obj->label);
}


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

static void CallbackDoubleClickFromListBox(GuiObject* obj, int data)
{
	printf("Listbox : %s\n", obj->label);
}

int main(int argc, char** argv)
{
	char* szTitle = "Widget Window";
	int width = 640, height = 480;
	QWORD qwWindowID = CreateWindow(width, height, "Widget Window", WINDOW_FLAGS_DEFAULT | WINDOW_FLAGS_RESIZABLE);

	if (qwWindowID == WINDOW_INVALIDID)
	{
		return 0;
	}

	WidgetManager* pManager = new WidgetManager();
	pManager->Initialize(qwWindowID, width, height, "Widget Window");

	RECT pos = { 0, 0, 200, 105 };
	RECT size = { 5, 25, 70, 2 };
	ListBoxWidget* pListBox = pManager->CreateListBox("ListBox1", pos, size);
	pListBox->AddElement("Testing 1", CallbackClickFromListBox, 0);

	ListBoxWidget* pListBox2 = pManager->CreateListBox("ListBox2", pos, size);
	pListBox2->AddElement("Testing 2", 0, CallbackDoubleClickFromListBox);
	pListBox2->AddElement("Testing 100", 0, CallbackDoubleClickFromListBox);
	pManager->Activate();

	EVENT stReceivedEvent;
	WINDOWEVENT* pstWindowEvent;

	while (1)
	{
		if (Syscall_ReceiveEventFromWindowQueue(&qwWindowID, &stReceivedEvent) == FALSE)
		{
			Syscall_Sleep(1);
			continue;
		}

		pManager->SVGA_SetEvent(stReceivedEvent);

		switch (stReceivedEvent.qwType)
		{
		case EVENT_WINDOW_SELECT:
		case EVENT_WINDOW_DESELECT:
		case EVENT_WINDOW_MOVE:
		case EVENT_WINDOW_RESIZE:
		case EVENT_WINDOW_CLOSE:
			pstWindowEvent = &(stReceivedEvent.stWindowEvent);
			if (stReceivedEvent.qwType == EVENT_WINDOW_CLOSE)
			{
				Syscall_DeleteWindow(&qwWindowID);
				return 1;
			}
			break;

		default:
			break;
		}

	}

	return 0;
}