#include <Core/src/win/boot.h>
#include <Core/src/win/CopperWin.h>
#include <Core/src/ioc/Container.h>
#include <Core/src/log/Log.h>
#include <Core/src/log/SeverityLevelPolicy.h>

using namespace CPR;

void Boot()
{
	LOG::Boot();
	IOC::Get().Register<LOG::ISeverityLevelPolicy>([] {
		return std::make_shared<LOG::SeverityLevelPolicy>(LOG::LogLevel::Info);
	});
	WIN::Boot();
}

int WINAPI wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PWSTR pCmdLine,
	int nCmdShow
)
{
	Boot();

	cprlog.Info(L"Hello Window");
	MessageBoxA(nullptr, "", "Heyo", MB_APPLMODAL | MB_ICONEXCLAMATION);

	return 0;
}
