#pragma once

#include "Mesh.h"

namespace CPR::GFX::D12
{
	struct RenderObject
	{
		ResourceIndex transformBuffer = ResourceIndex(-1);
		Mesh mesh;
	};
}
