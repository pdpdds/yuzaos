#pragma once
#include <skyoswindow.h>
#include <svgagui.h>
#include <vector>

class YuzaWidget
{
public:
	void Activate();
	virtual void Create() = 0;

protected:
	std::vector<YuzaWidget*> m_mapWidgets;
};