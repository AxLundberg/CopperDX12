#include "Level.h"

namespace CPR::LOG
{
	std::wstring GetLevelName(LogLevel lvl)
	{
		switch (lvl)
		{
		case CPR::LOG::LogLevel::Trace:
			return L"Trace";
		case CPR::LOG::LogLevel::Debug:
			return L"Debug";
		case CPR::LOG::LogLevel::Info:
			return L"Info";
		case CPR::LOG::LogLevel::Warn:
			return L"Warn";
		case CPR::LOG::LogLevel::Error:
			return L"Error";
		case CPR::LOG::LogLevel::Fatal:
			return L"Fatal";
		default:
			return L"Undefined";
		}
	};
}
