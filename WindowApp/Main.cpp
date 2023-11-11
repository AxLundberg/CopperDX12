#include <Core/src/win/CopperWin.h>
#include <Core/src/win/boot.h>
#include <Core/src/log/Log.h>
#include <Core/src/win/IWindow.h>
#include <Core/src/ioc/Container.h>
#include <Core/src/ioc/Singletons.h>
#include <Core/src/log/SeverityLevelPolicy.h>
#include <format>
#include <ranges>

#include <Core/src/gfx/d12/D12.h>
#include <Core/src/gfx/d11/BootD11.h>
#include <Core/src/gfx/d11/cmn/D11Headers.h>
#include <Core/src/ecs/Ecs.h>
#include <Core/src/gfx/d12/Renderer.h>
#include <Core/src/gfx/d11/RendererD11.h>

#include "App/App.h"

using namespace CPR;
using namespace std::string_literals;
using namespace std::chrono_literals;
namespace rn = std::ranges;
namespace vi = rn::views;

void Boot()
{
	LOG::Boot();
	IOC::Get().Register<LOG::ISeverityLevelPolicy>([] {
		return std::make_shared<LOG::SeverityLevelPolicy>(LOG::LogLevel::Info);
	});
	WIN::Boot();
}

struct EcsTest {
	int asd = 5;
};

int WINAPI wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PWSTR pCmdLine,
	int nCmdShow
)
{
	try
	{
		Boot();
		auto keyboard = std::make_shared<WIN::Keyboard>();
		auto pWindow = IOC::Get().Resolve<WIN::IWindow>(
			WIN::IWindow::IocParams{
				.pKeySink = keyboard,
				.name = L"test",
				.size = SPA::DimensionsI{ 1280, 720 },
				.position = {} 
		});
		GFX::D11::Boot();
		auto pRenderer = IOC::Get().Resolve<GFX::D11::IRendererD11>({ pWindow->GetHandle() });
		return APP::Run(pWindow.get(), keyboard.get(), pRenderer.get(), hInstance);
	}
	catch (const std::exception& e)
	{
		cprlog.Error(UTL::ToWide(e.what()));
		MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR | MB_SETFOREGROUND);
	}

	return -1;
}
