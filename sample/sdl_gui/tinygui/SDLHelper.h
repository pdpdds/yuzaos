/* SDLHelper v6.0.1
   Date: 27.1.2013
   LINKER: -lmingw32 -lSDLmain -lSDL + additional (-lSDL_image -lSDL_Net, -lSDL_ttf, -lSDL_mixer, -lopengl32 -lglu32)
*/

#pragma once
#ifndef _SDLHelper_H_
#define _SDLHelper_H_
#define _SDLHelper_VER_ 601

/********** Set defines, includes below **********/

#include <SDL.h>
#include <SDL_image.h> //uncomment to enable
#include <SDL_ttf.h>   //uncomment to enable
//#include <SDL_net.h>   //uncomment to enable
//#include <SDL/SDL_mixer.h> //uncomment to enable
#define SDLHELPER_INIT_FLAGS SDL_INIT_TIMER|SDL_INIT_VIDEO //SDL_INIT_EVERYTHING
#define SDLHELPER_MIXER_FLAGS MIX_INIT_MP3 //MIX_INIT_MOD|MIX_INIT_MP3|MIX_INIT_OGG|MIX_INIT_FLAC

/********** Do not change anything below **********/

/** Initializes/Exits SDL and extensions if included **/
void SDLInit(); void SDLExit();

/** Creates/changes the SDL window for drawing **/
bool SDLSetVideo(int width, int height, bool resizable = false, bool fullscreen = false, bool noframe = false, bool useOpenGL = false);

/** Creates a new surface, or loads it from a file **/
SDL_Surface* SDLNew(Uint16 width, Uint16 height, bool alpha, bool hardware = true, bool fix = true); //fix only works after SDLSetVideo() has been called
SDL_Surface* SDLLoad(const char *file, bool fix = true, bool addAlpha = false); //If SDL_image is not included it only works with bmp

/** Blits src to dst on x,y with the size w,h, returns 0 on success **/
int SDLBlit(SDL_Surface *src, SDL_Surface *dst, int x = 0, int y = 0, int w = 0, int h = 0, int srcX = 0, int srcY = 0);

/** Pixels & conversion **/
Uint32 SDLGetPixel(SDL_Surface *src, int x, int y); //Set pixel
void   SDLPutPixel(SDL_Surface *dst, int x, int y, Uint32 color); //Get pixel
Uint32 RGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 0xFF); //Convert r,g,b,a to Uint32
SDL_Color Uint32ToColor(Uint32 color); //Convert Uint32 to SDL_Color
Uint32 ColorToUint32(SDL_Color color); //Convert SDL_Color to Uint32
Uint16 CharToUni(char chr); //character to UNICODE
char UniToChar(Uint16 chr, char unknown = '?'); //UNICODE to character

/** Push events, returns 0 on success **/
int SDLPushEvent(int code, void *data1 = NULL, void *data2 = NULL); //Push SDL_USEREVENT
int SDLPushResizeEvent(int w, int h); //Push SDL_VIDEORESIZE

/** Global variables **/
extern SDL_Surface* screen; //Screen surface
extern int SDLResolutionX, SDLResolutionY; //Screen resolution set by SDLInit()

/** Timer class **/
class SDLTimer{ public:
  SDLTimer(); ~SDLTimer();
  void Construct(); void Destruct();
  void Reset();
  void Frame(Uint32 minDelay = 1);
  void SetFPSUpdateTime(Uint32 updateTime); //Sets how often fps gets updated (default 1000)

  float fps; //Frames per second
  Uint32 time; //Milliseconds between last 2 calls to Frame()
  Uint32 total; //Milliseconds since last call to Reset()
  Uint32 now; //Ticks at last call to Frame()
private:
  Uint32 lastFrame, lastFpsUpdate, fpsUpdateTime, frameCount;
};

/** Events struct **/
struct SDLEvents{
  SDLEvents(); ~SDLEvents();
  void Construct(); void Destruct();
  void HandleEvent(SDL_Event *event);

  bool keys[323];
  Uint16 mouseX, mouseY;
  bool mouse_l, mouse_r, mouse_m, mouse_x1, mouse_x2;
  bool quit, appMouseFocus, appInputFocus, appActive;
};

#endif
