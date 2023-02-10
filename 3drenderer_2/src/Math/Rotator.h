#pragma once

#include <string>

struct rot3
{
	float pitch;
	float yaw;
	float roll;

	std::string to_string(int precision = 2);
};