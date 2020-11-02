//
//  GUI_TextUtil.hpp
//  GUI_EditText
//
//  Created by Panutat Tejasen on 17/1/2562 BE.
//  Copyright © 2562 Jimmy Software Co., Ltd. All rights reserved.
//

#ifndef GUI_TextUtil_hpp
#define GUI_TextUtil_hpp

#include <stdio.h>
#include <eastl/string.h>
#include <SDL_ttf.h>

int GUI_GetPreviousUTF8Index(eastl::string str, int index );
int GUI_GetPreviousMainUTF8Index(eastl::string str, int index );
int GUI_GetNextMainUTF8Index(eastl::string str, int i );
int GUI_GetUnicodeAtIndex(eastl::string str, int i, int *nextIndex=NULL );
int GUI_GetTextIndexFromPosition( TTF_Font *font, eastl::string str, int x );

#endif /* GUI_TextUtil_hpp */
