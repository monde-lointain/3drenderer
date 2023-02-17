#pragma once

#include <string>

struct rot3
{
	rot3(float pitch = 0.0f, float yaw = 0.0f, float roll = 0.0f);
	~rot3() = default;

	float pitch; // rotation about the x axis
	float yaw; // rotation about the y axis
	float roll; // rotation about the z axis

	std::string to_string(int precision = 2);
};