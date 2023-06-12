#include "Util.h"
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
}
