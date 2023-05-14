#include "../CprCppUnitTests.h"
#include "Core/src/log/Entry.h"
#include "Core/src/log/TextFormatter.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace CPR;
using namespace std::string_literals;

namespace TST
{
	TEST_CLASS(LogTextFormatterTests)
	{
	public:
		TEST_METHOD(TestFormat)
		{
			const LOG::Entry e{
				.level = LOG::LogLevel::Info,
				.text = L"Log Test Format",
				.srcFile = __FILEW__,
				.srcFunction = __FUNCTIONW__,
				.srcLine = __LINE__,
				.timeStamp = std::chrono::system_clock::time_point{
					std::chrono::days{ 10'000 }
				}
			};
			Assert::AreEqual(
				L"@Info {1997-05-19 02:00:00.0000000 CEST} Log Test Format\n  >> in function: TST::LogTextFormatterTests::TestFormat\n     C:\\Users\\axel_\\Brewski\\Copper2.0\\UnitTest\\Log\\LogTextFormatter.cpp(22)\n"s,
				LOG::TextFormatter{}.Format(e)
			);
		}
	};
}
