//
//  GUI_Fonts.cpp
//  GUI_TextView
//
//  Created by Panutat Tejasen on 12/1/2562 BE.
//  Copyright © 2562 Jimmy Software Co., Ltd. All rights reserved.
//

#include "GUI_Fonts.h"
#include "SDL_gui.h"
#include <sprintf.h>

eastl::map<eastl::string, TTF_Font *>* GUI_Fonts::font_map = nullptr;

GUI_Fonts::GUI_Fonts() {
}

GUI_Fonts::~GUI_Fonts() {
    
}

TTF_Font *GUI_Fonts::getFont(eastl::string fontName, int fontSize ) {
	//eastl::string font_id = fontName + eastl::to_string(fontSize * GUI_scale);

	if (font_map == nullptr)
		font_map = new eastl::map<eastl::string, TTF_Font *>();

	int font_num = fontSize * GUI_scale;
	char buf[256];
	itoa(font_num, 10, buf);
	eastl::string font_id = fontName + buf;	
    
    if(font_map->find(font_id) != font_map->end())
        return (*font_map)[font_id];
    
    GUI_Log( font_id.c_str() );
    
	eastl::string fontPath = "data/"+fontName;
    TTF_Font *font = TTF_OpenFont(fontPath.c_str(), fontSize * GUI_scale);
    
    if (!font) {
        GUI_Log("font-spec %s not found\n", fontPath.c_str());
        return NULL;
    }
    
    font_map->insert(eastl::make_pair(font_id, font));
    
    return font;
}

void GUI_Fonts::clear() {
	eastl::map<eastl::string, TTF_Font *>::iterator it;
    
    for ( it = font_map->begin(); it != font_map->end(); it++ )
    {
        TTF_Font *font = it->second;   // string's value
        TTF_CloseFont( font );
    }
    font_map->clear();
}
