#include "skse/GameMenus.h"
#include "BSFixedString.h"

namespace SkyrimSouls
{
	typedef tHashSet<MenuTableItem, BSFixedString> MenuTable;

	typedef IMenu* (*MenuConstructor)(void);
	class CustomMenuManager
	{
	public:

		tArray<IMenu*>			menuStack;					// 094
		MenuTable				menuTable;					// 0A0
		UInt32					unk_0C0;					// 0C0 (= 0)
		UInt32					unk_0C4;					// 0C4 (= 0)
		UInt32					numPauseGame;				// 0C8 (= 0) += 1 if (imenu->flags & 0x0001)
		UInt32					numItemMenu;				// 0CC (= 0) += 1 if (imenu->flags & 0x2000)
		UInt32					numPreventGameLoad;			// 0D0 (= 0) += 1 if (imenu->flags & 0x0080)
		UInt32					numDoNotPreventSaveGame;	// 0D4 (= 0) += 1 if (imenu->flags & 0x0800)
		UInt32					numStopCrosshairUpdate;		// 0D8 (= 0) += 1 if (imenu->flags & 0x4000)
		UInt32					numFlag8000;				// 0DC (= 0) += 1 if (imenu->flags & 0x8000)
		UInt32					numModal;					// 0E0 (= 0)  = 1 if (imenu->flags & 0x0010)
		UInt32					unk_0E4;					// 0E4

		static CustomMenuManager* GetSingleton()
		{
			return reinterpret_cast<CustomMenuManager*>(reinterpret_cast<UInt32>(MenuManager::GetSingleton()) + 0x94);
		}
	};


	IMenu* CreateMenuss();

	void LookUpMenuTable();
}



//--StatsMenu:				0x0088D340
//--FavoritesMenu:			0x0085BF50
//--Journal Menu:			0x008AC1B0
//--ContainerMenu:			0x008497D0
//Credits Menu:				0x008598A0
//Main Menu:				0x00876FC0
//--InventoryMenu:			0x0086A3F0
//--LevelUp Menu:			0x0086E8B0
//TitleSequence Menu:		0x008934B0
//Tutorial Menu:			0x008945F0
//Console Native UI Menu:	0x008488C0
//--Lockpicking Menu:		0x00871FD0
//--RaceSex Menu:			0x00885C90
//SafeZoneMenu:				0x00887980
//Fader Menu:				0x0085B020
//Sleep/Wait Menu:			0x00887E60
//--Mist Menu:				0x0087C920
//Kinect Menu:				0x0086E5E0
//--MessageBoxMenu:			0x0087A7D0
//--Book Menu:				0x00845C60
//Loading Menu:				0x0086F290
//--Training Menu:			0x00893FC0
//--Crafting Menu:			0x0084ECD0
//Cursor Menu:				0x00859B70
//--TweenMenu:				0x00895870
//--MapMenu:				0x008A1D90
//--BarterMenu:				0x00842B30
//--Console:				0x00847280
//--GiftMenu:				0x0085D1E0
//--MagicMenu:				0x00873D10
//Dialogue Menu:			0x0085A2E0
//HUD Menu:					0x00865F50
