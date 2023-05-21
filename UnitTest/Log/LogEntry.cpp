#include "../CprCppUnitTests.h"
#include "Core/src/log/EntryBuilder.h"
#include "Core/src/log/Channel.h"
#include "Core/src/log/Policy.h"

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
	void AttachDriver(std::shared_ptr<LOG::IDriver>) override {}
	void AttachPolicy(std::shared_ptr<LOG::IPolicy>) override {}
	void Flush() override {}
	LOG::Entry mEntry;
};

template<> inline std::wstring _cdecl
Microsoft::VisualStudio::CppUnitTestFramework::ToString<LOG::LogLevel>(const LOG::LogLevel& level)
{
	return LOG::GetLevelName(level);
}

namespace TST
{
	TEST_CLASS(LogEntryTests)
	{
	public:
		TEST_METHOD(TestFluent)
		{
			TestChannel ch;
			cprlog.Info(L"Log TestFluent").Channel(&ch);
			Assert::AreEqual(L"Log TestFluent"s, ch.mEntry.text);
			Assert::IsTrue(LOG::LogLevel::Info == ch.mEntry.level);
			Assert::AreEqual(41, ch.mEntry.srcLine);
		}

		TEST_METHOD(BasicLevelNote)
		{
			TestChannel ch;
			cprlog.Info(L"Log TestBasicLevelNote").Channel(&ch);
			Assert::AreEqual(L"Log TestBasicLevelNote"s, ch.mEntry.text);
			Assert::IsTrue(LOG::LogLevel::Info == ch.mEntry.level);
		}
	};
}
