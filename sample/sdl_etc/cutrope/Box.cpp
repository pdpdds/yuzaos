/*
 * Box.cpp
 *
 *  Created on: Oct 27, 2016
 *      Author: myths
 */

#include"Box.h"
#include"Environment.h"

Box::Box(b2World *w, SDL_Renderer *r, double posx, double posy,
		double halfWidth, double halfHeight) {
	this->world = w;
	this->render = r;
	this->halfHeight = halfHeight;
	this->halfWidth = halfWidth;
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(posx, posy);
	this->body = world->CreateBody(&bodyDef);
	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(halfWidth, halfHeight);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	fixture = this->body->CreateFixture(&fixtureDef);
	SDL_Surface *surface = SDL_CreateRGBSurface(0, 100, 100, 32, 0xff000000,
			0x00ff0000, 0x0000ff00, 0x000000ff);
	if (surface == NULL) {
		printf("Could not create surface: %s\n", SDL_GetError());
	}
	SDL_Rect re = { 0, 0, 100, 100 };
	SDL_FillRect(surface, &re, SDL_MapRGB(surface->format, 120, 120, 120));
	texture = SDL_CreateTextureFromSurface(render, surface);
	SDL_FreeSurface(surface);
}
void Box::setId(int *id) {
	body->SetUserData((void*) id);
}
void Box::draw() {
	b2Vec2 position = body->GetPosition();
	double angle = body->GetAngle();
	SDL_Rect r = { (int) ((position.x - halfWidth) * 100), (int) ((position.y
			- halfHeight) * 100), int(halfWidth * 2 * 100), int(
			halfHeight * 2 * 100) };
	SDL_RenderCopyEx(render, this->texture, NULL, &r, angle * 180 / M_PI, NULL,
			SDL_FLIP_NONE);
}
Box::~Box() {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(render);
}
