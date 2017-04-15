#include <SKSE.h>
#include <SKSE/PluginAPI.h>
#include <SKSE/DebugLog.h>



//========================================================
// skse plugin
//========================================================

class SkyrimSoulsPlugin : public SKSEPlugin
{
public:
	SkyrimSoulsPlugin()
	{
	}

	virtual bool InitInstance() override
	{
		if (!Requires(kSKSEVersion_1_7_1, SKSETaskInterface::Version_2))
		{
			gLog << "ERROR: your skse version is too old." << std::endl;
			return false;
		}

		SetName("SKyrimSouls");
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

