#include <Core/src/ioc/Container.h>
#include <Core/src/ioc/Singletons.h>

#include "WindowClass.h"
#include "Window.h"
#include "Boot.h" 


template<class T>
auto operator|(std::shared_ptr<T> lhs, std::shared_ptr<T> rhs)
{
	if (bool(lhs)) {
		return std::forward<std::shared_ptr<T>>(lhs);
	}
	else {
		return std::forward<std::shared_ptr<T>>(rhs);
	}
}

namespace CPR::WIN
{
	void Boot()
	{
		// container 
		IOC::Get().Register<IWindow>([](IWindow::IocParams args) {
			return std::make_shared<Window>(
				(args.pClass | IOC::Sing().Resolve<IWindowClass>()),
				args.name.value_or(L"Main Window"),
				args.size.value_or(SPA::DimensionsI{ 1280, 720 }),
				args.position
				);
			});
		IOC::Get().Register<IWindowClass>([] {
			return std::make_shared<WindowClass>();
		});
		// singleton
		IOC::Sing().RegisterPassThrough<IWindowClass>();
	}
}
