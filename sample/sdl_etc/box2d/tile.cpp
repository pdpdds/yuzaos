#include "tile.h"
#include <iostream>

Tile::Tile(TextureCache * cache, b2World * world, string filepath,  float x, float y, float w, float h) : 
PhysicsObject(b2_staticBody, world, (x * pixel_to_meter), (y * pixel_to_meter) * -1) {
    tile.SetAsBox(pixel_to_meter * (w/2), pixel_to_meter * (h/2));
    CreateFixture(tile, 0, .4);
    surface_area = {x, y, w, h};
    tile_texture = cache->LoadTexture(filepath);
    renderer  = cache->renderer;
}

void Tile::Render(SDL_Point camera_position) {
    auto body_pos = GetPos();
    SDL_FPoint center;
    center.x = (surface_area.w * 0.5);
    center.y = (surface_area.h * 0.5);
    surface_area.x = ((meter_to_pixel * body_pos.x) - center.x) + camera_position.x;
    surface_area.y = ((meter_to_pixel * (body_pos.y * -1)) - center.y) + camera_position.y;
    SDL_RenderCopyExF(renderer, tile_texture, NULL, &surface_area, GetAngle(), &center, SDL_FLIP_NONE);
}
