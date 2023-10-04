#pragma once
#include <queue>
#include <optional>
#include "ResourceJobs.h"
#include "Queue.h"

namespace CPR::GFX::D12
{
	class JobQueue
	{
	public:
		JobQueue(IQueue&);
		~JobQueue();
		void SubmitJob(ResourceJob);
	private:
		std::queue<ResourceJob> _jobQueue;
		IQueue& _workerQueue;
	};
	class JobManager
	{
	public:
		JobManager();
		~JobManager();
		void SubmitJob(ResourceJob, std::optional<u32> dependency = std::nullopt);
		void PerformJobs();
	private:
	};

	JobManager::JobManager()
	{
	}

	JobManager::~JobManager()
	{
	}
}