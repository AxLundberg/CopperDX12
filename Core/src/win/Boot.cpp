#include "Boot.h"
#include "WindowClass.h"
#include "Core/src/ioc/Singletons.h"
#include "Core/src/ioc/Container.h"


namespace CPR::WIN
{
	void Boot()
	{
		// container
		IOC::Get().Register<IWindowClass>([] {
			return std::make_shared<WindowClass>();
		});
		// singleton
		IOC::Sing().RegisterPassThrough<IWindowClass>();
	}
}
