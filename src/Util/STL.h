#pragma once

constexpr auto MAX_CIRCLE_ANGLE = 512;
constexpr auto HALF_MAX_CIRCLE_ANGLE = MAX_CIRCLE_ANGLE / 2;
constexpr auto QUARTER_MAX_CIRCLE_ANGLE = MAX_CIRCLE_ANGLE / 4;
constexpr auto MASK_MAX_CIRCLE_ANGLE = MAX_CIRCLE_ANGLE - 1;
constexpr auto PI = 3.14159265358979323846f;

namespace stl
{
	inline float fast_cossin_table[MAX_CIRCLE_ANGLE + 1]; // Declare table of fast cosinus and sinus

	inline float cos(float n)
	{
		float f = n * HALF_MAX_CIRCLE_ANGLE / PI;
		int i = static_cast<int>(f);
		if (i < 0)
			return fast_cossin_table[((-i) + QUARTER_MAX_CIRCLE_ANGLE) & MASK_MAX_CIRCLE_ANGLE];
		else
			return fast_cossin_table[(i + QUARTER_MAX_CIRCLE_ANGLE) & MASK_MAX_CIRCLE_ANGLE];
	}

	inline float sin(float n)
	{
		float f = n * HALF_MAX_CIRCLE_ANGLE / PI;
		int i = static_cast<int>(f);
		if (i < 0) {
			int idx = (-((-i) & MASK_MAX_CIRCLE_ANGLE)) + MAX_CIRCLE_ANGLE;
			assert(idx >= 0 && idx <= MAX_CIRCLE_ANGLE);
			return fast_cossin_table[idx];
		} else {
			int idx = i & MASK_MAX_CIRCLE_ANGLE;
			assert(idx >= 0 && idx <= MAX_CIRCLE_ANGLE);
			return fast_cossin_table[idx];
		}
	}

	inline void build_table()
	{
		for (long i = 0; i <= MAX_CIRCLE_ANGLE; i++)
			fast_cossin_table[i] = (float)std::sin((double)i * PI / HALF_MAX_CIRCLE_ANGLE);
	}
}
