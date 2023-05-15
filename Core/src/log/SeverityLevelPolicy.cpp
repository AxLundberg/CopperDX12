#include "SeverityLevelPolicy.h"
#include "Entry.h"

namespace CPR::LOG
{
	CPR::LOG::SeverityLevelPolicy::SeverityLevelPolicy(LogLevel level)
		:
		mLevel{ level }
	{}

	bool CPR::LOG::SeverityLevelPolicy::TransformFilter(Entry& e)
	{
		return e.level <= mLevel;
	}
}
