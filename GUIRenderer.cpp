#include "GUIRenderer.h"

GUIRenderer::GUIRenderer(int screenWidth, int screenHeight) {
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "SDL could not be initialized!" << endl << "SDL_Error: " << SDL_GetError() << endl;
        guiInitialized = false;
        return;
    }

#if defined linux && SDL_VERSION_ATLEAST(2, 0, 8)
    // Disable compositor bypass
    if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0")) {
        cout << "SDL can not disable compositor bypass!" << endl;
        guiInitialized = false;
        return;
    }
#endif

    // Create window
    window = SDL_CreateWindow("Parallel Kmeans project",  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              screenWidth, screenHeight,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        cout << "Window could not be created!" << endl << "SDL_Error: " << SDL_GetError() << endl;
        guiInitialized = false;
        return;
    } else {
        // Create renderer
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            cout << "Renderer could not be created!" << endl
                 << "SDL_Error: " << SDL_GetError() << endl;
            SDL_DestroyWindow(window);
            guiInitialized = false;
            return;
        }
    }
    guiInitialized = true;
}

int GUIRenderer::displayRect(int screenWidth, int screenHeight) {
    // Declare rect of square
    SDL_Rect squareRect;

    // Square dimensions: Half of the min(SCREEN_WIDTH, SCREEN_HEIGHT)
    squareRect.w = std::min(screenWidth, screenHeight) / 2;
    squareRect.w = 10;
    squareRect.h = std::min(screenWidth, screenHeight) / 2;
    squareRect.h = 10;

    // Square position: In the middle of the screen
    squareRect.x = screenWidth / 2 - squareRect.w / 2;
    squareRect.y = screenHeight / 2 - squareRect.h / 2;

    // Initialize renderer color white for the background
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    // Clear screen
    SDL_RenderClear(renderer);
    // Set renderer color red to draw the square
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
    // Draw filled square
    SDL_RenderFillRect(renderer, &squareRect);
    // Update screen
    SDL_RenderPresent(renderer);
    return 0;
}

// Locks a surface
void GUIRenderer::lockSurface(SDL_Surface *srf) {
    if (SDL_MUSTLOCK(srf))
        if (SDL_LockSurface(srf) < 0)
            return;
}

// Unlocks a surface
void GUIRenderer::unlockSurface(SDL_Surface *srf) {
    if (SDL_MUSTLOCK(srf))
        SDL_UnlockSurface(srf);
}

void GUIRenderer::setPixelSurface(SDL_Surface* srf, int x, int y, SDL_Color color) {
    if (x < 0 || y < 0 || x >= srf->w || y >= srf->h) return;
    SDL_PixelFormat *fmt = srf->format;
    Uint32 colorSDL = SDL_MapRGBA(fmt, color.r, color.g, color.b, color.a);
    Uint32* bufp = (Uint32*)srf->pixels + y * srf->pitch / 4 + x;
    *bufp = colorSDL;
}

// Update the screen.  Has to be called to view new pixels.
void GUIRenderer::redrawTexture(SDL_Texture *tex, SDL_Surface* srf, bool freeSurface) {
    SDL_UpdateTexture(tex, NULL, srf->pixels, srf->pitch);
    if(freeSurface) SDL_FreeSurface(srf);
    //    SDL_RenderClear(renderer);
    SDL_Rect dest_rect = {0, 0, srf->w, srf->h};
    SDL_RenderCopy(renderer, tex, nullptr, &dest_rect);
    SDL_RenderPresent(renderer);
}

// Clears the screen with color parameter
void GUIRenderer::clearScreen(const SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void GUIRenderer::quit() {
    // Destroy renderer
    SDL_DestroyRenderer(renderer);
    // Destroy window
    SDL_DestroyWindow(window);
    // Quit SDL
    SDL_Quit();
}

SDL_Texture* GUIRenderer::renderSurface(SDL_Surface *sdlSurface, bool freeSurface, int width_size, int height_size) {
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, sdlSurface);
    if(freeSurface) SDL_FreeSurface(sdlSurface);
    if(width_size<0 || height_size<0){
        width_size = sdlSurface->w;
        height_size = sdlSurface->h;
    }
    // scale and print
    int* max_width = new int;
    int* max_height = new int;
    SDL_GetWindowSize(window, max_width, max_height);
    if(width_size>*max_width) width_size=*max_width;
    if(height_size>*max_height) height_size=*max_height;
    SDL_Rect dest_rect = {0, 0, width_size, height_size};
    SDL_SetWindowSize(window, width_size, height_size);
    SDL_RenderCopy(renderer, texture, nullptr, &dest_rect);
    SDL_RenderPresent(renderer);
    return texture;
}
