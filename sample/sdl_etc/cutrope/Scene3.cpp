/*
 * Scene3.cpp
 *
 *  Created on: Nov 4, 2016
 *      Author: myths
 */

#include "Scene3.h"

#include"Rope.h"
#include"Environment.h"
Scene3::Scene3(SDL_Window *win, SDL_Renderer *ren) :
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
	star1 = new Ball(world, render, 2.4, 2.2, 0.06);
	star1->body->SetType(b2_staticBody);
	star1->fixture->SetFilterData(filter);
	items.push_back(star1);

	star2 = new Ball(world, render, 1.8, 4, 0.06);
	star2->body->SetType(b2_staticBody);
	star2->fixture->SetFilterData(filter);
	items.push_back(star2);

	star3 = new Ball(world, render, 3.6, 5.2, 0.06);
	star3->body->SetType(b2_staticBody);
	star3->fixture->SetFilterData(filter);
	items.push_back(star3);

	b2Vec2 startPos(3.6, 1);
	b2Vec2 endPos(3.6, 1.5);

	ball = new Ball(world, render, 4.5, 2, 0.06);
	items.push_back(ball);
	rope1 = new Rope(world, render, 1.8, startPos, ball);
	items.push_back(rope1);

	startPos.x = 5.2;
	rope2 = new Rope(world, render, 1.2, startPos, ball);
	items.push_back(rope2);

	startPos.x = 3.6;
	startPos.y=3.5;
	rope3 = new Rope(world, render, 1.8, startPos, ball);
	items.push_back(rope3);

	mouth = new Ball(world, render, 5.2, 5, 0.4);
	mouth->body->SetType(b2_staticBody);
	mouth->fixture->SetFilterData(filter);
	items.push_back(mouth);
}

Scene3::~Scene3() {
	delete world;
}
void Scene3::step() {
	float timeStep = 1.0f / 100.0f;
	int velocityIterations = 6;
	int positionIterations = 2;
	world->Step(timeStep, velocityIterations, positionIterations);
}

void Scene3::paint() {
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
unsigned int SceneThreeStepCallBack(unsigned int interval, void *param) {
	Scene3 *SceneThree = (Scene3*) param;
	SceneThree->step();
	if (SceneThree->rope1 != NULL
			&& SceneThree->rope1->intersect(SceneThree->mouseList)) {
		SceneThree->rope1->cut();

		for (unsigned int i = 0; i < SceneThree->items.size(); i++) {
			if (SceneThree->items[i] == SceneThree->rope1) {
				SceneThree->items.erase(SceneThree->items.begin() + i);
				break;
			}
		}
		SceneThree->rope1 = NULL;
	}
	if (SceneThree->rope2 != NULL
			&& SceneThree->rope2->intersect(SceneThree->mouseList)) {
		SceneThree->rope2->cut();
		for (unsigned int i = 0; i < SceneThree->items.size(); i++) {
			if (SceneThree->items[i] == SceneThree->rope2) {
				SceneThree->items.erase(SceneThree->items.begin() + i);
				break;
			}
		}
		SceneThree->rope2 = NULL;
	}
	if (SceneThree->rope3 != NULL
			&& SceneThree->rope3->intersect(SceneThree->mouseList)) {
		SceneThree->rope3->cut();
		for (unsigned int i = 0; i < SceneThree->items.size(); i++) {
			if (SceneThree->items[i] == SceneThree->rope3) {
				SceneThree->items.erase(SceneThree->items.begin() + i);
				break;
			}
		}
		SceneThree->rope3 = NULL;
	}
	if (SceneThree->star1 != NULL && SceneThree->ball->intersect(SceneThree->star1)) {
		SceneThree->world->DestroyBody(SceneThree->star1->body);

		for (unsigned int i = 0; i < SceneThree->items.size(); i++) {
			if (SceneThree->items[i] == SceneThree->star1) {
				SceneThree->items.erase(SceneThree->items.begin() + i);
				break;
			}
		}
		SceneThree->star1 = NULL;
	}
	if (SceneThree->star2 != NULL && SceneThree->ball->intersect(SceneThree->star2)) {
		SceneThree->world->DestroyBody(SceneThree->star2->body);

		for (unsigned int i = 0; i < SceneThree->items.size(); i++) {
			if (SceneThree->items[i] == SceneThree->star2) {
				SceneThree->items.erase(SceneThree->items.begin() + i);
				break;
			}
		}
		SceneThree->star2 = NULL;
	}
	if (SceneThree->star3 != NULL && SceneThree->ball->intersect(SceneThree->star3)) {
		SceneThree->world->DestroyBody(SceneThree->star3->body);

		for (unsigned int i = 0; i < SceneThree->items.size(); i++) {
			if (SceneThree->items[i] == SceneThree->star3) {
				SceneThree->items.erase(SceneThree->items.begin() + i);
				break;
			}
		}
		SceneThree->star3 = NULL;
	}

	if (SceneThree->ball->intersect(SceneThree->mouth)) {
		SceneThree->status = 1;
	}
	return interval;
}
unsigned int SceneThreePaintCallBack(unsigned int interval, void *param) {
	Scene3 *SceneThree = (Scene3*) param;
	SceneThree->paint();
	return interval;
}

int Scene3::execute() {

	int t1 = SDL_AddTimer(10, SceneThreeStepCallBack, this);
	int t2 = SDL_AddTimer(20, SceneThreePaintCallBack, this);

	bool quit = false;
	bool mouseDown;
	while (!quit) {
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
	SDL_RemoveTimer(t1);
	SDL_RemoveTimer(t2);
	return status;
}

