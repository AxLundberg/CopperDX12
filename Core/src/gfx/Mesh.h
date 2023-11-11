#pragma once
#include "cmn/TypeDefs.h"

namespace CPR::GFX
{
	struct Mesh
	{
		ResourceIndex vertexBuffer = ResourceIndex(-1);
		ResourceIndex indexBuffer = ResourceIndex(-1);
	};
}