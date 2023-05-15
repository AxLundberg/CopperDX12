#include "../CprCppUnitTests.h"
#include "Core/src/log/EntryBuilder.h"
#include "Core/src/log/Channel.h"
#include "Core/src/log/Driver.h"
#include "Core/src/log/SeverityLevelPolicy.h"

#include <memory>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace CPR;
using namespace std::string_literals;

#define cprlog LOG::EntryBuilder{ __FILEW__, __FUNCTIONW__, __LINE__ }

class TestDriver : public LOG::IDriver
{
public:
	void Submit(const LOG::Entry& e) override
	{
		mEntry = e;
	}
	LOG::Entry mEntry;
};

template<> inline std::wstring _cdecl
Microsoft::VisualStudio::CppUnitTestFramework::ToString<LOG::LogLevel>(const LOG::LogLevel& level)
{
	return LOG::GetLevelName(level);
}

namespace TST
{
	TEST_CLASS(LogChannelTests)
	{
	public:
		TEST_METHOD(TestForwarding)
		{
			LOG::Channel ch;
			auto driver1 = std::make_shared<TestDriver>();
			auto driver2 = std::make_shared<TestDriver>();
			ch.AttachDriver(driver1);
			ch.AttachDriver(driver2);
			cprlog.Info(L"Log TestForwarding").Channel(&ch);
			Assert::AreEqual(L"Log TestForwarding"s, driver1->mEntry.text);
			Assert::AreEqual(LOG::LogLevel::Info, driver1->mEntry.level);
			Assert::AreEqual(L"Log TestForwarding"s, driver2->mEntry.text);
			Assert::AreEqual(LOG::LogLevel::Info, driver2->mEntry.level);
		}
		TEST_METHOD(TestPolicyFiltering)
		{
			LOG::Channel ch;
			auto driver = std::make_shared<TestDriver>();
			ch.AttachDriver(driver);
			ch.AttachPolicy(std::make_unique<LOG::SeverityLevelPolicy>(LOG::LogLevel::Info));
			cprlog.Info(L"LogTest Forwarding").Channel(&ch);
			Assert::AreEqual(L"LogTest Forwarding"s, driver->mEntry.text);
			Assert::AreEqual(LOG::LogLevel::Info, driver->mEntry.level);
			cprlog.Debug(L"LogTest PolicyFiltering").Channel(&ch);
			Assert::AreEqual(L"LogTest Forwarding"s, driver->mEntry.text);
			Assert::AreEqual(LOG::LogLevel::Info, driver->mEntry.level);
		}
	};
}
