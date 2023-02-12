#pragma once

#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>

struct Line3D;
struct tex2;
struct Triangle;

enum EClipPlane
{
	IN_VIEW_PLANE, // Used to clip vertices with negative w coordinates
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
	EClipPlane planes[NUM_PLANES];
	void clip_line(Line3D& line);
	std::vector<Triangle> clip_triangles(std::vector<Triangle>& triangles);
	std::vector<Triangle> clip_triangles_to_plane(
		std::vector<Triangle> triangles, const EClipPlane& plane);
	void clip_triangle_to_plane(const Triangle& triangle, const EClipPlane& plane,
		std::vector<glm::vec4>& out_verts, std::vector<tex2>& out_uvs);
	bool is_inside_plane(const glm::vec4& vertex, const EClipPlane& plane);
	float compute_intersect_ratio(
		const glm::vec4& curr, const glm::vec4& prev, const EClipPlane& plane);
};