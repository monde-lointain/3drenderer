#pragma once

#include <glm/vec3.hpp>

struct Camera
{
	Camera();
	~Camera() = default;

	glm::vec3 position;
	float fov;          // in degrees (converted to radians when creating projection matrix)
	float aspect;
	float znear;
	float zfar;
};

