#pragma once

#include <memory>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "../Entity/Entity.h"

struct Texture;
struct Triangle;

struct Mesh : Entity
{
	Mesh(
		glm::vec3 scale = glm::vec3(1.0f), 
		rot3 rotation = rot3(0.0f),
		glm::vec3 translation = glm::vec3(0.0f)
	);
	~Mesh() = default;

	void load_from_obj(const char* filename);
	void rotate(rot3 rotation);
	int num_triangles() const;

	std::vector<Triangle> triangles;
	std::vector<std::shared_ptr<Texture>> textures;
};

std::unique_ptr<Mesh> create_mesh(const char* filename);
std::shared_ptr<Texture> load_texture(const char* filename);
