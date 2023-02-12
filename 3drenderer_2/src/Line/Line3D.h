#pragma once

#include "../Utils/3d_types.h"
#include <glm/vec4.hpp>

struct Line3D
{
	glm::vec4 points[2];
	uint32 color;
	bool should_render = true;
};

