#include <Engine/Graphics/GPUDevice.h>

namespace Spikey
{
	void GPUTask::Enqueue()
	{
		m_State = State::Queued;

		GPUTaskManager& manager = GPUDevice::Instance->GetTaskManager();
		manager.m_Queue.enqueue(this);
	}

	void GPUTask::Execute(GPUCommandContext* context)
	{
		assert(context && m_State == State::Queued);

		bool failed = OnExecute(context);
		m_State = failed ? State::Failed : State::Finished;
	}

	void GPUTaskManager::FrameBegin()
	{
		const uint32 maxTaskCount = 32;

		GPUTask* taskBuffer[maxTaskCount] = {};
		uint32 takenTasksCount = (uint32)m_Queue.try_dequeue_bulk(taskBuffer, maxTaskCount);

		for (uint32 i = 0; i < takenTasksCount; i++)
		{
			GPUTask* task = taskBuffer[i];
			if (task->GetState() == GPUTask::State::Queued)
			{
				task->Execute(GPUDevice::Instance->GetMainContext());
			}

			delete task;
		}
	}

	void GPUTaskManager::Dispose()
	{
		GPUTask* tasks[16];
		uint32 count;

		while ((count = m_Queue.try_dequeue_bulk(tasks, 16)) != 0)
		{
			for (uint32 i = 0; i < count; i++)
			{
				delete tasks[i];
			}
		}
	}
}