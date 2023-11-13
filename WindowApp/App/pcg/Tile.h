#pragma once
#include <DirectXMath.h>
#include "../typedefs.h"

namespace CPR::APP
{
	struct Tile
	{
		u32 renderObjectIndex;
		DirectX::XMFLOAT4X4 transformation;
		u32 tileTexture;
		u32 x;
		u32 y;
	};
}