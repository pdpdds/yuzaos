#pragma once
#include <YuzaWidget.h>
#include "ListBoxWidget.h"
#include <vector>

class WidgetManager : public YuzaWidget
{
public:
	bool Initialize(QWORD windowId, int width, int height, const char* title);
	bool Activate();
	void SVGA_SetEvent(const EVENT& stReceivedEvent);

	ListBoxWidget* CreateListBox(const char* name, RECT& pos, RECT& size);

protected:
	virtual void Create() override;

private:
	bool CreateSheet(QWORD windowId, int width, int height, int colors);
	void OpenScreen(int width, int height, int colors, const char* title);
	
	std::vector<YuzaWidget*> m_mapWidgets;
	GuiWinThread* m_pWinThread;
};
