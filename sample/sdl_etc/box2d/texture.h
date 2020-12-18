#pragma once

#include <SDL.h>
#include <string>
#include <vector>
#include <map>
using namespace std;

// used for many texture loading operation. this stores the texture data that we load for reuse.
class TextureCache {
    private:
        map<string, SDL_Texture *> textures;
        ~TextureCache();
    public:
        SDL_Renderer * renderer;
        TextureCache(SDL_Renderer * referenced_renderer);
        SDL_Texture * LoadTexture(string filepath);
};

// framebuffers for rendering to the screen. allows for screen effects like shaking, and for seperating render colors from the rendering.
class Framebuffers {
    private:
        SDL_Rect destination = {};
        uint32_t pixel_format;
        vector<SDL_Texture *>buffers;
        SDL_Renderer * renderer;
        ~Framebuffers();
    public:
        Framebuffers(SDL_Window * window, SDL_Renderer * target);
        void CreateFramebuffer(int width, int height);
        int SetFramebuffer(int i);
        void UnsetFramebuffer();
        int RenderBuffer(int i, int x, int y, int w, int h);
};