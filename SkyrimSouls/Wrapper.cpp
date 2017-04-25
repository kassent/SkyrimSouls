#include "Wrapper.h"

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
	//Wrapper<sleepWaitMenu>::GetSingleton()->RegisterMenu();
}

