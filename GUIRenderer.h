#include <iostream>
#include <sstream>
#include "SDL.h"

#ifndef PARALLUSTERK_GUIRENDERER_H
#define PARALLUSTERK_GUIRENDERER_H

using namespace std;

struct GUIRenderer {
    GUIRenderer(int screenWidth, int screenHeight);
    int displayRect(int screenWidth, int screenHeight);
    void lockSurface(SDL_Surface *srf);
    void unlockSurface(SDL_Surface *srf);
    void setPixelSurface(SDL_Surface *srf, int x, int y, SDL_Color color);
    void quit();
    void redrawTexture(SDL_Texture *tex, SDL_Surface* srf, bool freeSurface = false);
    void clearScreen(SDL_Color color);
    SDL_Texture* renderSurface(SDL_Surface *sdlSurface, bool freeSurface = false, int width_size = -1, int height_size = -1);

    SDL_Window *window;
    SDL_Renderer *renderer;
    int screenWidth, screenHeight;
    bool guiInitialized;
};


#endif //PARALLUSTERK_GUIRENDERER_H
