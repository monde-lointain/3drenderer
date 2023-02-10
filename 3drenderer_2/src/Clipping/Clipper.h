#pragma once

#include <glm/mat4x4.hpp>

struct Triangle;

enum EClipPlane
{
	LEFT_PLANE,
	RIGHT_PLANE,
	BOTTOM_PLANE,
	TOP_PLANE,
	NEAR_PLANE,
	FAR_PLANE,

	NUM_PLANES
};

struct Clipper
{
	glm::vec4 planes[NUM_PLANES];

	void create_frustum_planes(const glm::mat4& projection_matrix, const glm::mat4& view_matrix);
	bool cull_against_frustum_planes(const Triangle& triangle);
	bool cull_ndc(const Triangle& triangle);
	bool point_outside_frustums(const glm::vec4& point);
};
