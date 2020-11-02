/* SDLGui v1.2.8 Date: 16.1.2013 */

#pragma once
#ifndef _SDLGUI_H_
#define _SDLGUI_H_
#define _SDLGUI_VER_ 128

#include <SDL.h>
#include "SDLHelper.h"
#include <SDL_ttf.h> //include it from within SDLHelper
#include "SDL_gfxPrimitives.h"
#include <stdlib.h>

#ifndef _SDL_TTF_H
#  warning SDLGui relies on SDL_ttf. Please include it in your project.
#  define TTF_HINTING_NORMAL 0
#  define TTF_HINTING_LIGHT 1
#  define TTF_HINTING_MONO 2
#  define TTF_HINTING_NONE 3
#  define TTF_STYLE_NORMAL 0x00
#  define TTF_STYLE_BOLD 0x01
#  define TTF_STYLE_ITALIC 0x02
#  define TTF_STYLE_UNDERLINE 0x04
#  define TTF_STYLE_STRIKETHROUGH 0x08
#endif
/*#ifndef _SDL_gfxPrimitives_h
#  warning SDLGui relies on SDL_gfx. Please include it in your project.
#endif*/

//event.user.codes defined here (event.user.data1 = pointer to instance)
#define GUI_MOUSEIN 21
#define GUI_MOUSEOUT 22
#define GUICHECKBOX_CHECK 25
#define GUICHECKBOX_UNCHECK 26
#define GUIBUTTON_PRESS 27
#define GUIBUTTON_RELEASE 28
#define GUISCROLLBAR_SCROLL 29
#define GUITEXTBOX_RETURN 30
#define GUITEXTBOX_STARTTYPING 31
#define GUITEXTBOX_STOPTYPING 32
#define GUILISTBOX_SELECT 33
#define GUIDROPDOWN_SELECT 33

bool GUILoadFont(const char *font=NULL, int maxSize=512); //load default font for GUI
void GUIDraw(SDL_Surface *dst); //draw all registered elements
bool GUIEvent(SDL_Event *event, int offX=0, int offY=0); //pass event to all registered elements
void GUIUnregisterAll(); //unregister all elements

class GUIText{
 #ifdef _SDL_TTF_H
  TTF_Font **fonts; unsigned int fontsSize, fontsMinSize;
 #endif
  bool globalKerning; int globalStyle, globalHinting;
  SDL_Surface **surfaces; int *surfacesY; unsigned int surfacesSize;
  SDL_Color shadedBgColor; char renderMode; //0 = blended, 1 = solid, 2 = shaded

 #ifdef _SDL_TTF_H
  TTF_Font* GetFont(int size);
  SDL_Surface* RenderSurf(const char *text, SDL_Color color, bool useUTF8, TTF_Font *font=NULL);
  SDL_Surface* RenderSurf(Uint16 *text, SDL_Color color, TTF_Font *font=NULL);
 #else
  SDL_Surface* RenderSurf(const char *text, SDL_Color color, bool useUTF8);
  SDL_Surface* RenderSurf(Uint16 *text, SDL_Color color);
 #endif
  bool AddSurface(SDL_Surface *surf, int surfY);
  GUIText& operator = (const GUIText &param);
public:
  GUIText(); ~GUIText(); void Construct(); void Destruct();
  GUIText(const char *fontFile, int size);
  GUIText(const char *fontFile, int minSize, int maxSize);

