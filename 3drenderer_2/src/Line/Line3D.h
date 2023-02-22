#pragma once

#include <array>
#include <glm/vec4.hpp>
#include "../Utils/3d_types.h"

struct Line3D
{
	std::array<glm::vec4, 2> points;
	uint32 color;
	bool should_render = true;
};

