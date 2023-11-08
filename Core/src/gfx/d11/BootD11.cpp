#include <Core/src/ioc/Singletons.h>
#include <Core/src/ioc/Container.h>
#include <Core/src/win/IWindow.h>

#include "BootD11.h"
#include "RendererD11.h"
#include "DeviceD11.h"

namespace CPR::GFX::D11
{
	void Boot()
	{
		IOC::Get().Register<IRendererD11>([](IRendererD11::IocParams args) {
			auto device = IOC::Get().Resolve<IDev>({ args.hWnd });
			return std::make_shared<RendererD11>(args.hWnd, device);
			});

		IOC::Get().Register<IDev>([](IDev::IocParams args) {
			return std::make_shared<Device>(args.hWnd);
			});

		IOC::Sing().RegisterPassThrough<IDev>();
	}
}