#include "string_ops.h"

#include <iomanip>
#include <sstream>

std::string vec3_to_string(const glm::vec3& v, int precision)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(precision);
	ss << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	std::string result = ss.str();
	return result;
}

std::string vec4_to_string(const glm::vec4& v, int precision)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(precision);
	ss << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
	std::string result = ss.str();
	return result;
}
