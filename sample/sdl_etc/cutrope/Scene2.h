/*
 * Scene2.h
 *
 *  Created on: Nov 4, 2016
 *      Author: myths
 */

#ifndef SCENE2_H_
#define SCENE2_H_

#include"Scene.h"
#include"Rope.h"
#include"Environment.h"
class Scene2: public Scene {

public:
	std::vector<Drawable*> items;
	std::list<SDL_Point> mouseList;
	SDL_Texture *textureBg;
	b2World *world;

	Rope *rope1, *rope2, *rope3;
	Ball *ball;
	Ball *star1, *star2, *star3;
	Ball *mouth;
	int status;

	Scene2(SDL_Window *win, SDL_Renderer *ren);
	int execute();
	void step();
	void paint();
	~Scene2();
};

#endif /* SCENE2_H_ */
