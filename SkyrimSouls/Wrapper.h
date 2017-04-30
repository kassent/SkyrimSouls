#pragma once
#include <SKSE/PluginAPI.h>
#include <SKSE/GameMenus.h>
#include <SKSE/SafeWrite.h>
#include "Settings.h"

template<BSFixedString& N>
class Wrapper
{
public:
	typedef MenuManager::FnMenuConstructor CTOR;

	Wrapper() : m_ctor(nullptr), m_instance(nullptr){}

	bool RegisterMenu()
	{
		MenuManager* mm = MenuManager::GetSingleton();
		MenuManager::MenuTable* menuTable = &mm->menuTable;
		auto it = menuTable->find(N);
		if (it != menuTable->end())
		{
			m_ctor = it->value.menuConstructor;
			//_MESSAGE("Wrapper<%s>::Register()		ctor: %p", N.c_str(), m_ctor);
			_MESSAGE("Wrapper<%s>::Register()", N.c_str());
			it->value.menuConstructor = CreateIMenu;
			return true;
		}
		return false;
	}

	static IMenu* CreateIMenu()
	{
		CTOR ctor = GetSingleton()->m_ctor;
		IMenu* menu = nullptr;
		if (ctor != nullptr)
		{
			menu = (ctor)();
			if (menu != nullptr)
			{
				GetSingleton()->m_instance = menu;
				if (settings.m_menuConfig[N.c_str()])
				{
					menu->flags &= ~IMenu::kType_PauseGame;
					menu->flags &= ~IMenu::kType_StopDrawingWorld;
					menu->flags |= IMenu::kType_StopCrosshairUpdate;
					//menu->flags |= IMenu::kType_ItemMenu;
					if (menu->menuDepth < 0x3)
					{
						menu->menuDepth = 0x3;
					}
					GFxMovieView* view = menu->GetMovieView();
					if (view != nullptr)
					{
						view->SetPause(false);
					}
				}
			}
		}
		return menu;
	}

	static Wrapper* GetSingleton()
	{
		static Wrapper instance;
		return &instance;
	}

	CTOR				m_ctor;
	IMenu*				m_instance;
};



void RegisterMenu();