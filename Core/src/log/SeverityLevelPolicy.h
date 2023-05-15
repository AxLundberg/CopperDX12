#pragma once
#include "Policy.h"
#include "Level.h"

namespace CPR::LOG
{
	class SeverityLevelPolicy : public IPolicy
	{
	public:
		SeverityLevelPolicy(LogLevel level);
		bool TransformFilter(Entry&) override;
	private:
		LogLevel mLevel;
	};
}
