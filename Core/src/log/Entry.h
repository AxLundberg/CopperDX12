#pragma once
#include "Level.h"

#include <chrono>

namespace CPR::LOG
{
	struct Entry
	{
		LogLevel level = LogLevel::Error;
		std::wstring text;
		const wchar_t* srcFile = nullptr;
		const wchar_t* srcFunction = nullptr;
		int srcLine = -1;
		std::chrono::system_clock::time_point timeStamp;
	};
}