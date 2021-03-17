#ifndef SGLTEXTURE_H
#define SGLTEXTURE_H

#include "SGLColor.h"
struct SDL_Surface;
class SGLTexture
{
private:
	SDL_Surface* texture;
public:
	SGLTexture(void) : texture(0)
	{}
	SGLTexture(const char* filename);
	~SGLTexture(void);
	
	int getWidth(void);
	int getHeight(void);
	SGLColor getColorAt(int x, int y);
	SGLColor getColorAtf(float x, float y);
};

#endif