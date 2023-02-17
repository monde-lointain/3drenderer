#include "Rotator.h"

#include <iomanip>
#include <sstream>

rot3::rot3(float pitch, float yaw, float roll)
{
	this->pitch = pitch;
	this->yaw = yaw;
	this->roll = roll;
}

std::string rot3::to_string(int precision)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(precision);
	ss << "pitch=" << pitch << ", yaw=" << yaw << ", roll=" << roll;
	std::string result = ss.str();
	return result;
}
