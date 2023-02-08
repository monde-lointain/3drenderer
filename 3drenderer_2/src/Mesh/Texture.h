#pragma once

#include "../Utils/3d_types.h"
#include <memory>

struct Texture
{
	std::unique_ptr<uint32[]> pixels;
	int width;
	int height;
};