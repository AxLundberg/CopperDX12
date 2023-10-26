#pragma once
#include "../../ISwapChain.h"

namespace CPR::GFX::D12
{
	class ImguiD12
	{
	public:
		ImguiD12();
		~ImguiD12();

		// Public interface
		void BeginFrame();
		void EndFrame();
		void Render();

	};
}