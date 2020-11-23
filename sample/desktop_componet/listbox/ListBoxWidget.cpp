#include "ListBoxWidget.h"

static void CallbackDefaultClickFromListBox(GuiObject* obj, int data)
{
	
}


ListBoxWidget::ListBoxWidget(GuiWindow* pWindow, RECTINFO listboxRect)
{
	m_listboxRect = listboxRect;
	m_elementIndex = 1;
	
	listbox = add_listbox(pWindow, m_listboxRect.left, m_listboxRect.top, m_listboxRect.width, m_listboxRect.height);
}

void ListBoxWidget::AddElement(const char* text, void(*function)(GuiObject_* obj, int parameter), void(*function2)(GuiObject_* obj, int parameter))
{
	GuiObject* obj = NULL;
	obj = add_listentry(listbox, (char*)text);

	if(function == 0)
		set_object_callback(obj, CallbackDefaultClickFromListBox);
	else
		set_object_callback(obj, function);

	if (function2 == 0)
		set_object_callback2(obj, CallbackDefaultClickFromListBox);
	else
		set_object_callback2(obj, function2);
	
	set_object_user_data(obj, m_elementIndex);
	m_elementIndex++;
}


void ListBoxWidget::Create()
{
	create_listbox(listbox);
}