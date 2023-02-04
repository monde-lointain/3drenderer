#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>

struct Triangle;

struct Mesh
{
	Mesh() = default;
	~Mesh() = default;

	void load_from_obj(const char* filename);
	void rotate(float x, float y, float z);
	void mesh_scale(float x, float y, float z);
	int num_triangles();

	std::vector<Triangle> triangles;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec3 translation;
};

Mesh* create_mesh(const char* filename);
