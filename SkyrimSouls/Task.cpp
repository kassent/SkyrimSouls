#include "Task.h"
#include "ICriticalSection.h"
#include <SKSE/PluginAPI.h>
#include <SKSE/SafeWrite.h>
#include <SKSE/HookUtil.h>
#include <queue>

ICriticalSection			s_taskQueueLock;
std::queue<TaskDelegate*>	s_tasks;

namespace PauseTaskInterface
{
	class BSTaskPool
	{
	public:
		void BSTaskPool::ProcessTasks()
		{
			s_taskQueueLock.Enter();
			while (!s_tasks.empty())
			{
				TaskDelegate * cmd = s_tasks.front();
				s_tasks.pop();

				cmd->Run();
				cmd->Dispose();
			}
			s_taskQueueLock.Leave();

			this->ProcessTaskQueue_HookTarget();
		}

		void QueueTask(TaskDelegate * cmd)
		{
			s_taskQueueLock.Enter();
			s_tasks.push(cmd);
			s_taskQueueLock.Leave();
		}

		static BSTaskPool *	GetSingleton(void)
		{
			return *((BSTaskPool **)0x01B38308);
		}
	private:
		DEFINE_MEMBER_FN(ProcessTaskQueue_HookTarget, void, 0x006A0920);
	};

	void AddTask(TaskDelegate * task)
	{
		BSTaskPool * taskPool = BSTaskPool::GetSingleton();
		if (taskPool) 
		{
			taskPool->QueueTask(task);
		}
	}

	void Init()
	{
		WriteRelCall(0x0069D153, GetFnAddr(&BSTaskPool::ProcessTasks));
	}
}



