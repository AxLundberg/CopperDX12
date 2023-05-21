#pragma once
#include <memory>
#include <vector>

namespace CPR::LOG
{
	struct Entry;
	class IDriver;
	class IPolicy;

	class IChannel
	{
	public:
		virtual ~IChannel() = default;
		virtual void Submit(Entry&) = 0;
		virtual void Flush() = 0;
		virtual void AttachDriver(std::shared_ptr<IDriver>) = 0;
		virtual void AttachPolicy(std::shared_ptr<IPolicy>) = 0;
	};

	class Channel : public IChannel
	{
	public:
		Channel(std::vector<std::shared_ptr<IDriver>> = {});
		~Channel();
		// Inherited via IChannel
		void Flush() override;
		void Submit(Entry&) override;
		void AttachDriver(std::shared_ptr<IDriver>) override;
		void AttachPolicy(std::shared_ptr<IPolicy>) override;
	private:
		std::vector<std::shared_ptr<IDriver>> mDrivers;
		std::vector<std::shared_ptr<IPolicy>> mPolicies;
	};
}