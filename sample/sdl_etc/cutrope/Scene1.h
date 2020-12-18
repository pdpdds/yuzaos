/*
 * SceneOne.h
 *
 *  Created on: Oct 29, 2016
 *      Author: myths
 */

#ifndef SCENE1_H_
#define SCENE1_H_
#include"Scene.h"
#include"Rope.h"
#include"Environment.h"
class Scene1: public Scene {

public:
	std::vector<Drawable*> items;
	std::list<SDL_Point> mouseList;
	SDL_Texture *textureBg;
	b2World *world;

	Rope *rope;
	Ball *ball;
	Ball *star1, *star2, *star3;
	Ball *mouth;
	int status;

	Scene1(SDL_Window *win, SDL_Renderer *ren);
	int execute();
	void step();
	void paint();
	~Scene1();
};

#endif /* SCENE1_H_ */
