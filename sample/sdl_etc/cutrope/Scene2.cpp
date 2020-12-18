/*
 * Scene2.cpp
 *
 *  Created on: Nov 4, 2016
 *      Author: myths
 */

#include "Scene2.h"

#include"Rope.h"
#include"Environment.h"
Scene2::Scene2(SDL_Window *win, SDL_Renderer *ren) :
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

	b2Filter filter;
	filter.categoryBits = 0x0000;
	star1 = new Ball(world, render, 1.8, 2, 0.06);
	star1->body->SetType(b2_staticBody);
	star1->fixture->SetFilterData(filter);
	items.push_back(star1);

	star2 = new Ball(world, render, 5.4, 2, 0.06);
	star2->body->SetType(b2_staticBody);
	star2->fixture->SetFilterData(filter);
	items.push_back(star2);

	star3 = new Ball(world, render, 1.8, 4, 0.06);
	star3->body->SetType(b2_staticBody);
	star3->fixture->SetFilterData(filter);
	items.push_back(star3);

	b2Vec2 startPos(1.8, 0.5);
	b2Vec2 endPos(3.6, 1.5);

	ball = new Ball(world, render, 1.8, 2, 0.06);
	items.push_back(ball);
	rope1 = new Rope(world, render, 1.2, startPos, ball);
	items.push_back(rope1);

	startPos.x = 3.6;
	rope2 = new Rope(world, render, 2.4, startPos, ball);
	items.push_back(rope2);

	startPos.x = 5.4;
	rope3 = new Rope(world, render, 5.0, startPos, ball);
	items.push_back(rope3);

	mouth = new Ball(world, render, 3.6, 5, 0.4);
	mouth->body->SetType(b2_staticBody);
	mouth->fixture->SetFilterData(filter);
	items.push_back(mouth);
}

Scene2::~Scene2() {
	delete world;
}
void Scene2::step() {
	float timeStep = 1.0f / 100.0f;
	int velocityIterations = 6;
	int positionIterations = 2;
	world->Step(timeStep, velocityIterations, positionIterations);
}

void Scene2::paint() {
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
unsigned int sceneTwoStepCallBack(unsigned int interval, void *param) {
	Scene2 *sceneTwo = (Scene2*) param;
	sceneTwo->step();
	if (sceneTwo->rope1 != NULL
			&& sceneTwo->rope1->intersect(sceneTwo->mouseList)) {
		sceneTwo->rope1->cut();

		for (unsigned int i = 0; i < sceneTwo->items.size(); i++) {
			if (sceneTwo->items[i] == sceneTwo->rope1) {
				sceneTwo->items.erase(sceneTwo->items.begin() + i);
				break;
			}
		}
		sceneTwo->rope1 = NULL;
	}
	if (sceneTwo->rope2 != NULL
			&& sceneTwo->rope2->intersect(sceneTwo->mouseList)) {
		sceneTwo->rope2->cut();
		for (unsigned int i = 0; i < sceneTwo->items.size(); i++) {
			if (sceneTwo->items[i] == sceneTwo->rope2) {
				sceneTwo->items.erase(sceneTwo->items.begin() + i);
				break;
			}
		}
		sceneTwo->rope2 = NULL;
	}
	if (sceneTwo->rope3 != NULL
			&& sceneTwo->rope3->intersect(sceneTwo->mouseList)) {
		sceneTwo->rope3->cut();
		for (unsigned int i = 0; i < sceneTwo->items.size(); i++) {
			if (sceneTwo->items[i] == sceneTwo->rope3) {
				sceneTwo->items.erase(sceneTwo->items.begin() + i);
				break;
			}
		}
		sceneTwo->rope3 = NULL;
	}
	if (sceneTwo->star1 != NULL && sceneTwo->ball->intersect(sceneTwo->star1)) {
		sceneTwo->world->DestroyBody(sceneTwo->star1->body);

		for (unsigned int i = 0; i < sceneTwo->items.size(); i++) {
			if (sceneTwo->items[i] == sceneTwo->star1) {
				sceneTwo->items.erase(sceneTwo->items.begin() + i);
				break;
			}
		}
		sceneTwo->star1 = NULL;
	}
	if (sceneTwo->star2 != NULL && sceneTwo->ball->intersect(sceneTwo->star2)) {
		sceneTwo->world->DestroyBody(sceneTwo->star2->body);

		for (unsigned int i = 0; i < sceneTwo->items.size(); i++) {
			if (sceneTwo->items[i] == sceneTwo->star2) {
				sceneTwo->items.erase(sceneTwo->items.begin() + i);
				break;
			}
		}
		sceneTwo->star2 = NULL;
	}
	if (sceneTwo->star3 != NULL && sceneTwo->ball->intersect(sceneTwo->star3)) {
		sceneTwo->world->DestroyBody(sceneTwo->star3->body);

		for (unsigned int i = 0; i < sceneTwo->items.size(); i++) {
			if (sceneTwo->items[i] == sceneTwo->star3) {
				sceneTwo->items.erase(sceneTwo->items.begin() + i);
				break;
			}
		}
		sceneTwo->star3 = NULL;
	}

	if (sceneTwo->ball->intersect(sceneTwo->mouth)) {
		sceneTwo->status = 1;
	}
	return interval;
}
unsigned int sceneTwoPaintCallBack(unsigned int interval, void *param) {
	Scene2 *sceneTwo = (Scene2*) param;
	sceneTwo->paint();
	return interval;
}

int Scene2::execute() {

	//int t1 = SDL_AddTimer(10, sceneTwoStepCallBack, this);
	//int t2 = SDL_AddTimer(20, sceneTwoPaintCallBack, this);

	bool quit = false;
	bool mouseDown;
	while (!quit) {

		sceneTwoStepCallBack(10, this);
		sceneTwoPaintCallBack(20, this);

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

