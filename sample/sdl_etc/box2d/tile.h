#pragma once
//SDL2
#include <SDL.h>
//Game
#include "object.h"
#include "texture.h"

class Tile : private PhysicsObject {
    private:
        b2PolygonShape tile;
        SDL_Renderer * renderer;
        SDL_Texture * tile_texture;
        SDL_FRect surface_area;

    public:
        Tile(TextureCache *, b2World *,  string, float, float, float, float);
        void Render(SDL_Point camera_position = {0,0});
};