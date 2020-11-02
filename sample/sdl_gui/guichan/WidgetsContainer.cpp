#include "WidgetsContainer.h"
#include "GUIManager.h"

WidgetsContainer::WidgetsContainer(GUIManager* pManager)
: m_pManager(pManager)
{
}


WidgetsContainer::~WidgetsContainer()
{
	delete m_pContainer;
	delete m_pFont;
	delete m_pLabel;
	delete m_pIcon;
	delete m_pButton;
	delete m_pTextField;
	delete m_pTextBox;
	delete m_pTextBoxScrollArea;
	delete m_pCheckBox1;
	delete m_pCheckBox2;
	delete m_pRadioButton1;
	delete m_pRadioButton2;
	delete m_pRadioButton3;
	delete m_pSlider;
	delete m_pWindow;
	delete m_pDarkbitsIcon;
	delete m_pDarkbitsImage;
	delete m_pTabbedArea;
	delete m_pTabOneButton;
	delete m_pTabTwoCheckBox;
}

bool WidgetsContainer::BuildWidgets()
{
	m_pContainer = new gcn::Container();
	m_pContainer->setDimension(gcn::Rectangle(0, 0, m_pManager->m_winWidth, m_pManager->m_winHeight));

	m_pManager->m_pGcn->setTop(m_pContainer);

	// Now we load the font used in this example.
	m_pFont = new gcn::ImageFont("font.png", " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?-+/():;%&''*#=[]\"");
	// Widgets may have a global font so we don't need to pass the
	// font object to every created widget. The global font is static.
	gcn::Widget::setGlobalFont(m_pFont);

	// Now we create the widgets
	m_pLabel = new gcn::Label("abcd");

	m_pImage = gcn::Image::load("gui-chan.bmp");
	m_pIcon = new gcn::Icon(m_pImage);

	m_pButton = new gcn::Button("Button");

	m_pTextField = new gcn::TextField("Text field");

	m_pTextBox = new gcn::TextBox("Multiline\nText box");
	m_pTextBox->setBackgroundColor(gcn::Color(255, 150, 200, 0));


	m_pTextBoxScrollArea = new gcn::ScrollArea(m_pTextBox);
	m_pTextBoxScrollArea->setWidth(200);
	m_pTextBoxScrollArea->setHeight(100);
	m_pTextBoxScrollArea->setFrameSize(1);

	m_pCheckBox1 = new gcn::CheckBox("Checkbox 1");
	m_pCheckBox2 = new gcn::CheckBox("Checkbox 2");

	m_pRadioButton1 = new gcn::RadioButton("RadioButton 1", "radiogroup", true);
	m_pRadioButton2 = new gcn::RadioButton("RadioButton 2", "radiogroup");
	m_pRadioButton3 = new gcn::RadioButton("RadioButton 3", "radiogroup");

	m_pSlider = new gcn::Slider(0, 10);
	m_pSlider->setSize(100, 10);

	m_pWindow = new gcn::Window("Lyrics");
	m_pWindow->setBaseColor(gcn::Color(255, 150, 200, 190));

	m_pDarkbitsImage = gcn::Image::load("darkbitslogo_by_haiko.bmp");
	m_pDarkbitsIcon = new gcn::Icon(m_pDarkbitsImage);
	m_pWindow->add(m_pDarkbitsIcon);
	m_pWindow->resizeToContent();
	

	m_pTabbedArea = new gcn::TabbedArea();
	m_pTabbedArea->setSize(200, 100);
	m_pTabOneButton = new gcn::Button("A button in tab 1");
	m_pTabbedArea->addTab("Tab 1", m_pTabOneButton);
	m_pTabTwoCheckBox = new gcn::CheckBox("A check box in tab 2");
	m_pTabbedArea->addTab("Tab 2", m_pTabTwoCheckBox);

	// Now it's time to add the widgets to the top container
	// so they will be conected to the GUI.

	m_pContainer->add(m_pLabel, 10, 10);
	m_pContainer->add(m_pIcon, 10, 30);
	m_pContainer->add(m_pButton, 200, 10);
	m_pContainer->add(m_pTextField, 300, 10);
	m_pContainer->add(m_pTextBoxScrollArea, 200, 50);
	m_pContainer->add(m_pCheckBox1, 500, 130);
	m_pContainer->add(m_pCheckBox2, 500, 150);
	m_pContainer->add(m_pRadioButton1, 500, 200);
	m_pContainer->add(m_pRadioButton2, 500, 220);
	m_pContainer->add(m_pRadioButton3, 500, 240);
	m_pContainer->add(m_pSlider, 500, 300);
	m_pContainer->add(m_pWindow, 50, 350);
	m_pContainer->add(m_pTabbedArea, 400, 350);

	return true;
}
