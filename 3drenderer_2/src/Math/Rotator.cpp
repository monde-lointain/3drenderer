#include "Rotator.h"

#include <iomanip>
#include <sstream>

std::string rot3::to_string(int precision)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(precision);
	ss << "pitch=" << pitch << ", yaw=" << yaw << ", roll=" << roll;
	std::string result = ss.str();
	return result;
}
