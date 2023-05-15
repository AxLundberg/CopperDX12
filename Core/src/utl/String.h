#pragma once
#include <string>

namespace CPR::UTL
{
	std::wstring ToWide(const std::string& narrow);
	std::string ToNarrow(const std::wstring& wide);
}
