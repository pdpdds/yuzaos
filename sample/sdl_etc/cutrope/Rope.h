/*
 * Rope.h
 *
 *  Created on: Oct 28, 2016
 *      Author: myths
 */

#ifndef ROPE_H_
#define ROPE_H_

#include"Drawable.h"
#include"Ball.h"
#include"Box.h"
#include"Environment.h"
class Rope: public Drawable {
private:
	b2World *world;
	SDL_Renderer *render;
	std::vector<Box*> boxes;
	b2Vec2 startPos;
	b2Vec2 endPos;
	Ball *endBall;
	int randId;
	double length;
	double width;
public:
	Rope(b2World *w, SDL_Renderer *r, double length, b2Vec2 startPos,
			Ball *endBall);
	void draw();
	bool cross(b2Vec2 v11, b2Vec2 v12, b2Vec2 v21, b2Vec2 v22);
	bool intersect(std::list<SDL_Point> points);
	void cut();
	~Rope();
};

#endif /* ROPE_H_ */
