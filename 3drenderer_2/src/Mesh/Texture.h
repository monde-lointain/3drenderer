#pragma once

#include <memory>
#include "../Utils/3d_types.h"

struct Texture
{
	std::unique_ptr<uint32[]> pixels;
	int width;
	int height;
};