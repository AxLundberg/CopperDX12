#include "GenericTaskQueue.h"

namespace CPR::CCR
{
	void GenericTaskQueue::PopExecute()
	{
		Task task;
		{
			std::lock_guard lck{ mtx };
			task = std::move(tasks.front());
			tasks.pop_front();
		}
		task();
	}
	void GenericTaskQueue::PushWrappedTask(Task task)
	{
		std::lock_guard lck{ mtx };
		tasks.push_back(std::move(task));
	}
}