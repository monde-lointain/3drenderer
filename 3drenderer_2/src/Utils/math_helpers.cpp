#include "math_helpers.h"

#include <cmath>

bool is_nearly_zero(float val)
{
	bool result = fabs(val) < EPSILON;
	return result;
}