  bool Load(const char *fontFile, int size);
  bool PreLoad(const char *fontFile, int minSize, int maxSize);
  bool Render(const char   *text, Uint32    color, int size=0, bool checkNewline=true, bool useUTF8=false);
  bool Render(char         *text, Uint32    color, int size=0, bool checkNewline=true, bool useUTF8=false);
  bool Render(Uint16       *text, Uint32    color, int size=0, bool checkNewline=true);
  bool Render(const char   *text, SDL_Color color, int size=0, bool checkNewline=true, bool useUTF8=false);
  bool Render(char         *text, SDL_Color color, int size=0, bool checkNewline=true, bool useUTF8=false);
  bool Render(Uint16       *text, SDL_Color color, int size=0, bool checkNewline=true);
  int Blit(SDL_Surface *dst, int x, int y, int w=0, int h=0, bool centerX=true, bool centerY=true, int blitOffX=0, int blitOffY=0);
  SDL_Surface* BlitToSurface(int x, int y, int w=0, int h=0, bool centerX=true, bool centerY=true, int blitOffX=0, int blitOffY=0);

  void SetRenderBlended();
  void SetRenderSolid();
  void SetRenderShaded(Uint32 bgColor);
  void SetRenderShaded(SDL_Color bgColor);
  void SetFontStyle(int style=TTF_STYLE_NORMAL);
  void SetFontHinting(int hinting=TTF_HINTING_NORMAL);
  void SetFontKerning(bool kerning=true);

  int RecommendedSize(const char *text, int desiredWidth, int desiredHeight, bool useUTF8=false);
  int RecommendedSize(Uint16 *text, int desiredWidth, int desiredHeight);
  bool IsLoaded();
  bool IsRendered();
  void SwitchRender(GUIText *guiText); //switch rendered surfaces from one GUIText to another
  void SwitchFonts(GUIText *guiText); //switch loaded fonts from one GUIText to another
  int GetTextHeight(int size=0, char *text=NULL);
  int GetTextHeight(int size, Uint16 *text);
  int GetTextWidth(const char *text, int size=0, bool useUTF8=false);
  int GetTextWidth(Uint16 *text, int size=0);
  void Unload();
  void FreeRender();
}; extern GUIText GUITextDefault;

class GUILabel{
  int posX1, posY1, posX2, posY2; bool isDisabled, isHovered;
  GUIText text; bool centerX, centerY;
  GUILabel& operator = (const GUILabel &param); void StyleConstruct();
public:
  GUILabel(); ~GUILabel(); void Construct(); void Destruct();
  bool Event(SDL_Event *event, int offX=0, int offY=0);
  void Draw(SDL_Surface *dst);
  bool IsDisabled();
  bool IsHovered();
  void SetDisabled(bool disabled);
  void SetPos(int x1, int y1, int x2, int y2);
  void Register(int zIndex=0);
  void Unregister();

  GUIText* GetText();
  bool GetCenterX();
  bool GetCenterY();
  void SetCenterX(bool set);
  void SetCenterY(bool set);
  bool Render(const char *txt, Uint32 color, int size = 0, bool checkNewline = false, bool useUTF8 = false);
  bool Render(Uint16     *txt, Uint32 color, int size = 0, bool checkNewline = false);
};

class GUICheckBox{
  int posX1, posY1, posX2, posY2; bool isDisabled, isHovered;
  bool isChecked, isCircle;
  GUICheckBox& operator = (const GUICheckBox &param); void StyleConstruct();
public:
  GUICheckBox(); ~GUICheckBox(); void Construct(); void Destruct();
  bool Event(SDL_Event *event, int offX=0, int offY=0);
  void Draw(SDL_Surface *dst);
  bool IsDisabled();
  bool IsHovered();
  void SetDisabled(bool disabled);
  void SetPos(int x1, int y1, int x2, int y2);
  void Register(int zIndex=0);
  void Unregister();

  bool IsChecked();
  bool IsCircle();
  void SetChecked(bool checked);
  void SetCircle(bool circle);
};

class GUIButton{
  int posX1, posY1, posX2, posY2; bool isDisabled, isHovered;
  GUIText renderedText; bool isPressed;
  SDL_Color textColor;
  GUIButton& operator = (const GUIButton &param); void StyleConstruct();
public:
  GUIButton(); ~GUIButton(); void Construct(); void Destruct();
  bool Event(SDL_Event *event, int offX=0, int offY=0);
  void Draw(SDL_Surface *dst);
  bool IsDisabled();
  bool IsHovered();
  void SetDisabled(bool disabled);
  void SetPos(int x1, int y1, int x2, int y2);
  void Register(int zIndex=0);
  void Unregister();

