#pragma once
#include <string>

namespace CPR::LOG
{
	enum class LogLevel
	{
		None,
		Fatal,
		Error,
		Warn,
		Info,
		Debug,
		Trace,
	};

	std::wstring GetLevelName(LogLevel);
}
