#include "Util.h"
#include "Exception.h"
#include "Core/src/log/log.h"

namespace CPR::WIN
{
	std::wstring GetErrorDescription(HRESULT hr)
	{
		wchar_t* descriptionWinAlloc = nullptr;
		const auto res = FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPWSTR>(&descriptionWinAlloc), 0, nullptr
		);

		std::wstring description;
		if (!res)
		{
			cprlog.Warn(L"Failed to format windows error");
		}
		else
		{
			description = descriptionWinAlloc;
			if (LocalFree(descriptionWinAlloc))
			{
				cprlog.Warn(L"Failed to free memory for windows error formatting");
			}
			if (description.ends_with(L"\r\n"))
			{
				description.resize(description.size() - 2);
			}
		}
		return description;
	}
	RECT ToWinRect(const SPA::RectI& sRect)
	{
		return RECT {
			.left = sRect.left,
			.top = sRect.top,
			.right = sRect.right,
			.bottom = sRect.bottom,
		};
	}
	SPA::RectI ToSpaRect(const RECT& wRect)
	{
		return SPA::RectI{
			.left = wRect.left,
			.top = wRect.top,
			.right = wRect.right,
			.bottom = wRect.bottom,
		};
	}
	SPA::DimensionsI ClientToWindowDimensions(const SPA::DimensionsI& sDim, DWORD styles)
	{
		using namespace SPA;
		auto rect = ToWinRect(RectI::FromTopLeftAndDimensions({ 0, 0 }, sDim));
		if (AdjustWindowRect(&rect, styles, false) == false)
		{
			cprlog.Error(L"Failed to adjust window rect").Hr();
			throw WindowException{};
		}
		return ToSpaRect(rect).GetDimensions();
	}
}
