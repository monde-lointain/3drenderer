#pragma once

#include <array>

#include "../Line/Line3D.h"

struct Gizmo
{
	Gizmo();

	void reset();

	std::array<Line3D, 3> bases;
};

