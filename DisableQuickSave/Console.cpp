#include "Console.h"
#include <SKSE.h>
#include <SKSE/CommandTable.h>
#include <SKSE/DebugLog.h>
#include <SKSE/SafeWrite.h>
#include <Skyrim/Menus/Console.h>


static bool Cmd_EnableQuickSave_Execute(const SCRIPT_PARAMETER *paramInfo, CommandInfo::ScriptData *scriptData, TESObjectREFR *thisObj, TESObjectREFR *containingObj, Script *scriptObj, ScriptLocals *locals, double &result, UInt32 &opcodeOffsetPtr)
{
	SafeWrite8(0x00878BB8, 0x8);
	SafeWrite8(0x00878BF3, 0x10);
	ConsoleManager::GetSingleton()->Print("Pressing F5 will create a quick save from now on.");
	return true;
}


static bool Cmd_DisableQuickSave_Execute(const SCRIPT_PARAMETER *paramInfo, CommandInfo::ScriptData *scriptData, TESObjectREFR *thisObj, TESObjectREFR *containingObj, Script *scriptObj, ScriptLocals *locals, double &result, UInt32 &opcodeOffsetPtr)
{
	SafeWrite8(0x00878BB8, 0x2);
	SafeWrite8(0x00878BF3, 0x4);
	ConsoleManager::GetSingleton()->Print("Pressing F5 will create a normal save from now on.");
	return true;
}

static void DebugLog(const CommandInfo *info)
{
	gLog << "  registered: ";
	gLog << info->longName;
	if (info->shortName && info->shortName[0])
	{
		gLog << " (" << info->shortName << ")";
	}
	gLog << std::endl;
}


namespace ConsoleCommand
{
	void Register()
	{
		_MESSAGE("ConsoleCommand::Register()");

		CommandInfo *info;
		info = CommandInfo::Create();
		if (info)
		{
			info->longName = "EnableQuickSave";
			info->shortName = "enableqs";
			info->helpText = "";
			info->isRefRequired = false;
			info->SetParameters();
			info->execute = &Cmd_EnableQuickSave_Execute;
			info->eval = nullptr;

			DebugLog(info);
		}

		info = CommandInfo::Create();
		if (info)
		{
			info->longName = "DisableQuickSave";
			info->shortName = "disableqs";
			info->helpText = "";
			info->isRefRequired = false;
			info->SetParameters();
			info->execute = &Cmd_DisableQuickSave_Execute;
			info->eval = nullptr;

			DebugLog(info);
		}

		_MESSAGE("");
	}
}
