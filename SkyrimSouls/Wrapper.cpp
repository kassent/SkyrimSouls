#include "Wrapper.h"
#include "Settings.h"
//#include <SKSE/GameEvents.h>
//#include <SKSE/GameRTTI.h>
//#include <Skyrim/Forms/PlayerCharacter.h>
#include <SKyrim/BSDevices/PlayerControls.h>
#include <vector>
#include <string>
#include <algorithm>
//#include <windows.h>

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


class MenuOpenCloseEventHandler : public BSTEventSink<MenuOpenCloseEvent>
{
public:
	static SInt32 unpausedCount;

	virtual EventResult ReceiveEvent(MenuOpenCloseEvent *evn, BSTEventSource<MenuOpenCloseEvent> *src) override
	{
		//_MESSAGE("menu: %s, isOpening: %d, count: %d", evn->menuName.c_str(), evn->opening, unpausedCount);
		auto it = settings.m_menuConfig.find(std::string(evn->menuName.c_str()));
		if (it != settings.m_menuConfig.end())
		{
			MenuManager* mm = MenuManager::GetSingleton();
			UIStringHolder* holder = UIStringHolder::GetSingleton();

			if (evn->opening)
			{
				if (mm->IsMenuOpen(holder->dialogueMenu))
					mm->CloseMenu(holder->dialogueMenu);
				if (unpausedCount++ == 0)
				{
					PlayerControls* control = PlayerControls::GetSingleton();
					for (auto element : control->handlers)
						*((UInt32*)element + 1) = false;
				}
			}
			else
			{
				if (--unpausedCount < 0)
					unpausedCount = 0;
				if (unpausedCount == 0)
				{
					PlayerControls* control = PlayerControls::GetSingleton();
					for (auto element : control->handlers)
						*((UInt32*)element + 1) = true;
				}
				if (evn->menuName == holder->console)
				{
					IMenu* console = mm->GetMenu(holder->console);
					if (console != nullptr)
					{
						if (settings.m_menuConfig["Console"])
							console->flags &= ~IMenu::kType_PauseGame;
						else
							console->flags |= IMenu::kType_PauseGame;
					}
				}
			}
#ifdef DEBUG_LOG
			_MESSAGE("menu: %s, isOpening: %d, count: %d", evn->menuName.c_str(), evn->opening, unpausedCount);
#endif
		}
		return kEvent_Continue;
	}


	static MenuOpenCloseEventHandler* GetSingleton()
	{
		static MenuOpenCloseEventHandler instance;
		return &instance;
	}
};

SInt32 MenuOpenCloseEventHandler::unpausedCount = 0;


void OnInit(SKSEMessagingInterface::Message* msg)
{
	if (msg->type == SKSEMessagingInterface::kMessage_DataLoaded)
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
		//Wrapper<bookMenu>::GetSingleton()->RegisterMenu();

		MenuManager* mm = MenuManager::GetSingleton();
		if (mm)
		{
			BSTEventSource<MenuOpenCloseEvent>* eventDispatcher = mm->GetMenuOpenCloseEventSource();
			eventDispatcher->AddEventSink(MenuOpenCloseEventHandler::GetSingleton());
		}
		_MESSAGE("");
		_MESSAGE("Init complete...");
	}
}
