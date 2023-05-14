#include "CprCppUnitTests.h"
#include "Core/src/ioc/Container.h"

#include <memory>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

struct Base
{
	virtual int Test() { return 0; }
	virtual ~Base() = default;
};

struct Derived : public Base
{
	int Test() override { return 1; }
};

using namespace CPR;

namespace Infrastructure
{
	TEST_CLASS(IocTests)
	{
	public:
		TEST_METHOD_INITIALIZE(Init)
		{
			pIoc = std::make_unique<IOC::Container>();
		}
		TEST_METHOD(BasicResolve)
		{
			pIoc->Register<Base>([this] {return std::make_shared<Derived>(); });
			Assert::AreEqual(1, pIoc->Resolve<Base>()->Test());
		}
		TEST_METHOD(BasicResolveFail)
		{
			Assert::ExpectException<std::runtime_error>([this] {
				IOC::Get().Resolve<Base>();
			});
		}
	private:
		std::unique_ptr<IOC::Container> pIoc;
	};
}
