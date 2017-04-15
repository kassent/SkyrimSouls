#include "Console.h"
#include "LootMenu.h"
#include "Settings.h"
#include <SKSE/CommandTable.h>
#include <SKSE/PluginAPI.h>
#include <SKSE/DebugLog.h>

static bool Cmd_StartQuickLoot_Execute(const SCRIPT_PARAMETER *paramInfo, CommandInfo::ScriptData *scriptData, TESObjectREFR *thisObj, TESObjectREFR *containingObj, Script *scriptObj, ScriptLocals *locals, double &result, UInt32 &opcodeOffsetPtr)
{
	LootMenu *lootMenu = LootMenu::GetSingleton();

	if (!lootMenu)
		LootMenu::SendRequestStart();

	return true;
}


static bool Cmd_StopQuickLoot_Execute(const SCRIPT_PARAMETER *paramInfo, CommandInfo::ScriptData *scriptData, TESObjectREFR *thisObj, TESObjectREFR *containingObj, Script *scriptObj, ScriptLocals *locals, double &result, UInt32 &opcodeOffsetPtr)
{
	LootMenu *lootMenu = LootMenu::GetSingleton();

	if (lootMenu)
		LootMenu::SendRequestStop();

	return true;
}


static bool Cmd_DumpQuickLoot_Execute(const SCRIPT_PARAMETER *paramInfo, CommandInfo::ScriptData *scriptData, TESObjectREFR *thisObj, TESObjectREFR *containingObj, Script *scriptObj, ScriptLocals *locals, double &result, UInt32 &opcodeOffsetPtr)
{
	LootMenu *lootMenu = LootMenu::GetSingleton();

	if (lootMenu && lootMenu->IsOpen())
		lootMenu->Dump();

	return true;
}


static bool Cmd_SetQuickLootVariable_Execute(const SCRIPT_PARAMETER *paramInfo, CommandInfo::ScriptData *scriptData, TESObjectREFR *thisObj, TESObjectREFR *containingObj, Script *scriptObj, ScriptLocals *locals, double &result, UInt32 &opcodeOffsetPtr)
{
	if (scriptData->strLen < 60)
	{
		CommandInfo::StringChunk *strChunk = (CommandInfo::StringChunk*)scriptData->GetChunk();
		std::string name = strChunk->GetString();

		if (name.length() > 1)
		{
			CommandInfo::IntegerChunk *intChunk = (CommandInfo::IntegerChunk*)strChunk->GetNext();
			int val = intChunk->GetInteger();

			ConsoleManager *console = ConsoleManager::GetSingleton();

			if (Settings::Set(name.c_str(), val))
			{
				LootMenu *lootMenu = LootMenu::GetSingleton();
				if (lootMenu)
					lootMenu->Setup();

				if (console && ConsoleManager::IsConsoleMode())
					console->Print("> set Settings::%s=%d", name.c_str(), val);
			}
			else
			{
				if (console && ConsoleManager::IsConsoleMode())
					console->Print("> (Error) Settings::%s is not found.", name.c_str(), val);
			}

		}
	}
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
			info->longName = "StartQuickLoot";
			info->shortName = "startql";
			info->helpText = "";
			info->isRefRequired = false;
			info->SetParameters();
			info->execute = &Cmd_StartQuickLoot_Execute;
			info->eval = nullptr;

			DebugLog(info);
		}

		info = CommandInfo::Create();
		if (info)
		{
			info->longName = "StopQuickLoot";
			info->shortName = "stopql";
			info->helpText = "";
			info->isRefRequired = false;
			info->SetParameters();
			info->execute = &Cmd_StopQuickLoot_Execute;
			info->eval = nullptr;

			DebugLog(info);
		}

		info = CommandInfo::Create();
		if (info)
		{
			info->longName = "DumpQuickLoot";
			info->shortName = "dumpql";
			info->helpText = "";
			info->isRefRequired = false;
			info->SetParameters();
			info->execute = &Cmd_DumpQuickLoot_Execute;
			info->eval = nullptr;

			DebugLog(info);
		}

		info = CommandInfo::Create();
		if (info)
		{
			static SCRIPT_PARAMETER params[] = {
				{ "String", SCRIPT_PARAMETER::kType_String, 0 },
				{ "Integer", SCRIPT_PARAMETER::kType_Integer, 0 }
			};
			info->longName = "SetQuickLootVariable";
			info->shortName = "sqlv";
			info->helpText = "";
			info->isRefRequired = false;
			info->SetParameters(params);
			info->execute = &Cmd_SetQuickLootVariable_Execute;
			info->eval = nullptr;

			DebugLog(info);
		}

		_MESSAGE("");
	}
}
