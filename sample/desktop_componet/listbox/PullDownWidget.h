#pragma once
#include "YuzaWidget.h"

class PullDownWidget : public YuzaWidget
{
public:
	PullDownWidget(GuiWindow* pWindow, const char* name, int  pos_x, int pos_y);

	void AddElement(const char* text, void(*function)(GuiObject_* obj, int parameter));
	
protected:
	virtual void Create() override;

private:
	GuiObject* pulldown;
	int m_pos_x;
	int m_pos_y;
	int m_elementIndex;
};