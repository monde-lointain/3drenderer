#pragma once

#include "Texture.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <memory>
#include <vector>

struct Triangle;

struct Mesh
{
	Mesh();
	~Mesh() = default;;

	void load_from_obj(const char* filename);
	std::shared_ptr<Texture> load_texture(const char* filename);
	void rotate(float x, float y, float z);
	void mesh_scale(float x, float y, float z);
	int num_triangles();

	std::vector<Triangle> triangles;
	std::vector<std::shared_ptr<Texture>> textures;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec3 translation;
};

Mesh* create_mesh(const char* filename);
