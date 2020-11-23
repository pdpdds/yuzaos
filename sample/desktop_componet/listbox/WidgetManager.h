#pragma once
#include <YuzaWidget.h>
#include "WindowWidget.h"
#include "ListBoxWidget.h"
#include "PullDownWidget.h"
#include <vector>

class WidgetManager
{
public:
	bool Initialize(QWORD windowId, int width, int height, const char* title);
	bool Activate();
	WindowWidget* AddWindow(const char* name, RECTINFO& windowRect, int flags);
	
	void Run();

	void MessageDialog(const char* message, int type);
	bool FileDialog(const char* title, const char* lable, const char* mask);
	

	static WidgetManager* GetInstance()
	{
		if (m_pWidgetManager == nullptr)
			m_pWidgetManager = new WidgetManager();

		return m_pWidgetManager;
	}

	QWORD m_qwTaskId;

protected:
	void Create();

private:
	WidgetManager();
	~WidgetManager();
	static WidgetManager* m_pWidgetManager;

	bool CreateSheet(QWORD windowId, int width, int height, int colors);
	void OpenScreen(int width, int height, int colors, const char* title);
	
	std::vector<YuzaWidget*> m_mapWidgets;
	GuiWinThread* m_pWinThread;
	
};
