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
		StackTrace(size_t skip = 0);
		StackTrace(const StackTrace& src);
		StackTrace(StackTrace&&) noexcept;
		StackTrace& operator=(const StackTrace& src);
		StackTrace& operator=(StackTrace&&) noexcept;
		~StackTrace();
		std::wstring Print() const;

	private:
		std::unique_ptr<backward::StackTrace> mTrace;
	};
}
