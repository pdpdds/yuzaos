#ifndef __SDL_PANGO_H__
#define __SDL_PANGO_H__

#include <SDL.h>

typedef struct __SDLPango_Context SDLPango_Context;

extern DECLSPEC int SDLCALL
SDLPango_Init(void);

extern DECLSPEC int SDLCALL
SDLPango_WasInit(void);

extern DECLSPEC SDLPango_Context* SDLCALL
SDLPango_CreateContext(void);

extern DECLSPEC void SDLCALL
SDLPango_FreeContext(SDLPango_Context *context);

extern DECLSPEC void SDLCALL
SDLPango_SetColor(SDLPango_Context* context,
                  const SDL_Color* color);

extern DECLSPEC SDL_Color SDLCALL
SDLPango_GetColor(SDLPango_Context* context);

extern DECLSPEC void SDLCALL
SDLPango_SetMinimumSize(SDLPango_Context* context,
                        int width,
                        int height);

extern DECLSPEC int SDLCALL
SDLPango_GetLayoutHeight(SDLPango_Context *context);

extern DECLSPEC int SDLCALL
SDLPango_GetLayoutWidth(SDLPango_Context *context);

extern DECLSPEC void SDLCALL
SDLPango_SetMarkup(SDLPango_Context *context,
                   const char *markup,
                   int length);

extern DECLSPEC void SDLCALL
SDLPango_Draw(SDLPango_Context *context,
              SDL_Surface *surface,
              int x,
              int y);

#endif