/*
 * SceneOne.cpp
 *
 *  Created on: Oct 29, 2016
 *      Author: myths
 */

#include "Scene1.h"

#include"Rope.h"
#include"Environment.h"
Scene1::Scene1(SDL_Window *win, SDL_Renderer *ren) :
		Scene(win, ren) {
	int w, h;
	status = 0;
	SDL_GetWindowSize(window, &w, &h);
	SDL_Surface *surface = SDL_GetWindowSurface(window);
	SDL_Rect rectBg = { 0, 0, w, h };
	SDL_FillRect(surface, &rectBg, SDL_MapRGB(surface->format, 255, 255, 255));
	textureBg = SDL_CreateTextureFromSurface(render, surface);
	SDL_FreeSurface(surface);

	world = new b2World(b2Vec2(0, 10));
	b2BodyDef groundBodyDef;
	groundBodyDef.position.SetZero();

	b2Body *groundBody = world->CreateBody(&groundBodyDef);
	b2EdgeShape groundBox;
	groundBox.Set(b2Vec2(0, 0), b2Vec2(w / PTM_RATIO, 0));
	groundBody->CreateFixture(&groundBox, 0);
	groundBox.Set(b2Vec2(0, 0), b2Vec2(0, h / PTM_RATIO));
	groundBody->CreateFixture(&groundBox, 0);
	groundBox.Set(b2Vec2(0, h / PTM_RATIO),
			b2Vec2(w / PTM_RATIO, h / PTM_RATIO));
	groundBody->CreateFixture(&groundBox, 0);
	groundBox.Set(b2Vec2(w / PTM_RATIO, h / PTM_RATIO),
			b2Vec2(w / PTM_RATIO, 0));
	groundBody->CreateFixture(&groundBox, 0);

	b2Vec2 startPos(3.6, 0.5);
	b2Vec2 endPos(3.6, 1.5);

	b2Filter filter;
	filter.categoryBits = 0x0000;
	star1 = new Ball(world, render, 3.6, 2, 0.06);
	star1->body->SetType(b2_staticBody);
	star1->fixture->SetFilterData(filter);
	items.push_back(star1);

	star2 = new Ball(world, render, 3.6, 3, 0.06);
	star2->body->SetType(b2_staticBody);
	star2->fixture->SetFilterData(filter);
	items.push_back(star2);

	star3 = new Ball(world, render, 3.6, 4, 0.06);
	star3->body->SetType(b2_staticBody);
	star3->fixture->SetFilterData(filter);
	items.push_back(star3);

	ball = new Ball(world, render, 3.6, 1.5, 0.06);
	items.push_back(ball);
	rope = new Rope(world, render, 1.0, startPos, ball);
	items.push_back(rope);

	mouth = new Ball(world, render, 3.6, 5, 0.4);
	mouth->body->SetType(b2_staticBody);
	mouth->fixture->SetFilterData(filter);
	items.push_back(mouth);
}

Scene1::~Scene1() {
	delete world;
}
void Scene1::step() {
	float timeStep = 1.0f / 100.0f;
	int velocityIterations = 6;
	int positionIterations = 2;
	world->Step(timeStep, velocityIterations, positionIterations);
}

void Scene1::paint() {
	SDL_RenderClear(render);
	SDL_RenderCopy(render, textureBg, NULL, NULL);
	for (unsigned int i = 0; i < items.size(); i++) {
		items[i]->draw();
	}
	std::list<SDL_Point>::iterator it1 = mouseList.begin(), it2 =
			mouseList.begin();
	for (it2++; it2 != mouseList.end(); it1++, it2++) {
		thickLineColor(render, it1->x, it1->y, it2->x, it2->y, 5, 0xff00ff00);
	}
	SDL_RenderPresent(render);

}
unsigned int sceneOneStepCallBack(unsigned int interval, void *param) {
	Scene1 *sceneOne = (Scene1*) param;
	sceneOne->step();
	if (sceneOne->rope->intersect(sceneOne->mouseList)) {
		sceneOne->rope->cut();
		for (unsigned int i = 0; i < sceneOne->items.size(); i++) {
			if (sceneOne->items[i] == sceneOne->rope) {
				sceneOne->items.erase(sceneOne->items.begin() + i);
				break;
			}
		}
	}
	if (sceneOne->star1 != NULL && sceneOne->ball->intersect(sceneOne->star1)) {
		sceneOne->world->DestroyBody(sceneOne->star1->body);

		for (unsigned int i = 0; i < sceneOne->items.size(); i++) {
			if (sceneOne->items[i] == sceneOne->star1) {
				sceneOne->items.erase(sceneOne->items.begin() + i);
				break;
			}
		}
		sceneOne->star1 = NULL;
	}
	if (sceneOne->star2 != NULL && sceneOne->ball->intersect(sceneOne->star2)) {
		sceneOne->world->DestroyBody(sceneOne->star2->body);

		for (unsigned int i = 0; i < sceneOne->items.size(); i++) {
			if (sceneOne->items[i] == sceneOne->star2) {
				sceneOne->items.erase(sceneOne->items.begin() + i);
				break;
			}
		}
		sceneOne->star2 = NULL;
	}
	if (sceneOne->star3 != NULL && sceneOne->ball->intersect(sceneOne->star3)) {
		sceneOne->world->DestroyBody(sceneOne->star3->body);

		for (unsigned int i = 0; i < sceneOne->items.size(); i++) {
			if (sceneOne->items[i] == sceneOne->star3) {
				sceneOne->items.erase(sceneOne->items.begin() + i);
				break;
			}
		}
		sceneOne->star3 = NULL;
	}

	if (sceneOne->ball->intersect(sceneOne->mouth)) {
		sceneOne->status = 1;
	}
	return interval;
}
unsigned int sceneOnePaintCallBack(unsigned int interval, void *param) {
	Scene1 *sceneOne = (Scene1*) param;
	sceneOne->paint();
	return interval;
}

int Scene1::execute() {

	//int t1 = SDL_AddTimer(10, sceneOneStepCallBack, this);
	//int t2 = SDL_AddTimer(20, sceneOnePaintCallBack, this);

	bool quit = false;
	bool mouseDown;
	while (!quit) 
	{
		sceneOneStepCallBack(10, this);
		sceneOnePaintCallBack(20, this);
		if (status == 1) {
			quit = true;
		}
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT: {
				quit = true;
				status = 0;
				break;
			}
			case SDL_MOUSEMOTION: {
				if (mouseDown) {
					if (mouseList.size() == 5) {
						mouseList.pop_front();
					}
					SDL_Point p;
					p.x = e.motion.x;
					p.y = e.motion.y;
					mouseList.push_back(p);
				}
				break;
			}
			case SDL_MOUSEBUTTONDOWN: {
				mouseDown = true;
				break;
			}
			case SDL_MOUSEBUTTONUP: {
				mouseList.clear();
				mouseDown = false;
				break;
			}
			default:
				break;
			}
		}
	}
	SDL_RenderClear(render);
	//SDL_RemoveTimer(t1);
	//SDL_RemoveTimer(t2);
	return status;
}

