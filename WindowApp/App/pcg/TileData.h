#pragma once
#include <Core/src/utl/String.h>
#include "../typedefs.h"

namespace CPR::APP
{
	using col = u32;

	struct Material
	{
		u8 r, g, b, a;
		std::string prompt = "";
	};

	static constexpr u32 COLOR_GRANULARITY = 2;
	static constexpr u32 GRID_DIM = 4;
	static constexpr u32 IMAGE_DIM = 512;
	static constexpr i32 TILE_PIXEL_COUNT = IMAGE_DIM / GRID_DIM;

	static constexpr i32 TILE_TEXEL_COUNT = 32;
	static constexpr i32 TEXEL_PIXEL_COUNT = TILE_PIXEL_COUNT / TILE_TEXEL_COUNT;

	static constexpr i32 FAIL_ID = -2;
	static constexpr col BLACK = 0x00000000;
	static constexpr col RED = 0x00FF0000;
	static constexpr col GREEN = 0x0000FF00;
	static constexpr col BLUE = 0x000000FF;
	static constexpr col WHITE = 0x00FFFFFF;
	static constexpr col YELLOW = 0x00FFFF00;
	static constexpr col BROWN = 0x008B4513;
}