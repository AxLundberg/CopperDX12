#pragma once
#include <memory>
#include <vector>

namespace CPR::LOG
{
	struct Entry;
	class IDriver;

	class IChannel
	{
	public:
		virtual ~IChannel() = default;
		virtual void Submit(Entry&) = 0;
		virtual void AttachDriver(std::shared_ptr<IDriver>) = 0;
	};

	class Channel : public IChannel
	{
	public:
		Channel(std::vector<std::shared_ptr<IDriver>> = {});
		// Inherited via IChannel
		virtual void Submit(Entry&) override;
		virtual void AttachDriver(std::shared_ptr<IDriver>) override;
	private:
		std::vector<std::shared_ptr<IDriver>> mDrivers;
	};
}