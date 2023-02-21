#include "Clipper.h"

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/vec4.hpp>
#include <tracy/tracy/Tracy.hpp>

#include "../Line/Line3D.h"
#include "../Logger/Logger.h"
#include "../Mesh/tex2.h"
#include "../Triangle/Triangle.h"
#include "../Utils/Constants.h"
#include "../Utils/string_ops.h"

void clip_line(Line3D& line)
{
	glm::vec4 curr = line.points[0];
	glm::vec4 prev = line.points[1];

	bool curr_outside = false;
	bool prev_outside = false;

	float t;
	EClipPlane plane;
	for (int i = 0; i < NUM_PLANES; i++)
	{
		plane = (EClipPlane)i;
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
			t = compute_intersect_ratio(prev, curr, plane);
			prev = glm::lerp(prev, curr, t);
		}
		// Inside moving out
		else if (curr_outside)
		{
			t = compute_intersect_ratio(curr, prev, plane);
			curr = glm::lerp(curr, prev, t);
		}
	}

	// Set the new points on the line once we're done clipping it
	line.should_render = true;
	line.points[0] = curr;
	line.points[1] = prev;
}

//std::vector<Triangle> clip_triangles(std::vector<Triangle>& triangles)
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
//std::vector<Triangle> clip_triangles_to_plane(
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

constexpr int MAX_TRIANGLES_PER_CLIP = 50;

