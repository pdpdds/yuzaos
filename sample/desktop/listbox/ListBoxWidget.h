#pragma once
#include "YuzaWidget.h"

class ListBoxWidget : public YuzaWidget
{
public:
	ListBoxWidget(GuiWindow* pWindow, RECTINFO listboxRect);
	void AddElement(const char* text, void (*function) (struct GuiObject_* obj, int parameter), void (*function2) (struct GuiObject_* obj, int parameter));

protected:
	virtual void Create() override;

private:
	GuiObject* listbox;
	RECTINFO m_listboxRect;
	int m_elementIndex;
};