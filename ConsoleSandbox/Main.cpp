#include <iostream>

#include "Core/src/log/EntryBuilder.h"
#include "Core/src/log/Channel.h"
#include "Core/src/log/MsvcDebugDriver.h"
#include "Core/src/log/TextFormatter.h"

using namespace CPR;
using namespace std::string_literals;

#define cprlog LOG::EntryBuilder{ __FILEW__, __FUNCTIONW__, __LINE__ }.Channel(ch.get())

int main()
{
	std::unique_ptr<LOG::IChannel> ch = 
		std::make_unique<LOG::Channel>(
			std::vector<std::shared_ptr<LOG::IDriver>>{
			std::make_shared<LOG::MsvcDebugDriver>(std::make_unique<LOG::TextFormatter>())
		}
	);
	cprlog.Fatal(L"NOES!");
	return 0;
}