void clip_triangles(
	const std::vector<Triangle>& in_tris,
	Triangle* out_tris,
	int& out_tri_count
)
{
	ZoneScoped; // for tracy

	// Number of triangles that are left after clipping
	out_tri_count = 0;
	// The triangles created for single triangle after clipping against all clip
	// planes
	std::array<Triangle, MAX_TRIANGLES_PER_CLIP> tris_current_clip;
	//Triangle tris_current_clip[MAX_TRIANGLES_PER_CLIP];
	int num_tris_current_clip = 0;
	// Temporary container to store the triangles clipped from a single plane in
	std::array<Triangle, MAX_TRIANGLES_PER_CLIP> tmp;
	//Triangle tmp[MAX_TRIANGLES_PER_CLIP];

	int i, j;
	// Loop over each triangle
	for (const Triangle& triangle : in_tris)
	{
		// Insert the current triangle into the array of triangles for the
		// current clip test
		tris_current_clip[0] = triangle;
		num_tris_current_clip++;
		// Loop over each clip plane
		for (i = 0; i < NUM_PLANES; i++)
		{
			const EClipPlane& plane = (EClipPlane)i;
			// Clip all current triangles for the current clip against the
			// current clip plane
			clip_triangles_to_plane(
				tmp.data(), 
				tris_current_clip.data(), 
				num_tris_current_clip, plane
			);
			// Place all triangles along with the new ones back in the clip
			// container
			for (j = 0; j < num_tris_current_clip; j++)
			{
				tris_current_clip[j] = tmp[j];
			}
		}
		// After clipping against all planes, add the result triangles to the
		// final array of triangles
		for (i = 0; i < num_tris_current_clip; i++)
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

void clip_triangles_to_plane(
	Triangle* tmp,
	const Triangle* tris_current_clip, 
	int& num_tris_current_clip,
	const EClipPlane plane
)
{
	ZoneScoped; // for tracy

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
			std::array<glm::vec4, 4> vertices;
			std::array<tex2, 4> texcoords;
			std::array<float, 4> gouraud;
			int num_clip_verts = 0;
			clip_triangle_to_plane(
				triangle, 
				plane, 
				vertices.data(), 
				texcoords.data(), 
				gouraud.data(), 
				num_clip_verts
			);

			// Clipped polygon is a triangle. No new polygons need to be created
			if (num_clip_verts == 3)
			{
				Triangle a = triangle;
				a.vertices[0].position = vertices[0];
				a.vertices[1].position = vertices[1];
				a.vertices[2].position = vertices[2];
				a.vertices[0].uv = texcoords[0];
				a.vertices[1].uv = texcoords[1];
				a.vertices[2].uv = texcoords[2];
				a.vertices[0].gouraud = gouraud[0];
				a.vertices[1].gouraud = gouraud[1];
				a.vertices[2].gouraud = gouraud[2];

				tmp[num_new_tris] = a;
				num_new_tris++;
			}
			// Clipped polygon is a quad. Split into two triangles
			else if (num_clip_verts == 4)
			{
				Triangle a = triangle;
				a.vertices[0].position = vertices[0];
				a.vertices[1].position = vertices[1];
				a.vertices[2].position = vertices[2];
				a.vertices[0].uv = texcoords[0];
				a.vertices[1].uv = texcoords[1];
				a.vertices[2].uv = texcoords[2];
				a.vertices[0].gouraud = gouraud[0];
				a.vertices[1].gouraud = gouraud[1];
				a.vertices[2].gouraud = gouraud[2];

				Triangle b = triangle;
				b.vertices[0].position = vertices[2];
				b.vertices[1].position = vertices[3];
				b.vertices[2].position = vertices[0];
				b.vertices[0].uv = texcoords[2];
				b.vertices[1].uv = texcoords[3];
				b.vertices[2].uv = texcoords[0];
				b.vertices[0].gouraud = gouraud[2];
				b.vertices[1].gouraud = gouraud[3];
				b.vertices[2].gouraud = gouraud[0];

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

void clip_triangle_to_plane(
	const Triangle& triangle,
	EClipPlane plane,
	glm::vec4* out_verts,
	tex2* out_uvs,
	float* out_gouraud,
	int& num_clip_verts
)
{
	ZoneScoped; // for tracy

	// Intersection point between the two vertices and the plane
	glm::vec4 intersection;
	// Interpolated UV coordinate
	tex2 interp_uv;
	// Interpolated Gouraud color
	float interp_gouraud;
	// Percentage factor for interpolating the intersection point and UVs
	float t;

	// Extract the vertices
	std::array<glm::vec4, NUM_VERTICES_PER_TRIANGLE> vertices;
	//glm::vec4 vertices[NUM_VERTICES_PER_TRIANGLE];
	vertices[0] = triangle.vertices[0].position;
	vertices[1] = triangle.vertices[1].position;
	vertices[2] = triangle.vertices[2].position;
	// Extract the UVs
	std::array<tex2, NUM_VERTICES_PER_TRIANGLE> texcoords;
	//tex2 texcoords[NUM_VERTICES_PER_TRIANGLE];
	texcoords[0] = triangle.vertices[0].uv;
	texcoords[1] = triangle.vertices[1].uv;
	texcoords[2] = triangle.vertices[2].uv;
	// Extract the Gouraud color
	std::array<float, NUM_VERTICES_PER_TRIANGLE> gouraud;
	//float gouraud[NUM_VERTICES_PER_TRIANGLE];
	gouraud[0] = triangle.vertices[0].gouraud;
	gouraud[1] = triangle.vertices[1].gouraud;
	gouraud[2] = triangle.vertices[2].gouraud;

	// Start with the last vertex as the previous vertex
	glm::vec4 prev_vert = vertices[NUM_VERTICES_PER_TRIANGLE - 1];
	glm::vec4 curr_vert;
	tex2 prev_texcoord = texcoords[NUM_VERTICES_PER_TRIANGLE - 1];
	tex2 curr_texcoord;
	float prev_gouraud = gouraud[NUM_VERTICES_PER_TRIANGLE - 1];
	float curr_gouraud;

	bool prev_inside = is_inside_plane(prev_vert, plane);
	bool curr_inside;

	// TODO: Handle degenerate triangle cases

	// Loop through all the vertices of the input triangle
	for (int i = 0; i < NUM_VERTICES_PER_TRIANGLE; i++)
	{
		// Current vertex
		curr_vert = vertices[i];
		curr_texcoord = texcoords[i];
		curr_gouraud = gouraud[i];
		curr_inside = is_inside_plane(curr_vert, plane);

		// If we're moving out or moving in
		if (prev_inside != curr_inside)
		{
			// Interpolate the vertex locations to find the intersection point
			t = compute_intersect_ratio(prev_vert, curr_vert, plane);
			intersection = glm::lerp(prev_vert, curr_vert, t);
			// Interpolate the UVS
			interp_uv = tex2_lerp(prev_texcoord, curr_texcoord, t);
			// Interpolate the Gouraud intensity
			interp_gouraud = glm::lerp(prev_gouraud, curr_gouraud, t);
			// Add the interpolated attributes to the out arrays
			out_verts[num_clip_verts] = intersection;
			out_uvs[num_clip_verts] = interp_uv;
			out_gouraud[num_clip_verts] = interp_gouraud;
			num_clip_verts++;
		}

		// If the current vertex is inside the clip plane
		if (curr_inside)
		{
			out_verts[num_clip_verts] = curr_vert;
			out_uvs[num_clip_verts] = curr_texcoord;
			out_gouraud[num_clip_verts] = curr_gouraud;
			num_clip_verts++;
		}

		// Set the current attributes/dp as the previous attributes/dp for the
		// next iteration
		prev_vert = curr_vert;
		prev_texcoord = curr_texcoord;
		prev_gouraud = curr_gouraud;
		prev_inside = curr_inside;
	}
}

bool is_unmodified(const Triangle& triangle, const EClipPlane plane)
{
	// Get the clip distance for the current plane for each of the vertices on
	// the triangle
	const bool v0_inside = is_inside_plane(triangle.vertices[0].position, plane);
	const bool v1_inside = is_inside_plane(triangle.vertices[1].position, plane);
	const bool v2_inside = is_inside_plane(triangle.vertices[2].position, plane);

	// If all vertices are inside the plane, the triangle does not need
	// to be clipped and can be passed to the next clip stage unmodified
	const bool triangle_inside = v0_inside && v1_inside && v2_inside;
	return triangle_inside;
}

/**
 * The clip space inequalities are as follows: no points are allowed in that
 * don't satisfy the following:
 * -w <= x <= w (for the left and right clip planes)
 * -w <= y <= w (for the bottom and top clip planes)
 * -w <= z <= w (for the near and far clip planes)
 * We also want to clip against 0 < w, in order to prevent negative w
 * coordinates from occuring.
 */
bool is_inside_plane(const glm::vec4& vertex, const EClipPlane plane)
{
	bool result;
	switch (plane)
	{
		case NEGATIVE_W_PLANE:
			result = vertex.w >= EPSILON;
			return result;
		case RIGHT_PLANE:
			result = vertex.x <= +vertex.w;
			return result;
		case LEFT_PLANE:
			result = vertex.x >= -vertex.w;
			return result;
		case TOP_PLANE:
			result = vertex.y <= +vertex.w;
			return result;
		case BOTTOM_PLANE:
			result = vertex.y >= -vertex.w;
			return result;
		// The inequalities for the near and far clip planes here are correct.
		// The projection matrix flips positive z from going out of the screen
		// to going in to it
		case NEAR_PLANE:
			result = vertex.z >= -vertex.w;
			return result;
		case FAR_PLANE:
			result = vertex.z <= +vertex.w;
			return result;
		default:
			assert(false);
			return false;
	}
}

float compute_intersect_ratio(
	const glm::vec4& a, 
	const glm::vec4& b, 
	const EClipPlane plane
)
{
	float result;
	switch (plane)
	{
		case NEGATIVE_W_PLANE:
			result = (a.w - EPSILON) / (a.w - b.w);
			return result;
		case RIGHT_PLANE:
			result = (a.w - a.x) / ((a.w - a.x) - (b.w - b.x));
			return result;
		case LEFT_PLANE:
			result = (a.w + a.x) / ((a.w + a.x) - (b.w + b.x));
			return result;
		case TOP_PLANE:
			result = (a.w - a.y) / ((a.w - a.y) - (b.w - b.y));
			return result;
		case BOTTOM_PLANE:
			result = (a.w + a.y) / ((a.w + a.y) - (b.w + b.y));
			return result;
		case NEAR_PLANE:
			result = (a.w + a.z) / ((a.w + a.z) - (b.w + b.z));
			return result;
		case FAR_PLANE:
			result = (a.w - a.z) / ((a.w - a.z) - (b.w - b.z));
			return result;
		default:
			assert(0.0f);
			return 0.0f;
	}
}

