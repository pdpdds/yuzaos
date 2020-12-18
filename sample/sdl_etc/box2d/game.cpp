#include "game.h"

int Game::Start(int argc, char** argv) {
    //Initialize SDL2 with appropriate subsystems.
    SDL_Init(SDL_INIT_VIDEO); 

    //process command-line arguments.

    //set the width and height to user set resolution, or set it to constants
    SetResolution();

    //create window
    window = SDL_CreateWindow("Physics Test SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_W, SCREEN_H, SDL_WINDOW_RESIZABLE);
    if (!window){
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Couldn't create window!", SDL_GetError(), NULL);
        return 0;
    }

    //create renderer
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer){
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Couldn't create renderer!", SDL_GetError(), NULL);
        return 0;
    }

    //create event queue
    event = SDL_Event();

    //set defaults for various variables 
    running = true;

    //Box2D stuff
	b2Vec2 gravity(0, -10.0f);

    //initialize various game objects
    cache = new TextureCache(renderer);
    world = new b2World(gravity);
    framebuffer = new Framebuffers(window, renderer);
    framebuffer->CreateFramebuffer(WIDTH, HEIGHT);

    
    box = new Box(cache, world, 240, 300, 50, 50);
    box2 = new Box(cache, world, 260, 100, 50, 50);
    debug_ground = new Tile(cache, world, "resources/sprites/debug-block.bmp", 100, 400, 400, 49);

    //Timer object
    clock = new Clock();

    return 1;
}


void Game::Logic() {
    //no matter how the window is resized, the aspect ratio should remain the same.
    //SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

    //timer update
    clock->Tick();

    //event loop
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
            break;
        }
        if (event.type == SDL_KEYDOWN){
            if (event.key.keysym.scancode == SDL_SCANCODE_RETURN){
                SetFullscreen();
            }
        }
    }

    //keyboard state
    keystate = SDL_GetKeyboardState(NULL);
    
    //game logic and state
    accumulator += clock->dt_sec;
    if (accumulator >= timestep) {
        world->Step(timestep, velocity_iterations, position_iterations);
		accumulator -= timestep; 

        //camera manipulation
        if (keystate[SDL_SCANCODE_SPACE]){camera.speed = 12;}
        else {camera.speed = 6;}

        if (keystate[SDL_SCANCODE_UP])
            camera.position.y += camera.speed;
        if (keystate[SDL_SCANCODE_DOWN])
            camera.position.y -= camera.speed;
        if (keystate[SDL_SCANCODE_LEFT])
            camera.position.x += camera.speed;
        if (keystate[SDL_SCANCODE_RIGHT])
            camera.position.x -= camera.speed;
    }
    
}

void Game::Render() {
    //state based rendering.
    //framebuffer->SetFramebuffer(0);
    SDL_SetRenderDrawColor(renderer, 242, 242, 242, 255);
    SDL_RenderClear(renderer);

    //draw here...
    debug_ground->Render(camera.position);
    box2->Render(camera.position);
    box->Render(camera.position);

    //framebuffer->UnsetFramebuffer();
    //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    //SDL_RenderClear(renderer);
    //framebuffer->RenderBuffer(0, 0, 0, WIDTH, HEIGHT);
    SDL_RenderPresent(renderer);
}

void Game::Loop() {
    while (running){
        Logic();
        Render();
    }
}

void Game::End() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    finished = true;
}

Game::Game() {
    
}

Game::~Game() {
    if (!finished)
        End();
}

//allows for the toggling of fullscreen for the game.
void Game::SetFullscreen(){
    if (!fullscreen){
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        fullscreen = true;
    }
    else if (fullscreen){
        SDL_SetWindowFullscreen(window, 0);
        fullscreen = false;
    }
}

//sets the default resolution when the game starts.
void Game::SetResolution(){
    WIDTH = 640, HEIGHT = 480;
    SCREEN_W = WIDTH, SCREEN_H = HEIGHT;
    #ifdef MOBILE
    SDL_Rect display;
    SDL_GetDisplayBounds(0, &display);
    if (display.h > display.w)
        SCREEN_W = display.h, SCREEN_H = display.w;
    else
        SCREEN_W = display.w, SCREEN_H = display.h;
    #endif
}