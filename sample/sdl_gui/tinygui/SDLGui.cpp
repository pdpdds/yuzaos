#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "SDLGui.h"

#include "SDL_gfxPrimitives_font.h"
#if _SDLGUI_VER_ != 128
#warning Invalid SDLGui version
#endif

#define GUIIsInside(x,y,x1,y1,x2,y2) (x>x1&&x<x2&&y>y1&&y<y2)
	enum GUIElementType { GUI_UNKNOWN, GUI_LABEL, GUI_CHECKBOX, GUI_BUTTON, GUI_SCROLLBAR, GUI_TEXTBOX, GUI_LISTBOX, GUI_DROPDOWN };
void GUIElementAdd(void* element, int zIndex, GUIElementType eType);
void GUIElementRemove(void* element, GUIElementType eType);
static struct GUIElement* GUIElementList = NULL; static size_t GUIElementList_size = 0;

/* GUIDropDown */

GUIDropDown::GUIDropDown() {
	Construct();
}

GUIDropDown::~GUIDropDown() {
	Destruct();
}

void GUIDropDown::Construct() {
	posX1 = 0; posY1 = 0; posX2 = 0; posY2 = 0; isDisabled = false; isHovered = false;
	listBox.Construct(); isPressed = false;
	triX1 = 0; triY1 = 0; triX2 = 0; triY2 = 0; triX2 = 0; triY3 = 0; arrX1 = 0; arrY1 = 0; arrX2 = 0; arrY2 = 0;
	selectedText.Construct();
	StyleConstruct();
	hasRegistered = false;
	listBox.doDraw = false;
}

void GUIDropDown::Destruct() {
	listBox.Destruct();
	selectedText.Destruct();
}

