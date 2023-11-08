#include <Core/src/ioc/Singletons.h>
#include <Core/src/ioc/Container.h>
#include <Core/src/win/IWindow.h>

#include "BootD11.h"
#include "RendererD11.h"
#include "DeviceD11.h"
#include "BufferManagerD11.h"


namespace CPR::GFX::D11
{
	void Boot()
	{
		IOC::Get().Register<IRendererD11>([](IRendererD11::IocParams args) {
			auto device = IOC::Sing().Resolve<IDevice>({ args.hWnd });
			auto bufferMan = IOC::Get().Resolve<IBufferManager>();
			auto samplerMan = IOC::Get().Resolve<ISamplerManager>();
			return std::make_shared<RendererD11>(args.hWnd, device, bufferMan, samplerMan);
			});

		IOC::Get().Register<IBufferManager>([](){
			auto device = IOC::Sing().Resolve<IDevice>();
			return std::make_shared<BufferManagerD11>(device);
			});

		IOC::Get().Register<ISamplerManager>([]() {
			auto device = IOC::Sing().Resolve<IDevice>();
			return std::make_shared<SamplerManagerD11>(device);
			});

		IOC::Sing().Register<IDevice>([](IDevice::IocParams args) {
			return std::make_shared<Device>(args.hWnd);
			});
	}
}