#include <Core/src/ioc/Container.h>
#include <Core/src/ioc/Singletons.h>
#include <Core/src/win/IWindow.h>

#include "RendererInterface.h"
#include "Renderer.h"
#include "D12.h"

namespace CPR::GFX::D12
{
	void Boot()
	{
		IOC::Get().Register<CPR::GFX::D12::IRenderer>([]() {
			return std::make_shared<CPR::GFX::D12::Renderer>();
			});
	}
}