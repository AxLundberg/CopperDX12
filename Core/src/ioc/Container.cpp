#include "Container.h"

namespace CPR::IOC
{
	Container& Get() noexcept
	{
		static Container container;
		return container;
	}
}
