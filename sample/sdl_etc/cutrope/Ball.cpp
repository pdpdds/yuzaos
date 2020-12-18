/*
 * Ball.cpp
 *
 *  Created on: Oct 28, 2016
 *      Author: myths
 */

#include"Environment.h"
#include"Ball.h"
Ball::Ball(b2World *w, SDL_Renderer *r, double posx, double posy,
		double radius) {
	this->world = w;
	this->render = r;
	this->radius = radius;
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(posx, posy);
	this->body = world->CreateBody(&bodyDef);
	b2CircleShape dynamicCircle;
	dynamicCircle.m_radius = radius;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicCircle;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	fixture = this->body->CreateFixture(&fixtureDef);

}

void DrawCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y)
    {
        //  Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

        if (error <= 0)
        {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0)
        {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

void Ball::draw() {
	b2Vec2 position = body->GetPosition();
    //DrawCircle(render, position.x * 100, position.y * 100, radius * 100);
	filledCircleColor(render, position.x * 100, position.y * 100, radius * 100,
			0xff0000ff);
}

bool Ball::intersect(Ball *next) {
	b2Vec2 position1 = body->GetPosition();
	b2Vec2 position2 = next->body->GetPosition();
	double dist = hypot(position1.x - position2.x, position1.y - position2.y);
	if (dist < radius + next->radius) {
		return true;
	}
	return false;
}
Ball::~Ball() {

}
