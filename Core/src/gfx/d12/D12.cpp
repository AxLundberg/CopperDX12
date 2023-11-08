#include <Core/src/ioc/Container.h>
#include <Core/src/ioc/Singletons.h>
#include <Core/src/win/IWindow.h>

#include "Types/SwapChainD12.h"
#include "../IDevice.h"
#include "DeviceD12.h"
#include "IRenderer.h"
#include "Renderer.h"
#include "D12.h"

#include "experiment/SyncCommander.h"

namespace CPR::GFX::D12
{
	void Boot(HWND window)
	{
		/*IOC::Get().Register<IRenderer>([]() {
			auto device = IOC::Sing().Resolve<IDevice>();
			auto swapChain = IOC::Get().Resolve<ISwapChain>();
			auto syncCommander = IOC::Get().Resolve<ISyncCommander>();
			return std::make_shared<Renderer>(device, swapChain, syncCommander);
			});

		IOC::Get().Register<ISyncCommander>([] {
			auto fence = IOC::Get().Resolve<IFence>();
			auto queue = IOC::Get().Resolve<IQueue>();
			auto atorAndList = IOC::Get().Resolve<IAllocatorAndList>();
			auto sc = std::make_shared<SyncCommander>();
			sc->AttachFence(fence);
			sc->AttachQueue(queue);
			sc->AttachAllocatorAndList(atorAndList);
			return sc;
			});

		IOC::Get().Register<IFence>([] {
			auto device = IOC::Sing().Resolve<IDevice>();
			return std::make_shared<Fence>(device);
		});

		IOC::Get().Register<IQueue>([] {
			auto device = IOC::Sing().Resolve<IDevice>();
			auto flushFence = IOC::Get().Resolve<IFence>();
			return std::make_shared<QueueD12>(device, flushFence, D3D12_COMMAND_LIST_TYPE_DIRECT);
			});

		IOC::Get().Register<IAllocatorAndList>([] {
			auto device = IOC::Sing().Resolve<IDevice>();
			return std::make_shared<AllocatorAndList>(device);
		});

		IOC::Get().Register<GFX::IDevice>([]() {
			return std::make_shared<CPR::GFX::D12::Device>();
			});

		IOC::Sing().RegisterPassThrough<IDevice>();

		IOC::Get().Register<GFX::ISwapChain>([&](ISwapChain::IocParams args) {
			return std::make_shared<SwapChainD12>(
				args.window = window,
				args.nrOfBuffers.value_or(2));
			});*/
	}
}
