#include <iostream>

#include "Core/src/ioc/Container.h"

struct Base
{
	virtual int Test() { return 420; }
	virtual ~Base() = default;
};

struct Derived : public Base
{
	int Test() override { return 69; }
};

int main()
{
	using namespace CPR;

	IOC::Get().Register<Base>([] {return std::make_shared<Derived>(); });

	std::cout << IOC::Get().Resolve<Base>()->Test() << std::endl;

	return 0;
}