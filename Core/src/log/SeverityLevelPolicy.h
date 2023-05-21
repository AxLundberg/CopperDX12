#pragma once
#include "Policy.h"
#include "Level.h"

namespace CPR::LOG
{
	class ISeverityLevelPolicy : public IPolicy {};

	class SeverityLevelPolicy : public ISeverityLevelPolicy
	{
	public:
		SeverityLevelPolicy(LogLevel level);
		bool TransformFilter(Entry&) override;
	private:
		LogLevel mLevel;
	};
}
