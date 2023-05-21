#include "NoReturn.h"
#include "Assert.h"

namespace chil::utl
{
	void NoReturnImpl_()
	{
		cpr_assert(false).Msg(L"Reached no_return guard!");
	}
}
