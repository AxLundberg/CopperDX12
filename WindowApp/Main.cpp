#include <Core/src/win/CopperWin.h>
#include <Core/src/win/boot.h>
#include <Core/src/log/Log.h>
#include <Core/src/win/IWindow.h>
#include <Core/src/ioc/Container.h>
#include <Core/src/ioc/Singletons.h>
#include <Core/src/log/SeverityLevelPolicy.h>
#include <format>
#include <ranges>

#include <Core/src/ecs/Ecs.h>

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
	Boot();

	auto windowPtrs = vi::iota(0, 3) |
		vi::transform([](auto i) {return IOC::Get().Resolve<WIN::IWindow>(); }) |
		rn::to<std::vector>();

	auto& ecs = ECS::Get();
	auto& entity1 = ecs.createEntity();
	ecs.addComponent<EcsTest>(entity1,
		EcsTest{.asd = 3 }
	);

	ecs.Collect<EcsTest>().Do(
		[&](ECS::Entity& entity, EcsTest& test)
		{
			auto res = std::format("\n--- Entity #{} ---\n    EcsTest: {}\n",
			entity.getID(), test.asd);
			cprlog.Info(UTL::ToWide(res));
		}
	);

	auto x = 0;
	while (!windowPtrs.empty())
	{
		std::erase_if(windowPtrs, [](auto& p) {return p->IsClosing(); });
		for (auto& p : windowPtrs)
		{
			p->SetTitle(std::format(L"Animated Window Title [{:*<{}}]", L'*', x+1));
		}
		x = (x+1) % 20;
		std::this_thread::sleep_for(50ms);
	}
	//cprlog.Info(L"Hello Window");
	//MessageBoxA(nullptr, "", "Heyo", MB_APPLMODAL | MB_ICONEXCLAMATION);

	return 0;
}
