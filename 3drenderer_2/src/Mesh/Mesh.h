#pragma once

#include "../Entity/Entity.h"
#include "Texture.h"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <memory>
#include <vector>

struct Triangle;

struct Mesh : Entity
{
	Mesh(glm::vec3 scale = glm::vec3(1.0f), 
		 rot3 rotation = rot3(0.0f),
		 glm::vec3 translation = glm::vec3(0.0f));
	~Mesh() = default;

	void load_from_obj(const char* filename);
	std::shared_ptr<Texture> load_texture(const char* filename);
	void rotate(rot3 rotation);
	int num_triangles();

	std::vector<Triangle> triangles;
	std::vector<std::shared_ptr<Texture>> textures;
};

Mesh* create_mesh(const char* filename);
