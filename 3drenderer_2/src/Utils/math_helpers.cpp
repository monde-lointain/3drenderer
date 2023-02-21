#include "math_helpers.h"

#include <cmath>
#include "Constants.h"

bool is_nearly_zero(float val)
{
	bool result = fabs(val) < EPSILON;
	return result;
}
