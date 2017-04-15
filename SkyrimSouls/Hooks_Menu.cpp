#include "Hooks_Menu.h"
#include "SkyrimSouls.h"
#include "GFxEvent.h"
#include <SKSE/SafeWrite.h>
#include <map>
using std::map;

const UInt32 kRegisterMenuHook_Ent = 0x00A5D2A0;
const UInt32 kRegisterMenuHook_Ret = 0x00A5D2A7;

class MenuManagerEx : public MenuManager
{
	//typedef void(__thiscall MenuManagerEx::*_RegisterMenu)(const char*, CreatorFunc);
	void CustomRegisterMenu(const char* name, CreatorFunc creator);
	void OriginalRegisterMenu(const char* name, CreatorFunc creator);
};

void MenuManagerEx::CustomRegisterMenu(const char* name, CreatorFunc creator)
{
	//_RegisterMenu OriginalRegisterMenu = nullptr;
	_MESSAGE("REGISTER MENU: %s", name);
	this->OriginalRegisterMenu(name, creator);
}

__declspec(naked) void MenuManagerEx::OriginalRegisterMenu(const char* name, CreatorFunc creator)
{
	__asm
	{
		sub esp, 0x8
		mov eax, dword ptr [esp + 0x10]
		jmp [kRegisterMenuHook_Ret]
	}
}

__declspec(naked) void Hooked_OriginalRegisterMenu()
{
	__asm
	{
		mov eax, dword ptr [esp + 0x8]
		push eax
		mov eax, dword ptr [esp + 0x8]
		push eax
		call MenuManagerEx::CustomRegisterMenu
		retn 0x8
	}
}

bool __stdcall IsInItemMenuMode()
{
	MenuManager* manager = MenuManager::GetSingleton();
	UIStringHolder* holder = UIStringHolder::GetSingleton();
	static BSFixedString customMenu("CustomMenu");
	if (manager && holder && (manager->IsMenuOpen(holder->favoritesMenu) || manager->IsMenuOpen(holder->inventoryMenu) || manager->IsMenuOpen(holder->containerMenu) || manager->IsMenuOpen(holder->magicMenu) || manager->IsMenuOpen(&customMenu)))
		return true;
	return false;
}

const UInt32 kWaitForSingleObjectHook_Ent = 0x00A59089;
const UInt32 kWaitForSingleObjectHook_Ret = 0x00A5908E;

__declspec(naked) void Hooked_WaitForSingleObject()
{
	__asm
	{
		mov edx, dword ptr [eax + ecx * 4]
		push eax
		pushad
		call IsInItemMenuMode
		mov [esp + 0x20], eax
		popad
		cmp dword ptr [esp], 0x0
		pop eax
		jnz NonZero
		push 0xFFFFFFFF
		jmp [kWaitForSingleObjectHook_Ret]
	NonZero:
		push 0x75
		jmp [kWaitForSingleObjectHook_Ret]
	}
}

typedef UInt32(__fastcall *_ProcessMessage)(IMenu* menu, void* unk, UIMessage* message);

map<UInt32, UInt32> menuHandlers;

UInt32 __fastcall Hooked_ProcessMessage(IMenu* menu, void* unk, UIMessage* message)
{
	static MenuManager* mm = MenuManager::GetSingleton();
	if (message->message == UIMessage::kMessage_Open)//{}
	{
		_MESSAGE("OPEN MESSAGE RECEIVED: (name=%s, vtbl=0x%08X, fn=0x%08X, view=%p)", message->strData.data, *(UInt32*)menu, *(*(UInt32**)menu + 0x4), menu->view);
		auto it = menuHandlers.find(*(UInt32*)menu);
		if (it != menuHandlers.end())
		{
			_ProcessMessage ProcessMessage = (_ProcessMessage)(it->second);
			UInt32 result = ProcessMessage(menu, NULL, message);
			*(UInt32*)((char*)mm + 0xC8) -= 1;
			return result;
		}
		_MESSAGE("Error:can't find menu's process function in map container...");
		return NULL;
	}
	else if (message->message == UIMessage::kMessage_Close)//{}
	{
		_MESSAGE("CLOSE MESSAGE RECEIVED: (name=%s)", message->strData.data);
		auto it = menuHandlers.find(*(UInt32*)menu);
		if (it != menuHandlers.end())
		{
			*(UInt32*)((char*)mm + 0xC8) += 1;
			_ProcessMessage ProcessMessage = (_ProcessMessage)(it->second);
			UInt32 result = ProcessMessage(menu, NULL, message);
			return result;
		}
		_MESSAGE("Error:can't find menu's process function in map container...");
		return NULL;
	}
	auto it = menuHandlers.find(*(UInt32*)menu);
	if (it != menuHandlers.end())
	{
		_ProcessMessage ProcessMessage = (_ProcessMessage)(it->second);
		UInt32 result = ProcessMessage(menu, NULL, message);
	}
	_MESSAGE("Error:can't find menu's process function in map container...");
	return NULL;
}



