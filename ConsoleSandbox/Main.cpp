#include <iostream>

#include "Core/src/log/Log.h"
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

	StackTraceError();
	return 0;
}