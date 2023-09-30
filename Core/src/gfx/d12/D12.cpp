#include <Core/src/ioc/Container.h>
#include <Core/src/ioc/Singletons.h>
#include <Core/src/win/IWindow.h>

#include "Types/SwapChainD12.h"
#include "Types/DeviceD12.h"
#include "IRenderer.h"
#include "Renderer.h"
#include "D12.h"

namespace CPR::GFX::D12
{
	void Boot(HWND window)
	{
		IOC::Get().Register<IRenderer>([]() {
			auto device = IOC::Sing().Resolve<IDevice>();
			auto swapChain = IOC::Get().Resolve<ISwapChain>();
			return std::make_shared<Renderer>(device, swapChain);
			});

		IOC::Get().Register<GFX::IDevice>([]() {
			return std::make_shared<DeviceD12>();
			});

		IOC::Sing().RegisterPassThrough<IDevice>();

		IOC::Get().Register<GFX::ISwapChain>([&](ISwapChain::IocParams args) {
			return std::make_shared<SwapChainD12>(
				args.window = window,
				args.nrOfBuffers.value_or(2));
			});
	}
}
