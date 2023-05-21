#include <iostream>

#include "Core/src/log/Log.h"
#include "Core/src/utl/Assert.h"
#include "Core/src/log/Channel.h"
#include "Core/src/log/EntryBuilder.h"
#include "Core/src/log/TextFormatter.h"
#include "Core/src/log/MsvcDebugDriver.h"
#include "Core/src/log/SeverityLevelPolicy.h"

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
	cprlog.Fatal(L"FATAL!");
	cprlog.Warn(L"WARN!");

	UTL::StackTrace st;
	auto st2 = std::move(st);

	std::wcout << st2.Print() << std::endl;
	std::wcout << st.Print() << std::endl;

	int x = 0, y = 1;
	cpr_assert(x > y).Msg(L"TEST ASSERT").assert_watch(x).assert_watch(y);

	cpr_check(x > y);

	try
	{
		cpr_check(x > y).assert_watch(x).Ex();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}