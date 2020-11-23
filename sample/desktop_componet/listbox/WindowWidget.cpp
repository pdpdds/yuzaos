#include "WindowWidget.h"

WindowWidget::WindowWidget(GuiWinThread* winThread, const char* name, RECTINFO& windowRect, int flags)
{
	m_windowRect = windowRect;
	m_pulldown_anchor_x = 4;
	m_pulldown_anchor_y = 2;

	win = add_window(winThread, flags, m_windowRect.left, m_windowRect.top, m_windowRect.width, m_windowRect.height, (char*)name, FALSE, FALSE);
}

ListBoxWidget* WindowWidget::CreateListBox(RECTINFO& listboxRect)
{
	ListBoxWidget* pWidget = new ListBoxWidget(win, listboxRect);
	m_mapWidgets.push_back(pWidget);
	return pWidget;
}

PullDownWidget* WindowWidget::CreatePullDown(const char* name)
{

	PullDownWidget* pWidget = new PullDownWidget(win, name, m_pulldown_anchor_x, m_pulldown_anchor_y);
	m_pulldown_anchor_x += 31;

	m_mapWidgets.push_back(pWidget);
	return pWidget;
}

void WindowWidget::Create()
{
	create_window(win);
}