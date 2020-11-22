#pragma once
#include "YuzaWidget.h"

class ListBoxWidget : public YuzaWidget
{
public:
	ListBoxWidget(GuiWinThread* pWindow, const char* name, RECT pos, RECT size);
	void AddElement(const char* text, void (*function) (struct GuiObject_* obj, int parameter), void (*function2) (struct GuiObject_* obj, int parameter));

protected:
	virtual void Create() override;

private:
	GuiObject* listbox;
	GuiWindow* win;
	
	RECT m_pos;
	RECT m_size;
	int m_elementIndex;
};