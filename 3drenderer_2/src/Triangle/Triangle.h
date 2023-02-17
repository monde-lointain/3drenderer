#pragma once

#include "Vertex.h"
#include "../Mesh/Texture.h"
#include "../Utils/3d_types.h"
#include <glm/vec3.hpp>
#include <memory>

struct Triangle
{
	Vertex vertices[3];
	glm::vec3 face_normal;
	float signed_area; // For backface culling
	uint32 color; // for flat-colored triangles
	float flat_value; // for flat shading
	std::shared_ptr<Texture> texture;

	bool is_front_facing();
};