#pragma once

#include "../Line/Line3D.h"

struct Gizmo
{
	Gizmo();
	~Gizmo() = default;

	void reset();

	Line3D bases[3];
};

