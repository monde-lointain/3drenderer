#pragma once

#include "../Math/Rotator.h"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct Entity
{
	Entity(glm::vec3 scale = glm::vec3(1.0f),
		   rot3 rotation = rot3(0.0f),
		   glm::vec3 translation = glm::vec3(0.0f));
	~Entity() = default;

	void update();

	glm::vec3 scale;
	rot3 rotation; // in degrees
	glm::vec3 translation;
	glm::mat4 transform;
};

