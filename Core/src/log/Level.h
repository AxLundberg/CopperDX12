#pragma once
#include <string>

namespace CPR::LOG
{
	enum class LogLevel
	{
		Trace,
		Debug,
		Info,
		Warn,
		Error,
		Fatal,
	};

	std::wstring GetLevelName(LogLevel);
}
