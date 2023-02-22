#include "Triangle.h"

#include <glm/vec2.hpp>
#include "../Math/Math3D.h"

bool Triangle::is_front_facing()
{
	const glm::vec2 a(vertices[0].position.x, vertices[0].position.y);
	const glm::vec2 b(vertices[1].position.x, vertices[1].position.y);
	const glm::vec2 c(vertices[2].position.x, vertices[2].position.y);

	signed_area = Math3D::orient2d_f(a, b, c);
	return signed_area > 0.0f;
}
