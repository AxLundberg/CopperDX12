#pragma once
#include <DirectXMath.h>
#include "../typedefs.h"

namespace CPR::APP
{
	class Tile
	{
	public:
		u32 renderObjectIndex;
		DirectX::XMFLOAT4X4 transformation;
		u32 tileTexture;
		u32 x;
		u32 y;
	public:
		Tile();
		~Tile();

	private:

	};
}