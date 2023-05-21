#pragma once  

namespace CPR::UTL
{
	[[noreturn]]
	void NoReturnImpl_();

}
#define no_return CPR::UTL::NoReturnImpl_()
