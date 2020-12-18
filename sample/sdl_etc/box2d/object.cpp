#include "object.h"


PhysicsObject:: ~PhysicsObject() {
    area->GetWorld()->DestroyBody(area);
}

PhysicsObject::PhysicsObject(b2BodyType body_type, b2World * world, float x, float y) {
    //set position and angle.
    position.x = x;
    position.y = y;
    angle = 0 * (M_PI/180);

    //define the limits of the body.
    body_def.type = body_type;
    body_def.position.Set(position.x, position.y);
    body_def.angle = angle;

    //create the body of the physics object.
    area = world->CreateBody(&body_def);

    area->SetFixedRotation(false);
}

void PhysicsObject::CreateFixture(b2PolygonShape shape, float density, float friction, float restitution) {

    //add a fixture to the body, to define movement and mass.
    fixture_def.shape = &shape;
    fixture_def.density = density;
    fixture_def.friction = friction;
    fixture_def.restitution = restitution;

    //creates a fixture.
    area->CreateFixture(&fixture_def);
}

void PhysicsObject::SetPos(float x, float y) {
    position.x = x;
    position.y = y;
    angle = area->GetAngle();
    area->SetTransform(position, angle);
}

void PhysicsObject::SetAngle(float a) {
    position = area->GetPosition();
    angle = a * (M_PI/180);
    area->SetTransform(position, angle);
}

float PhysicsObject::GetAngle() {
    return area->GetAngle() * (180/M_PI);
}

Pos PhysicsObject::GetPos() {
    position = area->GetPosition();
    return Pos{position.x, position.y};
}
