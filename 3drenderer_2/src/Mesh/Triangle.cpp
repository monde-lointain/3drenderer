#include "Triangle.h"

#include "../Math/Math3D.h"
#include <glm/vec2.hpp>

bool Triangle::is_front_facing()
{
	glm::vec2 a(vertices[0].x, vertices[0].y);
	glm::vec2 b(vertices[1].x, vertices[1].y);
	glm::vec2 c(vertices[2].x, vertices[2].y);

	signed_area = Math3D::orient2d_f(a, b, c);
	if (signed_area > 0.0f)
	{
		return true;
	}
	else
	{
		return false;
	}
}
