#pragma once

#include <string>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

std::string vec3_to_string(const glm::vec3& v, int precision = 2);
std::string vec4_to_string(const glm::vec4& v, int precision = 2);

