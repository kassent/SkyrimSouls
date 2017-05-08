#pragma once

class TaskDelegate;

namespace PauseTaskInterface
{
	void Init();

	void AddTask(TaskDelegate * task);
}