#include "GraphicsError.h"
#include <Core/src/win/Util.h>
#include <Core/src/utl/String.h>
#include <ranges>
#include <format>

namespace rn = std::ranges;
namespace vi = rn::views;

namespace CPR::GFX
{
	CheckerToken hrVerify;

	HrGrabber::HrGrabber(u32 hr, std::source_location loc) noexcept
		:
		hr(hr),
		loc(loc)
	{}
	void operator>>(HrGrabber g, CheckerToken)
	{
		if (FAILED(g.hr))
		{
			auto errorString = UTL::ToNarrow(WIN::GetErrorDescription(g.hr)) |
				vi::transform([](char c) {return c == '\n' ? ' ' : c; }) |
				vi::filter([](char c) {return c != '\r'; }) |
				rn::to<std::basic_string>();
			throw std::runtime_error{
				std::format("Graphics Error: {}\n   {}({})",
					errorString, g.loc.file_name(), g.loc.line())
			};
		}

	}
}
