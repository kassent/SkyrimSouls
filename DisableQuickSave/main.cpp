#include <SKSE.h>
#include <SKSE/PluginAPI.h>
#include <SKSE/DebugLog.h>
#include <SKSE/SafeWrite.h>
#include "Console.h"

//========================================================
// skse plugin
//========================================================

class DisableQuickSave : public SKSEPlugin
{
public:
	DisableQuickSave()
	{
	}

	virtual bool InitInstance() override
	{
		SetName("DisableQuickSave");
		SetVersion(1);

		return true;
	}

	virtual bool OnLoad() override
	{
		SKSEPlugin::OnLoad();

		_MESSAGE("");

		return true;
	}

	virtual void OnModLoaded() override
	{
		SafeWrite8(0x00878BB8, 0x2);
		SafeWrite8(0x00878BF3, 0x4);
		ConsoleCommand::Register();
	}
} thePlugin;
