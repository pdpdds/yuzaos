#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H

SDL_Texture* CreateTextureFromFT_Bitmap(SDL_Renderer* renderer,
    const FT_Bitmap& bitmap,
    const SDL_Color& color);

#endif