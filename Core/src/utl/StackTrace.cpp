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
	StackTrace::StackTrace()
	{
		backward::TraceResolver thisIsAWorkAround; // https://github.com/bombela/backward-cpp/issues/206
		mTrace = std::make_unique<backward::StackTrace>();
		mTrace->load_here(64);
		mTrace->skip_n_firsts(6);
	}
	StackTrace::~StackTrace()
	{}
	StackTrace::StackTrace(const StackTrace& src)
		:
		mTrace{ std::make_unique<backward::StackTrace>(*mTrace) }
	{}

	StackTrace& StackTrace::operator=(const StackTrace& src)
	{
		mTrace = std::make_unique<backward::StackTrace>(*mTrace);
		return *this;
	}

	std::wstring StackTrace::Print() const
	{
		std::ostringstream oss;
		backward::Printer printer;
		printer.print(*mTrace, oss);
		return ToWide(oss.str());
	}
}