bool GUIDropDown::Event(SDL_Event* event, int offX, int offY) {
	if (isDisabled)return false;
	if (listBox.doDraw && listBox.Event(event, offX, offY)) {
		RenderSelectedText();
		return true;
	}
	int mx, my; bool is;
	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN:
		if (event->button.button == SDL_BUTTON_LEFT) {
			mx = event->button.x + offX;
			my = event->button.y + offY;
			if (GUIIsInside(mx, my, arrX1, arrY1, arrX2, arrY2)) {
				isPressed = true;
				listBox.doDraw = !listBox.doDraw;
				if (listBox.posY2 - listBox.posY1 <= 0)listBox.doDraw = false;
				return true;
			}
			else {
				listBox.doDraw = false;
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if (isPressed) {
			isPressed = false;
			return true;
		}
		break;
	case SDL_MOUSEMOTION:
		mx = event->motion.x;
		my = event->motion.y;
		is = GUIIsInside(mx, my, posX1, posY1, posX2, posY1);
		if (is != isHovered) {
			SDLPushEvent((is) ? GUI_MOUSEIN : GUI_MOUSEOUT, this, NULL);
			isHovered = is;
		}
		break;
	case SDL_USEREVENT:
		if (event->user.code == GUILISTBOX_SELECT) {
			if (event->user.data1 == &listBox) {
				SDLPushEvent(GUIDROPDOWN_SELECT, this, NULL);
				return true;
			}
		}
		break;
	}
	return false;
}

bool GUIDropDown::IsDisabled() {
	return isDisabled;
}

bool GUIDropDown::IsHovered() {
	return isHovered || (listBox.isHovered && listBox.doDraw);
}

void GUIDropDown::SetDisabled(bool disabled) {
	isDisabled = true;
	if (disabled) {
		listBox.SetDisabled(true);
		listBox.doDraw = false;
	}
}

void GUIDropDown::SetPos(int x1, int y1, int x2, int y2) {
	if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
	if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
	if (y1 == y2)y2 += listBox.textHeight;
	posX1 = x1;
	posY1 = y1;
	posX2 = x2;
	posY2 = y2;
	SetDropDownHeight(listBox.posY2 - listBox.posY1);
}

void GUIDropDown::Register(int zIndex) {
	GUIElementAdd((void*)this, zIndex, GUI_DROPDOWN);
	GUIElementAdd((void*)& listBox, zIndex + 1, GUI_LISTBOX);
	hasRegistered = true;
}

void GUIDropDown::Unregister() {
	GUIElementRemove((void*)this, GUI_DROPDOWN);
	GUIElementRemove((void*)& listBox, GUI_LISTBOX);
	hasRegistered = false;
}

void GUIDropDown::SetDropDownHeight(int height) {
	if (height <= 0)height = (posY2 - posY1) * 6; //DONE HERE
	listBox.SetPos(posX1, posY2, posX2, posY2 + height);
	RecalculateCoords();
}

void GUIDropDown::AddItem(const char* item, int pos) {
	listBox.AddItem(item, pos);
}

void GUIDropDown::EraseItem(int pos) {
	listBox.EraseItem(pos);
}

void GUIDropDown::ClearItems() {
	listBox.ClearItems();
}

char* GUIDropDown::GetItem(int pos) {
	return listBox.GetItem(pos);
}

int GUIDropDown::GetItemCount() {
	return listBox.GetItemCount();
}

void GUIDropDown::SetSelected(int pos) {
	listBox.SetSelected(pos);
	RenderSelectedText();
}

int GUIDropDown::GetSelected() {
	return listBox.GetSelected();
}

void GUIDropDown::SetFontGUIText(GUIText* guiText) {
	listBox.SetFontGUIText(guiText);
}

void GUIDropDown::SetFontSize(int size) {
	listBox.SetFontSize(size);
}

void GUIDropDown::SetFontColor(Uint32 color) {
	listBox.SetFontColor(color);
}

void GUIDropDown::SetFontBackColor(Uint32 backColor) {
	listBox.SetFontBackColor(backColor);
}

void GUIDropDown::RenderSelectedText() {
	char* sel = listBox.GetItem(listBox.GetSelected());
	if (sel == NULL || sel[0] == 0) {
		selectedText.FreeRender();
	}
	else {
		selectedText.SwitchRender(listBox.fontGuiText);
		listBox.fontGuiText->Render(sel, listBox.fontColor, listBox.fontSize, false);
		selectedText.SwitchRender(listBox.fontGuiText);
	}
}

void GUIDropDown::RecalculateCoords() {
	int textBoxHeight = posY2 - posY1;
	triX1 = posX2 - (textBoxHeight / 2); triY1 = posY1 + (textBoxHeight * 0.625f);
	triX2 = triX1 - (textBoxHeight / 4); triY2 = triY1 - (textBoxHeight / 4);
	triX3 = triX1 + (textBoxHeight / 4); triY3 = triY2;
	arrX1 = posX2 - textBoxHeight; arrY1 = posY1;
	arrX2 = posX2; arrY2 = posY1 + textBoxHeight;
}

GUIDropDown& GUIDropDown::operator = (const GUIDropDown& param) {
	if (this == &param)return *this;
	listBox = param.listBox;
	RenderSelectedText();
	return *this;
}

GUIDropDown& GUIDropDown::operator = (const GUIListBox& param) {
	listBox = param;
	RenderSelectedText();
	return *this;
}

/* GUIListBox */

GUIListBox::GUIListBox() {
	Construct();
}

GUIListBox::~GUIListBox() {
	Destruct();
}

void GUIListBox::Construct() {
	posX1 = 0; posY1 = 0; posX2 = 0; posY2 = 0; isDisabled = false; isHovered = false;
	fontGuiText = NULL; fontSize = 0; fontColor = 0x000000FF; fontBackColor = 0xFFFFFFFF;
	renderedText.Construct(); scroll.Construct();
	text = NULL; textSize = 0; itemCount = 0; selected = -1; textHeight = 0;
	curOffX = 0; curOffY = 0; isMouseDown = false; showSelectionBox = false;
	textBorderUp = 0; textBorderDown = 0; textBorderLeft = 0; textBorderRight = 0; textSliderThick = 0;
	textX1 = 0; textY1 = 0; textX2 = 0; textY2 = 0; selX1 = 0; selY1 = 0; selX2 = 0; selY2 = 0;
	renderedText.SetRenderBlended();
	scroll.SetHorizontal(false);
	scroll.SetScrollSize(1.f);
	scroll.SetDisabled(true);
	SetFontGUIText(); SetFontSize(); SetFontColor(); SetFontBackColor();
	StyleConstruct();
	scroll.SetArrowWidth(textSliderThick);
	doDraw = true;
}

void GUIListBox::Destruct() {
	if (text != NULL)free(text); text = NULL; textSize = 0;
	renderedText.Destruct();
	scroll.Destruct();
	Unregister();
}

bool GUIListBox::Event(SDL_Event* event, int offX, int offY) {
	if (isDisabled)return false;
	if (scroll.Event(event, offX, offY))return true;
	int mx, my; bool is;
	switch (event->type)
	{
	case SDL_MOUSEWHEEL:
	{
		//20141121
		if (event->wheel.y > 0) {
			if (isHovered) {
				scroll.SetScrollPos(scroll.GetScrollPos() - scroll.GetScrollTrackMove());
				curOffY = -scroll.GetScrollPos() * ((itemCount * textHeight) - textY2 + textY1);
				RecalculateSelectionBox();
				return true;
			}
		}
		else if (event->wheel.y < 0) {
			if (isHovered) {
				scroll.SetScrollPos(scroll.GetScrollPos() + scroll.GetScrollTrackMove());
				curOffY = -scroll.GetScrollPos() * ((itemCount * textHeight) - textY2 + textY1);
				RecalculateSelectionBox();
				return true;
			}
		}
		return true;
	}
	break;
	case SDL_MOUSEBUTTONDOWN:
		if (event->button.button == SDL_BUTTON_LEFT) {
			mx = event->button.x + offX;
			my = event->button.y + offY;
			is = GUIIsInside(mx, my, textX1, textY1, textX2, textY2);
			if (is) {
				int s = (my - textY1 - curOffY) / textHeight;
				if (s >= itemCount)s = itemCount - 1;
				if (s < 0)s = 0;
				if (s != selected) {
					selected = s;
					SDLPushEvent(GUILISTBOX_SELECT, this, NULL);
					RecalculateSelectionBox();
				}
				isMouseDown = true;
				return true;
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		isMouseDown = false;
		break;
	case SDL_MOUSEMOTION:
		mx = event->motion.x + offX;
		my = event->motion.y + offY;
		is = GUIIsInside(mx, my, posX1, posY1, posX2, posY2);
		if (is != isHovered) {
			SDLPushEvent((is) ? GUI_MOUSEIN : GUI_MOUSEOUT, this, NULL);
			isHovered = is;
		}
		if (isMouseDown && GUIIsInside(mx, my, textX1, textY1, textX2, textY2)) {
			int s = (my - textY1 - curOffY) / textHeight;
			if (s >= itemCount)s = itemCount - 1;
			if (s < 0)s = 0;
			if (s != selected) {
				selected = s;
				SDLPushEvent(GUILISTBOX_SELECT, this, NULL);
				RecalculateSelectionBox();
			}
			return true;
		}
		if (is)return true;
		break;
	case SDL_KEYDOWN:
		if (isHovered) {
			if (event->key.keysym.sym == SDLK_UP) {
				selected--;
				if (selected < 0)selected = 0;
				else {
					SDLPushEvent(GUILISTBOX_SELECT, this, NULL);
					float t = ((itemCount <= 1) ? 0. : ((float)selected / (itemCount - 1)));
					scroll.SetScrollPos(t);
					curOffY = -scroll.GetScrollPos() * ((itemCount * textHeight) - textY2 + textY1);
					RecalculateSelectionBox();
					return true;
				}
			}
			else if (event->key.keysym.sym == SDLK_DOWN) {
				selected++;
				if (selected >= itemCount)selected = itemCount - 1;
				else {
					SDLPushEvent(GUILISTBOX_SELECT, this, NULL);
					float t = ((itemCount <= 1) ? 0. : ((float)selected / (itemCount - 1)));
					scroll.SetScrollPos(t);
					curOffY = -scroll.GetScrollPos() * ((itemCount * textHeight) - textY2 + textY1);
					RecalculateSelectionBox();
					return true;
				}
			}
		}

		break;
	case SDL_USEREVENT:
		if (event->user.code == GUISCROLLBAR_SCROLL) {
			if (event->user.data1 == &scroll) {
				curOffY = -scroll.GetScrollPos() * ((itemCount * textHeight) - textY2 + textY1);
				RecalculateSelectionBox();
				return true;
			}
		}
	}
	return false;
}

bool GUIListBox::IsDisabled() {
	return isDisabled;
}

bool GUIListBox::IsHovered() {
	return isHovered;
}

void GUIListBox::SetDisabled(bool disabled) {
	isDisabled = disabled;
	if (disabled) {
		isHovered = false;
		scroll.SetDisabled(true);
	}
}

void GUIListBox::SetPos(int x1, int y1, int x2, int y2) {
	posX1 = x1;
	posY1 = y1;
	posX2 = x2;
	posY2 = y2;
	textX1 = x1 + textBorderLeft;
	textY1 = y1 + textBorderUp;
	textX2 = x2 - textSliderThick - textBorderRight;
	textY2 = y2 - textBorderDown;
	scroll.SetPos(x2 - textSliderThick, y1, x2, y2);
	RenderText();
}

void GUIListBox::Register(int zIndex) {
	GUIElementAdd((void*)this, zIndex, GUI_LISTBOX);
}

void GUIListBox::Unregister() {
	GUIElementRemove((void*)this, GUI_LISTBOX);
}

void GUIListBox::AddItem(const char* item, int pos) {
	if (item == NULL)return;
	if (text == NULL) {
		textSize = 0;
		text = (char*)malloc(sizeof(char) * 1);
		if (text == NULL)return;
		text[0] = 0;
	}
	int loc = 0, locPos = 0, itemLen, txtSize = textSize;
	for (itemLen = 0; item[itemLen] != 0; itemLen++)if (item[itemLen] == '\n')return;
	if (pos < 0) {
		if (textSize > 0)locPos = textSize + 1;
	}
	else if (pos > 0) {
		for (int n = 0; n <= textSize; n++) {
			if (text[n] == 0) {
				loc++;
				locPos = n + 1;
				if (loc == pos)break;
			}
		}
	}
	textSize = txtSize + itemLen; if (txtSize > 0)textSize++;
	char* _text = (char*)realloc(text, (textSize + 1) * sizeof(char));
	if (_text == NULL) {
		textSize -= itemLen; return;
	}
	else text = _text;
	if (txtSize > 0) {
		for (int n = txtSize; n >= locPos; n--) {
			text[n + itemLen + 1] = text[n];
		}
	}
	for (int n = 0; n <= itemLen; n++) {
		char c;
		if (n == itemLen || item[n] == '\n')c = 0;
		else c = item[n];
		text[n + locPos] = c;
	}
	RenderText();
}

void GUIListBox::EraseItem(int pos) {
	if (textSize <= 0 || text == NULL)return;
	int loc = 0, locPos = -1, locSec = 0, txtSize = textSize;
	for (int n = 0; n <= textSize; n++) {
		if (text[n] == 0) {
			if (pos == loc) {
				locSec = n;
				break;
			}
			loc++;
			if (n != textSize)locPos = n;
		}
	}
	if (pos < 0)locSec = textSize;
	if (locSec == 0)return;
	textSize = txtSize - locSec + locPos;
	if (textSize <= 0) {
		textSize = 0;
		text[0] = 0;
	}
	else {
		for (int n = locSec; n <= txtSize; n++) {
			int p = n - locSec + locPos;
			if (p >= 0 && p <= txtSize)text[n - locSec + locPos] = text[n];
		}
	}
	char* _text = (char*)realloc(text, (textSize + 1) * sizeof(char));
	if (_text != NULL)text = _text;
	RenderText();
}

void GUIListBox::ClearItems() {
	textSize = 0;
	char* _text = (char*)realloc(text, (textSize + 1) * sizeof(char));
	if (_text != NULL)text = _text;
	text[0] = 0;
	RenderText();
	RecalculateSelectionBox();
}

char* GUIListBox::GetItem(int pos) {
	if (text == NULL) {
		textSize = 0;
		text = (char*)malloc(sizeof(char) * 1);
		if (text == NULL)return NULL;
		text[0] = 0;
	}
	if (pos < 0)return NULL;
	else if (pos == 0)return text;
	int loc = 0;
	for (int n = 0; n < textSize; n++) {
		if (text[n] == 0) {
			loc++;
			if (loc == pos) {
				return (text + n + 1);
			}
		}
	}
	return NULL;
}

int GUIListBox::GetItemCount() {
	return itemCount;
}

void GUIListBox::SetSelected(int pos) {
	selected = pos;
	if (selected >= itemCount)selected = itemCount - 1;
	if (selected < 0)selected = -1;
	RecalculateSelectionBox();
}

int GUIListBox::GetSelected() {
	return selected;
}

void GUIListBox::SetFontGUIText(GUIText* guiText) {
	fontGuiText = guiText;
	if (fontGuiText == NULL)fontGuiText = &GUITextDefault;
	textHeight = fontGuiText->GetTextHeight(fontSize);
}

void GUIListBox::SetFontSize(int size) {
	fontSize = size;
	textHeight = fontGuiText->GetTextHeight(fontSize);
}

void GUIListBox::SetFontColor(Uint32 color) {
	fontColor = color;
}

void GUIListBox::SetFontBackColor(Uint32 backColor) {
	fontBackColor = backColor;
}

bool GUIListBox::RenderText() {
	if (text == NULL) {
		textSize = 0;
		text = (char*)malloc(sizeof(char) * 1);
		if (text == NULL)return false;
		text[0] = 0;
	}
	itemCount = 1;
	if (textSize > 0) {
		for (int n = 0; n < textSize; n++) {
			if (text[n] == 0) {
				text[n] = '\n';
				itemCount++;
			}
		}
	}
	else itemCount = 0;
	textHeight = fontGuiText->GetTextHeight(fontSize);
	float ss = ((float)(textY2 - textY1) / (itemCount * textHeight));
	scroll.SetScrollSize(ss);
	if (ss < 1.f)scroll.SetDisabled(false);
	else scroll.SetDisabled(true);
	ss = ((float)(textY2 - textY1) / (itemCount * textHeight));
	scroll.SetScrollSize(ss);
	bool ret = false;
	if (textSize > 1) {
		for (int n = 0; n < textSize; n++)if (text[n] == 0)text[n] = '\n';
		renderedText.SwitchRender(fontGuiText);
		ret = fontGuiText->Render(text, fontColor, fontSize, true);
		renderedText.SwitchRender(fontGuiText);
		for (int n = 0; n < textSize; n++)if (text[n] == '\n')text[n] = 0;
	}
	else {
		renderedText.FreeRender();
		ret = true;
	}
	return ret;
}

void GUIListBox::RecalculateSelectionBox() {
	curOffY = -scroll.GetScrollPos() * ((itemCount * textHeight) - textY2 + textY1);
	if (selected < 0)showSelectionBox = false;
	else {
		showSelectionBox = true;
		selX1 = textX1;
		selY1 = textY1 + (selected * textHeight) + curOffY;
		selX2 = textX2;
		selY2 = selY1 + textHeight;
		if (selY2 < textY1)showSelectionBox = false;
		if (selY1 > textY2)showSelectionBox = false;
		if (showSelectionBox) {
			if (selY1 < textY1)selY1 = textY1;
			if (selY2 > textY2)selY2 = textY2;
		}
	}
}

GUIListBox& GUIListBox::operator = (const GUIListBox& param) {
	if (this == &param)return *this;
	char* txt = (char*)realloc(text, sizeof(char) * (param.textSize + 1));
	if (txt != NULL) {
		text = txt;
		textSize = param.textSize;
		for (int n = 0; n <= textSize; n++)text[n] = param.text[n];
		itemCount = param.itemCount;
		selected = param.selected;
	}
	RenderText();
	return *this;
}

GUIListBox& GUIListBox::operator = (const class GUIDropDown& param) {
	*this = param.listBox;
	RenderText();
	return *this;
}

/* GUITextBox */

GUITextBox::GUITextBox() {
	Construct();
}

GUITextBox::~GUITextBox() {
	Destruct();
}

void GUITextBox::Construct() {
	posX1 = 0; posY1 = 0; posX2 = 0; posY2 = 0; isDisabled = false; isHovered = false;
	text = NULL; textUni = NULL; textSize = 0;
	renderedText.Construct();
	fontGuiText = NULL; fontSize = 0; setFontSize = 0; fontColor = 0x000000FF, fontBackColor = 0xFFFFFFFF; fontUseUnicode = true;
	textX1 = 0; textY1 = 0; textX2 = 0; textY2 = 0; isTyping = false;
	textBorderDown = 0; textBorderLeft = 0; textBorderRight = 0; textBorderUp = 0;
	textSliderThick = 20; textMaxLineWidth = 0; textNewlineCount = 0; textHeight = 1;
	curBlink = true; curTick = 0;
	curPos = 0; curDrawX = 0; curDrawY = 0; curDrawH = 0; curX = 0; curY = 0; curOffX = 0; curOffY = 0;
	mouseX = 0; mouseY = 0; scrollX.Construct(); scrollY.Construct();
	showScroll = false; showPointer = false; setIsScrollable = true; setAutoRender = true; setMaxLines = 1; setAutoHideCursor = true;
	StyleConstruct();
	scrollX.SetHorizontal(true); scrollY.SetHorizontal(false);
	scrollX.SetScrollSize(1.f);
	scrollY.SetScrollSize(1.f);
	scrollX.SetDisabled(true);
	scrollY.SetDisabled(true);
	scrollX.SetArrowWidth(textSliderThick);
	scrollY.SetArrowWidth(textSliderThick);
	SetFontGUIText(); SetFontSize(); SetFontColor(); SetFontUseUnicode();
}

void GUITextBox::Destruct() {
	if (text != NULL)free(text); text = NULL;
	if (textUni != NULL)free(textUni); textUni = NULL;
	textSize = 0;
	renderedText.Destruct();
	scrollX.Destruct(); scrollY.Destruct();
	Unregister();
}

bool GUITextBox::Event(SDL_Event* event, int offX, int offY) {
	if (isDisabled)return false;
	Check();
	if (scrollX.Event(event, offX, offY))return true;
	if (scrollY.Event(event, offX, offY))return true;
	int mx, my; bool is;
	switch (event->type) {
	case SDL_MOUSEWHEEL:
	{

		if (event->wheel.y > 0) {
			if (isTyping) {
				scrollY.SetScrollPos(scrollY.GetScrollPos() - scrollY.GetScrollTrackMove());
				curOffY = -scrollY.GetScrollPos() * ((textNewlineCount * textHeight) - textY2 + textY1);
				return true;
			}
		}
		if (event->wheel.y < 0) {
			if (isTyping) {
				scrollY.SetScrollPos(scrollY.GetScrollPos() + scrollY.GetScrollTrackMove());
				curOffY = -scrollY.GetScrollPos() * ((textNewlineCount * textHeight) - textY2 + textY1);
				return true;
			}
		}

	}
	break;
	case SDL_MOUSEBUTTONDOWN:
		if (event->button.button == SDL_BUTTON_LEFT) {
			mx = event->button.x + offX;
			my = event->button.y + offY;
			is = GUIIsInside(mx, my, textX1, textY1, textX2, textY2);
			if (is != isTyping) {
				SDLPushEvent((is) ? GUITEXTBOX_STARTTYPING : GUITEXTBOX_STOPTYPING, this, NULL);
				isTyping = is;
			}
			if (is) {
				SetCurPos(mx - textX1 - curOffX, my - textY1 - curOffY);
				return true;
			}
		}
		break;
	case SDL_MOUSEMOTION:
		mx = event->motion.x + offX;
		my = event->motion.y + offY;
		is = GUIIsInside(mx, my, textX1, textY1, textX2, textY2);
		if (is != isHovered) {
			if (setAutoHideCursor)SDL_ShowCursor((is) ? SDL_DISABLE : SDL_ENABLE);
			SDLPushEvent((is) ? GUI_MOUSEIN : GUI_MOUSEOUT, this, NULL);
			isHovered = is;
		}
		if (is) {
			mouseX = mx; mouseY = my;
			showPointer = true;
		}
		else {
			mouseX = 0; mouseY = 0;
			showPointer = false;
		}
		break;
	case SDL_KEYDOWN:
		if (isTyping) {
			if (event->key.keysym.sym == SDLK_RETURN) {
				SDLPushEvent(GUITEXTBOX_RETURN, this, NULL);
				AddChar('\n', CharToUni('\n'));
			}
			else if (event->key.keysym.sym == SDLK_BACKSPACE) {
				RemoveChar(false);
			}
			else if (event->key.keysym.sym == SDLK_DELETE) {
				RemoveChar(true);
			}
			else if (event->key.keysym.sym == SDLK_LEFT) {
				CurLeft();
			}
			else if (event->key.keysym.sym == SDLK_RIGHT) {
				CurRight();
			}
			else if (event->key.keysym.sym == SDLK_UP) {
				CurUp();
			}
			else if (event->key.keysym.sym == SDLK_DOWN) {
				CurDown();
			}
			else {

				/*if(SDL_EnableUNICODE(-1)==1&&fontUseUnicode){
				  Uint16 chr=event->key.keysym.unicode;
				  if(chr>0)AddChar(UniToChar(chr),chr);
				}else*/
				{
					char chr = 0;
					if ((event->key.keysym.sym > 31 && event->key.keysym.sym < 36) || (event->key.keysym.sym > 36 && event->key.keysym.sym < 65) || (event->key.keysym.sym > 90 && event->key.keysym.sym < 97)) {
						if (SDL_GetModState() & KMOD_LSHIFT) {
							switch (event->key.keysym.sym) {
							case '1': chr = '!'; break; case '2': chr = '"'; break; case '3': chr = '#'; break; case '4': chr = '$'; break; case '5': chr = '%'; break;
							case '6': chr = '&'; break; case '7': chr = '/'; break; case '8': chr = '('; break; case '9': chr = ')'; break;
							case '-': chr = '?'; break; case '=': chr = '*'; break; case '<': chr = '>'; break;
							default: chr = event->key.keysym.sym;
							}
						}
						else { chr = event->key.keysym.sym; }
					}
					else if (event->key.keysym.sym > 96 && event->key.keysym.sym < 123) {
						chr = event->key.keysym.sym;
						if ((SDL_GetModState() & KMOD_CAPS) || (SDL_GetModState() & KMOD_LSHIFT))chr -= 32;
					}
					if (chr > 31 && chr < 127)AddChar(chr, CharToUni(chr));
				}
			}
			return true;
		}
		break;
	case SDL_USEREVENT:
		if (event->user.code == GUISCROLLBAR_SCROLL) {
			if (event->user.data1 == &scrollX) {
				curOffX = -scrollX.GetScrollPos() * (textMaxLineWidth - textX2 + textX1);
				return true;
			}
			else if (event->user.data1 == &scrollY) {
				curOffY = -scrollY.GetScrollPos() * ((textNewlineCount * textHeight) - textY2 + textY1);
				return true;
			}
		}
		break;
	}
	return false;
}

bool GUITextBox::IsDisabled() {
	return isDisabled;
}

bool GUITextBox::IsHovered() {
	return isHovered;
}

void GUITextBox::SetDisabled(bool disabled) {
	isDisabled = disabled;
	if (disabled) {
		isHovered = false;
		scrollX.SetDisabled(true);
		scrollY.SetDisabled(true);
	}
}

void GUITextBox::SetPos(int x1, int y1, int x2, int y2) {
	posX1 = x1;
	posY1 = y1;
	posX2 = x2;
	posY2 = y2;
	textX1 = x1 + textBorderLeft;
	textY1 = y1 + textBorderUp;
	textX2 = x2 - textBorderRight;
	textY2 = y2 - textBorderDown;
	if (showScroll) {
		textX2 -= textSliderThick;
		textY2 -= textSliderThick;
		scrollX.SetPos(x1, y2 - textSliderThick, x2 - textSliderThick, y2);
		scrollY.SetPos(x2 - textSliderThick, y1, x2, y2 - textSliderThick);
	}
	RenderText();
}

void GUITextBox::Register(int zIndex) {
	GUIElementAdd((void*)this, zIndex, GUI_TEXTBOX);
}

void GUITextBox::Unregister() {
	GUIElementRemove((void*)this, GUI_TEXTBOX);
}

void GUITextBox::SetMaxLines(int maxLines) {
	Check();
	if (setMaxLines < maxLines) {
		int nl = 0;
		for (int n = 0; n < textSize; n++) {
			if (text[n] == '\n') {
				nl++;
				if (nl >= maxLines) {
					text[n] = 0;
					textUni[n] = 0;
					textSize = n + 1;
					char* _text;
					Uint16* _textUni;
					_text = (char*)realloc(text, (textSize + 1) * sizeof(char));
					if (_text != NULL)text = _text;
					_textUni = (Uint16*)realloc(textUni, (textSize + 1) * sizeof(Uint16));
					if (_textUni != NULL)textUni = _textUni;
					textSize = n;
					break;
				}
			}
		}
	}
	setMaxLines = maxLines;
	if (setIsScrollable)showScroll = (setMaxLines != 1);
	SetPos(posX1, posY1, posX2, posY2);
	RecalculateCursor();
	if (setAutoRender)RenderText();
}

void GUITextBox::SetIsScrollable(bool isScrollable) {
	setIsScrollable = isScrollable;
	if (setIsScrollable == false && showScroll == true) {
		showScroll = false;
		SetPos(posX1, posY1, posX2, posY2);
	}
}

void GUITextBox::SetAutoHideCursor(bool autoHide) {
	setAutoHideCursor = autoHide;
}

void GUITextBox::SetAutoRender(bool autoRender) {
	setAutoRender = autoRender;
}

void GUITextBox::SetCurPos(int x, int y) {
	Check();
	int n = 0, k = (y / textHeight), t;
	if (setMaxLines == 1)k = 0;
	for (; n <= textSize && k > 0; n++)if (text[n] == '\n')k--;
	for (t = n; t <= textSize && text[t] != '\n'; t++);
	if (fontUseUnicode) {
		Uint16 chr = text[t];
		textUni[t] = 0;
		if (fontGuiText->GetTextWidth(textUni + n, fontSize) < x) {
			k = t + 1;
			textUni[t] = chr;
		}
		else {
			textUni[t] = chr;
			for (k = n; k <= textSize && k < t; k++) {
				chr = textUni[k]; textUni[k] = 0;
				if (fontGuiText->GetTextWidth(textUni + n, fontSize) > x) {
					textUni[k] = chr;
					break;
				}
				textUni[k] = chr;
			}
		}
	}
	else {
		char chr = text[t];
		text[t] = 0;
		if (fontGuiText->GetTextWidth(text + n, fontSize) < x) {
			k = t + 1;
			text[t] = chr;
		}
		else {
			text[t] = chr;
			for (k = n; k <= textSize && k < t; k++) {
				chr = text[k]; text[k] = 0;
				if (fontGuiText->GetTextWidth(text + n, fontSize) > x) {
					text[k] = chr;
					break;
				}
				text[k] = chr;
			}
		}
	}
	curPos = k - 1;
	if (curPos > textSize)curPos = textSize;
	if (curPos < 0)curPos = 0;
	CursorBlink(true);
	RecalculateCursor();
}

void GUITextBox::SetText(const char* txt) {
	Check();
	if (txt == NULL)txt = "";
	int oldTextSize = textSize;
	textSize = strlen(txt) + 1;
	char* _text = (char*)realloc(text, (textSize + 1) * sizeof(char));
	if (_text == NULL) { textSize = oldTextSize; return; }
	text = _text;
	Uint16* _textUni = (Uint16*)realloc(textUni, (textSize + 1) * sizeof(Uint16));
	if (_textUni == NULL) { textSize = oldTextSize; return; }
	textUni = _textUni;
	for (int n = 0; n < textSize; n++) {
		text[n] = txt[n];
		textUni[n] = CharToUni(txt[n]);
	}
	textSize--; curPos = 0;
	RecalculateCursor();
	if (setAutoRender)RenderText();
}

const char* GUITextBox::GetText() {
	Check();
	return text;
}

const Uint16* GUITextBox::GetUnicode() {
	Check();
	return textUni;
}

int GUITextBox::GetTextSize() {
	return textSize;
}

bool GUITextBox::RenderText() {
	Check();
	if (textSize > 0) {
		renderedText.SwitchRender(fontGuiText);
		bool ret;
		if (fontUseUnicode)ret = fontGuiText->Render(textUni, fontColor, fontSize, true);
		else ret = fontGuiText->Render(text, fontColor, fontSize, true);
		renderedText.SwitchRender(fontGuiText);
		if (!ret)return false;
		return renderedText.IsRendered();
	}
	else renderedText.FreeRender();
	return true;
}

void GUITextBox::SetTyping(bool typing) {
	isTyping = typing;
}

bool GUITextBox::IsTyping() {
	return isTyping;
}

void GUITextBox::SetFontGUIText(GUIText* guiText) {
	fontGuiText = guiText;
	if (fontGuiText == NULL)fontGuiText = &GUITextDefault;
}

void GUITextBox::SetFontSize(int size) {
	setFontSize = size;
	fontSize = setFontSize;
}

void GUITextBox::SetFontColor(Uint32 color) {
	fontColor = color;
}

void GUITextBox::SetFontBackColor(Uint32 backColor) {
	fontBackColor = backColor;
}

void GUITextBox::SetFontUseUnicode(bool useUnicode) {
	fontUseUnicode = useUnicode;
}

bool GUITextBox::CursorBlink(bool forceShow) {
	Uint32 cTick = SDL_GetTicks();
	if (cTick - curTick > 500) {
		curBlink = !curBlink;
		curTick = cTick;
	}
	if (forceShow) { curBlink = true; curTick = cTick; }
	return (isTyping && curBlink);
}

void GUITextBox::CurUp() {
	SetCurPos(curX + 1, curY - textHeight);
}

void GUITextBox::CurDown() {
	SetCurPos(curX + 1, curY + textHeight);
}

void GUITextBox::CurLeft() {
	Check();
	if (curPos > 0)curPos--;
	int pos; for (pos = curPos - 1; pos >= 0; pos--)if (text[pos] == '\n')break;
	CursorBlink(true);
	RecalculateCursor();
}

void GUITextBox::CurRight() {
	Check();
	if (curPos < textSize)curPos++;
	int pos; for (pos = curPos - 1; pos >= 0; pos--)if (text[pos] == '\n')break;
	CursorBlink(true);
	RecalculateCursor();
}

void GUITextBox::RemoveChar(bool del) {
	Check();
	if (!del && curPos == 0)return;
	if (del && curPos == textSize)return;
	if (!del)curPos--;
	for (int n = curPos; n < textSize; n++) {
		text[n] = text[n + 1];
		textUni[n] = textUni[n + 1];
	}
	if (!del)curPos++;
	textSize--;
	if (!del)curPos--;
	CursorBlink(true);
	RecalculateCursor();
	if (setAutoRender)RenderText();
}

void GUITextBox::AddChar(char chr, Uint16 uni) {
	if ((chr == '\n') && (textNewlineCount + 1 >= setMaxLines) && (setMaxLines > 0))return;
	textSize++;
	textSize++;
	char* _text = (char*)realloc(text, (textSize + 1) * sizeof(char));
	if (_text == NULL) { textSize -= 2; return; }
	text = _text;
	Uint16* _textUni = (Uint16*)realloc(textUni, (textSize + 1) * sizeof(Uint16));
	if (_textUni == NULL) { textSize -= 2; return; }
	textUni = _textUni;
	textSize--;
	for (int n = textSize; n > curPos; n--) {
		text[n] = text[n - 1];
		textUni[n] = textUni[n - 1];
	}
	text[curPos] = chr;
	textUni[curPos] = uni;
	text[textSize] = 0;
	textUni[textSize] = 0;
	curPos++;
	int pos; for (pos = curPos - 1; pos >= 0; pos--)if (text[pos] == '\n')break;
	CursorBlink(true);
	RecalculateCursor();
	if (setAutoRender)RenderText();
}

void GUITextBox::RecalculateCursor() {
	Check();
	textNewlineCount = 0; textMaxLineWidth = 0;
	textHeight = fontGuiText->GetTextHeight(fontSize);
	int textNLinesToCur = 0, nlPos = -1, nlPosCur = -1;
	for (int n = 0; n <= textSize; n++) {
		if (text[n] == '\n' || text[n] == 0) {
			if (fontUseUnicode) {
				Uint16 chr = textUni[n]; textUni[n] = 0;
				int s = fontGuiText->GetTextWidth(textUni + nlPos + 1, fontSize);
				if (textMaxLineWidth < s)textMaxLineWidth = s;
				textUni[n] = chr;
			}
			else {
				char chr = text[n]; text[n] = 0;
				int s = fontGuiText->GetTextWidth(text + nlPos + 1, fontSize);
				if (textMaxLineWidth < s)textMaxLineWidth = s;
				text[n] = chr;
			}
		}
		if (text[n] == '\n') {
			textNewlineCount++;
			nlPos = n;
			if (n < curPos) {
				textNLinesToCur++;
				nlPosCur = n;
			}
		}
	}
	curY = textNLinesToCur * textHeight;
	if (fontUseUnicode) {
		Uint16 t = textUni[curPos];
		textUni[curPos] = 0;
		curX = fontGuiText->GetTextWidth(textUni + nlPosCur + 1, fontSize);
		textUni[curPos] = t;
	}
	else {
		char t = text[curPos];
		text[curPos] = 0;
		curX = fontGuiText->GetTextWidth(text + nlPosCur + 1, fontSize);
		text[curPos] = t;
	}
	if (textX1 + curX + curOffX > textX2)curOffX = textX2 - textX1 - curX;
	if (textX1 + curX + curOffX < textX1)curOffX = -curX;
	if (textY1 + curY + curOffY + textHeight > textY2)curOffY = textY2 - textY1 - curY - textHeight;
	if (textY1 + curY + curOffY < textY1)curOffY = -curY;
	curDrawX = curX + textX1 + curOffX; curDrawY = curY + textY1 + curOffY; curDrawH = (setMaxLines == 1) ? (textY2 - textY1) : textHeight;
	if (showScroll) {
		float ss = ((float)(textX2 - textX1) / textMaxLineWidth);
		scrollX.SetScrollSize(ss);
		if (ss < 1.f)scrollX.SetDisabled(false);
		else scrollX.SetDisabled(true);
		ss = ((float)(textY2 - textY1) / (textNewlineCount * textHeight));
		scrollY.SetScrollSize(ss);
		if (ss < 1.f)scrollY.SetDisabled(false);
		else scrollY.SetDisabled(true);
		ss = (float)(-curOffX) / (textMaxLineWidth - textX2 + textX1);
		scrollX.SetScrollPos(ss);
		ss = (float)(-curOffY) / ((textNewlineCount * textHeight) - textY2 + textY1);
		scrollY.SetScrollPos(ss);
		ss = 1.f / (textNewlineCount - ((textY2 - textY1) / textHeight));
		scrollY.SetScrollMove(ss, ss * 3);
	}
}

void GUITextBox::Check() {
	if (text == NULL || textUni == NULL) {
		if (text != NULL)free(text); text = NULL;
		if (textUni != NULL)free(textUni); textUni = NULL;
		text = (char*)malloc(sizeof(char));
		if (text != NULL)text[0] = 0;
		textUni = (Uint16*)malloc(sizeof(Uint16));
		if (textUni != NULL)textUni[0] = 0;
		textSize = 0;
	}
	if (fontGuiText == NULL)fontGuiText = &GUITextDefault;
	if (setMaxLines == 1)fontSize = textY2 - textY1;
	else fontSize = setFontSize;
}

/* GUIScrollBar */

GUIScrollBar::GUIScrollBar() {
	Construct();
}

GUIScrollBar::~GUIScrollBar() {
	Destruct();
}

void GUIScrollBar::Construct() {
	posX1 = 0; posY1 = 0; posX2 = 0; posY2 = 0; isDisabled = false; isHovered = false;
	scrollPos = 0.f; scrollSize = 0.25f;
	isScrolling = false; isHorizontal = true;
	arrADown = false; arrBDown = false;
	SetScrollMove();
	scrollMouse = 0; SetArrowWidth();
	StyleConstruct();
}

void GUIScrollBar::Destruct() {
	Unregister();
}

bool GUIScrollBar::Event(SDL_Event* event, int offX, int offY) {
	if (isDisabled)return false;
	int mx, my; bool is;
	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN:
		if (event->button.button == SDL_BUTTON_LEFT) {
			mx = event->button.x + offX;
			my = event->button.y + offY;
			if (GUIIsInside(mx, my, slidX1, slidY1, slidX2, slidY2)) {
				isScrolling = true;
				if (isHorizontal)scrollMouse = slidX1 + (slidW / 2) - mx;
				else scrollMouse = slidY1 + (slidW / 2) - my;
				return true;
			}
			else if (GUIIsInside(mx, my, arrAX1, arrAY1, arrAX2, arrAY2)) {
				arrADown = true;
				scrollPos -= scrollMove; if (scrollPos < 0.f)scrollPos = 0.f;
				RecalculateSlider();
				SDLPushEvent(GUISCROLLBAR_SCROLL, this, NULL);
				return true;
			}
			else if (GUIIsInside(mx, my, arrBX1, arrBY1, arrBX2, arrBY2)) {
				arrBDown = true;
				scrollPos += scrollMove; if (scrollPos > 1.f)scrollPos = 1.f;
				RecalculateSlider();
				SDLPushEvent(GUISCROLLBAR_SCROLL, this, NULL);
				return true;
			}
			else if (GUIIsInside(mx, my, trackX1, trackY1, trackX2, trackY2)) {
				if ((isHorizontal) ? (mx < slidX1) : (my < slidY1)) {
					scrollPos -= scrollMoveTrack;
					if (scrollPos < 0.f)scrollPos = 0.f;
				}
				else {
					scrollPos += scrollMoveTrack;
					if (scrollPos > 1.f)scrollPos = 1.f;
				}
				RecalculateSlider();
				SDLPushEvent(GUISCROLLBAR_SCROLL, this, NULL);
				return true;
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if (event->button.button == SDL_BUTTON_LEFT) {
			mx = event->button.x + offX;
			my = event->button.y + offY;
			if (isScrolling || arrADown || arrBDown) {
				isScrolling = false;
				arrADown = false;
				arrBDown = false;
				return true;
			}
		}
		break;
	case SDL_MOUSEMOTION:
		mx = event->motion.x + offX;
		my = event->motion.y + offY;
		is = GUIIsInside(mx, my, posX1, posY1, posX2, posY2);
		if (is != isHovered)SDLPushEvent((is) ? GUI_MOUSEIN : GUI_MOUSEOUT, this, NULL);
		isHovered = is;
		if (isScrolling) {
			if (isHorizontal) {
				scrollPos = (mx - trackX1 - (slidW / 2) + scrollMouse) / float(trackX2 - trackX1 - slidW);
			}
			else {
				scrollPos = (my - trackY1 - (slidW / 2) + scrollMouse) / float(trackY2 - trackY1 - slidW);
			}
			if (scrollPos > 1.f)scrollPos = 1.f;
			if (scrollPos < 0.f)scrollPos = 0.f;
			SDLPushEvent(GUISCROLLBAR_SCROLL, this, NULL);
			RecalculateSlider();
			return true;
		}
		break;
	}
	return false;
}

bool GUIScrollBar::IsDisabled() {
	return isDisabled;
}

bool GUIScrollBar::IsHovered() {
	return isHovered;
}

void GUIScrollBar::SetDisabled(bool disabled) {
	isDisabled = disabled;
	if (disabled)isHovered = false;
}

void GUIScrollBar::SetPos(int x1, int y1, int x2, int y2) {
	posX1 = x1;
	posY1 = y1;
	posX2 = x2;
	posY2 = y2;
	RecalculateCoords();
}

void GUIScrollBar::Register(int zIndex) {
	GUIElementAdd((void*)this, zIndex, GUI_SCROLLBAR);
}

void GUIScrollBar::Unregister() {
	GUIElementRemove((void*)this, GUI_SCROLLBAR);
}

void GUIScrollBar::SetHorizontal(bool horizontal) {
	isHorizontal = horizontal;
	RecalculateCoords();
}

void GUIScrollBar::SetArrowWidth(int width) {
	arrowW = width;
	RecalculateCoords();
}

void GUIScrollBar::SetScrollSize(float size) {
	if (size < 0.f)size = 0.f; if (size > 1.f)size = 1.f;
	scrollSize = size;
	RecalculateSlider();
}

void GUIScrollBar::SetScrollPos(float pos) {
	if (pos < 0.f)pos = 0.f; if (pos > 1.f)pos = 1.f;
	scrollPos = pos;
	RecalculateSlider();
}

void GUIScrollBar::SetScrollMove(float arrowMove, float trackMove) {
	if (arrowMove < 0.f)arrowMove = 0.f; if (arrowMove > 1.f)arrowMove = 1.f;
	if (trackMove < 0.f)trackMove = 0.f; if (trackMove > 1.f)trackMove = 1.f;
	scrollMove = arrowMove;
	scrollMoveTrack = trackMove;
}

float GUIScrollBar::GetScrollSize() {
	return scrollSize;
}

float GUIScrollBar::GetScrollPos() {
	return scrollPos;
}

float GUIScrollBar::GetScrollArrowMove() {
	return scrollMove;
}

float GUIScrollBar::GetScrollTrackMove() {
	return scrollMoveTrack;
}

bool GUIScrollBar::IsHorizontal() {
	return isHorizontal;
}

bool GUIScrollBar::IsScrolling() {
	return isScrolling;
}

void GUIScrollBar::RecalculateCoords() {
	trackX1 = posX1 + ((isHorizontal) ? arrowW : 0);
	trackY1 = posY1 + ((isHorizontal) ? 0 : arrowW);
	trackX2 = posX2 - ((isHorizontal) ? arrowW : 0);
	trackY2 = posY2 - ((isHorizontal) ? 0 : arrowW);

	if (isHorizontal) {
		arrAX1 = posX1; arrAY1 = posY1; arrAX2 = posX1 + arrowW; arrAY2 = posY2;
		arrBX1 = posX2 - arrowW; arrBY1 = posY1; arrBX2 = posX2; arrBY2 = posY2;
	}
	else {
		arrAX1 = posX1; arrAY1 = posY1; arrAX2 = posX2; arrAY2 = posY1 + arrowW;
		arrBX1 = posX1; arrBY1 = posY2 - arrowW; arrBX2 = posX2; arrBY2 = posY2;
	}

	if (isHorizontal) {
		triAX1 = posX1 + (arrowW * 0.375f); triAY1 = (posY1 + posY2) / 2;
		triAX2 = triAX1 + (arrowW / 4);     triAY2 = triAY1 - (arrowW / 4);
		triAX3 = triAX2;                triAY3 = triAY1 + (arrowW / 4);
		triBX1 = posX2 - (arrowW * 0.375f); triBY1 = (posY1 + posY2) / 2;
		triBX2 = triBX1 - (arrowW / 4);     triBY2 = triBY1 - (arrowW / 4);
		triBX3 = triBX2;                triBY3 = triBY1 + (arrowW / 4);
	}
	else {
		triAX1 = (posX1 + posX2) / 2;       triAY1 = posY1 + (arrowW * 0.375f);
		triAX2 = triAX1 - (arrowW / 4);     triAY2 = triAY1 + (arrowW / 4);
		triAX3 = triAX1 + (arrowW / 4);     triAY3 = triAY2;
		triBX1 = (posX1 + posX2) / 2;       triBY1 = posY2 - (arrowW * 0.375f);
		triBX2 = triBX1 - (arrowW / 4);     triBY2 = triBY1 - (arrowW / 4);
		triBX3 = triBX1 + (arrowW / 4);     triBY3 = triBY2;
	}

	RecalculateSlider();
}

void GUIScrollBar::RecalculateSlider() {
	if (isHorizontal) {
		slidW = scrollSize * (trackX2 - trackX1);
		slidX1 = trackX1 + (scrollPos * (trackX2 - trackX1 - slidW)) + 1; slidY1 = trackY1;
		slidX2 = slidX1 + slidW - 2; slidY2 = trackY2;
	}
	else {
		slidW = scrollSize * (trackY2 - trackY1);
		slidX1 = trackX1; slidY1 = trackY1 + (scrollPos * (trackY2 - trackY1 - slidW)) + 1;
		slidX2 = trackX2; slidY2 = slidY1 + slidW - 2;
	}
}

/* GUIButton */

GUIButton::GUIButton() {
	Construct();
}

GUIButton::~GUIButton() {
	Destruct();
}

void GUIButton::Construct() {
	posX1 = 0; posY1 = 0; posX2 = 0; posY2 = 0; isDisabled = false; isHovered = false;
	renderedText.Construct();
	isPressed = false;
	textColor.r = 0x00; textColor.g = 0x00; textColor.b = 0x00; textColor.a = 0xFF;
	StyleConstruct();
}

void GUIButton::Destruct() {
	renderedText.Destruct();
	Unregister();
}

bool GUIButton::Event(SDL_Event* event, int offX, int offY) {
	if (isDisabled)return false;
	int mx, my; bool is;
	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN:
		if (event->button.button == SDL_BUTTON_LEFT) {
			mx = event->button.x + offX;
			my = event->button.y + offY;
			if (!isPressed && GUIIsInside(mx, my, posX1, posY1, posX2, posY2)) {
				SDLPushEvent(GUIBUTTON_PRESS, this, NULL);
				isPressed = true;
				return true;
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if (event->button.button == SDL_BUTTON_LEFT) {
			mx = event->button.x + offX;
			my = event->button.y + offY;
			if (isPressed) {
				if (GUIIsInside(mx, my, posX1, posY1, posX2, posY2))SDLPushEvent(GUIBUTTON_RELEASE, this, NULL);
				isPressed = false;
				return true;
			}
		}
		break;
	case SDL_MOUSEMOTION:
		mx = event->motion.x + offX;
		my = event->motion.y + offY;
		is = GUIIsInside(mx, my, posX1, posY1, posX2, posY2);
		if (is != isHovered)SDLPushEvent((is) ? GUI_MOUSEIN : GUI_MOUSEOUT, this, NULL);
		isHovered = is;
		break;
	}
	return false;
}

bool GUIButton::IsDisabled() {
	return isDisabled;
}

bool GUIButton::IsHovered() {
	return isHovered;
}

void GUIButton::SetDisabled(bool disabled) {
	isDisabled = disabled;
	if (disabled)isHovered = false;
}

void GUIButton::SetPos(int x1, int y1, int x2, int y2) {
	posX1 = x1;
	posY1 = y1;
	posX2 = x2;
	posY2 = y2;
}

void GUIButton::Register(int zIndex) {
	GUIElementAdd((void*)this, zIndex, GUI_BUTTON);
}

void GUIButton::Unregister() {
	GUIElementRemove((void*)this, GUI_BUTTON);
}

bool GUIButton::IsPressed() {
	return isPressed;
}

bool GUIButton::SetText(const char* text, int textSize, GUIText* guiText) {
	if (text == NULL) {
		renderedText.FreeRender();
		return true;
	}
	int w = posX2 - posX1, h = posY2 - posY1;
	if (guiText == NULL)guiText = &GUITextDefault;
	if (text != NULL) {
		if (textSize <= 0) {
			textSize = guiText->RecommendedSize(text, w + textSize, h);
		}
		renderedText.SwitchRender(guiText);
		bool ret = guiText->Render(text, textColor, textSize);
		renderedText.SwitchRender(guiText);
		if (!ret)return false;
	}
	if (!renderedText.IsRendered())return false;
	return true;
}

/* GUICheckBox */

GUICheckBox::GUICheckBox() {
	Construct();
}

GUICheckBox::~GUICheckBox() {
	Destruct();
}

void GUICheckBox::Construct() {
	posX1 = 0; posY1 = 0; posX2 = 0; posY2 = 0; isDisabled = false; isHovered = false;
	isChecked = false; isCircle = false;
	StyleConstruct();
}

void GUICheckBox::Destruct() {
	Unregister();
}

bool GUICheckBox::Event(SDL_Event* event, int offX, int offY) {
	if (isDisabled)return false;
	int mx, my; bool is;
	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN:
		if (event->button.button == SDL_BUTTON_LEFT) {
			mx = event->button.x + offX;
			my = event->button.y + offY;
			if (GUIIsInside(mx, my, posX1, posY1, posX2, posY2)) {
				isChecked = !isChecked;
				SDLPushEvent((isChecked) ? GUICHECKBOX_CHECK : GUICHECKBOX_UNCHECK, this, NULL);
				return true;
			}
		}
		break;
	case SDL_MOUSEMOTION:
		mx = event->motion.x + offX;
		my = event->motion.y + offY;
		is = GUIIsInside(mx, my, posX1, posY1, posX2, posY2);
		if (is != isHovered)SDLPushEvent((is) ? GUI_MOUSEIN : GUI_MOUSEOUT, this, NULL);
		isHovered = is;
		break;
	}
	return false;
}

bool GUICheckBox::IsDisabled() {
	return isDisabled;
}

bool GUICheckBox::IsHovered() {
	return isHovered;
}

void GUICheckBox::SetDisabled(bool disabled) {
	isDisabled = disabled;
	if (disabled)isHovered = false;
}

void GUICheckBox::SetPos(int x1, int y1, int x2, int y2) {
	posX1 = x1;
	posY1 = y1;
	posX2 = x2;
	posY2 = y2;
}

void GUICheckBox::Register(int zIndex) {
	GUIElementAdd((void*)this, zIndex, GUI_CHECKBOX);
}

void GUICheckBox::Unregister() {
	GUIElementRemove((void*)this, GUI_CHECKBOX);
}

bool GUICheckBox::IsChecked() {
	return isChecked;
}

bool GUICheckBox::IsCircle() {
	return isCircle;
}

void GUICheckBox::SetChecked(bool checked) {
	isChecked = checked;
}

void GUICheckBox::SetCircle(bool circle) {
	isCircle = circle;
}

/* GUILabel */

GUILabel::GUILabel() {
	Construct();
}

GUILabel::~GUILabel() {
	Destruct();
}

void GUILabel::Construct() {
	posX1 = 0; posY1 = 0; posX2 = 0; posY2 = 0; isDisabled = false; isHovered = false;
	text.Construct(); centerX = true; centerY = true;
	StyleConstruct();
}

void GUILabel::Destruct() {
	Unregister();
	text.Destruct();
}

bool GUILabel::Event(SDL_Event* event, int offX, int offY) {
	if (isDisabled)return false;
	int mx, my; bool is;
	switch (event->type) {
	case SDL_MOUSEMOTION:
		mx = event->motion.x + offX;
		my = event->motion.y + offY;
		is = GUIIsInside(mx, my, posX1, posY1, posX2, posY2);
		if (is != isHovered)SDLPushEvent((is) ? GUI_MOUSEIN : GUI_MOUSEOUT, this, NULL);
		isHovered = is;
		break;
	}
	return false;
}

bool GUILabel::IsDisabled() {
	return isDisabled;
}

bool GUILabel::IsHovered() {
	return isHovered;
}

void GUILabel::SetDisabled(bool disabled) {
	isDisabled = disabled;
	if (disabled)isHovered = false;
}

void GUILabel::SetPos(int x1, int y1, int x2, int y2) {
	if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
	if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
	posX1 = x1;
	posY1 = y1;
	posX2 = x2;
	posY2 = y2;
}

void GUILabel::Register(int zIndex) {
	GUIElementAdd((void*)this, zIndex, GUI_LABEL);
}

void GUILabel::Unregister() {
	GUIElementRemove((void*)this, GUI_LABEL);
}

GUIText* GUILabel::GetText() {
	return &text;
}

bool GUILabel::GetCenterX() {
	return centerX;
}

bool GUILabel::GetCenterY() {
	return centerY;
}

void GUILabel::SetCenterX(bool set) {
	centerX = set;
}

void GUILabel::SetCenterY(bool set) {
	centerY = set;
}

bool GUILabel::Render(const char* txt, Uint32 color, int size, bool checkNewline, bool useUTF8) {
	if (size <= 0)size = text.RecommendedSize(txt, posX2 - posX1, posY2 - posY1);
	return text.Render(txt, color, size, checkNewline, useUTF8);
}

bool GUILabel::Render(Uint16* txt, Uint32 color, int size, bool checkNewline) {
	if (size <= 0)size = text.RecommendedSize(txt, posX2 - posX1, posY2 - posY1);
	return text.Render(txt, color, size, checkNewline);
}

/* GUIText */

GUIText GUITextDefault;
static const unsigned char* GUIcurrentFontdata = gfxPrimitivesFontdata;
#define NOFONT_W 8
#define NOFONT_H 8

GUIText::GUIText() {
	Construct();
}

GUIText::~GUIText() {
	Destruct();
}

GUIText::GUIText(const char* fontFile, int size) {
	Construct();
	Load(fontFile, size);
}

GUIText::GUIText(const char* fontFile, int minSize, int maxSize) {
	Construct();
	PreLoad(fontFile, minSize, maxSize);
}

void GUIText::Construct() {
#ifdef _SDL_TTF_H
	fonts = NULL; fontsSize = 0; fontsMinSize = 0;
#endif
	globalStyle = TTF_STYLE_NORMAL; globalHinting = TTF_HINTING_NORMAL; globalKerning = true;
	surfaces = NULL; surfacesY = NULL; surfacesSize = 0;
	shadedBgColor.r = 0xFF; shadedBgColor.g = 0xFF; shadedBgColor.b = 0xFF; shadedBgColor.a = 0xFF;
	renderMode = 0; /* 0 = blended, 1 = solid, 2 = shaded */
}

void GUIText::Destruct() {
	Unload();
	FreeRender();
	Construct();
}

bool GUIText::Load(const char* fontFile, int size) {
	return PreLoad(fontFile, size, size);
}

bool GUIText::PreLoad(const char* fontFile, int minSize, int maxSize) {
#ifdef _SDL_TTF_H
	if (minSize <= 0)minSize = 1;
	if (maxSize < minSize)return false;
	if (TTF_WasInit() == 0)return false;
	Unload();
	fontsMinSize = minSize;
	fontsSize = maxSize - minSize + 1;
	fonts = (TTF_Font * *)malloc(sizeof(TTF_Font*) * fontsSize);
	if (fonts == NULL) {
		fontsMinSize = 0; fontsSize = 0;
		return false;
	}
	memset(fonts, 0, sizeof(TTF_Font*) * fontsSize);
	for (unsigned int n = 0; n < fontsSize; n++) 
	{
		fonts[n] = TTF_OpenFont(fontFile, n + fontsMinSize);
		if (fonts[n] == NULL) {

			return true;
			for (unsigned int k = 0; k < n; k++)TTF_CloseFont(fonts[k]);
			free(fonts); fonts = NULL;
			fontsMinSize = 0; fontsSize = 0;
			return false;
		}
	}
	return true;
#else
	return true;
#endif
}

bool GUIText::Render(const char* text, Uint32 color, int size, bool checkNewline, bool useUTF8) {
	return Render(text, Uint32ToColor(color), size, checkNewline, useUTF8);
}

bool GUIText::Render(char* text, Uint32 color, int size, bool checkNewline, bool useUTF8) {
	return Render(text, Uint32ToColor(color), size, checkNewline, useUTF8);
}

bool GUIText::Render(Uint16* text, Uint32 color, int size, bool checkNewline) {
	return Render(text, Uint32ToColor(color), size, checkNewline);
}

bool GUIText::Render(const char* text, SDL_Color color, int size, bool checkNewline, bool useUTF8) {
	char* txt = NULL;
	int txtSize = strlen(text) + 1;
	txt = (char*)malloc(sizeof(char) * txtSize);
	if (txt == NULL)return false;
	memcpy(txt, text, txtSize * sizeof(char));
	bool ret = Render(txt, color, size, checkNewline, useUTF8);
	free(txt);
	return ret;
}

bool GUIText::Render(char* text, SDL_Color color, int size, bool checkNewline, bool useUTF8) {
#ifdef _SDL_TTF_H
	if (TTF_WasInit() == 0)return false;
	TTF_Font* useFont = GetFont(size);
	if (useFont != NULL) {
		TTF_SetFontHinting(useFont, globalHinting);
		TTF_SetFontKerning(useFont, globalKerning);
		TTF_SetFontStyle(useFont, globalStyle);
	}
#endif
	FreeRender();
	if (checkNewline) {
		int fh, fy = 0;
#ifdef _SDL_TTF_H
		if (useFont == NULL)fh = NOFONT_H;
		else fh = TTF_FontHeight(useFont);
#else
		fh = NOFONT_H;
#endif
		unsigned int lastNewline = 0;
		for (unsigned int n = 0; text[n] != 0; n++) {
			if (text[n] == '\n') {
				text[n] = 0;
				if ((text + lastNewline)[0] != 0) {
#ifdef _SDL_TTF_H
					if (!AddSurface(RenderSurf(text + lastNewline, color, useUTF8, useFont), fy))return false;
#else
					if (!AddSurface(RenderSurf(text + lastNewline, color, useUTF8), fy))return false;
#endif
				}
				fy += fh;
				lastNewline = n + 1;
				text[n] = '\n';
			}
		}
#ifdef _SDL_TTF_H
		if (!AddSurface(RenderSurf(text + lastNewline, color, useUTF8, useFont), fy))return false;
#else
		if (!AddSurface(RenderSurf(text + lastNewline, color, useUTF8), fy))return false;
#endif
	}
	else {
#ifdef _SDL_TTF_H
		if (!AddSurface(RenderSurf(text, color, useUTF8, useFont), 0))return false;
#else
		if (!AddSurface(RenderSurf(text, color, useUTF8), 0))return false;
#endif
	}
	return true;
}

bool GUIText::Render(Uint16* text, SDL_Color color, int size, bool checkNewline) {
#ifdef _SDL_TTF_H
	if (TTF_WasInit() == 0)return false;
	TTF_Font* useFont = GetFont(size);
	if (useFont != NULL) {
		TTF_SetFontHinting(useFont, globalHinting);
		TTF_SetFontKerning(useFont, globalKerning);
		TTF_SetFontStyle(useFont, globalStyle);
	}
#endif
	FreeRender();
	if (checkNewline) {
		int fh, fy = 0;
#ifdef _SDL_TTF_H
		if (useFont == NULL)fh = NOFONT_H;
		else fh = TTF_FontHeight(useFont);
#else
		fh = NOFONT_H;
#endif
		unsigned int lastNewline = 0;
		for (unsigned int n = 0; text[n] != 0; n++) {
			if (text[n] == CharToUni('\n')) {
				text[n] = 0;
				if ((text + lastNewline)[0] != 0) {
#ifdef _SDL_TTF_H
					if (!AddSurface(RenderSurf(text + lastNewline, color, useFont), fy))return false;
#else
					if (!AddSurface(RenderSurf(text + lastNewline, color), fy))return false;
#endif
				}
				fy += fh;
				lastNewline = n + 1;
				text[n] = CharToUni('\n');
			}
		}
#ifdef _SDL_TTF_H
		if (!AddSurface(RenderSurf(text + lastNewline, color, useFont), fy))return false;
#else
		if (!AddSurface(RenderSurf(text + lastNewline, color), fy))return false;
#endif
	}
	else {
#ifdef _SDL_TTF_H
		if (!AddSurface(RenderSurf(text, color, useFont), 0))return false;
#else
		if (!AddSurface(RenderSurf(text, color), 0))return false;
#endif
	}
	return true;
}

int GUIText::Blit(SDL_Surface* dst, int x, int y, int w, int h, bool centerX, bool centerY, int blitOffX, int blitOffY) {
	if (surfaces == NULL)return -1;
	if (w <= 0)centerX = false;
	if (h <= 0)centerY = false;
	int ret = 0, offY = 0;
	int lastH = 0; if (surfaces[surfacesSize - 1] != NULL)lastH = surfaces[surfacesSize - 1]->h;
	if (centerY)offY = (h - (lastH + surfacesY[surfacesSize - 1])) / 2;
	for (unsigned int n = 0; n < surfacesSize; n++) {
		if (surfaces[n] == NULL)continue;
		int posX = x, posY = y + surfacesY[n] + offY, srcX = 0, srcY = 0;
		if (centerX)posX = (x + (w / 2)) - (surfaces[n]->w / 2);
		posX += blitOffX; posY += blitOffY;
		if (posX < x) { srcX = x - posX; posX = x; }
		if (posY < y) { srcY = y - posY; posY = y; }
		if (((w <= 0) || (x + w - posX > 0)) && ((h <= 0) || (y + h - posY > 0))) {
			if (SDLBlit(surfaces[n], dst, posX, posY, x + w - posX, y + h - posY, srcX, srcY) != 0)ret = -1;
		}
	}
	return ret;
}

SDL_Surface* GUIText::BlitToSurface(int x, int y, int w, int h, bool centerX, bool centerY, int blitOffX, int blitOffY) {
	if (surfaces == NULL)return NULL;
	int lastH = 0; if (surfaces[surfacesSize - 1] != NULL)lastH = surfaces[surfacesSize - 1]->h;
	int wi = 0, he = lastH + surfacesY[surfacesSize - 1];
	for (unsigned int n = 0; n < surfacesSize; n++) {
		if (surfaces[n] == NULL)continue;
		if (surfaces[n]->w > wi)wi = surfaces[n]->w;
	}
	if (wi <= 0 || he <= 0)return NULL;
	SDL_Surface* dst = SDLNew(wi, he, true);
	if (dst != NULL)Blit(dst, x, y, w, h, centerX, centerY, blitOffX, blitOffY);
	return dst;
}

void GUIText::SetRenderBlended() {
	renderMode = 0;
}

void GUIText::SetRenderSolid() {
	renderMode = 1;
}

void GUIText::SetRenderShaded(Uint32 bgColor) {
	renderMode = 2;
	shadedBgColor = Uint32ToColor(bgColor);
}

void GUIText::SetRenderShaded(SDL_Color bgColor) {
	renderMode = 2;
	shadedBgColor = bgColor;
}

void GUIText::SetFontStyle(int style) {
	globalStyle = style;
}

void GUIText::SetFontHinting(int hinting) {
	globalHinting = hinting;
}

void GUIText::SetFontKerning(bool kerning) {
	globalKerning = kerning;
}

int GUIText::RecommendedSize(const char* text, int desiredWidth, int desiredHeight, bool useUTF8) {
#ifdef _SDL_TTF_H
	GUIText* t = this;
	if (t->fonts == NULL)t = &GUITextDefault;
	if (t->fonts == NULL)return NOFONT_H;
	int textSize; for (textSize = 0; text[textSize] != 0; textSize++);
	unsigned int recMax = desiredHeight + 1 - t->fontsMinSize;
	if (recMax > t->fontsSize - 1)recMax = t->fontsSize - 1;
	for (int fi = recMax; fi >= 0; fi--) {
		int w = desiredWidth + 1, h = desiredHeight + 1;
		int ret;
		if (useUTF8)ret = TTF_SizeUTF8(t->fonts[fi], text, &w, &h);
		else ret = TTF_SizeText(t->fonts[fi], text, &w, &h);
		if (ret != 0)continue;
		if (w <= desiredWidth && h <= desiredHeight)return fi + t->fontsMinSize;
	}
	return t->fontsMinSize;
#else
	return NOFONT_H;
#endif
}

int GUIText::RecommendedSize(Uint16* text, int desiredWidth, int desiredHeight) {
#ifdef _SDL_TTF_H
	GUIText* t = this;
	if (t->fonts == NULL)t = &GUITextDefault;
	if (t->fonts == NULL)return NOFONT_H;
	int textSize; for (textSize = 0; text[textSize] != 0; textSize++);
	unsigned int recMax = desiredHeight + 1 - t->fontsMinSize;
	if (recMax > t->fontsSize - 1)recMax = t->fontsSize - 1;
	for (int fi = recMax; fi >= 0; fi--) {
		int w = desiredWidth + 1, h = desiredHeight + 1;
		int ret = TTF_SizeUNICODE(t->fonts[fi], text, &w, &h);
		if (ret != 0)continue;
		if (w <= desiredWidth && h <= desiredHeight)return fi + t->fontsMinSize;
	}
	return t->fontsMinSize;
#else
	return NOFONT_H;
#endif
}

bool GUIText::IsLoaded() {
#ifdef _SDL_TTF_H
	if (fonts != NULL && fontsSize > 0)return true;
	else return false;
#else
	return true;
#endif
}

bool GUIText::IsRendered() {
	if (surfaces != NULL && surfacesSize > 0)return true;
	else return false;
}

void GUIText::SwitchRender(GUIText* guiText) {
	SDL_Surface** surfaces_; int* surfacesY_; unsigned int surfacesSize_;
	SDL_Color shadedBgColor_; char renderMode_;
	surfaces_ = surfaces; surfacesY_ = surfacesY; surfacesSize_ = surfacesSize; shadedBgColor_ = shadedBgColor; renderMode_ = renderMode;
	surfaces = guiText->surfaces; surfacesY = guiText->surfacesY; surfacesSize = guiText->surfacesSize; shadedBgColor = guiText->shadedBgColor; renderMode = guiText->renderMode;
	guiText->surfaces = surfaces_; guiText->surfacesY = surfacesY_; guiText->surfacesSize = surfacesSize_; guiText->shadedBgColor = shadedBgColor_; guiText->renderMode = renderMode_;
}

void GUIText::SwitchFonts(GUIText* guiText) {
#ifdef _SDL_TTF_H
	TTF_Font** fonts_; unsigned int fontsSize_, fontsMinSize_;
	bool globalKerning_; int globalStyle_, globalHinting_;
	fonts_ = fonts; fontsSize_ = fontsSize; fontsMinSize_ = fontsMinSize;
	globalKerning_ = globalKerning; globalStyle_ = globalStyle; globalHinting_ = globalHinting;
	fonts = guiText->fonts; fontsSize = guiText->fontsSize; fontsMinSize = guiText->fontsMinSize;
	globalKerning = guiText->globalKerning; globalStyle = guiText->globalStyle; globalHinting = guiText->globalHinting;
	guiText->fonts = fonts_; guiText->fontsSize = fontsSize_; guiText->fontsMinSize = fontsMinSize_;
	guiText->globalKerning = globalKerning_; guiText->globalStyle = globalStyle_; guiText->globalHinting = globalHinting_;
#else
	bool globalKerning_; int globalStyle_, globalHinting_;
	globalKerning_ = globalKerning; globalStyle_ = globalStyle; globalHinting_ = globalHinting;
	globalKerning = guiText->globalKerning; globalStyle = guiText->globalStyle; globalHinting = guiText->globalHinting;
	guiText->globalKerning = globalKerning_; guiText->globalStyle = globalStyle_; guiText->globalHinting = globalHinting_;
#endif
}

int GUIText::GetTextHeight(int size, char* text) {
	int ln = 1;
	for (int n = 0; text != NULL && text[n] != 0; n++)if (text[n] == '\n')ln++;
#ifdef _SDL_TTF_H
	TTF_Font* font = GetFont(size);
	if (font == NULL)return NOFONT_H;
	return ln * TTF_FontHeight(font);
#else
	return NOFONT_H;
#endif
}

int GUIText::GetTextHeight(int size, Uint16* text) {
	int ln = 1;
	for (int n = 0; text != NULL && text[n] != 0; n++)if (text[n] == CharToUni('\n'))ln++;
#ifdef _SDL_TTF_H
	TTF_Font* font = GetFont(size);
	if (font == NULL)return NOFONT_W;
	return ln * TTF_FontHeight(font);
#else
	return NOFONT_W;
#endif
}

int GUIText::GetTextWidth(const char* text, int size, bool useUTF8) {
#ifdef _SDL_TTF_H
	TTF_Font* font = GetFont(size);
	if (font == NULL)return strlen(text) * NOFONT_W;
	int w = 0, h = 0;
	if (useUTF8)TTF_SizeUTF8(font, text, &w, &h);
	else TTF_SizeText(font, text, &w, &h);
	return w;
#else
	return strlen(text) * NOFONT_W;
#endif
}

int GUIText::GetTextWidth(Uint16* text, int size) {
#ifdef _SDL_TTF_H
	TTF_Font* font = GetFont(size);
	if (font == NULL) {
		int len; for (len = 0; text[len] != 0; len++);
		return len * NOFONT_W;
	}
	int w = 0, h = 0;
	TTF_SizeUNICODE(font, text, &w, &h);
	return w;
#else
	int len; for (len = 0; text[len] != 0; len++);
	return len * NOFONT_W;
#endif
}

void GUIText::Unload() {
#ifdef _SDL_TTF_H
	if (TTF_WasInit() == 0)return;
	if (fonts != NULL) {
		for (unsigned int n = 0; n < fontsSize; n++) {
			if (fonts[n] != NULL)TTF_CloseFont(fonts[n]);
			fonts[n] = NULL;
		}
		free(fonts);
		fonts = NULL;
	}
	fontsSize = 0;
	fontsMinSize = 0;
#endif
	globalStyle = TTF_STYLE_NORMAL; globalHinting = TTF_HINTING_NORMAL; globalKerning = true;
}

void GUIText::FreeRender() {
	if (!(SDL_WasInit(SDL_INIT_EVERYTHING) & SDL_INIT_VIDEO))return;
	if (surfaces != NULL) {
		for (unsigned int n = 0; n < surfacesSize; n++) {
			if (surfaces[n] != NULL)SDL_FreeSurface(surfaces[n]);
			surfaces[n] = NULL;
		}
		free(surfaces);
		surfaces = NULL;
	}
	if (surfacesY != NULL)free(surfacesY);
	surfacesY = NULL;
	surfacesSize = 0;
}

#ifdef _SDL_TTF_H
TTF_Font* GUIText::GetFont(int size) {
	if (fontsSize > 0 && fonts != NULL) {
		if (size == 0)return fonts[fontsSize / 2];
		else if (size - fontsMinSize < fontsSize)return fonts[size - fontsMinSize];
	}
	else if (GUITextDefault.fontsSize > 0 && GUITextDefault.fonts != NULL) {
		if (size == 0)return GUITextDefault.fonts[GUITextDefault.fontsSize / 2];
		else if (size - GUITextDefault.fontsMinSize < GUITextDefault.fontsSize)return GUITextDefault.fonts[size - GUITextDefault.fontsMinSize];
	}
	return NULL;
}

SDL_Surface* GUIText::RenderSurf(const char* text, SDL_Color color, bool useUTF8, TTF_Font* font) {
	if (font == NULL) {
		int size = 0; for (size = 0; text[size] != 0; size++);
		SDL_Surface* surf = SDLNew(size * NOFONT_W, NOFONT_H, true);
		if (surf == NULL)return NULL;
		SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
		for (int n = 0; n < size; n++)for (int x = 0; x < 8; x++)
			for (int y = 0; y < 8; y++)
			{
				if (GUIcurrentFontdata[(8 * text[n]) + y] & (1 << x))
				{
					//  pixelColor(surf,(8-x)+(n*8),y,ColorToUint32(color));
				}
			}
		return surf;
	}
	switch (renderMode) {
	default: case 0:
		if (useUTF8)return TTF_RenderUTF8_Blended(font, text, color);
		else return TTF_RenderText_Blended(font, text, color);
		break;
	case 1:
		if (useUTF8)return TTF_RenderUTF8_Solid(font, text, color);
		else return TTF_RenderText_Solid(font, text, color);
		break;
	case 2:
		if (useUTF8)return TTF_RenderUTF8_Shaded(font, text, color, shadedBgColor);
		else return TTF_RenderText_Shaded(font, text, color, shadedBgColor);
		break;
	}
}

SDL_Surface* GUIText::RenderSurf(Uint16* text, SDL_Color color, TTF_Font* font) {
	if (font == NULL) {
		int size = 0; for (size = 0; text[size] != 0; size++);
		SDL_Surface* surf = SDLNew(size * NOFONT_W, NOFONT_H, true);
		if (surf == NULL)return NULL;
		SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
		for (int n = 0; n < size; n++)for (int x = 0; x < 8; x++)for (int y = 0; y < 8; y++)
		{
			if (GUIcurrentFontdata[(8 * UniToChar(text[n])) + y] & (1 << x))
			{
				//pixelColor(surf,(8-x)+(n*8),y,ColorToUint32(color));
			}
		}
		return surf;
	}
	switch (renderMode) {
	default: case 0:
		return TTF_RenderUNICODE_Blended(font, text, color);
		break;
	case 1:
		return TTF_RenderUNICODE_Solid(font, text, color);
		break;
	case 2:
		return TTF_RenderUNICODE_Shaded(font, text, color, shadedBgColor);
		break;
	}
}

#else

SDL_Surface* GUIText::RenderSurf(const char* text, SDL_Color color, bool useUTF8) {
	int size = 0; for (size = 0; text[size] != 0; size++);
	SDL_Surface* surf = SDLNew(size * NOFONT_W, NOFONT_H, true);
	if (surf == NULL)return NULL;
	SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
	for (int n = 0; n < size; n++)for (int x = 0; x < 8; x++)for (int y = 0; y < 8; y++) {
		if (GUIcurrentFontdata[(8 * text[n]) + y] & (1 << x))pixelColor(surf, (8 - x) + (n * 8), y, ColorToUint32(color));
	}
	return surf;
}

SDL_Surface* GUIText::RenderSurf(Uint16* text, SDL_Color color) {
	int size = 0; for (size = 0; text[size] != 0; size++);
	SDL_Surface* surf = SDLNew(size * NOFONT_W, NOFONT_H, true);
	if (surf == NULL)return NULL;
	SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
	for (int n = 0; n < size; n++)for (int x = 0; x < 8; x++)for (int y = 0; y < 8; y++) {
		if (GUIcurrentFontdata[(8 * UniToChar(text[n])) + y] & (1 << x))pixelColor(surf, (8 - x) + (n * 8), y, ColorToUint32(color));
	}
	return surf;
}
#endif

bool GUIText::AddSurface(SDL_Surface* surf, int surfY) {
	SDL_Surface** srf = NULL; int* srfY;
	surfacesSize++;
	srf = (SDL_Surface * *)realloc(surfaces, sizeof(SDL_Surface*) * surfacesSize);
	if (srf == NULL) { surfacesSize--; return false; }
	srfY = (int*)realloc(surfacesY, sizeof(int) * surfacesSize);
	if (srfY == NULL) { surfacesSize--; return false; }
	surfaces = srf;
	surfacesY = srfY;
	surfaces[surfacesSize - 1] = surf;
	surfacesY[surfacesSize - 1] = surfY;
	return true;
}

/* global */

bool GUILoadFont(const char* font, int maxSize) {
	GUITextDefault.FreeRender();
	GUITextDefault.Unload();
	return GUITextDefault.PreLoad(font, 1, maxSize);
}

struct GUIElement {
	void* element; int z;
	GUIElementType type;
};

void GUIDraw(SDL_Surface* dst) {
	for (size_t n = 0; n < GUIElementList_size; n++) {
		switch (GUIElementList[n].type) {
		case GUI_BUTTON: ((GUIButton*)(GUIElementList[n].element))->Draw(dst); break;
		case GUI_LABEL: ((GUILabel*)(GUIElementList[n].element))->Draw(dst); break;
		case GUI_CHECKBOX: ((GUICheckBox*)(GUIElementList[n].element))->Draw(dst); break;
		case GUI_SCROLLBAR: ((GUIScrollBar*)(GUIElementList[n].element))->Draw(dst); break;
		case GUI_TEXTBOX: ((GUITextBox*)(GUIElementList[n].element))->Draw(dst); break;
		case GUI_LISTBOX:
			if (((GUIListBox*)GUIElementList[n].element)->doDraw) {
				((GUIListBox*)(GUIElementList[n].element))->Draw(dst);
			}
			break;
		case GUI_DROPDOWN: ((GUIDropDown*)(GUIElementList[n].element))->Draw(dst); break;
		default: break;
		}
	}
}

bool GUIEvent(SDL_Event* event, int offX, int offY) {
	for (size_t n = 0; n < GUIElementList_size; n++) {
		switch (GUIElementList[n].type) {
		case GUI_BUTTON: if (((GUIButton*)(GUIElementList[n].element))->Event(event, offX, offY))return true; break;
		case GUI_LABEL: if (((GUILabel*)(GUIElementList[n].element))->Event(event, offX, offY))return true; break;
		case GUI_CHECKBOX: if (((GUICheckBox*)(GUIElementList[n].element))->Event(event, offX, offY))return true; break;
		case GUI_SCROLLBAR: if (((GUIScrollBar*)(GUIElementList[n].element))->Event(event, offX, offY))return true; break;
		case GUI_TEXTBOX: if (((GUITextBox*)(GUIElementList[n].element))->Event(event, offX, offY))return true; break;
		case GUI_LISTBOX: if (((GUIListBox*)(GUIElementList[n].element))->Event(event, offX, offY))return true; break;
		case GUI_DROPDOWN: if (((GUIDropDown*)(GUIElementList[n].element))->Event(event, offX, offY))return true; break;
		default: break;
		}
	}
	return false;
}

void GUIUnregisterAll() {
	if (GUIElementList != NULL) {
		free(GUIElementList);
		GUIElementList = NULL;
	}
	GUIElementList_size = 0;
}

void GUIElementAdd(void* element, int zIndex, GUIElementType eType) {
	for (unsigned int n = 0; n < GUIElementList_size; n++) {
		if (GUIElementList[n].element == element) {
			GUIElementList[n].z = zIndex;
			GUIElementList[n].type = eType;
			return;
		}
	}
	GUIElementList_size++;
	GUIElement* t = (GUIElement*)realloc(GUIElementList, GUIElementList_size * sizeof(GUIElement));
	if (t == NULL) {
		GUIElementList_size--;
		return;
	}
	t[GUIElementList_size - 1].element = NULL;
	t[GUIElementList_size - 1].type = GUI_UNKNOWN;
	t[GUIElementList_size - 1].z = zIndex - 1;
	GUIElementList = t;
	int p = 0;
	for (unsigned int n = 0; n < GUIElementList_size; n++) {
		p = n;
		if (GUIElementList[n].z > zIndex)break;
	}
	for (int n = GUIElementList_size - 1; n > p; n--) {
		GUIElementList[n] = GUIElementList[n - 1];
	}
	GUIElementList[p].element = element;
	GUIElementList[p].type = eType;
	GUIElementList[p].z = zIndex;
}

void GUIElementRemove(void* element, GUIElementType eType) {
	bool rev = false;
	for (size_t n = 0; n < GUIElementList_size; n++) {
		if (rev)GUIElementList[n - 1] = GUIElementList[n];
		if (GUIElementList[n].element == element)rev = true;
	}
	if (rev)GUIElementList_size--;
	if (GUIElementList_size == 0) {
		if (GUIElementList != NULL) {
			free(GUIElementList);
			GUIElementList = NULL;
		}
	}
}
