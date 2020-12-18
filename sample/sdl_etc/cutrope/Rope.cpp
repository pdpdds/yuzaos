/*
 * Rope.cpp
 *
 *  Created on: Oct 28, 2016
 *      Author: myths
 */
#include"Drawable.h"
#include"Environment.h"
#include"Ball.h"
#include"Rope.h"

Rope::Rope(b2World *w, SDL_Renderer *r, double length, b2Vec2 startPos,
		Ball* endBall) {
	this->randId = rand();
	this->world = w;
	this->render = r;
	this->length = length;
	this->width = 0.05;
	this->startPos = startPos;
	this->endBall = endBall;
	this->endPos = endBall->body->GetPosition();
	int count = 20;

	Box *startBox = new Box(w, r, startPos.x, startPos.y, 0.02, 0.02);
	startBox->setId(&randId);
	startBox->body->SetType(b2_staticBody);
	b2Filter filter;
	filter.categoryBits = 0x0000;
	startBox->fixture->SetFilterData(filter);
	boxes.push_back(startBox);
	for (int i = 0; i < count; i++) {
		Box *box = new Box(w, r, startPos.x + length / count / 2, startPos.y,
				length / count / 2, width / 2);
		box->fixture->SetFilterData(filter);
		b2MassData massData;
		box->body->GetMassData(&massData);
		massData.mass=0.001;
		box->body->SetMassData(&massData);
		box->setId(&randId);
		boxes.push_back(box);
	}

	b2RevoluteJointDef jointDef;
	b2Vec2 left(startPos.x, startPos.y);
	b2Vec2 right(startPos.x + length / count, startPos.y);

	for (unsigned int i = 1; i < boxes.size(); i++) {
		if (i & 1)
			jointDef.Initialize(boxes[i - 1]->body, boxes[i]->body, left);
		else
			jointDef.Initialize(boxes[i - 1]->body, boxes[i]->body, right);
		jointDef.userData = &randId;
		jointDef.collideConnected = true;
		jointDef.referenceAngle = -30 * M_PI / 180;
		world->CreateJoint(&jointDef);
	}
	jointDef.userData = &randId;
	if (count & 1) {
		endBall->body->SetTransform(right, 0);
		jointDef.Initialize(boxes[boxes.size() - 1]->body, endBall->body,
				right);
	} else {
		endBall->body->SetTransform(left, 0);
		jointDef.Initialize(boxes[boxes.size() - 1]->body, endBall->body, left);

	}
	b2MassData md;
	endBall->body->GetMassData(&md);
	md.mass=2;
	endBall->body->SetMassData(&md);
	world->CreateJoint(&jointDef);

//	//Add rope joint
	b2RopeJointDef ropeJointDef;
	ropeJointDef.bodyA = boxes[0]->body;
	ropeJointDef.bodyB = endBall->body;
	ropeJointDef.maxLength = length;
	b2Vec2 zero(0, 0);
	ropeJointDef.localAnchorA = zero;
	ropeJointDef.localAnchorB = zero;
	ropeJointDef.userData = &randId;
	world->CreateJoint(&ropeJointDef);

//Move to required location
	double deltaX = (endPos.x - startPos.x) / count;
	double deltaY = (endPos.y - startPos.y) / count;
	for (unsigned int i = 1; i < boxes.size(); i++) {
		b2Vec2 vec(startPos.x + deltaX * i - deltaX / 2,
				startPos.y + deltaY * i - deltaY / 2);
		boxes[i]->body->SetTransform(vec, 0);
	}
	endBall->body->SetTransform(endPos, 0);
}
bool Rope::cross(b2Vec2 v11, b2Vec2 v12, b2Vec2 v21, b2Vec2 v22) {
	double area1 = (v11.x - v21.x) * (v12.y - v21.y)
			- (v11.y - v21.y) * (v12.x - v21.x);
	double area2 = (v11.x - v22.x) * (v12.y - v22.y)
			- (v11.y - v22.y) * (v12.x - v22.x);
	if (area1 * area2 >= 0)
		return false;
	double area3 = (v21.x - v11.x) * (v22.y - v11.y)
			- (v21.y - v11.y) * (v22.x - v11.x);
	double area4 = area3 + area1 - area2;
	if (area3 * area4 >= 0)
		return false;
	return true;
}
bool Rope::intersect(std::list<SDL_Point> points) {
	for (unsigned int i = 0; i < boxes.size() - 1; i++) {
		b2Vec2 v11 = boxes[i]->body->GetPosition();
		b2Vec2 v12 = boxes[i + 1]->body->GetPosition();
		std::list<SDL_Point>::iterator it = points.begin(), itnext =
				points.begin();
		itnext++;
		for (; itnext != points.end(); it++, itnext++) {
			b2Vec2 v21(it->x / PTM_RATIO, it->y / PTM_RATIO);
			b2Vec2 v22(itnext->x / PTM_RATIO, itnext->y / PTM_RATIO);
			if (cross(v11, v12, v21, v22)) {
				return true;
			}
		}
	}
	return false;
}
void Rope::draw() {
	for (unsigned int i = 0; i < boxes.size(); i++) {
		boxes[i]->draw();
	}
}
void Rope::cut() {
	b2Body *currBody = world->GetBodyList();
	b2Body *nextBody;
	while (currBody != NULL) {
		nextBody = currBody->GetNext();
		if (currBody->GetUserData() != NULL
				&& *(int*) (currBody->GetUserData()) == randId) {
			world->DestroyBody(currBody);
		}
		currBody = nextBody;
	}

	b2Joint *currJoint = world->GetJointList();
	b2Joint *nextJoint = NULL;
	while (currJoint != NULL) {
		nextJoint = currJoint->GetNext();
		if (*(int*) (currJoint->GetUserData()) == randId) {
			world->DestroyJoint(currJoint);
		}
		currJoint = nextJoint;
	}
}

Rope::~Rope() {
	for (unsigned int i = 0; i < boxes.size(); i++) {
		delete boxes[i];
	}
}
