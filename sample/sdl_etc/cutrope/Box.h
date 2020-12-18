#ifndef BOX_H_
#define BOX_H_

#include"Environment.h"
#include"Drawable.h"

class Box: public Drawable {
private:
	SDL_Texture *texture;
	b2World *world;
	SDL_Renderer *render;
	double halfWidth, halfHeight;
public:
	b2Body *body;
	b2Fixture *fixture;
	Box(b2World *w, SDL_Renderer *r, double posx, double posy, double halfWidth,
			double halfHeight);
	void draw();
	void setId(int *id);
	~Box();
};

#endif
