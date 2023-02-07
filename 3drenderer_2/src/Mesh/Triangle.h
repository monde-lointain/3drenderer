#pragma once

#include "tex2.h"
#include "Texture.h"
#include "../Misc/3d_types.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Triangle
{
	glm::vec4 vertices[3];
	tex2 texcoords[3];
	glm::vec4 normals[3];
	glm::vec3 face_normal;
	float signed_area; // For backface culling
	uint32 color; // for flat-colored triangles
	float inv_w[3]; // 1/w
	Texture* texture;

	bool is_front_facing();
};