#include "SDLGui.h"

#if _SDLGUI_VER_ != 128
  #warning Invalid SDLGui version
#endif

#define BORDER 0x404040FF
#define CLICKABLE_UP 0xD4D0C8FF
#define CLICKABLE_DOWN 0x808080FF

void GUIDropDown::Draw(SDL_Surface *dst){
  boxColor(dst,posX1,posY1,posX2,posY2,0xFFFFFFFF);
  rectangleColor(dst,posX1,posY1,posX2,posY2,BORDER);
  selectedText.Blit(dst,posX1+3,posY1+2,posX2-posX1-3,posY2-posY1-2,false,true);
  if(isPressed)boxColor(dst,arrX1,arrY1,arrX2,arrY2,CLICKABLE_DOWN);
  else boxColor(dst,arrX1,arrY1,arrX2,arrY2,CLICKABLE_UP);
  rectangleColor(dst,arrX1,arrY1,arrX2,arrY2,BORDER);
  filledTrigonColor(dst,triX1,triY1,triX2,triY2,triX3,triY3,0x00000080);
  if(listBox.doDraw&&!hasRegistered)listBox.Draw(dst);
}

void GUIDropDown::StyleConstruct(){

}

void GUIListBox::Draw(SDL_Surface *dst){
  boxColor(dst,posX1,posY1,posX2,posY2,0xFFFFFFFF);
  if(showSelectionBox)boxColor(dst,selX1,selY1,selX2,selY2,0x6699FFFF);
  renderedText.Blit(dst,textX1,textY1,textX2-textX1,textY2-textY1,false,false,curOffX,curOffY);
  rectangleColor(dst,posX1,posY1,posX2,posY2,BORDER);
  scroll.Draw(dst);
}

void GUIListBox::StyleConstruct(){
  textBorderUp = 1; textBorderLeft = 1; textBorderDown = 0; textBorderRight = 0;
  textSliderThick = 14;
}

void GUITextBox::Draw(SDL_Surface *dst){
  boxColor(dst,posX1,posY1,posX2,posY2,fontBackColor);
  renderedText.Blit(dst,textX1,textY1,textX2-textX1,textY2-textY1,false,false,curOffX,curOffY);
  rectangleColor(dst,posX1,posY1,posX2,posY2,BORDER);
  if(CursorBlink())boxColor(dst,curDrawX,curDrawY,curDrawX,curDrawY+curDrawH,0x330000FF);
  if(showScroll){
    scrollX.Draw(dst);
    scrollY.Draw(dst);
  }
  if(showPointer){
    vlineColor(dst,mouseX,mouseY-6,mouseY+6,0x00000080);
    hlineColor(dst,mouseX-3,mouseX+3,mouseY-7,0x00000080);
    hlineColor(dst,mouseX-3,mouseX+3,mouseY+7,0x00000080);
  }
}

void GUITextBox::StyleConstruct(){
  textBorderUp = 1; textBorderLeft = 1; textBorderDown = 0; textBorderRight = 0;
  textSliderThick = 15;
}

void GUIScrollBar::Draw(SDL_Surface *dst){
  boxColor(dst,trackX1,trackY1,trackX2,trackY2,0xFFFFFFFF);
  rectangleColor(dst,trackX1,trackY1,trackX2,trackY2,BORDER);
  if(arrowW>0){

    if(arrADown)boxColor(dst,arrAX1,arrAY1,arrAX2,arrAY2,CLICKABLE_DOWN);
    else boxColor(dst,arrAX1,arrAY1,arrAX2,arrAY2,CLICKABLE_UP);
    if(arrBDown) boxColor(dst,arrBX1,arrBY1,arrBX2,arrBY2,CLICKABLE_DOWN);
    else boxColor(dst,arrBX1,arrBY1,arrBX2,arrBY2,CLICKABLE_UP);

    rectangleColor(dst,arrBX1,arrBY1,arrBX2,arrBY2,BORDER);
    rectangleColor(dst,arrAX1,arrAY1,arrAX2,arrAY2,BORDER);

    filledTrigonColor(dst,triAX1,triAY1,triAX2,triAY2,triAX3,triAY3,0x00000080);
    filledTrigonColor(dst,triBX1,triBY1,triBX2,triBY2,triBX3,triBY3,0x00000080);
  }
  if(!isDisabled){
    boxColor(dst,slidX1,slidY1,slidX2,slidY2,0xD4D0C8FF);
    rectangleColor(dst,slidX1,slidY1,slidX2,slidY2,0x404040FF);
  }
}

void GUIScrollBar::StyleConstruct(){

}

void GUIButton::Draw(SDL_Surface *dst){
  if(isPressed&&isHovered)boxColor(dst,posX1,posY1,posX2,posY2,CLICKABLE_DOWN);
  else boxColor(dst,posX1,posY1,posX2,posY2,CLICKABLE_UP);
  rectangleColor(dst,posX1,posY1,posX2,posY2,BORDER);
  renderedText.Blit(dst,posX1,posY1,posX2-posX1,posY2-posY1,true,true);
  if(isDisabled)boxColor(dst,posX1,posY1,posX2,posY2,0xFFFFFFA0);
}

void GUIButton::StyleConstruct(){
  textColor.r=0x00; textColor.g=0x00; textColor.b=0x00; textColor.a=0xFF;
}

void GUICheckBox::Draw(SDL_Surface *dst){
  if(isCircle){
    int rad = ((posX2-posX1<posY2-posY1)?(posX2-posX1):(posY2-posY1))-7;
    filledCircleColor(dst,(posX1+posX2)/2,(posY1+posY2)/2,rad,0xFFFFFFFF);
    aacircleColor(dst,(posX1+posX2)/2,(posY1+posY2)/2,rad,BORDER);
    if(isChecked){
      filledCircleColor(dst,(posX1+posX2)/2,(posY1+posY2)/2,rad-3,0x000000FF);
      aacircleColor(dst,(posX1+posX2)/2,(posY1+posY2)/2,rad-3,0x000000FF);
    }
  }else{
    boxColor(dst,posX1,posY1,posX2,posY2,0xFFFFFFFF);
    rectangleColor(dst,posX1,posY1,posX2,posY2,BORDER);
    if(isChecked){
      thickLineColor(dst,posX1+1,posY1+1,posX2-1,posY2-1,3,0x000000FF);
      thickLineColor(dst,posX1+1,posY2-1,posX2-1,posY1+1,3,0x000000FF);
    }
  }
}

void GUICheckBox::StyleConstruct(){

}

void GUILabel::Draw(SDL_Surface *dst){
  text.Blit(dst,posX1,posY1,posX2-posX1,posY2-posY1,centerX,centerY);
}

void GUILabel::StyleConstruct(){

}
