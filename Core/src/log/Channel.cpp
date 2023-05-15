#include "Channel.h"
#include "Driver.h"
#include "Policy.h"

namespace CPR::LOG
{
	Channel::Channel(std::vector<std::shared_ptr<IDriver>> drivers)
		:
		mDrivers{ std::move(drivers) }
	{}

	Channel::~Channel()
	{}

	void Channel::Submit(Entry& e)
	{
		for (auto& p : mPolicies)
		{
			if (!p->TransformFilter(e))
				return;
		}
		for (auto& d : mDrivers)
		{
			d->Submit(e);
		}
	}

	void Channel::AttachDriver(std::shared_ptr<IDriver> d)
	{
		mDrivers.push_back(std::move(d));
	}

	void Channel::AttachPolicy(std::shared_ptr<IPolicy> p)
	{
		mPolicies.push_back(std::move(p));
	}
}
