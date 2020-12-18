#pragma once
//Math
#include <math.h>
//Box2D
#include <Box2D/Box2D.h>

constexpr float meter_to_pixel = 60;
constexpr float pixel_to_meter = 1 / meter_to_pixel;

struct Pos {
    float x;
    float y;
};

class PhysicsObject {
    protected:
        b2Vec2 position;
        b2BodyDef body_def;
        b2FixtureDef fixture_def; 

        float angle;   
        virtual ~PhysicsObject();

    public:
        b2Body * area;
        PhysicsObject(b2BodyType, b2World *, float, float);
        void CreateFixture(b2PolygonShape shape, float density = 0, float friction = 0, float restitution = 0);
        void SetPos(float x, float y);
        void SetAngle(float);
        float GetAngle();
        Pos GetPos();
};