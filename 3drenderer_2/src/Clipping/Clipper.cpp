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
#include <iterator>
#include <ranges>

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

std::vector<Triangle> Clipper::clip_triangles(std::vector<Triangle>& triangles)
{
	// The collection of triangles created from all triangles after they have
	// been clipped
	std::vector<Triangle> out_triangles;
	// Reserve memory for the maximum number of triangles that could be generated
	out_triangles.reserve(6 * triangles.size());
	// Iterate over each of the six frustum planes
	for (Triangle& triangle : triangles) {
		// Collection of triangles for a single triangle after it has been clipped
		std::vector<Triangle> clip_result;
		clip_result.push_back(triangle);

		clip_result = clip_triangles_to_plane(clip_result, LEFT_PLANE);
		clip_result = clip_triangles_to_plane(clip_result, RIGHT_PLANE);
		clip_result = clip_triangles_to_plane(clip_result, BOTTOM_PLANE);
		clip_result = clip_triangles_to_plane(clip_result, TOP_PLANE);
		clip_result = clip_triangles_to_plane(clip_result, NEAR_PLANE);
		clip_result = clip_triangles_to_plane(clip_result, FAR_PLANE);

		// Allocate memory into the array of clipped triangles for the triangles created from this clip
		out_triangles.reserve(out_triangles.size() + clip_result.size());
		// Move the final results of the clipping into the full array of clipped triangles
		std::move(clip_result.begin(), clip_result.end(), std::back_inserter(out_triangles));
	}
	Logger::print(LOG_CATEGORY_CLIPPING, "Out triangles: " + std::to_string(out_triangles.size()));
	return out_triangles;
}

std::vector<Triangle> Clipper::clip_triangles_to_plane(
	std::vector<Triangle> triangles, const EClipPlane& plane)
{
	std::vector<Triangle> clip_result;
	for (Triangle triangle : triangles)
	{
		// Get the clip distance for the current plane for each of the
		// vertices on the triangle
		bool v0_inside = is_inside_plane(triangle.vertices[0], plane);
		bool v1_inside = is_inside_plane(triangle.vertices[1], plane);
		bool v2_inside = is_inside_plane(triangle.vertices[2], plane);

		// If all vertices are inside the plane, the triangle does not need
		// to be clipped and can be passed to the next clip stage unmodified
		if (v0_inside && v1_inside && v2_inside)
		{
			clip_result.push_back(triangle);
			continue;
		}

		std::vector<glm::vec4> vertices;
		std::vector<tex2> texcoords;
		clip_triangle_to_plane(triangle, plane, vertices, texcoords);

		//assert(vertices.size() == 3 || vertices.size() == 4);

		// Clipped polygon is a triangle. No new polygons need to be created
		if (vertices.size() == 3)
		{
			Triangle a = triangle;
			a.vertices[0] = vertices[0];
			a.vertices[1] = vertices[1];
			a.vertices[2] = vertices[2];
			a.texcoords[0] = texcoords[0];
			a.texcoords[1] = texcoords[1];
			a.texcoords[2] = texcoords[2];

			clip_result.push_back(a);
		}
		// Clipped polygon is a quad. Split into two triangles
		else if (vertices.size() == 4)
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

			clip_result.push_back(a);
			clip_result.push_back(b);
		}
	}
	return clip_result;
}

void Clipper::clip_triangle_to_plane(const Triangle& triangle,
	const EClipPlane& plane, std::vector<glm::vec4>& out_verts,
	std::vector<tex2>& out_uvs)
{
	// Intersection point between the two vertices and the plane
	glm::vec4 intersection;
	// Interpolated UV coordinate
	tex2 interp_uv;
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
			out_verts.push_back(intersection);
			out_uvs.push_back(interp_uv);
		}

		// If the current vertex is inside the clip plane
		if (curr_inside)
		{
			out_verts.push_back(curr_vert);
			out_uvs.push_back(curr_texcoord);
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
	bool result;
	switch (plane)
	{
		// case POSITIVE_W:
		//	return vertex.w >= EPSILON;
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
		case FAR_PLANE:
			result = vertex.z <= +vertex.w;
			return result;
		case NEAR_PLANE:
			result = vertex.z >= -vertex.w;
			return result;
		default:
			assert(false);
			result = false;
			return result;
	}
}

float Clipper::compute_intersect_ratio(
	const glm::vec4& curr, const glm::vec4& prev, const EClipPlane& plane)
{
	float result;
	switch (plane)
	{
		// case POSITIVE_W:
		//	return (prev.w - EPSILON) / (prev.w - curr.w);
		case RIGHT_PLANE:
			result = (prev.w - prev.x) / ((prev.w - prev.x) - (curr.w - curr.x));
			return result;
		case LEFT_PLANE:
			result = (prev.w + prev.x) / ((prev.w + prev.x) - (curr.w + curr.x));
			return result;
		case TOP_PLANE:
			result = (prev.w - prev.y) / ((prev.w - prev.y) - (curr.w - curr.y));
			return result;
		case BOTTOM_PLANE:
			result = (prev.w + prev.y) / ((prev.w + prev.y) - (curr.w + curr.y));
			return result;
		case FAR_PLANE:
			result = (prev.w - prev.z) / ((prev.w - prev.z) - (curr.w - curr.z));
			return result;
		case NEAR_PLANE:
			result = (prev.w + prev.z) / ((prev.w + prev.z) - (curr.w + curr.z));
			return result;
		default:
			assert(0.0f);
			result = 0.0f;
			return result;
	}
}
