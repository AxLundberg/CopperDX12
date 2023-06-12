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
	//cprlog.Error(L"==No trace==").No_Trace().No_Line();
	cprlog.Error(L"==Trace==");

	int x = 0, y = 1;
	try{
		cpr_check(x > y).assert_watch(x).Ex();
	}
	catch (const std::exception& e){
		std::cout << e.what() << std::endl;
	}

	return 0;
}