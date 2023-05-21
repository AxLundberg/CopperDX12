#pragma once
#include <exception>
#include <string>

namespace CPR::UTL
{
	class IException : public std::exception {};

	class BufferedException : public IException
	{
	public:
		BufferedException() = default;
		BufferedException(std::string message);
		BufferedException(const std::wstring& message);
		const char* what() const override;

	private:
		std::string mMessage;
		mutable std::string mBuffer;
	};
}

#define ZC_EX_DEF_FROM(NewType, BaseType) class NewType : public BaseType{ using Base = BaseType; public: using Base::Base; }
#define ZC_EX_DEF(NewType) ZC_EX_DEF_FROM(NewType, CPR::UTL::BufferedException)
