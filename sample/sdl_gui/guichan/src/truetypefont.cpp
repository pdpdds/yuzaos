/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __TRUETYPEFONT_H__
#define __TRUETYPEFONT_H__

#include <guichan/color.hpp>
#include <guichan/exception.hpp>

#include "guichan/graphics.hpp"

#include "guichan/image.hpp"

#include "../core/utils/dtor.h"
#include "guichan/truetypefont.h"
#include "guichan/sdl/sdlgraphics.hpp"


#define CACHE_SIZE 256

class TextChunk
{
    public:
        TextChunk(const std::string &text, const gcn::Color &color) :
			surface(NULL), text(text), color(color)
        {
        }

        ~TextChunk()
        {
			if(surface)				
				SDL_FreeSurface(surface);
        }

        bool operator==(const std::string &str) const
        {
            return (str == text);
        }

        bool operator==(const TextChunk &chunk) const
        {
            return (chunk.text == text && chunk.color == color);
        }

        void generate(TTF_Font *font)
        {
            SDL_Color sdlCol;
            sdlCol.b = color.b;
            sdlCol.r = color.r;
            sdlCol.g = color.g;

/*#ifdef WIN32
			wchar_t* unicodeStr = new wchar_t[text.size() + 1];
			mbstowcs(unicodeStr, text.c_str(), text.size() + 1);

			SDL_Surface *surface = TTF_RenderUNICODE_Solid(font, (const Uint16*)unicodeStr, sdlCol);

			delete unicodeStr;
#else*/
			surface = TTF_RenderUTF8_Blended(
				font, text.c_str(), sdlCol); 
//#endif
            
        }

		SDL_Surface *surface;
        std::string text;
        gcn::Color color;
};

typedef std::list<TextChunk>::iterator CacheIterator;

static int fontCounter;

TrueTypeFont::TrueTypeFont(const std::string &filename, int size, int style)
{
    if (fontCounter == 0 && TTF_Init() == -1)
    {
        //throw GCN_EXCEPTION("Unable to initialize SDL_ttf: " +
          //  std::string(TTF_GetError()));

		char buf[256];
		sprintf(buf, "Unable to initialize SDL_ttf: %s\n", std::string(TTF_GetError()).c_str());
		Syscall_Panic(buf);
    }

    ++fontCounter;
    mFont = TTF_OpenFont(filename.c_str(), size);

    if (!mFont)
    {
        //throw GCN_EXCEPTION("SDLTrueTypeFont::SDLTrueTypeFont: " +
          //  std::string(TTF_GetError()));
		char buf[256];
		sprintf(buf, "SDLTrueTypeFont::SDLTrueTypeFont: %s\n", std::string(TTF_GetError()).c_str());
		Syscall_Panic(buf);
    }

	char* file_contents = NULL;
	int filesize;
	read_text(filename.c_str(), &file_contents, filesize);
	SDL_RWops *file;
	file = SDL_RWFromMem(file_contents, filesize);

	size_t file_length = SDL_RWseek(file, 0, SEEK_END);

	SDL_RWseek(file, 0, SEEK_SET);

	mFont = TTF_OpenFontRW(file, file_length, size);

	//SDL_RWclose(file);

    TTF_SetFontStyle (mFont, style);
}

bool TrueTypeFont::read_text(const char* source_file, char** destination, int& length)
{
	// Open the file
	SDL_RWops *file;
	file = SDL_RWFromFile(source_file, "r");

	if (file == 0)
		return false;

	size_t file_length = SDL_RWseek(file, 0, SEEK_END);
	(*destination) = new char[file_length + 1];
	// Reset seek to beginning of file and read text
	SDL_RWseek(file, 0, SEEK_SET);
	int n_blocks = SDL_RWread(file, (*destination), 1, file_length);
	if (n_blocks <= 0)
	{

	}
	SDL_RWclose(file);

	(*destination)[file_length] = '\0';
	length = file_length;
	return true;
}

TrueTypeFont::~TrueTypeFont()
{
    TTF_CloseFont(mFont);
    --fontCounter;

    if (fontCounter == 0)
        TTF_Quit();
}

TrueTypeFont *TrueTypeFont::load(const std::string &fileName, int fontSize,
                                 int style)
{
    return new TrueTypeFont(fileName, fontSize, style);
}

void TrueTypeFont::drawString(gcn::Graphics *graphics,
                              const std::string &text,
                              int x, int y)
{
    if (text.empty())
        return;

	//gcn::SDLGraphics *g = dynamic_cast<gcn::SDLGraphics *>(graphics);
	gcn::SDLGraphics *g = (gcn::SDLGraphics *)(graphics);

	if (!g)
	{
		//throw "Not a valid graphics object!";
		Syscall_Panic("Not a valid graphics object!");
	}

    gcn::Color col = g->getColor();
    const float alpha = col.a / 255.0f;

    /* The alpha value is ignored at string generation so avoid caching the
     * same text with different alpha values.
     */
    col.a = 255;

    TextChunk chunk(text, col);

    bool found = false;

    for (CacheIterator i = cache.begin(); i != cache.end(); i++)
    {
        if (chunk == (*i))
        {
            // Raise priority: move it to front
            cache.splice(cache.begin(), cache, i);
            found = true;
            break;
        }
    }

    // Surface not found
    if (!found)
    {
        if (cache.size() >= CACHE_SIZE)
            cache.pop_back();

        cache.push_front(chunk);
        cache.front().generate(mFont);
    }

    //cache.front().img->setAlpha(alpha);
	
	if (cache.front().surface)
	{
		SDL_Rect src;
		SDL_Rect dst;
		src.x = 0;
		src.y = 0;
		src.w = cache.front().surface->w;
		src.h = cache.front().surface->h;
		dst.x = x;
		dst.y = y;

		g->drawSDLSurface(cache.front().surface, src, dst);
	}
}

int TrueTypeFont::getWidth(const std::string& text) const
{
    for (CacheIterator i = cache.begin(); i != cache.end(); i++)
    {
        if ((*i) == text)
        {
            // Raise priority: move it to front
            // Assumption is that TTF::draw will be called next
            cache.splice(cache.begin(), cache, i);
			
			if(i->surface)
				return i->surface->w;
        }
    }

    int w, h;
    TTF_SizeUTF8(mFont, text.c_str(), &w, &h);
    return w;
}

int TrueTypeFont::getHeight() const
{
    return TTF_FontHeight(mFont);
}

#endif