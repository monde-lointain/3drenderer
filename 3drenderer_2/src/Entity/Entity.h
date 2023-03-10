#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "../Math/Rotator.h"

struct Entity
{
	Entity(
		glm::vec3 scale_ = glm::vec3(1.0f),
		rot3 rotation_ = rot3(0.0f), 
		glm::vec3 translation_ = glm::vec3(0.0f)
	);

	void update();

	glm::vec3 scale;
	rot3 rotation; // in degrees
	glm::vec3 translation;
	glm::mat4 transform;
};

