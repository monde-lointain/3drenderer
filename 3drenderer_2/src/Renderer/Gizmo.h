#pragma once

#include <glm/vec4.hpp>

struct Gizmo
{
	Gizmo();
	~Gizmo() = default;

	void reset();

	glm::vec4 bases[3];
	glm::vec4 origin;
};

