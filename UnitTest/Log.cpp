#include "CprCppUnitTests.h"
#include "Core/src/log/EntryBuilder.h"
#include "Core/src/log/Channel.h"

#include <memory>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace CPR;
using namespace std::string_literals;

#define cprlog LOG::EntryBuilder{ __FILEW__, __FUNCTIONW__, __LINE__ }

class TestChannel : public LOG::IChannel
{
public:
	void Submit(LOG::Entry& e) override
	{
		mEntry = e;
	}
	LOG::Entry mEntry;
};

namespace Infrastructure
{
	TEST_CLASS(LogTests)
	{
	public:
		TEST_METHOD(TestFluent)
		{
			TestChannel ch;
			cprlog.Level(LOG::LogLevel::Info).Note(L"LOG TEST").Channel(&ch);
			Assert::AreEqual(L"LOG TEST"s, ch.mEntry.text);
			Assert::IsTrue(LOG::LogLevel::Info == ch.mEntry.level);
			Assert::AreEqual(32, ch.mEntry.srcLine);
		}
	};
}
