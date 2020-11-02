//
//  GUI_Fonts.hpp
//  GUI_ImageView
//
//  Created by Panutat Tejasen on 12/1/2562 BE.
//  Copyright © 2562 Jimmy Software Co., Ltd. All rights reserved.
//

#ifndef GUI_Fonts_hpp
#define GUI_Fonts_hpp

#include <stdio.h>
#include <eastl/string.h>
#include <eastl/map.h>
#include <SDL_ttf.h>

class GUI_Fonts {
protected:

public:
    static eastl::map<eastl::string, TTF_Font *>* font_map;
    
    GUI_Fonts();
    ~GUI_Fonts();
    
    static TTF_Font *getFont(eastl::string fontName, int fontSize );
    static void clear();
};

#endif /* GUI_Fonts_hpp */
