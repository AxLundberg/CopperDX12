#pragma once

#include "Mesh.h"

namespace CPR::GFX
{
	struct SurfaceProperty
	{
		ResourceIndex diffuseTexture = ResourceIndex(-1);
		ResourceIndex specularTexture = ResourceIndex(-1);
		ResourceIndex sampler = ResourceIndex(-1);
	};

	struct RenderObject
	{
		ResourceIndex transformBuffer = ResourceIndex(-1);
		SurfaceProperty surfaceProperty;
		Mesh mesh;
	};
}
