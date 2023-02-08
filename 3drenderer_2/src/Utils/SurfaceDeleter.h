#pragma once

#include <SDL.h>

/*
 * Wrapper around an SDL_Surface class to add a custom deleter
 */

struct SurfaceDeleter {
    void operator()(SDL_Surface* surface) const {
        SDL_FreeSurface(surface);
    }
};