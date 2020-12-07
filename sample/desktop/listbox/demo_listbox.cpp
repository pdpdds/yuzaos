#include <windef.h>
#include "svgagui.h"
#include <stdio.h>
#include <string.h>

static void listbox_cb(GuiObject* obj, int data)
{
	printf("Listbox: Testing %d pressed\n", data);
}


static void listbox_cb2(GuiObject* obj, int data)
{
	printf("Listbox: Testing %d double clicked\n", data);
}

void CreateListBox(GuiWinThread* win_thread)
{
	GuiObject* listbox;
	GuiWindow* win;
	GuiObject* obj = NULL;

	win = add_window(win_thread, NORMAL_WINDOW, 0, 0, 200, 105, "Listbox", FALSE, FALSE);
	listbox = add_listbox(win, 5, 25, 70, 2);
	obj = add_listentry(listbox, "Testing 1");
	set_object_callback(obj, listbox_cb);
	set_object_callback2(obj, listbox_cb2);
	set_object_user_data(obj, 1);
	obj = add_listentry(listbox, "Testing 2");
	set_object_callback(obj, listbox_cb);
	set_object_callback2(obj, listbox_cb2);
	set_object_user_data(obj, 2);
	obj = add_listentry(listbox, "Testing 3");
	set_object_callback(obj, listbox_cb);
	set_object_callback2(obj, listbox_cb2);
	set_object_user_data(obj, 3);
	obj = add_listentry(listbox, "Testing 4");
	set_object_callback(obj, listbox_cb);
	set_object_callback2(obj, listbox_cb2);
	set_object_user_data(obj, 4);
	obj = add_listentry(listbox, "Testing 5");
	set_object_callback(obj, listbox_cb);
	set_object_callback2(obj, listbox_cb2);
	set_object_user_data(obj, 5);
	obj = add_listentry(listbox, "Testing 6");
	set_object_callback(obj, listbox_cb);
	set_object_callback2(obj, listbox_cb2);
	set_object_user_data(obj, 6);
	create_listbox(listbox);
	create_window(win);
}


static void endtask_cb(GuiObject* obj, int data)
{
	show_button(obj);
	if (question_dialog((obj->win)->win_thread, NULL, "Do you want to end this task?", DIA_QUESTION)) {
		message_dialog((obj->win)->win_thread, NULL, "You cannot end this task!", DIA_INFO);
	}
}

GuiWindow* CreateThreadView(GuiWinThread* win_thread)
{
	GuiWindow* win;
	GuiObject* obj, * listbox;
	//ProcessManager::ProcessList* pProcessList = ProcessManager::GetInstance()->GetProcessList();
	int k;

	win = add_window(win_thread, NORMAL_WINDOW, (1024 - 300) / 2, (768 - 500) / 2, 300, 500, "SkyOS ThreadsView", FALSE, FALSE);
	obj = add_button(win, NORMAL_BUTTON, 10, 30, 80, 17, "Applications");
	obj = add_button(win, NORMAL_BUTTON, 95, 30, 80, 17, "Performance");
	obj = add_button(win, NORMAL_BUTTON, 180, 30, 80, 17, "Networking");
	obj = add_text(win, NORMAL_TEXT, 5, 60, "Taskname");
	listbox = add_listbox(win, 10, 80, 300 - 35, 25);
	k = 0;

	/*auto iter = pProcessList->begin();
	for (; iter != pProcessList->end(); iter++)
	{
		k++;
		Process* pProcess = (*iter).second;
		obj = add_listentry(listbox, pProcess->m_processName);
		set_object_callback(obj, threadsview_listbox_cb);
		set_object_callback2(obj, threadsview_listbox_cb2);
		set_object_user_data(obj, k);
	}*/

	create_listbox(listbox);
	obj = add_button(win, NORMAL_BUTTON, 200, 450, 80, 20, "End Task");
	set_object_callback(obj, endtask_cb);
	create_window(win);

	return win;
}

void terminal_dispatch(GuiObject* obj, char* command)
{

}

static void terminalinput_cb(GuiObject* obj, int user_data)
{
	GuiWinThread* win_thread = (obj->win)->win_thread;
	GuiWindow* win;
	GuiObject* object;

	win = obj->win;
	object = win->first;

	if (object->objclass == BROWSER) {
		terminal_dispatch(object, obj->label);
		memset(obj->label, 0, strlen(obj->label));
		//OutputBrowser(object, (char*)user_data);
		return;
	}

	while (object->next != NULL) {
		object = object->next;
		if (object->objclass == BROWSER) {
			terminal_dispatch(object, obj->label);
			memset(obj->label, 0, strlen(obj->label));
			return;
		}
	}
}

GuiWindow* CreateTerminal(GuiWinThread* win_thread)
{
	GuiWindow* win;
	GuiObject* browser, * text;

	win = add_window(win_thread, NORMAL_WINDOW, (1024 - 640) / 2, (768 - 480) / 2, 640, 480, "System Terminal", FALSE, FALSE);
	browser = add_browser(win, 5, 20, 630, 430, VERT_SLIDER);
	set_browser_text(browser, "SkyOS System Terminal initialized.\n");
	text = add_input(win, NORMAL_INPUT, 5, 480 - 20, 625, 256);
	set_object_callback(text, terminalinput_cb);
	create_window(win);

	return win;
}