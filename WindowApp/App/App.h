#pragma once
#include <Core/src/win/IWindow.h>
#include <Core/src/gfx/d11/RendererD11.h>

namespace CPR::APP
{
	int Run(WIN::IWindow*, WIN::Keyboard*, GFX::D11::IRendererD11*, HINSTANCE);
}