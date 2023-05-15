#pragma once

namespace CPR::LOG
{
	struct Entry;

	class IPolicy
	{
	public:
		virtual ~IPolicy() = default;
		virtual bool TransformFilter(Entry&) = 0;
	};
}
