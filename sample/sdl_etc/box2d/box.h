#pragma once
//SDL2
#include <SDL.h>
//Game
#include "object.h"
#include "texture.h"

class Box : private PhysicsObject {
    private:
        b2PolygonShape box;
        SDL_Renderer * renderer;
        SDL_Texture * box_texture;
        SDL_FRect surface_area;
        PhysicsObject * object;

    public:
        Box(TextureCache * cache, b2World * world, float x, float y, float w, float h);
        void Logic();
        void Render(SDL_Point camera_position = {0,0});
};