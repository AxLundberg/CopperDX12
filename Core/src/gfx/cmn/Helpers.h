#pragma once
#include "TypeDefs.h"
#include <Core/src/log/Log.h>

namespace CPR::GFX
{
	static u32 AlignAdress(u32 adress, u32 alignment)
	{
		if ((0 == alignment) || (alignment & (alignment - 1)))
			cprlog.Error(L"Error: non-pow2 alignment");

		return ((adress + (alignment - 1)) & ~(alignment - 1));
	}
}
