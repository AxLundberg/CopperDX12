#pragma once
#include "Level.h"
#include <chrono>
#include <optional>
#include <Core/src/utl/StackTrace.h>

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
		std::optional<CPR::UTL::StackTrace> trace;
		std::optional<unsigned int> hResult;
		// behavior override flags 
		std::optional<bool> captureTrace;
		std::optional<bool> showSourceLine;
	};
}