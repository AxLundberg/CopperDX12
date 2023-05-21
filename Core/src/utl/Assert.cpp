#include "Assert.h"
#include "Core/src/log/log.h"

namespace CPR::UTL
{
	namespace {
#ifdef NDEBUG
		constexpr int SKIP_DEPTH = 3;
		constexpr int SKIP_DEPTH_EX = 23;
#else
		constexpr int SKIP_DEPTH = 7;
		constexpr int SKIP_DEPTH_EX = 29;
#endif
	}
	Assertion::Assertion(std::wstring expression, const wchar_t* file, const wchar_t* function, int line, Consequence consequence)
		: 
		mFile{ file },
		mFunction{ function },
		mLine{ line },
		mConsequence{ consequence },
		mSkipDepth{ SKIP_DEPTH }
	{
		mStream << L"Assertion failed, " << expression << "\n";
	}

	Assertion::~Assertion()
	{
		LOG::EntryBuilder{ mFile, mFunction, mLine }
			.Trace_Skip(SKIP_DEPTH)
			.Channel(LOG::GetDefaultChannel())
			.Level(mConsequence == Consequence::Terminate ? LOG::LogLevel::Fatal : LOG::LogLevel::Error)
			.Note(mStream.str());
		if (mConsequence == Consequence::Terminate) {
			LOG::GetDefaultChannel()->Flush();
			std::terminate();
		}
	}
	Assertion& Assertion::Msg(const std::wstring& message)
	{
		mStream << L"  Msg: " << message << L"\n";
		return *this;
	}
	void Assertion::Ex()
	{
		mConsequence = Consequence::Exception;
		mSkipDepth = SKIP_DEPTH_EX;
		throw FailedAssertion{};
	}
}