  bool IsPressed();
  bool SetText(const char *text=NULL, int textSize=0, GUIText *guiText=NULL);
};

class GUIScrollBar{
  int posX1, posY1, posX2, posY2; bool isDisabled, isHovered;
  float scrollPos,scrollSize;
  bool isScrolling,isHorizontal;
  bool arrADown,arrBDown;
  float scrollMove,scrollMoveTrack;
  int scrollMouse,arrowW;
  int trackX1, trackY1, trackX2, trackY2;
  int arrAX1,arrAY1,arrAX2,arrAY2;
  int arrBX1,arrBY1,arrBX2,arrBY2;
  int triAX1,triAY1,triAX2,triAY2,triAX3,triAY3;
  int triBX1,triBY1,triBX2,triBY2,triBX3,triBY3;
  int slidX1,slidY1,slidX2,slidY2,slidW;
  void RecalculateCoords();
  void RecalculateSlider();
  GUIScrollBar& operator = (const GUIScrollBar &param); void StyleConstruct();
public:
  GUIScrollBar(); ~GUIScrollBar(); void Construct(); void Destruct();
  bool Event(SDL_Event *event, int offX=0, int offY=0);
  void Draw(SDL_Surface *dst);
  bool IsDisabled();
  bool IsHovered();
  void SetDisabled(bool disabled);
  void SetPos(int x1, int y1, int x2, int y2);
  void Register(int zIndex=0);
  void Unregister();

  void SetHorizontal(bool horizontal=true);
  void SetArrowWidth(int width=20);
  void SetScrollSize(float size);
  void SetScrollPos(float pos);
  void SetScrollMove(float arrowMove=0.05f, float trackMove=0.25f);
  float GetScrollSize();
  float GetScrollPos();
  float GetScrollArrowMove();
  float GetScrollTrackMove();
  bool IsHorizontal();
  bool IsScrolling();
};

class GUITextBox{
  int posX1, posY1, posX2, posY2; bool isDisabled, isHovered;
  char *text; Uint16 *textUni; int textSize;
  GUIText renderedText, *fontGuiText; int fontSize, setFontSize; Uint32 fontColor, fontBackColor; bool fontUseUnicode;
  int textX1,textY1,textX2,textY2; bool isTyping;
  int textBorderUp,textBorderDown,textBorderLeft,textBorderRight;
  int textSliderThick, textMaxLineWidth, textNewlineCount, textHeight;
  bool curBlink; Uint32 curTick;
  int curPos,curDrawX,curDrawY,curDrawH,curX,curY,curOffX,curOffY;
  int mouseX,mouseY; GUIScrollBar scrollX, scrollY;
  bool showScroll, showPointer, setIsScrollable, setAutoRender, setMaxLines, setAutoHideCursor;
  bool CursorBlink(bool forceShow=false); void CurUp(); void CurDown(); void CurLeft(); void CurRight();
  void RemoveChar(bool del); void AddChar(char chr, Uint16 uni);
  void RecalculateCursor(); void Check();
  GUITextBox& operator = (const GUITextBox &param); void StyleConstruct();
public:
  GUITextBox(); ~GUITextBox(); void Construct(); void Destruct();
  bool Event(SDL_Event *event, int offX=0, int offY=0);
  void Draw(SDL_Surface *dst);
  bool IsDisabled();
  bool IsHovered();
  void SetDisabled(bool disabled);
  void SetPos(int x1, int y1, int x2, int y2);
  void Register(int zIndex=0);
  void Unregister();

