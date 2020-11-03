#ifdef SDL

#include "SDLfilter.h"
#include <memory.h>


pixel mask(SDL_Surface *screen)
{
  return ~( (1<<screen->format->Rshift) | 
	    (1<<screen->format->Gshift) |
	    (1<<screen->format->Bshift) ) & ~screen->format->Amask;
}


void blur(SDL_Surface *screen,pixel** buffer)
{
  int i,j;

  pixel M = mask(screen);
  
  for(i=0;i<screen->w*screen->h;i+=2*screen->w)
    
    for(j=0;j<(screen->w-2);j+=2)
      
      (*buffer)[i+j+1] = (((*buffer)[i+j+1]&M)+((*buffer)[i+j+2]&M))>>1;

}


void mix_scanline(SDL_Surface *screen,pixel** buffer)
{  
  int i,j;
  
  pixel M = mask(screen);
  
  for(i=screen->w;i<screen->w*(screen->h-2);i+=2*screen->w)
    
    for(j=0;j<screen->w;j++)
 
      (*buffer)[i+j] = (((*buffer)[i+j-screen->w]&M)+((*buffer)[i+j+screen->w]&M))>>1;

  memset(&(*buffer)[i],0,screen->w*sizeof(pixel));
}


void half_scanline(SDL_Surface *screen,pixel** buffer)
{ 
  int i,j;

  pixel M = mask(screen);

  for(i=screen->w;i<screen->w*(screen->h-2);i+=2*screen->w)
    
    for(j=0;j<screen->w;j++)
      
      (*buffer)[i+j]= ((*buffer)[i+j-screen->w]&M)>>1;

  memset(&(*buffer)[i],0,screen->w*sizeof(pixel));
}

 
void remove_scanline(SDL_Surface *screen, pixel** buffer)
{ 
  int i;

  for(i=screen->w;i<screen->w*screen->h;i+=2*screen->w) 

    memcpy(&(*buffer)[i],&(*buffer)[i-screen->w],screen->w*sizeof(pixel));
}

void full_scanline(SDL_Surface *screen, pixel** buffer)
{
  int i;

  for(i=screen->w;i<screen->w*screen->h;i+=2*screen->w)

    memset(&(*buffer)[i],0,screen->w*sizeof(pixel));
}

#endif
