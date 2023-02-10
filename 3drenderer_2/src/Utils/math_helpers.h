#pragma once

constexpr float EPSILON = 0.01f;

bool is_close_to_zero(float val)
{
	bool result = val < EPSILON && val > -EPSILON;
	return result;
}