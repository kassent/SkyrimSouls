#include "Console.h"
#include "Settings.h"
#include "Wrapper.h"
#include <SKSE/CommandTable.h>
#include <SKSE/SafeWrite.h>
#include <Skyrim/Menus/Console.h>
#include <Skyrim/Forms/PlayerCharacter.h>

static bool Cmd_SetSkyrimSoulsVariable_Execute(const SCRIPT_PARAMETER *paramInfo, CommandInfo::ScriptData *scriptData, TESObjectREFR *thisObj, TESObjectREFR *containingObj, Script *scriptObj, ScriptLocals *locals, double &result, UInt32 &opcodeOffsetPtr)
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

			if (settings.Set(name.c_str(), val))
			{
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


static bool Cmd_CenterOnCell_Execute(const SCRIPT_PARAMETER *paramInfo, CommandInfo::ScriptData *scriptData, TESObjectREFR *thisObj, TESObjectREFR *containingObj, Script *scriptObj, ScriptLocals *locals, double &result, UInt32 &opcodeOffsetPtr)
{
	if (scriptData->strLen < 60)
	{
		CommandInfo::StringChunk *strChunk = (CommandInfo::StringChunk*)scriptData->GetChunk();
		std::string name = strChunk->GetString();

		if (name.length() > 1)
		{
			MenuManager* mm = MenuManager::GetSingleton();
			UIStringHolder* holder = UIStringHolder::GetSingleton();

			if (mm->IsMenuOpen(holder->console) && !(mm->GetMenu(holder->console)->flags & IMenu::kType_PauseGame) && (1 == 0))
			{
				ConsoleManager *console = ConsoleManager::GetSingleton();
				if (console && ConsoleManager::IsConsoleMode())
				{
					console->Print("> This command is disabled when Console is in unpaused state.Please type \"sssv Console 0\" to disable Console at runtime.");
				}
			}
			else
			{
				g_thePlayer->CenterOnCell(name.c_str(), 0);
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
		//00536DD0
		//WriteRelJump(0x00536DD0, (UInt32)Cmd_CenterOnCell_Execute);

		CommandInfo *info;

		info = CommandInfo::Create();
		if (info)
		{
			static SCRIPT_PARAMETER params[] = {
				{ "String", SCRIPT_PARAMETER::kType_String, 0 },
				{ "Integer", SCRIPT_PARAMETER::kType_Integer, 0 }
			};
			info->longName = "SetSkyrimSoulsVariable";
			info->shortName = "sssv";
			info->helpText = "";
			info->isRefRequired = false;
			info->SetParameters(params);
			info->execute = &Cmd_SetSkyrimSoulsVariable_Execute;
			info->eval = nullptr;

			DebugLog(info);
		}

		_MESSAGE("");

		//CommandInfo *commands = *(CommandInfo**)0x00516B0B;
		//const UInt32 maxIdx = *(UInt32*)(0x00516C10 + 6);
		//for (int idx = 0; idx <= maxIdx; ++idx)
		//{
		//	CommandInfo *p = &commands[idx];
		//	if (p->longName && p->longName[0])
		//	{
		//		_MESSAGE("longName: %s", p->longName);
		//		_MESSAGE("shortName: %s", p->shortName);
		//		_MESSAGE("execute: %p", p->execute);
		//		_MESSAGE("");
		//	}
		//}

	}
}
