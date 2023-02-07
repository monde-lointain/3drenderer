#pragma once

#include "../Misc/3d_types.h"
#include <SDL_image.h>

struct Texture
{
public:
	uint32* pixels = nullptr;
	int width;
	int height;

	void destroy();

	SDL_Surface* surface = nullptr;
};