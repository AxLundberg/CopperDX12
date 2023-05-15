#define _CRT_SECURE_NO_WARNINGS
#include "StackTrace.h"
#include "String.h"
#include <sstream>

#pragma warning (push)
#pragma warning (disable : 26495 26439 26451)
#include <Core/thirdParty/backward.h>
#pragma warning (pop)

namespace CPR::UTL
{
	StackTrace::StackTrace(size_t skip)
	{
		backward::TraceResolver thisIsAWorkaround; // https://github.com/bombela/backward-cpp/issues/206
		mTrace = std::make_unique<backward::StackTrace>();
		mTrace->load_here(64);
		if (skip != 0) {
			mTrace->skip_n_firsts(skip);
		}
	}

	StackTrace::StackTrace(const StackTrace& src)
		:
		mTrace{ src.mTrace ? std::make_unique<backward::StackTrace>(*src.mTrace) : std::unique_ptr<backward::StackTrace>{} }
	{}

	StackTrace::StackTrace(StackTrace&& donor) noexcept
		:
		mTrace{ std::move(donor.mTrace) }
	{}

	StackTrace& StackTrace::operator=(const StackTrace& src)
	{
		mTrace = src.mTrace ? std::make_unique<backward::StackTrace>(*src.mTrace) : std::unique_ptr<backward::StackTrace>{};
		return *this;
	}

	StackTrace& StackTrace::operator=(StackTrace&& donor) noexcept
	{
		if (&donor != this) {
			mTrace = std::move(donor.mTrace);
		}
		return *this;
	}

	std::wstring StackTrace::Print() const
	{
		if (mTrace) {
			std::ostringstream oss;
			backward::Printer printer;
			printer.print(*mTrace, oss);
			return UTL::ToWide(oss.str());
		}
		else {
			return L"== EMPTY TRACE ==";
		}
	}

	StackTrace::~StackTrace() {}
}
