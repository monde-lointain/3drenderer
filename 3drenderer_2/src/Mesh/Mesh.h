#pragma once

#include "Texture.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vector>

struct Triangle;

struct Mesh
{
	Mesh();
	~Mesh();

	void load_from_obj(const char* filename);
	void load_texture(const char* filename);
	void set_texture_on_triangles(Texture* texture_to_set);
	void rotate(float x, float y, float z);
	void mesh_scale(float x, float y, float z);
	int num_triangles();

	std::vector<Triangle> triangles;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec3 translation;
	Texture* texture;
};

Mesh* create_mesh(const char* filename);
