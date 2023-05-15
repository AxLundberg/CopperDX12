#pragma once
#include <string>
#include <memory>

namespace backward
{
	class StackTrace;
}

namespace CPR::UTL
{
	// wrapper for backward stacktrace
	class StackTrace
	{
	public:
		StackTrace();
		~StackTrace();
		StackTrace(const StackTrace& src);
		StackTrace& operator=(const StackTrace& src);
		std::wstring Print() const;
	private:
		std::unique_ptr<backward::StackTrace> mTrace;
	};
}