void Hooks_Menu_Commit()
{
	WriteRelJump(kRegisterMenuHook_Ent, (UInt32)Hooked_OriginalRegisterMenu);
	SafeWrite16(kRegisterMenuHook_Ent + 0x5, 0x9090);

	WriteRelJump(kWaitForSingleObjectHook_Ent, (UInt32)Hooked_WaitForSingleObject);
	
	//menuHandlers[0x010E6B34] = 0x0087B2B0;//MessageBoxMenu
	menuHandlers[0X010E8140] = 0x00895170;//TweenMenu
	menuHandlers[0x010E5B90] = 0x0086BA80;//InventoryMenu
	menuHandlers[0x010E6594] = 0x00875E40;//MagicMenu
	menuHandlers[0x010E4098] = 0x0084B970;//ContainerMenu
	menuHandlers[0x010E4324] = 0x00858FF0;//CraftingMenu
	menuHandlers[0x010E6308] = 0x008723B0;//LockpickingMenu
	menuHandlers[0x010E3AA4] = 0x00846680;//BookMenu
	menuHandlers[0x010E95B4] = 0x008A2B60;//MapMenu
	menuHandlers[0x010E7AD4] = 0x00890E90;//StatsMenu
	//menuHandlers[0x010E3CB4] = 0x00848230;//ConsoleMenu
	for (const auto& element : menuHandlers)
	{
		SafeWrite32(element.first + 0x10, (UInt32)Hooked_ProcessMessage);
	}

	/*
	GetInfomation("MessageBoxMenu", 0x010e6b34);
	GetInfomation("TweenMenu", 0x010e8140);
	GetInfomation("InventoryMenu", 0x010e5b90);
	GetInfomation("MagicMenu", 0x010e6594);
	GetInfomation("ContainerMenu", 0x010e4098);
	GetInfomation("CraftingMenu", 0x010e4324);
	//GetInfomation("BarterMenu", 0x010e4e18);
	GetInfomation("LockpickingMenu", 0x010e6308);
	GetInfomation("BookMenu", 0x010e3aa4);
	GetInfomation("MapMenu", 0x010e95b4);
	GetInfomation("StatsMenu", 0x010e7ad4);
	GetInfomation("ConsoleMenu", 0x010e3cb4);
	*/
}


/*
const IntPtr IMenuVTable_MessageBoxMenu		= 0x010e6b34;
const IntPtr IMenuVTable_TweenMenu			= 0x010e8140;
const IntPtr IMenuVTable_InventoryMenu		= 0x010e5b90;
const IntPtr IMenuVTable_MagicMenu			= 0x010e6594;
const IntPtr IMenuVTable_ContainerMenu		= 0x010e4098;
const IntPtr IMenuVTable_FavoritesMenu		= 0x010e4e18;
const IntPtr IMenuVTable_CraftingMenu		= 0x010e4324;
const IntPtr IMenuVTable_BarterMenu			= nullptr;
const IntPtr IMenuVTable_TrainingMenu		= nullptr;
const IntPtr IMenuVTable_LockpickingMenu	= 0x010e6308;
const IntPtr IMenuVTable_BookMenu			= 0x010e3aa4;
const IntPtr IMenuVTable_MapMenu			= 0x010e95b4;
const IntPtr IMenuVTable_StatsMenu			= 0x010e7ad4;

const IntPtr IMenuVTable_HUDMenu			= 0x010e537c;
const IntPtr IMenuVTable_ConsoleMenu		= 0x010e3cb4;
const IntPtr IMenuVTable_CursorMenu			= 0x010e4bd4;









void GetInfomation(std::string name, UInt32 addr)
{
_MESSAGE("name=%s, vtbl=0x%08X, offset = 0x%08X, fn=0x%08X", name.c_str(), addr, (UInt32)((UInt32*)addr + 0x4), *((UInt32*)addr + 0x4));
}
*/