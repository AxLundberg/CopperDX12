#pragma once
#include "CopperWin.h"
#include "Core/src/spa/Rect.h"
#include "Core/src/spa/Dimensions.h"
#include <string>

namespace CPR::WIN
{
	std::wstring GetErrorDescription(HRESULT hr);
	RECT ToWinRect(const SPA::RectI&);
	SPA::RectI ToSpaRect(const RECT&);
	SPA::DimensionsI ClientToWindowDimensions(const SPA::DimensionsI&, DWORD);
}