#pragma once
//SDL2
#include <SDL.h>
//Game Includes
#include "box.h"
#include "tile.h"
#include "texture.h"
#include "clock.h"
//Standard
#include <iostream>
using namespace std;


class Game {
    private:
        SDL_Window * window;       // window handle
        SDL_Renderer * renderer;  // renderer handle
        SDL_Event event;          // event handle

        //resolution
        int WIDTH , HEIGHT, SCREEN_W, SCREEN_H;

        //game variables
        bool finished = false;
        bool fullscreen = false;
        const Uint8 * keystate;
        double accumulator = 0.0;

        //box2d variables
        float timestep = (1.0f/60.0f);
	    int32 velocity_iterations = 8;
	    int32 position_iterations = 3;

        //game objects
        TextureCache * cache;
        Clock * clock;
        b2World * world;
        Framebuffers * framebuffer;
        Box * box;
        Box * box2;
        Tile * debug_ground;

        //enums and structs
        struct Camera {
            SDL_Rect bounds = {};
            SDL_Point position = {0, 0};
            int speed = 3;
        } camera;

        //private functions
        void Logic();
        void Render();
        ~Game();
        
    public:
        bool running;
        Game();
        int Start(int argc, char** argv);
        void Loop();
        void End();

        void SetFullscreen();
        void SetResolution();
};

