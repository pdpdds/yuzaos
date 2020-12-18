/*
 * Scene.h
 *
 *  Created on: Oct 29, 2016
 *      Author: myths
 */

#ifndef SCENE_H_
#define SCENE_H_
#include"Environment.h"
#include"Drawable.h"
class Scene {
protected:
	SDL_Renderer *render;
	SDL_Window *window;
public:
	Scene(SDL_Window *win, SDL_Renderer *render);
	virtual int execute();
	virtual ~Scene();
};

#endif /* SCENE_H_ */
