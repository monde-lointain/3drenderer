#include "Clipper.h"

#include "../Line/Line3D.h"
#include "../Logger/Logger.h"
#include "../Mesh/tex2.h"
#include "../Mesh/Triangle.h"
#include "../Utils/math_helpers.h"
#include "../Utils/string_ops.h"
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/vec4.hpp>
#include <tracy/tracy/Tracy.hpp>

void Clipper::clip_line(Line3D& line)
{
	glm::vec4 curr = line.points[0];
	glm::vec4 prev = line.points[1];

	bool curr_outside = false;
	bool prev_outside = false;

	for (int i = 0; i < NUM_PLANES; i++)
	{
		EClipPlane plane = (EClipPlane)i;
		curr_outside = !is_inside_plane(curr, plane);
		prev_outside = !is_inside_plane(prev, plane);

		// If the line is outside the view frustum entirely, do not render it
		if (prev_outside && curr_outside)
		{
			line.points[0] = glm::vec4(0.0f);
			line.points[1] = glm::vec4(0.0f);
			line.should_render = false;
			return;
		}
		// Outside moving in
		if (prev_outside)
		{
			float t = compute_intersect_ratio(curr, prev, plane);
			prev = glm::lerp(prev, curr, t);
		}
		// Inside moving out
		else if (curr_outside)
		{
			float t = compute_intersect_ratio(prev, curr, plane);
			curr = glm::lerp(curr, prev, t);
		}
	}

	line.should_render = true;
	line.points[0] = curr;
	line.points[1] = prev;
}

//std::vector<Triangle> Clipper::clip_triangles(std::vector<Triangle>& triangles)
//{
//	// The collection of triangles created from all triangles after they have
//	// been clipped
//	std::vector<Triangle> out_triangles;
//	// Reserve memory for the maximum number of triangles that could be generated
//	out_triangles.reserve(6 * triangles.size());
//	// Iterate over each of the six frustum planes
//	for (Triangle& triangle : triangles) {
//		// Collection of triangles for a single triangle after it has been clipped
//		std::vector<Triangle> clip_result;
//		clip_result.push_back(triangle);
//
//		clip_result = clip_triangles_to_plane(clip_result, IN_VIEW_PLANE);
//		clip_result = clip_triangles_to_plane(clip_result, LEFT_PLANE);
//		clip_result = clip_triangles_to_plane(clip_result, RIGHT_PLANE);
//		clip_result = clip_triangles_to_plane(clip_result, BOTTOM_PLANE);
//		clip_result = clip_triangles_to_plane(clip_result, TOP_PLANE);
//		clip_result = clip_triangles_to_plane(clip_result, NEAR_PLANE);
//		clip_result = clip_triangles_to_plane(clip_result, FAR_PLANE);
//
//		// Allocate memory into the array of clipped triangles for the triangles created from this clip
//		out_triangles.reserve(out_triangles.size() + clip_result.size());
//		// Move the final results of the clipping into the full array of clipped triangles
//		std::move(clip_result.begin(), clip_result.end(), std::back_inserter(out_triangles));
//	}
//	Logger::print(LOG_CATEGORY_CLIPPING, "Out triangles: " + std::to_string(out_triangles.size()));
//	return out_triangles;
//}
//
//std::vector<Triangle> Clipper::clip_triangles_to_plane(
//	std::vector<Triangle> triangles, const EClipPlane& plane)
//{
//	std::vector<Triangle> clip_result;
//	for (Triangle triangle : triangles)
//	{
//		// Get the clip distance for the current plane for each of the
//		// vertices on the triangle
//		bool v0_inside = is_inside_plane(triangle.vertices[0], plane);
//		bool v1_inside = is_inside_plane(triangle.vertices[1], plane);
//		bool v2_inside = is_inside_plane(triangle.vertices[2], plane);
//
//		// If all vertices are inside the plane, the triangle does not need
//		// to be clipped and can be passed to the next clip stage unmodified
//		if (v0_inside && v1_inside && v2_inside)
//		{
//			clip_result.push_back(triangle);
//			continue;
//		}
//
//		std::vector<glm::vec4> vertices;
//		std::vector<tex2> texcoords;
//		clip_triangle_to_plane(triangle, plane, vertices, texcoords);
//
//		//assert(vertices.size() == 3 || vertices.size() == 4);
//
//		// Clipped polygon is a triangle. No new polygons need to be created
//		if (vertices.size() == 3)
//		{
//			Triangle a = triangle;
//			a.vertices[0] = vertices[0];
//			a.vertices[1] = vertices[1];
//			a.vertices[2] = vertices[2];
//			a.texcoords[0] = texcoords[0];
//			a.texcoords[1] = texcoords[1];
//			a.texcoords[2] = texcoords[2];
//
//			clip_result.push_back(a);
//		}
//		// Clipped polygon is a quad. Split into two triangles
//		else if (vertices.size() == 4)
//		{
//			Triangle a = triangle;
//			a.vertices[0] = vertices[0];
//			a.vertices[1] = vertices[1];
//			a.vertices[2] = vertices[2];
//			a.texcoords[0] = texcoords[0];
//			a.texcoords[1] = texcoords[1];
//			a.texcoords[2] = texcoords[2];
//
//			Triangle b = triangle;
//			b.vertices[0] = vertices[2];
//			b.vertices[1] = vertices[3];
//			b.vertices[2] = vertices[0];
//			b.texcoords[0] = texcoords[2];
//			b.texcoords[1] = texcoords[3];
//			b.texcoords[2] = texcoords[0];
//
//			clip_result.push_back(a);
//			clip_result.push_back(b);
//		}
//	}
//	return clip_result;
//}

