#pragma once

#include <string>

struct rot3
{
	rot3(float pitch_ = 0.0f, float yaw_ = 0.0f, float roll_ = 0.0f);

	float pitch; // rotation about the x axis
	float yaw; // rotation about the y axis
	float roll; // rotation about the z axis

	std::string to_string(int precision = 2) const;
};