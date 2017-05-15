#pragma once
#include <SKSE/PluginAPI.h>
#include <Skyrim/Memory.h>
#include <functional>

namespace PauseTaskInterface
{
	void Init();

	void AddTask(TaskDelegate * task);
}

class CallbackDelegate //create a template in future.
{
public:
	TES_FORMHEAP_REDEFINE_NEW();

	enum TaskType
	{
		kType_Normal,
		kType_Pause,
		kType_UI
	};

	CallbackDelegate(std::function<void(void)>&& delegate) : m_callback(delegate)
	{
	}

	virtual void Run()
	{
		m_callback();
	}
	virtual void Dispose()
	{
		delete this;
	}

	template<std::size_t TYPE>
	static void Register(std::function<void(void)> callback)
	{
		_MESSAGE("UNDEFINED TASK TYPE.");
	}

	template <>
	static void Register<kType_Normal>(std::function<void(void)> callback)
	{
		const SKSEPlugin *plugin = SKSEPlugin::GetSingleton();
		const SKSETaskInterface *task = plugin->GetInterface(SKSETaskInterface::Version_2);
		if (task)
		{
			CallbackDelegate *delg = new CallbackDelegate(std::move(callback));
			task->AddTask(reinterpret_cast<TaskDelegate*>(delg));
		}
	}

	template <>
	static void Register<kType_Pause>(std::function<void(void)> callback)
	{
		CallbackDelegate *delg = new CallbackDelegate(std::move(callback));
		PauseTaskInterface::AddTask(reinterpret_cast<TaskDelegate*>(delg));
	}

	template <>
	static void Register<kType_UI>(std::function<void(void)> callback)
	{
		const SKSEPlugin *plugin = SKSEPlugin::GetSingleton();
		const SKSETaskInterface *task = plugin->GetInterface(SKSETaskInterface::Version_2);
		if (task)
		{
			CallbackDelegate *delg = new CallbackDelegate(std::move(callback));
			task->AddUITask(reinterpret_cast<UIDelegate*>(delg));
		}
	}


private:
	std::function<void(void)>		m_callback;
};