void Clipper::clip_triangle_to_plane(const Triangle& triangle,
	const EClipPlane& plane, glm::vec4 out_verts[], tex2 out_uvs[],
	int& num_clip_verts)
{
	ZoneScoped;

	// Intersection point between the two vertices and the plane
	glm::vec4 intersection{};
	// Interpolated UV coordinate
	tex2 interp_uv{};
	// Percentage factor for interpolating the intersection point and UVs
	float t;

	constexpr int num_verts = 3;
	// Extract the vertices from the triangle
	glm::vec4 vertices[num_verts];
	vertices[0] = triangle.vertices[0];
	vertices[1] = triangle.vertices[1];
	vertices[2] = triangle.vertices[2];
	// Extract the UVs from the triangle
	tex2 texcoords[num_verts];
	texcoords[0] = triangle.texcoords[0];
	texcoords[1] = triangle.texcoords[1];
	texcoords[2] = triangle.texcoords[2];

	// Start with the last vertex as the previous vertex
	glm::vec4 prev_vert = vertices[num_verts - 1];
	glm::vec4 curr_vert;

	tex2 prev_texcoord = texcoords[num_verts - 1];
	tex2 curr_texcoord;

	bool prev_inside = is_inside_plane(prev_vert, plane);
	bool curr_inside;

	// TODO: Handle degenerate triangle cases

	// Loop through all the vertices of the input triangle
	for (int i = 0; i < num_verts; i++)
	{
		// Current vertex
		curr_vert = vertices[i];
		curr_texcoord = texcoords[i];
		curr_inside = is_inside_plane(curr_vert, plane);

		// If we're moving out or moving in
		if (prev_inside != curr_inside)
		{
			// Calculate the intersection point
			t = compute_intersect_ratio(curr_vert, prev_vert, plane);
			intersection = glm::lerp(prev_vert, curr_vert, t);
			// Calculate the interpolated UVs
			interp_uv = tex2_lerp(prev_texcoord, curr_texcoord, t);
			// Add the resulting intersection vertex to the output vertices
			out_verts[num_clip_verts] = intersection;
			out_uvs[num_clip_verts] = interp_uv;
			num_clip_verts++;
		}

		// If the current vertex is inside the clip plane
		if (curr_inside)
		{
			out_verts[num_clip_verts] = curr_vert;
			out_uvs[num_clip_verts] = curr_texcoord;
			num_clip_verts++;
		}

		// Set the current vertex/dp as the previous vertex/dp for the next
		// iteration
		prev_vert = curr_vert;
		prev_texcoord = curr_texcoord;
		prev_inside = curr_inside;
	}
}

