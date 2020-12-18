#include "box.h"
//Standard
#include <iostream>
using namespace std;

Box::Box(TextureCache * cache, b2World * world, float x, float y, float w, float h) : 
PhysicsObject(b2_dynamicBody, world, (x * pixel_to_meter), (y * pixel_to_meter) * -1) {
    box.SetAsBox((pixel_to_meter * (w/2)), (pixel_to_meter * (h/2)));
    CreateFixture(box, 1.0f, .4, .67);
    surface_area = {x, y, w, h};
    box_texture = cache->LoadTexture("resources/sprites/box.bmp");
    renderer = cache->renderer;
}

void Box::Logic() {}

void Box::Render(SDL_Point camera_position) {
    auto body_pos = GetPos();
    SDL_FPoint center;
    center.x = (surface_area.w * 0.5);
    center.y = (surface_area.h * 0.5);
    surface_area.x = ((meter_to_pixel * body_pos.x) - center.x) + camera_position.x;
    surface_area.y = ((meter_to_pixel * (body_pos.y * -1)) - center.y) + camera_position.y;
    SDL_RenderCopyExF(renderer, box_texture, NULL, &surface_area, GetAngle() * -1, &center, SDL_FLIP_NONE); 
}
