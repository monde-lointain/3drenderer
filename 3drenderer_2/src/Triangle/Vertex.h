#pragma once

#include "../Mesh/tex2.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Vertex
{
	glm::vec4 position; // (x, y, z, w) before perpsective divide, (x, y, z, 1/w) after perpsective divide
	tex2 uv;
	glm::vec3 normal;
	float gouraud;
};

