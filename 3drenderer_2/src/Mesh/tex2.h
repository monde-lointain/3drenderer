#pragma once

#include <string>

struct tex2
{
	float u;
	float v;
};

tex2 tex2_lerp(tex2 a, tex2 b, float t);