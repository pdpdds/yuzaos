#pragma once
#include "YuzaWidget.h"
#include "ListBoxWidget.h"
#include "PullDownWidget.h"

class WindowWidget : public YuzaWidget
{
public:
	WindowWidget(GuiWinThread* winThread, const char* name, RECTINFO& windowRect, int flags);
	ListBoxWidget* CreateListBox(RECTINFO& listboxRect);
	PullDownWidget* CreatePullDown(const char* name);

protected:
	virtual void Create() override;
	
private:
	GuiWindow* win;
	RECTINFO m_windowRect;
	int m_pulldown_anchor_x;
	int m_pulldown_anchor_y;
};