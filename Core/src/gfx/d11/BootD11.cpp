#include <Core/src/ioc/Container.h>
#include <Core/src/win/IWindow.h>

#include "BootD11.h"
#include "RendererD11.h"

namespace CPR::GFX::D11
{
	void Boot(HWND hWnd)
	{
		IOC::Get().Register<IRendererD11>([&]() {
			return std::make_shared<RendererD11>(hWnd);
			});
	}
}