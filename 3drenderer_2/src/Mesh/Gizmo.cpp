#include "Gizmo.h"

#include "../Utils/Colors.h"

Gizmo::Gizmo()
{
	reset();
}

void Gizmo::reset()
{
	const glm::vec4 origin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	// x
	bases[0].points[0] = origin;
	bases[0].points[1] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	bases[0].color = Colors::YELLOW;
	// y
	bases[1].points[0] = origin;
	bases[1].points[1] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	bases[1].color = Colors::MAGENTA;
	// z
	bases[2].points[0] = origin;
	bases[2].points[1] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	bases[2].color = Colors::CYAN;
}
