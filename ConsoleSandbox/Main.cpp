#include <iostream>

#include "Core/src/log/Log.h"
#include "Core/src/utl/Assert.h"
#include "Core/src/log/Channel.h"
#include "Core/src/log/EntryBuilder.h"
#include "Core/src/log/TextFormatter.h"
#include "Core/src/log/MsvcDebugDriver.h"
#include "Core/src/log/SeverityLevelPolicy.h"

#include "Core/src/win/WindowClass.h"

using namespace CPR;
using namespace std::string_literals;

void Boot()
{
	LOG::Boot();
}

void StackTraceError()
{
	cprlog.Error(L"ERROR!");
}

int main()
{
	Boot();
	auto pWin = std::make_shared<WIN::WindowClass>();
	cprlog.Fatal(L"FATAL!");
	cprlog.Warn(L"WARN!");

	UTL::StackTrace st;
	auto st2 = std::move(st);

	std::wcout << st2.Print() << std::endl;

	return 0;
}