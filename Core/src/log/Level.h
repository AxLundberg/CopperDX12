#pragma once
#include <string>

namespace CPR::LOG
{
	enum class Level
	{
		Trace,
		Debug,
		Info,
		Warn,
		Error,
		Fatal,
	};

	std::wstring GetLevelName(Level);
}
