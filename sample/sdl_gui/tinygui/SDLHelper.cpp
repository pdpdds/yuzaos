#include "SDLHelper.h"
#if _SDLHelper_VER_ != 601
  #warning Invalid SDLHelper header version
#endif

SDL_Surface* screen = NULL;
int SDLResolutionX = 0, SDLResolutionY = 0;

void SDLInit(){
  /*if(!(SDL_WasInit(SDLHELPER_INIT_FLAGS) & (SDLHELPER_INIT_FLAGS))){
    if(SDL_Init(SDLHELPER_INIT_FLAGS) == -1){ fprintf(stderr,"Could not initialize SDL: %s",SDL_GetError()); exit(1); }
  }
  #ifdef _SDL_NET_H
    if(SDLNet_Init() == -1){ fprintf(stderr,"Could not initialize SDL_net: %s",SDLNet_GetError()); exit(1); }
  #endif*/
  #ifdef _SDL_TTF_H
	if (TTF_WasInit() == 0)if (TTF_Init() == -1){ ; }// fprintf(stderr, "Could not initialize SDL_ttf: %s", TTF_GetError()); exit(1); }
  #endif
  #ifdef _SDL_MIXER_H
    int mixFlags = SDLHELPER_MIXER_FLAGS;
    if((mixFlags&Mix_Init(mixFlags)) != mixFlags){ fprintf(stderr,"Could not initialize SDL_mixer: %s",Mix_GetError()); exit(1); }
  #endif
  //SDLResolutionX = int(((SDL_VideoInfo*)SDL_GetVideoInfo())->current_w);
  //SDLResolutionY = int(((SDL_VideoInfo*)SDL_GetVideoInfo())->current_h);
}

void SDLExit(){
  if(screen != NULL){ SDL_FreeSurface(screen); screen = NULL; }
  #ifdef _SDL_NET_H
    SDLNet_Quit();
  #endif
  #ifdef _SDL_TTF_H
    if(TTF_WasInit() != 0)TTF_Quit();
  #endif
  #ifdef _SDL_MIXER_H
    Mix_Quit();
  #endif
  if(SDL_WasInit(SDLHELPER_INIT_FLAGS) & (SDLHELPER_INIT_FLAGS))SDL_Quit();
}

bool SDLSetVideo(int width, int height, bool resizable, bool fullscreen, bool noframe, bool useOpenGL){
  /*if(!(SDL_WasInit(SDL_INIT_VIDEO)))if(SDL_Init(SDL_INIT_VIDEO) == -1)return false;
  Uint8 bpp = ((SDL_VideoInfo*)SDL_GetVideoInfo())->vfmt->BitsPerPixel; if(bpp>32||(bpp!=32&&bpp!=24&&bpp!=16&&bpp!=8))bpp=32;
  Uint32 flags = SDL_ANYFORMAT|SDL_DOUBLEBUF|((((SDL_VideoInfo*)SDL_GetVideoInfo())->hw_available)?SDL_HWSURFACE:SDL_SWSURFACE)|((resizable)?SDL_RESIZABLE:0)|((fullscreen)?SDL_FULLSCREEN:0)|((noframe||fullscreen)?SDL_NOFRAME:0)|((useOpenGL)?SDL_OPENGL:0);
  screen = SDL_SetVideoMode(width, height, bpp, flags);
  return (screen != NULL);*/
	return false;
}

SDL_Surface* SDLNew(Uint16 width, Uint16 height, bool alpha, bool hardware, bool fix){
  /*if(!(SDL_WasInit(SDL_INIT_VIDEO)))if(SDL_Init(SDL_INIT_VIDEO) == -1)return NULL;
  SDL_Surface *tmp = NULL;
  Uint8 bpp = ((SDL_VideoInfo*)SDL_GetVideoInfo())->vfmt->BitsPerPixel; if(bpp>32||(bpp!=32&&bpp!=24&&bpp!=16&&bpp!=8))bpp=32;
  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    tmp = SDL_CreateRGBSurface((((SDL_VideoInfo*)SDL_GetVideoInfo())->hw_available && hardware)?SDL_HWSURFACE:SDL_SWSURFACE,width,height,bpp,0xff000000,0x00ff0000,0x0000ff00,(alpha)?0x000000ff:0);
  #else
    tmp = SDL_CreateRGBSurface((((SDL_VideoInfo*)SDL_GetVideoInfo())->hw_available && hardware)?SDL_HWSURFACE:SDL_SWSURFACE,width,height,bpp,0x000000ff,0x0000ff00,0x00ff0000,(alpha)?0xff000000:0);
  #endif
  if(tmp == NULL)return NULL;
  if(fix && (SDL_GetVideoSurface() != NULL)){
    SDL_Surface *t = ((alpha)?SDL_DisplayFormatAlpha(tmp):SDL_DisplayFormat(tmp));
    SDL_FreeSurface(tmp);
    return t;
  }
  return tmp;*/
	return NULL;
}

