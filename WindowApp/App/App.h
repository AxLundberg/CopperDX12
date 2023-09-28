#pragma once
#include <Core/src/win/IWindow.h>

namespace CPR::APP
{
	int Run(WIN::IWindow* window, GFX::D12::IRenderer* renderer);
}