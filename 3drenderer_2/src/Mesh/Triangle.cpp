#include "Triangle.h"

#include "../Math/Math3D.h"
#include <glm/vec2.hpp>

bool Triangle::is_facing_camera()
{
	glm::vec2 a(vertices[0].x, vertices[0].y);
	glm::vec2 b(vertices[1].x, vertices[1].y);
	glm::vec2 c(vertices[2].x, vertices[2].y);

	// Use the sign of the cross product of vectors (b-a) and (c-a) to determine
	// the vertex winding order of the triangle. If it's negative, render it. If
	// it's zero or negative cull it.
	if (Math3D::orient2d_f(a, b, c) < 0.0f)
	{
		return true;
	}
	else
	{
		return false;
	}
}
