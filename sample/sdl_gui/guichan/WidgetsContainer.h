#pragma once
#include "IWidgetContainer.h"

#include <guichan.hpp>
#include <guichan/sdl.hpp>

class GUIManager;

class WidgetsContainer : public IWidgetsContainer
{
public:
	WidgetsContainer(GUIManager* pManager);
	virtual ~WidgetsContainer();

	virtual bool BuildWidgets();

private:
	gcn::Container* m_pContainer;
	gcn::ImageFont* m_pFont;
	gcn::Label* m_pLabel;
	
	gcn::Icon* m_pIcon;
	gcn::Button* m_pButton;
	gcn::TextField* m_pTextField;
	gcn::TextBox* m_pTextBox;
	gcn::ScrollArea* m_pTextBoxScrollArea;
	
	gcn::CheckBox* m_pCheckBox1;
	gcn::CheckBox* m_pCheckBox2;
	gcn::RadioButton* m_pRadioButton1;
	gcn::RadioButton* m_pRadioButton2;
	gcn::RadioButton* m_pRadioButton3;
	gcn::Slider* m_pSlider;
	gcn::Image *m_pImage;
	gcn::Window *m_pWindow;
	gcn::Image *m_pDarkbitsImage;
	gcn::Icon* m_pDarkbitsIcon;
	gcn::TabbedArea* m_pTabbedArea;
	gcn::Button* m_pTabOneButton;
	gcn::CheckBox* m_pTabTwoCheckBox;

	GUIManager* m_pManager;
};

