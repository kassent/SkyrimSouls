#include <SKSE.h>
#include <SKSE/PluginAPI.h>


//========================================================
// skse plugin
//========================================================


class ImmersiveMerchant : public SKSEPlugin
{
public:
	ImmersiveMerchant()
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
		SetVersion(20);

		return true;
	}

	virtual bool OnLoad() override
	{
		SKSEPlugin::OnLoad();

		const SKSEMessagingInterface *message = GetInterface(SKSEMessagingInterface::Version_2);
		//message->RegisterListener("SKSE", OnInit);

		_MESSAGE("");

		return true;
	}

	virtual void OnModLoaded() override
	{

	}
} thePlugin;

