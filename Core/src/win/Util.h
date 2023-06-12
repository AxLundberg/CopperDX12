#pragma once
#include "CopperWin.h"
#include <string>

namespace CPR::WIN
{
	std::wstring GetErrorDescription(HRESULT hr);
}