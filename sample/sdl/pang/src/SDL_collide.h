#include <SDL.h>

/* Set up for C function definitions, even when using C++ */

/*
	SDL surface test if offset (u,v) is a transparent pixel
*/
int SDL_CollideTransparentPixelTest(SDL_Surface *surface , int u , int v);

/*
	SDL pixel perfect collision test
*/
int SDL_CollidePixel(SDL_Surface *as , int ax , int ay ,
                       SDL_Surface *bs , int bx , int by);

/*
	SDL bounding box collision test
*/
int SDL_CollideBoundingBoxSurface(SDL_Surface *sa , int ax , int ay ,
                             SDL_Surface *sb , int bx , int by);

/*
	SDL bounding box collision tests (works on SDL_Rect's)
*/
int SDL_CollideBoundingBox(SDL_Rect a , SDL_Rect b);

/*
	tests whether 2 circles intersect

	circle1 : centre (x1,y1) with radius r1
	circle2 : centre (x2,y2) with radius r2
	
	(allow distance between circles of offset)
*/
int SDL_CollideBoundingCircle(int x1 , int y1 , int r1 ,
                                 int x2 , int y2 , int r2 , int offset);

/*
	a circle intersection detection algorithm that will use
	the position of the centre of the surface as the centre of
	the circle and approximate the radius using the width and height
	of the surface (for example a rect of 4x6 would have r = 2.5).
*/
int SDL_CollideBoundingCircleSurface(SDL_Surface *a , int x1 , int y1 ,
                                 SDL_Surface *b , int x2 , int y2 ,
                                   int offset);


/* Ends C function definitions when using C++ */
