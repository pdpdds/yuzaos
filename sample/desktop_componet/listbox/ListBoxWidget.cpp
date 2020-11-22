#include "ListBoxWidget.h"

static void CallbackDefaultClickFromListBox(GuiObject* obj, int data)
{
	
}

ListBoxWidget::ListBoxWidget(GuiWinThread* pWindow, const char* name, RECT pos, RECT size)
{
	m_pos = pos;
	m_size = size;
	m_elementIndex = 1;

	win = add_window(pWindow, NORMAL_WINDOW, pos.left, pos.top, pos.right, pos.bottom, (char*)name, FALSE, FALSE);
	listbox = add_listbox(win, size.left, size.top, size.right, size.bottom);
}

void ListBoxWidget::AddElement(const char* text, void(*function)(GuiObject_* obj, int parameter), void (*function2) (struct GuiObject_* obj, int parameter))
{
	GuiObject* obj = NULL;
	obj = add_listentry(listbox, (char*)text);

	if(function == 0)
		set_object_callback(obj, CallbackDefaultClickFromListBox);

	else
		set_object_callback(obj, function);

	set_object_callback2(obj, function2);
	set_object_user_data(obj, m_elementIndex);
	m_elementIndex++;
}


void ListBoxWidget::Create()
{
	create_listbox(listbox);
	create_window(win);
}
