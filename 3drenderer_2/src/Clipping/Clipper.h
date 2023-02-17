#pragma once

#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>

struct Line3D;
struct tex2;
struct Triangle;

enum EClipPlane
{
	NEGATIVE_W_PLANE, // Used to clip vertices with coordinates with w < 0.0
	RIGHT_PLANE,
	LEFT_PLANE,
	TOP_PLANE,
	BOTTOM_PLANE,
	NEAR_PLANE,
	FAR_PLANE,

	NUM_PLANES
};

struct Clipper
{
	void clip_line(Line3D& line);
	// std::vector<Triangle> clip_triangles(std::vector<Triangle>& triangles);
	// std::vector<Triangle> clip_triangles_to_plane(
	//	std::vector<Triangle> triangles, const EClipPlane& plane);
	void clip_triangles(const std::vector<Triangle>& in_tris,
		std::unique_ptr<Triangle[]>& out_tris, int& out_tri_count);
	void clip_triangles_to_plane(Triangle tmp[], const Triangle tris_current_clip[],
		int& num_tris_current_clip, const EClipPlane& plane);
	void clip_triangle_to_plane(const Triangle& triangle,
		const EClipPlane& plane, glm::vec4 out_verts[], tex2 out_uvs[],
		float out_gouraud[], int& num_clip_verts);
	bool is_unmodified(const Triangle& triangle, const EClipPlane& plane);
	bool is_inside_plane(const glm::vec4& vertex, const EClipPlane& plane);
	float compute_intersect_ratio(
		const glm::vec4& curr, const glm::vec4& prev, const EClipPlane& plane);
};