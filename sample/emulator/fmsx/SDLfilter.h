#ifdef SDL
#ifndef FILTER_H
#include <wchar.h>
#include <SDL.h>

#ifndef pixel
typedef Uint16 pixel;
#endif

void blur(SDL_Surface *screen,pixel** buffer);
void mix_scanline(SDL_Surface *screen,pixel** buffer);
void half_scanline(SDL_Surface *screen,pixel** buffer);
void full_scanline(SDL_Surface *screen,pixel** buffer);
void remove_scanline(SDL_Surface *screen, pixel** buffer);

#endif
#endif
