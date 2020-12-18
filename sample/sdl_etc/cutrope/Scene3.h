/*
 * Scene3.h
 *
 *  Created on: Nov 4, 2016
 *      Author: myths
 */

#ifndef Scene3_H_
#define Scene3_H_

#include"Scene.h"
#include"Rope.h"
#include"Environment.h"
class Scene3: public Scene {

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

	Scene3(SDL_Window *win, SDL_Renderer *ren);
	int execute();
	void step();
	void paint();
	~Scene3();
};

#endif /* Scene3_H_ */
