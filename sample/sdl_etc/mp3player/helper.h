#ifndef _HELPER_H_
#define _HELPER_H_

#include <wchar.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

SDL_Window   *Window;
SDL_Renderer *Renderer;

void init (void);
void exit_(void);

#endif
