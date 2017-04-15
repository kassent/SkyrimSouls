#include <SKSE.h>
#include <SKSE/PluginAPI.h>
#include <SKSE/DebugLog.h>
#include "Hook_Shouts.h"
#include "Serialization.h"

//========================================================
// skse plugin
//========================================================

void Serialization_Save(SKSESerializationInterface * intfc)
{
	_MESSAGE("Saving...");

	storageData.Save(intfc, SKSESerializationInterface::Version_4);

	_MESSAGE("%s - Serialized string table", __FUNCTION__);
}

void Serialization_Load(SKSESerializationInterface * intfc)
{
	_MESSAGE("Loading...");

	UInt32 type, length, version;
	bool error = false;

	while (intfc->GetNextRecordInfo(&type, &version, &length))
	{
		switch (type)
		{
		case 'ISCD':
			storageData.Load(intfc, version);
			_MESSAGE("Loading shout map data...");
			break;
		default:
			_MESSAGE("unhandled type %08X (%.4s)", type, &type);
			error = true;
			break;
		}
	}
}

void Serialization_Revert(SKSESerializationInterface * intfc)
{
	_MESSAGE("Reverting...");
	storageData.Revert();
}



class IndividualizedShoutCooldowns : public SKSEPlugin
{
public:
	IndividualizedShoutCooldowns()
	{
	}

	virtual bool InitInstance() override
	{
		if (!Requires(kSKSEVersion_1_7_1, SKSESerializationInterface::Version_4))
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

		const SKSESerializationInterface * serialization = GetInterface(SKSESerializationInterface::Version_4);
		serialization->SetUniqueID('ISCD');
		serialization->SetLoadCallback(Serialization_Load);
		serialization->SetSaveCallback(Serialization_Save);
		serialization->SetRevertCallback(Serialization_Revert);

		_MESSAGE("");

		return true;
	}

	virtual void OnModLoaded() override
	{
		Hook_Shouts_Commit();
	}
} thePlugin;
