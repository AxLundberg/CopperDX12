#include "Channel.h"
#include "Driver.h"

namespace CPR::LOG
{
	Channel::Channel(std::vector<std::shared_ptr<IDriver>> drivers)
		:
		mDrivers{ std::move(drivers) }
	{}

	void Channel::Submit(Entry& e)
	{
		for (auto& d : mDrivers)
		{
			d->Submit(e);
		}
	}

	void Channel::AttachDriver(std::shared_ptr<IDriver> d)
	{
		mDrivers.push_back(std::move(d));
	}
}
