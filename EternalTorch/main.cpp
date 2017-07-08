
#include <SKSE.h>
#include <SKSE/PluginAPI.h>
#include <SKSE/GameData.h>
#include <Skyrim/Forms/TESObjectLIGH.h>

//========================================================
// skse plugin
//========================================================
void OnInit(SKSEMessagingInterface::Message* msg)
{
	if (msg->type == SKSEMessagingInterface::kMessage_DataLoaded)
	{
		TESDataHandler* pDataHandler = TESDataHandler::GetSingleton();
		for (auto& light : pDataHandler->lights)
		{
			if (light->CanBeCarried())
			{
				light->unk78.time = 0.0f;
			}
		}
		_MESSAGE("");
		_MESSAGE("Init complete...");
	}
}

class EternalTorch : public SKSEPlugin
{
public:
	EternalTorch()
	{
	}

	virtual bool InitInstance() override
	{
		if (!Requires(kSKSEVersion_1_7_3, SKSEMessagingInterface::Version_2, SKSETaskInterface::Version_2))
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

	}
} thePlugin;