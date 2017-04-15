#include <SKSE.h>
#include <SKSE/PluginAPI.h>
#include <SKSE/DebugLog.h>
#include <SKSE/GameRTTI.h>
#include "LootMenu.h"
#include "Settings.h"
#include "Console.h"
#include "Hooks.h"

//========================================================
// skse plugin
//========================================================

class QuickLootPlugin : public SKSEPlugin
{
public:
	QuickLootPlugin()
	{
	}

	virtual bool InitInstance() override
	{
		if (!Requires(kSKSEVersion_1_7_1, SKSETaskInterface::Version_2))
		{
			gLog << "ERROR: your skse version is too old." << std::endl;
			return false;
		}

		SetName("QuickLoot");
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
		Settings::Load();

		if (LootMenu::Install())
		{
			Hooks::Install();
			if (Settings::bUseConsole)
				ConsoleCommand::Register();
		}
	}
} thePlugin;