SDL_Surface* SDLLoad(const char *file, bool fix, bool addAlpha){
 /* if(!(SDL_WasInit(SDL_INIT_VIDEO)))if(SDL_Init(SDL_INIT_VIDEO) == -1)return NULL;
  SDL_Surface *tmp = NULL;
   #ifdef _SDL_IMAGE_H
    tmp = IMG_Load(file);
  #else
    tmp = SDL_LoadBMP(file);
  #endif
  if(tmp == NULL)return NULL;
  if((fix||addAlpha) && (SDL_GetVideoSurface() != NULL)){
    SDL_Surface *t = (((tmp->flags&SDL_SRCALPHA)||addAlpha)?SDL_DisplayFormatAlpha(tmp):SDL_DisplayFormat(tmp));
    SDL_FreeSurface(tmp);
    return t;
  }
  return tmp;*/
	return NULL;
}

int SDLBlit(SDL_Surface *src, SDL_Surface *dst, int x, int y, int w, int h, int srcX, int srcY){
  SDL_Rect srcRect, dstRect;
  if(w <= 0)w = src->w;
  if(h <= 0)h = src->h;
  srcRect.w = w; srcRect.h = h;
  srcRect.x = srcX; srcRect.y = srcY;
  dstRect.x = x; dstRect.y = y;
  return SDL_BlitSurface(src, &srcRect, dst, &dstRect);
}

Uint32 SDLGetPixel(SDL_Surface *src, int x, int y){
  if(src==NULL || x<0 || x>=src->w || y<0 || y>=src->h)return 0x00000000;
  int bpp = src->format->BytesPerPixel;
  Uint8 *p = (Uint8*)src->pixels+y*src->pitch+x*bpp;
  Uint32 color; Uint8 r,g,b,a;
  switch(bpp){
  case 1: color = *p; break;
  case 2: color = *(Uint16*)p; break;
  case 3:
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    color = p[0]<<16|p[1]<<8|p[2];
    #else
    color = p[0]|p[1]<<8|p[2]<<16;
    #endif
    break;
  case 4: color = *(Uint32*)p; break;
  default: return 0x00000000;
  }
  SDL_GetRGBA(color, src->format, &r, &g, &b, &a);
  return ((Uint32) r << 24) | ((Uint32) g << 16) | ((Uint32) b << 8) | (Uint32) a;
}

void SDLPutPixel(SDL_Surface *dst, int x, int y, Uint32 color){
  if(dst==NULL || x<0 || x>=dst->w || y<0 || y>=dst->h)return;
  if(SDL_MUSTLOCK(dst))SDL_LockSurface(dst);
  int bpp = dst->format->BytesPerPixel;
  Uint8 *p = (Uint8*)dst->pixels+y*dst->pitch+x*bpp;
  color = SDL_MapRGBA(dst->format, (color & 0xff000000) >> 24, (color & 0x00ff0000) >> 16, (color & 0x0000ff00) >> 8, color & 0x000000ff);
  switch(bpp){
  case 1: *p = color; break;
  case 2: *(Uint16*)p = color; break;
  case 3:
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    p[0] = (color>>16)&0xff; p[1] = (color>>8)&0xff; p[2] = color&0xff;
    #else
    p[0] = color&0xff; p[1] = (color>>8)&0xff; p[2] = (color>>16)&0xff;
    #endif
    break;
  case 4: *(Uint32*)p = color; break;
  }
  if(SDL_MUSTLOCK(dst))SDL_UnlockSurface(dst);
}

Uint32 RGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a){ return (((Uint32) r << 24) | ((Uint32) g << 16) | ((Uint32) b << 8) | (Uint32) a);  }

SDL_Color Uint32ToColor(Uint32 color){
  SDL_Color c;
  c.r = (Uint8) ((color >> 24) & 0xFF );
  c.g = (Uint8) ((color >> 16) & 0xFF );
  c.b = (Uint8) ((color >> 8 ) & 0xFF );
  c.a = (Uint8) (color & 0xFF);
  return c;
}

Uint32 ColorToUint32(SDL_Color color){
  Uint32 c = 0x00000000;
  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
  c = (c << 8) + color.unused;
  c = (c << 8) + color.b;
  c = (c << 8) + color.g;
  c = (c << 8) + color.r;
  #else
  c = (c << 8) + color.r;
  c = (c << 8) + color.g;
  c = (c << 8) + color.b;
  c = (c << 8) + color.a;
  #endif
  return c;
}

