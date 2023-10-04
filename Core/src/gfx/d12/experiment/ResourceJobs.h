#pragma once
#include <utility>
#include "../cmn/D12Headers.h"

namespace CPR::GFX::D12
{
	enum class JobType
	{
		TRANSITION,
	};
	struct ResourceJob
	{
		JobType type = JobType::TRANSITION;
		std::pair<u32, u32> indexRange = { 0, 1 };
	};

	struct TransitionJob : public ResourceJob
	{
		D3D12_RESOURCE_STATES stateBefore;
		D3D12_RESOURCE_STATES stateAfter;
	};
}