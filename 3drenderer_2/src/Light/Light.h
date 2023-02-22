#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "../Line/Line3D.h"
#include "../Math/Rotator.h"

struct Light
{
	Light(
		glm::vec3 translation_ = glm::vec3(0.0f), 
		rot3 rotation_ = rot3(0.0f, 0.0f, 0.0f),
		float intensity_ = 0.0f
	);

	void update();

	glm::vec3 translation;
	rot3 rotation; // NOTE: With zero rotation applied the light points straignt down
	glm::mat4 transform;
	glm::vec3 direction;
	float intensity;

	Line3D direction_vector; // used for representing the direction of the light in the renderer
};
