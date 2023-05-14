#include "Level.h"

namespace CPR::LOG
{
	std::wstring GetLevelName(Level lvl)
	{
		switch (lvl)
		{
		case CPR::LOG::Level::Trace:
			return L"Trace";
		case CPR::LOG::Level::Debug:
			return L"Debug";
		case CPR::LOG::Level::Info:
			return L"Info";
		case CPR::LOG::Level::Warn:
			return L"Warn";
		case CPR::LOG::Level::Error:
			return L"Error";
		case CPR::LOG::Level::Fatal:
			return L"Fatal";
		default:
			return L"Undefined";
		}
	};
}