bool Clipper::is_inside_plane(const glm::vec4& vertex, const EClipPlane& plane)
{
	switch (plane)
	{
		case NEGATIVE_W_PLANE:
			return vertex.w >= EPSILON;
		case RIGHT_PLANE:
			return vertex.x <= +vertex.w;
		case LEFT_PLANE:
			return vertex.x >= -vertex.w;
		case TOP_PLANE:
			return vertex.y <= +vertex.w;
		case BOTTOM_PLANE:
			return vertex.y >= -vertex.w;
		// These two are correct. Projection matrix flips positive z from going
		// out of the screen to going in to it
		case NEAR_PLANE:
			return vertex.z >= -vertex.w;
		case FAR_PLANE:
			return vertex.z <= +vertex.w;
		default:
			assert(0);
			return 0;
	}
}

float Clipper::compute_intersect_ratio(
	const glm::vec4& curr, const glm::vec4& prev, const EClipPlane& plane)
{
	switch (plane)
	{
		case NEGATIVE_W_PLANE:
			return (prev.w - EPSILON) / (prev.w - curr.w);
		case RIGHT_PLANE:
			return (prev.w - prev.x) / ((prev.w - prev.x) - (curr.w - curr.x));
		case LEFT_PLANE:
			return (prev.w + prev.x) / ((prev.w + prev.x) - (curr.w + curr.x));
		case TOP_PLANE:
			return (prev.w - prev.y) / ((prev.w - prev.y) - (curr.w - curr.y));
		case BOTTOM_PLANE:
			return (prev.w + prev.y) / ((prev.w + prev.y) - (curr.w + curr.y));
		case NEAR_PLANE:
			return (prev.w + prev.z) / ((prev.w + prev.z) - (curr.w + curr.z));
		case FAR_PLANE:
			return (prev.w - prev.z) / ((prev.w - prev.z) - (curr.w - curr.z));
		default:
			assert(0);
			return 0;
	}
}

//void clip_tris()
//{
//	Triangle in_tris[MAX_TRIANGLES];
//	int num_tris = 8770;
//	Triangle out_tris[MAX_TRIANGLES];
//	int out_tri_count = 0;
//	Triangle tris_current_clip[10];
//	int num_tris_current_clip = 0;
//	Triangle tmp[10];
//
//	for (triangle : in_tris)
//	{
//		tmp[0] = triangle;
//		for (plane : planes)
//		{
//			clip_triangles_to_plane(tmp, tris_current_clip, num_tris_current_clip, plane);
//			for (int i = 0; i < num_tris_current_clip; i++)
//			{
//				tris_current_clip[i] = tmp[i];
//			}
//		}
//		for (int i = 0; i < num_tris_current_clip; i++)
//		{
//			out_tris[out_tri_count + i] = tris_current_clip[i];
//			out_tri_count++;
//		}
//	}
//}
//
//void clip_triangles_to_plane(Triangle tmp[], Triangle tris_current_clip[],
//	int num_tris_current_clip, Plane plane)
//{
//	int num_new_tris = 0;
//	int i = 0;
//	while (i < num_tris_current_clip)
//	{
//		Triangle triangle = tris_current_clip[i];
//		if (!modified)
//		{
//			tmp[num_new_tris] = tris_current_clip[i];
//			num_new_tris++;
//		}
//		if (modified)
//		{
//			create_new_triangle(a);
//			create_new_triangle(b);
//			tmp[num_new_tris] = a;
//			num_new_tris++;
//			tmp[num_new_tris] = b;
//			num_new_tris++;
//		}
//		i++;
//	}
//}

