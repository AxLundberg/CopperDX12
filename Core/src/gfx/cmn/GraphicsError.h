#pragma once
#include <source_location>
#include "TypeDefs.h"

namespace CPR::GFX
{
	struct CheckerToken {};
	extern CheckerToken hrVerify;
	struct HrGrabber
	{
		HrGrabber(u32 hr, std::source_location = std::source_location::current()) noexcept;
		u32 hr;
		std::source_location loc;
	};
	void operator>>(HrGrabber, CheckerToken);
}
