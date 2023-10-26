#pragma once
#include "../cmn/D12Headers.h"

namespace CPR::GFX::D12
{
	struct TextureDesc
	{
		u32 width;
		u32 height;
		u32 mipLevels;
		DXGI_FORMAT format;
		D3D12_RESOURCE_FLAGS flags;
		D3D12_CLEAR_VALUE clearColour;
	};
}