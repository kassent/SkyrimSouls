#include <SKSE.h>
#include <SKSE/PluginAPI.h>
#include <SKSE/DebugLog.h>
#include "Wrapper.h"
#include "Console.h"
#include "Hook_Game.h"

//========================================================
// skse plugin
//========================================================

void OnInit(SKSEMessagingInterface::Message* msg)
{
	if (msg->type == SKSEMessagingInterface::kMessage_DataLoaded)
	{
		RegisterMenu();
		RegisterEventHandler();

		_MESSAGE("");
		_MESSAGE("Init complete...");

		_MESSAGE("");
	}
}

class SkyrimSoulsPlugin : public SKSEPlugin
{
public:
	SkyrimSoulsPlugin()
	{
	}

	virtual bool InitInstance() override
	{
		if (!Requires(kSKSEVersion_1_7_1, SKSEMessagingInterface::Version_2, SKSETaskInterface::Version_2))
		{
			gLog << "ERROR: your skse version is too old." << std::endl;
			return false;
		}

		SetName(GetDllName());
		SetVersion(1);

		return true;
	}

	virtual bool OnLoad() override
	{
		SKSEPlugin::OnLoad();

		const SKSEMessagingInterface *message = GetInterface(SKSEMessagingInterface::Version_2);
		message->RegisterListener("SKSE", OnInit);

		_MESSAGE("");

		return true;
	}

	virtual void OnModLoaded() override
	{
		settings.Load();
		ConsoleCommand::Register();
		Hook_Game_Commit();
	}
} thePlugin;

