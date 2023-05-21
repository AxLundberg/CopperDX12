#include "Exception.h"
#include "Core/src/utl/String.h"
#include <format>
#include <typeinfo>

namespace CPR::UTL
{
	BufferedException::BufferedException(std::string message)
		:
		mMessage{ std::move(message) } 
	{}

	BufferedException::BufferedException(const std::wstring& message)
		:
		mMessage{ ToNarrow(message) }
	{
	}

	const char* BufferedException::what() const
	{
		using namespace std::string_literals;
		mBuffer = "["s + typeid(const_cast<BufferedException&>(*this)).name() + "]"s;
		if (!mMessage.empty())
		{
			mBuffer += ": ";
			mBuffer += mMessage;
		}
		return mBuffer.c_str();
	}
}
