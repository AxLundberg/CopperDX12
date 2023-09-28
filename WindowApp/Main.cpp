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
#include <Core/src/ecs/Ecs.h>
#include <Core/src/gfx/d12/Renderer.h>

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
	GFX::D12::Boot();
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
		auto pWindow = IOC::Get().Resolve<WIN::IWindow>();
		auto pRenderer = IOC::Get().Resolve<GFX::D12::IRenderer>();
		pRenderer->Initialize(pWindow->GetHandle());
		return APP::Run(pWindow.get(), pRenderer.get());
	}
	catch (const std::exception& e)
	{
		cprlog.Error(UTL::ToWide(e.what())).No_Line().No_Trace();
		MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR | MB_SETFOREGROUND);
	}

	return -1;
}
