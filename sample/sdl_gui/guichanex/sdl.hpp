/*
 * Code that sets up an SDL application with Guichan using the 
 * Guichan SDL back end.
 */

#include <guichan.hpp>
#include <guichan/sdl.hpp>

namespace sdl
{
    bool running = true;
   

    // All back ends contain objects to make Guichan work on a
    // specific target - in this case SDL - and they are a Graphics
    // object to make Guichan able to draw itself using SDL, an
    // input objec to make Guichan able to get user input using SDL
    // and an ImageLoader object to make Guichan able to load images
    // using SDL.
    SDL_Renderer* pRenderer;
    gcn::SDLGraphics* graphics;
    gcn::SDLInput* input;
    gcn::SDLImageLoader* imageLoader;
    SDL_Surface* screen;
    /**
     * Initialises the SDL application. This function creates the global
     * Gui object that can be populated by various examples.
     */
    void init()
    {
        // We simply initialise SDL as we would do with any SDL application.
        SDL_Init(SDL_INIT_VIDEO);
        /*screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
        // We want unicode for the SDLInput object to function properly.
        SDL_EnableUNICODE(1);
        // We also want to enable key repeat.
        SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
        */
        // Now it's time to initialise the Guichan SDL back end.
        SDL_Window* pWindow;
        
        if (SDL_CreateWindowAndRenderer(640, 480, 0, &pWindow, &pRenderer) < 0)

        {
            printf("SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
            return;
        }

        imageLoader = new gcn::SDLImageLoader(pRenderer);
        // The ImageLoader Guichan should use needs to be passed to the Image object
        // using a static function.
        gcn::Image::setImageLoader(imageLoader);
        graphics = new gcn::SDLGraphics();
        // The Graphics object needs a target to draw to, in this case it's the
        // screen surface, but any surface will do, it doesn't have to be the screen.
        graphics->setTarget(SDL_GetWindowSurface(pWindow));
        input = new gcn::SDLInput();

        // Now we create the Gui object to be used with this SDL application.
        globals::gui = new gcn::Gui();
        // The Gui object needs a Graphics to be able to draw itself and an Input
        // object to be able to check for user input. In this case we provide the
        // Gui object with SDL implementations of these objects hence making Guichan
        // able to utilise SDL.
        globals::gui->setGraphics(graphics);
        globals::gui->setInput(input);
    }

    /**
     * Halts the SDL application.
     */
    void halt()
    {
        delete globals::gui;

        delete imageLoader;
        delete input;
        delete graphics;

        SDL_Quit();
    }

    /**
     * Runs the SDL application.
     */
    void run()
    {
        // The main loop
        while(running)
        {
            // Check user input
            SDL_Event event;
            while(SDL_PollEvent(&event))
            {
                if (event.type == SDL_KEYDOWN)
                {
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        running = false;
                    }
                   
                }
                else if(event.type == SDL_QUIT)
                {
                    running = false;
                }
                
                // After we have manually checked user input with SDL for
                // any attempt by the user to halt the application we feed
                // the input to Guichan by pushing the input to the Input
                // object.
                input->pushInput(event);
            }
            // Now we let the Gui object perform its logic.
            globals::gui->logic();
            // Now we let the Gui object draw itself.
            globals::gui->draw();
            // Finally we update the screen.
            
            SDL_RenderPresent(pRenderer);
        }
    }
}
