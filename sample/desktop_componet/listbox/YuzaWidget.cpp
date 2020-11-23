#include "YuzaWidget.h"

void YuzaWidget::Activate()
{
	auto iter = m_mapWidgets.begin();
	for (; iter != m_mapWidgets.end(); iter++)
	{
		(*iter)->Create();
	}

	Create();
}
