#pragma once

#include "3d_vector.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct Camera;
struct Mesh;
struct Viewport;

namespace Math3D
{
	glm::mat4 create_projection_matrix(const Camera& camera);
	glm::mat4 create_world_matrix(glm::vec3 scale = glm::vec3(1.0f),
		glm::vec3 rotation = glm::vec3(0.0f),
		glm::vec3 translation = glm::vec3(0.0f));
	void transform_point(glm::vec4& point, const glm::mat4& world_matrix, const glm::mat4& view_matrix);
	void project_point(glm::vec4& point, const glm::mat4& projection_matrix,
		const Viewport& viewport);
	void project(glm::vec4& point, const glm::mat4& projection_matrix);
	void homogenize(glm::vec4& point, const float& one_over_w);
	void to_ndc(glm::vec4& point, const float& one_over_w);
	void to_screen_space(glm::vec4& point, const Viewport& viewport);
	int orient2d_i(const vec2i& a, const vec2i& b, const vec2i& c);
	float orient2d_f(
		const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);
	float get_triangle_area_slow(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);
};	  // namespace Math3D