void Clipper::clip_triangles(const std::vector<Triangle>& in_tris,
	std::unique_ptr<Triangle[]>& out_tris, int& out_tri_count)
{
	ZoneScoped;

	// Total number of triangles in the scene (TODO: should maybe be an input variable)
	int num_tris = (int)in_tris.size();
	// Number of triangles that are left after clipping
	out_tri_count = 0;
	// The triangles created for single triangle after clipping against all clip planes
	Triangle tris_current_clip[20];
	int num_tris_current_clip = 0;
	// Temporary container to store the triangles clipped from a single plane in
	Triangle tmp[20];

	// Loop over each triangle
	for (const Triangle& triangle : in_tris)
	{
		// Insert the current triangle into the array of triangles for the
		// current clip test
		tris_current_clip[0] = triangle;
		num_tris_current_clip++;
		// Loop over each clip plane
		for (int i = 0; i < NUM_PLANES; i++)
		{
			const EClipPlane& plane = (EClipPlane)i;
			// Clip all current triangles for the current clip against the current clip plane
			clip_triangles_to_plane(tmp, tris_current_clip, num_tris_current_clip, plane);
			// Place all triangles along with the new ones back in the clip container
			for (int j = 0; j < num_tris_current_clip; j++)
			{
				tris_current_clip[j] = tmp[j];
			}
		}
		// After clipping against all planes, add the result triangles to the final array of triangles
		for (int i = 0; i < num_tris_current_clip; i++)
		{
			out_tris[(size_t)out_tri_count + (size_t)i] = tris_current_clip[i];
		}
		// Increment the final triangle amount by the number of triangles
		// created from the current clip
		out_tri_count += num_tris_current_clip;
		// Reset the clip counter
		num_tris_current_clip = 0;
	}
	Logger::print(LOG_CATEGORY_CLIPPING, "Out triangles: " + std::to_string(out_tri_count));
}

void Clipper::clip_triangles_to_plane(Triangle tmp[],
	const Triangle tris_current_clip[], int& num_tris_current_clip,
	const EClipPlane& plane)
{
	ZoneScoped;

	// Counts the number of triangles that will exist after the current clip
	int num_new_tris = 0;
	// Iterator for looping over the current number of triangles for the clip
	int i = 0;
	while (i < num_tris_current_clip)
	{
		const Triangle& triangle = tris_current_clip[i];

		// If all vertices of the triangle are inside the clip plane, it can
		// simply be added back into the array unmodified
		if (is_unmodified(triangle, plane))
		{
			tmp[num_new_tris] = tris_current_clip[i];
			num_new_tris++;
		}
		else
		{
			// Clip the triangle against the current plane
			glm::vec4 vertices[4];
			tex2 texcoords[4];
			int num_clip_verts = 0;
			clip_triangle_to_plane(triangle, plane, vertices, texcoords, num_clip_verts);

			// Clipped polygon is a triangle. No new polygons need to be created
			if (num_clip_verts == 3)
			{
				Triangle a = triangle;
				a.vertices[0] = vertices[0];
				a.vertices[1] = vertices[1];
				a.vertices[2] = vertices[2];
				a.texcoords[0] = texcoords[0];
				a.texcoords[1] = texcoords[1];
				a.texcoords[2] = texcoords[2];

				tmp[num_new_tris] = a;
				num_new_tris++;
			}
			// Clipped polygon is a quad. Split into two triangles
			else if (num_clip_verts == 4)
			{
				Triangle a = triangle;
				a.vertices[0] = vertices[0];
				a.vertices[1] = vertices[1];
				a.vertices[2] = vertices[2];
				a.texcoords[0] = texcoords[0];
				a.texcoords[1] = texcoords[1];
				a.texcoords[2] = texcoords[2];

				Triangle b = triangle;
				b.vertices[0] = vertices[2];
				b.vertices[1] = vertices[3];
				b.vertices[2] = vertices[0];
				b.texcoords[0] = texcoords[2];
				b.texcoords[1] = texcoords[3];
				b.texcoords[2] = texcoords[0];

				tmp[num_new_tris] = a;
				num_new_tris++;
				tmp[num_new_tris] = b;
				num_new_tris++;
			}
		}
		// Increment the iterator for each triangle that currently exists
		i++;
	}
	// Set the current number of triangles for this clip
	num_tris_current_clip = num_new_tris;
}

bool Clipper::is_unmodified(const Triangle& triangle, const EClipPlane& plane)
{
	// Get the clip distance for the current plane for each of the vertices on
	// the triangle
	bool v0_inside = is_inside_plane(triangle.vertices[0], plane);
	bool v1_inside = is_inside_plane(triangle.vertices[1], plane);
	bool v2_inside = is_inside_plane(triangle.vertices[2], plane);

	// If all vertices are inside the plane, the triangle does not need
	// to be clipped and can be passed to the next clip stage unmodified
	if (v0_inside && v1_inside && v2_inside)
	{
		return true;
	}
	else
	{
		return false;
	}
}

