#include "Gizmo.h"

Gizmo::Gizmo()
{
	// x
	bases[0] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	// y
	bases[1] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	// z
	bases[2] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	origin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

void Gizmo::reset()
{
	// x
	bases[0] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	// y
	bases[1] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	// z
	bases[2] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	origin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
