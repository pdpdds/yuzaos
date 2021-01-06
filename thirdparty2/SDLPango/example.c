#include <SDL2/SDL.h>

#include "SDL_Pango.h"

int main(int argc, char **argv)
{
  int width = 640;
  int height = 200;
  int margin = 10;
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Surface *surface;
  SDL_Texture* background;
  Uint32 rmask, gmask, bmask, amask;
  SDLPango_Context *context;
  SDL_Color textcolor = {0xa9, 0xf8, 0x69, 0xff};

  const char * const text =
    "<span font=\"50\">"
    "Hello world "
    "أهلاً بالعالم "
    "你好世界 "
    "γειά σου κόσμος "
    "Здравствулте мир "
    "</span>";

  (void) argc;
  (void) argv;

  /* Create window and renderer. */
  window = SDL_CreateWindow("Hello world!",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            width,
                            height,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  context = SDLPango_CreateContext();

  SDLPango_SetColor(context, &textcolor);
  SDLPango_SetMinimumSize(context, width - margin, height - margin);

  SDLPango_SetMarkup(context, text, -1);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

  surface = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);

  SDLPango_Draw(context, surface, margin, margin);

  background = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  SDL_SetRenderDrawColor(renderer, 0x83, 0x1c, 0xc5, 0xff);
  SDL_RenderClear(renderer);

  SDL_RenderCopy(renderer, background, NULL, NULL);

  for(;;) {
    SDL_Event event;

    if(SDL_PollEvent(&event) && event.type == SDL_QUIT)
      break;

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyTexture(background);
  SDLPango_FreeContext(context);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}