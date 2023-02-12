#include "tex2.h"

tex2 tex2_lerp(tex2 a, tex2 b, float t)
{
	tex2 result;
	result.u = a.u * (1.0f - t) + b.u * t;
	result.v = a.v * (1.0f - t) + b.v * t;
	return result;
}