  void SetMaxLines(int maxLines=1);
  void SetIsScrollable(bool isScrollable=true);
  void SetAutoHideCursor(bool autoHide=true);
  void SetAutoRender(bool autoRender=true);
  void SetCurPos(int x, int y);
  void SetText(const char *txt=NULL);
  const char *GetText();
  const Uint16 *GetUnicode();
  int GetTextSize();
  bool RenderText();
  void SetTyping(bool typing=true);
  bool IsTyping();
  void SetFontGUIText(GUIText *guiText=NULL);
  void SetFontSize(int size=12);
  void SetFontColor(Uint32 color=0x000000FF);
  void SetFontBackColor(Uint32 backColor=0xFFFFFFFF);
  void SetFontUseUnicode(bool useUnicode=true);
};

class GUIListBox{
  int posX1, posY1, posX2, posY2; bool isDisabled, isHovered;
  GUIText *fontGuiText; int fontSize; Uint32 fontColor, fontBackColor;
  GUIText renderedText; GUIScrollBar scroll;
  char *text; int textSize, itemCount, selected, textHeight;
  int curOffX,curOffY; bool isMouseDown, showSelectionBox;
  int textBorderUp,textBorderDown,textBorderLeft,textBorderRight,textSliderThick;
  int textX1,textY1,textX2,textY2,selX1,selY1,selX2,selY2;
  bool RenderText(); void RecalculateSelectionBox();
  void StyleConstruct();
  friend class GUIDropDown; bool doDraw;
  friend void GUIDraw(SDL_Surface*);
public:
  GUIListBox& operator = (const GUIListBox &param); //copies only items
  GUIListBox& operator = (const class GUIDropDown &param); //copies only items
  GUIListBox(); ~GUIListBox(); void Construct(); void Destruct();
  bool Event(SDL_Event *event, int offX=0, int offY=0);
  void Draw(SDL_Surface *dst);
  bool IsDisabled();
  bool IsHovered();
  void SetDisabled(bool disabled);
  void SetPos(int x1, int y1, int x2, int y2);
  void Register(int zIndex=0);
  void Unregister();

  void AddItem(const char *item, int pos=-1);
  void EraseItem(int pos);
  void ClearItems();
  char *GetItem(int pos);
  int GetItemCount();
  void SetSelected(int pos);
  int GetSelected();
  void SetFontGUIText(GUIText *guiText=NULL);
  void SetFontSize(int size=12);
  void SetFontColor(Uint32 color=0x000000FF);
  void SetFontBackColor(Uint32 backColor=0xFFFFFFFF);
};

class GUIDropDown{
  int posX1, posY1, posX2, posY2; bool isDisabled, isHovered;
  GUIListBox listBox; bool isPressed;
  int triX1,triY1,triX2,triY2,triX3,triY3, arrX1,arrY1,arrX2,arrY2;
  GUIText selectedText;
  void RenderSelectedText(); void RecalculateCoords();
  void StyleConstruct();
  friend class GUIListBox; bool hasRegistered;
public:
  GUIDropDown& operator = (const GUIDropDown &param); //copies only items
  GUIDropDown& operator = (const GUIListBox &param); //copies only items
  GUIDropDown(); ~GUIDropDown(); void Construct(); void Destruct();
  bool Event(SDL_Event *event, int offX=0, int offY=0);
  void Draw(SDL_Surface *dst);
  bool IsDisabled();
  bool IsHovered();
  void SetDisabled(bool disabled);
  void SetPos(int x1, int y1, int x2, int y2);
  void Register(int zIndex=0);
  void Unregister();

  void SetDropDownHeight(int height);
  void AddItem(const char *item, int pos=-1);
  void EraseItem(int pos);
  void ClearItems();
  char *GetItem(int pos);
  int GetItemCount();
  void SetSelected(int pos);
  int GetSelected();
  void SetFontGUIText(GUIText *guiText=NULL);
  void SetFontSize(int size=12);
  void SetFontColor(Uint32 color=0x000000FF);
  void SetFontBackColor(Uint32 backColor=0xFFFFFFFF);
};

#endif