char UniToChar(Uint16 chr, char unknown){
  return ((chr&0xFF80) == 0)?chr&0x7F:unknown;
}

Uint16 CharToUni(char chr){
  return (Uint16)chr;
}

int SDLPushEvent(int code, void *data1, void *data2){
  SDL_Event event;
  event.type = SDL_USEREVENT;
  event.user.code = code;
  event.user.data1 = data1;
  event.user.data2 = data2;
  return SDL_PushEvent(&event);
}

int SDLPushResizeEvent(int w, int h){
  /*SDL_Event event;
  event.type = SDL_VIDEORESIZE;
  event.resize.w = w;
  event.resize.h = h;
  return SDL_PushEvent(&event);*/
return 0;
}

SDLTimer::SDLTimer(){ Construct(); }
SDLTimer::~SDLTimer(){ Destruct(); }

void SDLTimer::Construct(){
  fps = 0.; time = 0; total = 0; now = 0;
  lastFrame = 0; lastFpsUpdate = 0; fpsUpdateTime = 1000; frameCount = 0;
}

void SDLTimer::Destruct(){
  Construct();
}

void SDLTimer::Reset(){
  total = 0;
  lastFrame = 0; lastFpsUpdate = 0; frameCount = 0;
}

void SDLTimer::SetFPSUpdateTime(Uint32 updateTime){
  fpsUpdateTime = updateTime;
}

void SDLTimer::Frame(Uint32 minDelay){
  now = SDL_GetTicks();
  if(lastFrame == 0){
    lastFrame = now;
    time = minDelay;
  }
  time = now - lastFrame;
  while(time < minDelay){
    SDL_Delay(minDelay - time);
    now = SDL_GetTicks();
    time = now - lastFrame;
  }
  lastFrame = now;
  total += time;
  lastFpsUpdate += time;
  frameCount++;
  if(lastFpsUpdate > fpsUpdateTime){
    fps = (1000.f*frameCount)/lastFpsUpdate;
    frameCount = 0;
    lastFpsUpdate = 0;
  }
}

SDLEvents::SDLEvents(){ Construct(); }
SDLEvents::~SDLEvents(){ Destruct(); }

void SDLEvents::Construct(){
  for(size_t n = 0; n<323; n++)keys[n] = false;
  mouseX = 0; mouseY = 0;
  mouse_l = false; mouse_r = false; mouse_m = false; mouse_x1 = false; mouse_x2 = false;
  quit = false; appMouseFocus = false; appInputFocus = false; appActive = false;
}

void SDLEvents::Destruct(){
  Construct();
}

void SDLEvents::HandleEvent(SDL_Event *event){
  switch(event->type){
  /*case SDL_ACTIVEEVENT:
    switch(event->active.state){
    case SDL_APPMOUSEFOCUS: appMouseFocus = (bool)event->active.gain; break;
    case SDL_APPINPUTFOCUS: appInputFocus = (bool)event->active.gain; break;
    case SDL_APPACTIVE: appActive = (bool)event->active.gain; break;
    }
    break;*/
  case SDL_KEYDOWN: case SDL_KEYUP:
    keys[event->key.keysym.sym] = (event->key.state==SDL_PRESSED)?true:false;
    break;
  case SDL_MOUSEMOTION:
    mouseX = event->motion.x; mouseY = event->motion.y;
    break;
  case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP:
    switch(event->button.button){
    case SDL_BUTTON_LEFT:   mouse_l = (event->button.state==SDL_PRESSED)?true:false; break;
    case SDL_BUTTON_MIDDLE: mouse_m = (event->button.state==SDL_PRESSED)?true:false; break;
    case SDL_BUTTON_RIGHT:  mouse_r = (event->button.state==SDL_PRESSED)?true:false; break;
    case SDL_BUTTON_X1:    mouse_x1 = (event->button.state==SDL_PRESSED)?true:false; break;
    case SDL_BUTTON_X2:    mouse_x2 = (event->button.state==SDL_PRESSED)?true:false; break;
    //case SDL_BUTTON_WHEELUP: break;
    //case SDL_BUTTON_WHEELDOWN: break;
    }
    break;
  //case SDL_VIDEORESIZE: /* event->resize.w & event->resize.h */ break;
 // case SDL_VIDEOEXPOSE: /* need redraw */ break;
  case SDL_QUIT: quit = true; break;
  case SDL_USEREVENT: /* (int)event->user.code & (void*)event->user.data1 & (void*)event->user.data2 */ break;
  case SDL_SYSWMEVENT: /* undefined (SDL_SysWMmsg*)event->syswm.msg */ break;
  }
}
