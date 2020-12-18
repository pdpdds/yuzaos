/*
 * Scene.cpp
 *
 *  Created on: Oct 29, 2016
 *      Author: myths
 */

#include"Scene.h"
#include"Environment.h"
Scene::Scene(SDL_Window *win, SDL_Renderer *ren) {
	this->window = win;
	this->render = ren;
}
int Scene::execute() {
	return 0;
}

Scene::~Scene() {
}

