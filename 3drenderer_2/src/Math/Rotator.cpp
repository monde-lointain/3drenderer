#include "Rotator.h"

#include <iomanip>
#include <sstream>

rot3::rot3(float pitch_, float yaw_, float roll_)
	: pitch(pitch_), yaw(yaw_), roll(roll_)
{
}

std::string rot3::to_string(int precision) const
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(precision);
	ss << "pitch=" << pitch << ", yaw=" << yaw << ", roll=" << roll;
	std::string result = ss.str();
	return result;
}
