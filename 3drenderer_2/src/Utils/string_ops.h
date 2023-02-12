#pragma once

#include "3d_types.h"
#include "../Mesh/tex2.h"
#include <string>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

std::string vec3_to_string(const glm::vec3& v, int precision = 2);
std::string vec4_to_string(const glm::vec4& v, int precision = 2);
std::string tex2_to_string(const tex2& t, int precision = 2);
std::string to_string_with_commas(uint64 n);

