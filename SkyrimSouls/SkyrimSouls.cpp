#include "SkyrimSouls.h"

namespace SkyrimSouls
{
	UInt32 addr;
	IMenu* CreateMenuss()
	{
		MenuConstructor construstor = reinterpret_cast<MenuConstructor>(addr);
		IMenu* menu = construstor();
		if (menu)
		{
			_MESSAGE("111111111111111111111111");
			menu->flags &= ~IMenu::kType_PauseGame;
		}
		return menu;
	}

	void LookUpMenuTable()
	{
		UIStringHolder* uiStringHolder = UIStringHolder::GetSingleton();
		MenuManager* manager = MenuManager::GetSingleton();
		MenuTable* table = (MenuTable*)((char*)manager + 0xA4);
		for (size_t i = 0; i < table->count(); ++i)
		{
			table->GetAt(i);
		}



		MenuTableItem* item = table->Find(((BSFixedString*)(&uiStringHolder->inventoryMenu)));
		if (item)
		{
			_MESSAGE("..................%08X", (UInt32)item->menuConstructor);
			addr = (UInt32)item->menuConstructor;
			item->menuConstructor = (void*)CreateMenuss;
		}
	}
}


