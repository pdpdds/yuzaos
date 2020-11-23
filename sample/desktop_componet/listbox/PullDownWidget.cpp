#include "PullDownWidget.h"

PullDownWidget::PullDownWidget(GuiWindow* pWindow, const char* name, int pos_x, int pos_y)
{
	m_pos_x = pos_x;
	m_pos_y = pos_y;
	m_elementIndex = 1;

	pulldown = add_pulldown(pWindow, pos_x, pos_y, (char*)name);
}

void PullDownWidget::AddElement(const char* text, void(*function)(GuiObject_* obj, int parameter))
{
	GuiObject* obj = add_item(pulldown, (char*)text, NORMAL_ITEM);
	set_object_callback(obj, function);
	set_object_user_data(obj, m_elementIndex);
	m_elementIndex++;
}

void PullDownWidget::Create()
{
	create_pulldown(pulldown);
}