#pragma once
#include "Level.h"

#include <chrono>

namespace CPR::LOG
{
	struct Entry
	{
		Level lvl;
		std::wstring str;
		const wchar_t* srcFile;
		const wchar_t* srcFunction;
		int srcLine;
		std::chrono::system_clock::time_point timeStamp;
	};
}