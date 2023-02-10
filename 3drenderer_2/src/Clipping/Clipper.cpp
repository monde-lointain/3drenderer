#include "Clipper.h"

#include "../Mesh/Triangle.h"
#include <glm/vec4.hpp>

void Clipper::create_frustum_planes(const glm::mat4& projection_matrix, const glm::mat4& view_matrix)
{
    // Create the combined projection-view matrix
    glm::mat4 pv_matrix = projection_matrix * view_matrix;

    // Extract the planes from the combined matrix
    planes[LEFT_PLANE] = pv_matrix[3] + pv_matrix[0];
    planes[RIGHT_PLANE] = pv_matrix[3] - pv_matrix[0];
    planes[BOTTOM_PLANE] = pv_matrix[3] + pv_matrix[1];
    planes[TOP_PLANE] = pv_matrix[3] - pv_matrix[1];
    planes[NEAR_PLANE] = pv_matrix[3] + pv_matrix[2];
    planes[FAR_PLANE] = pv_matrix[3] - pv_matrix[2];

    // Normalize the plane equations
    for (glm::vec4& plane : planes) {
        plane = glm::normalize(plane);
    }
}

bool Clipper::cull_against_frustum_planes(const Triangle& triangle)
{
	// Get the vertices of the triangle
	glm::vec4 v0 = triangle.vertices[0];
	glm::vec4 v1 = triangle.vertices[1];
	glm::vec4 v2 = triangle.vertices[2];

	for (const glm::vec4& plane : planes)
	{
		// Compute the signed distance from each vertex to the plane
		float d0 = glm::dot(plane, v0);
		float d1 = glm::dot(plane, v1);
		float d2 = glm::dot(plane, v2);

		// If all vertices are outside the same side of the plane, the triangle
		// can be culled
		if (d0 < 0.0f && d1 < 0.0f && d2 < 0.0f)
		{
			return true;
		}
	}

	// The triangle is not completely outside the frustum, so it should not be
	// culled
	return false;
}

bool Clipper::cull_ndc(const Triangle& triangle)
{
	int points_inside_screen = 0;
	for (const glm::vec4& vertex : triangle.vertices)
	{
		// Check to see if each point in the triangle lies within the NDC cube
		if (!point_in_screen(vertex))
		{
			points_inside_screen++;
		}
	}

	// Cull the triangle if all points lie outside the six frustum planes
	if (points_inside_screen == 0)
	{
		return true;
	}

	return false;
}

bool Clipper::point_in_screen(const glm::vec4& point)
{
	bool inside_left_right = fabs(point.x) <= point.w;
	bool inside_top_bottom = fabs(point.x) <= point.w;
	bool inside_near_far = fabs(point.x) <= point.w;
	bool inside_all = inside_left_right && inside_top_bottom && inside_near_far;
	return inside_all;
}
