#include "Ecs.h"
#include "EcsManager.h"
#include <Core/src/ioc/Container.h>
#include <Core/src/ioc/Singletons.h>

namespace CPR::ECS
{
	EcsManager& Get() {
		static EcsManager ecsManager;
		return ecsManager;
	}
}
