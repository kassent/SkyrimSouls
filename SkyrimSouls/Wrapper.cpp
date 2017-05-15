#include "Wrapper.h"
#include "Settings.h"
#include <SKSE/PluginAPI.h>
#include <SKSE/GameMenus.h>
#include <SKSE/SafeWrite.h>
#include <Skyrim/Forms/PlayerCharacter.h>

BSFixedString console = "Console";
BSFixedString tutorialMenu = "Tutorial Menu";
BSFixedString messageBoxMenu = "MessageBoxMenu";
BSFixedString tweenMenu = "TweenMenu";
BSFixedString inventoryMenu = "InventoryMenu";
BSFixedString magicMenu = "MagicMenu";
BSFixedString containerMenu = "ContainerMenu";
BSFixedString favoritesMenu = "FavoritesMenu";
BSFixedString barterMenu = "BarterMenu";
BSFixedString trainingMenu = "Training Menu";
BSFixedString lockpickingMenu = "Lockpicking Menu";
BSFixedString bookMenu = "Book Menu";
BSFixedString mapMenu = "MapMenu";
BSFixedString statsMenu = "StatsMenu";
BSFixedString giftMenu = "GiftMenu";
BSFixedString journalMenu = "Journal Menu";
BSFixedString customMenu = "CustomMenu";
BSFixedString sleepWaitMenu = "Sleep/Wait Menu";				// 09C "Sleep/Wait Menu"


template<BSFixedString& N>
class Wrapper
{
public:
	typedef MenuManager::FnMenuConstructor CTOR;

	Wrapper() : m_ctor(nullptr), m_instance(nullptr) {}

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
				GetSingleton()->m_instance = menu;//g_thePlayer->IsInCombat()
				//auto setting = settings.m_menuConfig[N.c_str()];
				if (settings.m_menuConfig[N.c_str()])
				{
					menu->flags &= ~IMenu::kType_PauseGame;
					menu->flags &= ~IMenu::kType_StopDrawingWorld;
					menu->flags |= IMenu::kType_StopCrosshairUpdate;

					if (menu->menuDepth < 0x3)
					{
						menu->menuDepth = 0x3;
					}
					GFxMovieView* view = menu->GetMovieView();
					if (view != nullptr)
					{
						view->SetPause(false);	// for book menu
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


void RegisterMenu()
{
	Wrapper<console>::GetSingleton()->RegisterMenu();
	Wrapper<tutorialMenu>::GetSingleton()->RegisterMenu();
	Wrapper<messageBoxMenu>::GetSingleton()->RegisterMenu();
	Wrapper<tweenMenu>::GetSingleton()->RegisterMenu();
	Wrapper<inventoryMenu>::GetSingleton()->RegisterMenu();
	Wrapper<magicMenu>::GetSingleton()->RegisterMenu();
	Wrapper<containerMenu>::GetSingleton()->RegisterMenu();
	Wrapper<favoritesMenu>::GetSingleton()->RegisterMenu();
	Wrapper<barterMenu>::GetSingleton()->RegisterMenu();
	Wrapper<trainingMenu>::GetSingleton()->RegisterMenu();
	Wrapper<giftMenu>::GetSingleton()->RegisterMenu();
	Wrapper<customMenu>::GetSingleton()->RegisterMenu();
	Wrapper<journalMenu>::GetSingleton()->RegisterMenu();
	Wrapper<sleepWaitMenu>::GetSingleton()->RegisterMenu();
}

