/*
 * Ball.h
 *
 *  Created on: Oct 28, 2016
 *      Author: myths
 */

#ifndef BALL_H_
#define BALL_H_

#include"Environment.h"
#include"Drawable.h"

class Ball: public Drawable {
private:
	b2World *world;
	SDL_Renderer *render;
public:
	b2Body *body;
	b2Fixture *fixture;
	double radius;
	Ball(b2World *w, SDL_Renderer *r, double posx, double posy, double radius);
	void draw();
	bool intersect(Ball *ball);
	~Ball();
};

#endif
