#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Rotator.h"

struct Camera;
struct rot3;
struct Viewport;

namespace Math3D
{
	glm::mat4 create_projection_matrix(const Camera& camera);
	glm::mat4 create_world_matrix(
		glm::vec3 scale = glm::vec3(1.0f),
		rot3 rotation = rot3(0.0f, 0.0f, 0.0f),
		glm::vec3 translation = glm::vec3(0.0f)
	
	);
	void transform_point(glm::vec4& point, const glm::mat4& modelview_matrix);
	void rotate_normal(glm::vec3& normal, const glm::mat4& modelview_matrix);
	void project_point(
		glm::vec4& point, 
		const glm::mat4& projection_matrix,
		Viewport* viewport
	);
	void project(glm::vec4& point, const glm::mat4& projection_matrix);
	void to_ndc(glm::vec4& point, float& one_over_w);
	void to_screen_space(glm::vec4& point, Viewport* viewport);
	int orient2d_i(const glm::ivec2& a, const glm::ivec2& b, const glm::ivec2& c);
	float orient2d_f(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);
};
