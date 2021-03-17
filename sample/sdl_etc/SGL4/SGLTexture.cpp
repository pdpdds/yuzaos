#include "SGLDevice.h"
#include "SGLTexture.h"
#include <SDL.h>

#define TEXADDRESS(x, y) (((unsigned char *)texture->pixels + (x*texture->format->BytesPerPixel) + (y*texture->pitch)));

SGLTexture::SGLTexture(const char* filename)
{
	SDL_Surface* bmp = 0;
	texture = SDL_LoadBMP(filename);
	
	if(!texture)
	{
		printf("%s 파일을 로드할 수 없습니다\n");
	}
}
SGLTexture::~SGLTexture(void)
{
	if(texture)
	{
		SDL_FreeSurface(texture);
	}
}
int SGLTexture::getWidth(void)
{
	return texture->w;
}
int SGLTexture::getHeight(void)
{
	return texture->h;
}
SGLColor SGLTexture::getColorAt(int x, int y)
{
	if(x<0 || y<0)
	{
		x = 0;
		y = 0;
	}
	if(x>=texture->w || y>=texture->w)
	{
		x = 0;
		y = 0;
	}
	unsigned char* pixel = TEXADDRESS(x, y);
	
	return SGLColor(pixel[2], pixel[1], pixel[0]);
}

SGLColor SGLTexture::getColorAtf(float x, float y)
{
	return getColorAt((int)(x*(float)texture->w),(int)(y*(float)texture->h));
}


