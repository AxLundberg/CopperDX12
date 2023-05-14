#pragma once

namespace CPR::LOG
{
	struct Entry;

	class IChannel
	{
	public:
		virtual ~IChannel() = default;
		virtual void Submit(Entry&) = 0;
	private:
		Entry mEntry;
	};
}