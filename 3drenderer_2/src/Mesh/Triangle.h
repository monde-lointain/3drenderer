#pragma once

#include "../Misc/3d_types.h"
#include "../Texture/tex2.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Triangle
{
	glm::vec4 vertices[3];
	tex2 texcoords[3];
	glm::vec4 normals[3];
	glm::vec3 face_normal;
	uint32 color; // for flat-colored triangles

	bool is_facing_camera();
};