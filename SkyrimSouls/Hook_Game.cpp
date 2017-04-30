#include "Hook_Game.h"
#include "Settings.h"
#include "Tools.h"
#include <SKSE/GameMenus.h>
#include <SKSE/SafeWrite.h>
#include <SKSE/GameRTTI.h>
#include <SKSE/PluginAPI.h>
#include <SKSE/HookUtil.h>
#include <Skyrim/BSDevices/InputDevice.h>
#include <Skyrim/BSDevices/InputManager.h>
#include <Skyrim/BSDevices/KeyCode.h>
#include <Skyrim/Forms/BGSEquipSlot.h>
#include <Skyrim/Forms/PlayerCharacter.h>
#include <Skyrim/BSDevices/InputMappingManager.h>
#include <Skyrim/BSDevices/MenuControls.h>
#include <Skyrim/Camera/PlayerCamera.h>
#include <SKyrim/BSDevices/PlayerControls.h>
#include <Skyrim/NetImmerse/NiAVObject.h>
#include <Skyrim/Forms/Character/Components/ActorProcessManager.h>
#include <Skyrim/Animation/IAnimationGraphManagerHolder.h>
#include <Skyrim/Menus/Inventory3DManager.h>
#include <Skyrim/FileIO/BGSSaveLoadManager.h>
#include <Skyrim/BSMain/Setting.h>

#include <vector>
#include <string>
#include <string>
#include <algorithm>

#define STDFN __stdcall


class MenuOpenCloseEventHandler : public BSTEventSink<MenuOpenCloseEvent>
{
public:
	static SInt32							unpausedCount;
	//127248C
	virtual EventResult ReceiveEvent(MenuOpenCloseEvent *evn, BSTEventSource<MenuOpenCloseEvent> *src) override
	{
		auto it = settings.m_menuConfig.find(std::string(evn->menuName.c_str()));
		if (it != settings.m_menuConfig.end())
		{
			MenuManager* mm = MenuManager::GetSingleton();
			UIStringHolder* holder = UIStringHolder::GetSingleton();

			if (evn->opening)
			{
				MenuControls* menuControls = MenuControls::GetSingleton();
				*((bool*)(menuControls->unk28) + 0x25) = true;

				InputMappingManager* input = InputMappingManager::GetSingleton();
				if (evn->menuName == holder->containerMenu 
					|| evn->menuName == holder->barterMenu 
					|| evn->menuName == holder->giftMenu 
					|| evn->menuName == holder->inventoryMenu 
					|| evn->menuName == holder->magicMenu)
				{
					input->DisableControl(InputMappingManager::ContextType::kContext_Gameplay);
				}
				if (unpausedCount++ == 0)
				{
					if (mm->IsMenuOpen(holder->dialogueMenu) && (evn->menuName == holder->containerMenu 
						|| evn->menuName == holder->barterMenu 
						|| evn->menuName == holder->giftMenu 
						|| evn->menuName == holder->trainingMenu))
					{
						GFxMovieView* view = mm->GetMovieView(holder->dialogueMenu);
						view->SetVisible(false);
					}
					PlayerControls* control = PlayerControls::GetSingleton();
					for (auto element : control->handlers)
					{
						*((UInt32*)element + 1) = false;
					}
				}
			}
			else
			{
				InputMappingManager* input = InputMappingManager::GetSingleton();
				if (evn->menuName == holder->containerMenu 
					|| evn->menuName == holder->barterMenu 
					|| evn->menuName == holder->giftMenu 
					|| evn->menuName == holder->inventoryMenu 
					|| evn->menuName == holder->magicMenu)
				{
					input->EnableControl(InputMappingManager::ContextType::kContext_Gameplay);
				}
				if (--unpausedCount < 0)
					unpausedCount = 0;
				if (unpausedCount == 0)
				{
					PlayerControls* control = PlayerControls::GetSingleton();
					for (auto element : control->handlers)
					{
						*((UInt32*)element + 1) = true;
					}
					MenuControls* menuControls = MenuControls::GetSingleton();
					*((bool*)(menuControls->unk28) + 0x25) = false;

					if (mm->IsMenuOpen(holder->dialogueMenu))
					{
						GFxMovieView* view = mm->GetMovieView(holder->dialogueMenu);
						if (!view->GetVisible())
						{
							view->SetVisible(true);
						}
					}
				}
				if (evn->menuName == holder->console)
				{
					IMenu* console = mm->GetMenu(holder->console);
					if (console != nullptr)
					{
						if (settings.m_menuConfig[holder->console.c_str()])
							console->flags &= ~IMenu::kType_PauseGame;
						else
							console->flags |= IMenu::kType_PauseGame;
					}
				}
			}
		}
		return kEvent_Continue;
	}

	static MenuOpenCloseEventHandler* GetSingleton()
	{
		static MenuOpenCloseEventHandler instance;
		return &instance;
	}
};
SInt32		MenuOpenCloseEventHandler::unpausedCount = 0;


void* STDFN Hook_GetFavoritesItem(void* form)
{
	MenuManager* manager = MenuManager::GetSingleton();
	UIStringHolder* holder = UIStringHolder::GetSingleton();
	if (holder && manager && manager->IsMenuOpen(holder->favoritesMenu))
	{
		form = nullptr;
	}
	return form;
}

void STDFN Hook_KillActor(Actor* actor)
{
	PlayerCharacter* player = DYNAMIC_CAST<PlayerCharacter*>(actor);
	if (player != nullptr)
	{
		bool isProcessing = false;
		MenuManager* mm = MenuManager::GetSingleton();
		for (auto pair : settings.m_menuConfig)
		{
			BSFixedString menu(pair.first.c_str());
			if (mm->IsMenuOpen(menu))
			{
				mm->CloseMenu(menu);
				isProcessing = true;
			}
		}
		if (isProcessing) Sleep(200);
	}
}

bool STDFN Hook_IsInMenuMode()
{
	MenuManager* mm = MenuManager::GetSingleton();
	for (auto pair : settings.m_menuConfig)
	{
		BSFixedString menu(pair.first.c_str());
		if (mm->IsMenuOpen(menu))
			return true;
	}
	if (*(UInt8*)0x1B2E85F || *(UInt8*)0x1B2E85E)
		return true;
	return false;
}

bool STDFN Hook_AddUIMessage(const BSFixedString& strData, UInt32 msgID)
{
	UIStringHolder* holder = UIStringHolder::GetSingleton();
	if (msgID == UIMessage::kMessage_Open && MenuOpenCloseEventHandler::unpausedCount != 0 && (strData == holder->sleepWaitMenu || strData == holder->favoritesMenu|| strData == "Loot Menu"))
	{
		return false;
	}
	return true;
}


class FavoritesHandler : public MenuEventHandler
{
public:
	typedef bool(FavoritesHandler::*FnCanProcess)(InputEvent *);

	static FnCanProcess fnCanProcess;

	bool CanProcess_Hook(InputEvent *evn)
	{
		return (this->*fnCanProcess)(evn);
	}

	static void InitHook()
	{
		SafeWrite8(0x0085BE1D, 0x00);
		//fnCanProcess = SafeWrite32(0x010E4DFC + 0x01 * 4, &CanProcess_Hook);
	}
};

FavoritesHandler::FnCanProcess FavoritesHandler::fnCanProcess;



class BookMenu : public IMenu,
				 public SimpleAnimationGraphManagerHolder,
				 public BSTEventSink<BSAnimationGraphEvent>
{
public:

	class GlobalBookData
	{
	public:
		ExtraTextDisplayData*			extraData;
		UInt32							unk04;
		TESObjectBOOK*					book;
		UInt32							padding;
		char*							content;
		UInt16							textLen;
		UInt16							buffLen;
		TESObjectREFR*					reference;
		float							unk1C;
		float							unk20;
		float							unk24;
		UInt32							unk28;

		static GlobalBookData* GetSingleton()
		{
			return (GlobalBookData*)0x01B3E520;
		}
	};

	class BookContentUpdater : public TaskDelegate
	{
	public:
		TES_FORMHEAP_REDEFINE_NEW();

		BookContentUpdater(bool direction) : m_direction(direction){}

		virtual void Run() override
		{
			UIStringHolder* stringHolder = UIStringHolder::GetSingleton();
			MenuManager* mm = MenuManager::GetSingleton();
			if (mm->IsMenuOpen(stringHolder->bookMenu))
			{
				BookMenu* bookMenu = static_cast<BookMenu*>(mm->GetMenu(stringHolder->bookMenu));
				if (bookMenu != nullptr)
				{
					bookMenu->bookView->SetPause(false);
					if (bookMenu->bookView->GetVariableDouble("BookMenu.BookMenuInstance.iPaginationIndex") != -1.00f)
					{
						GFxValue result;
						bookMenu->bookView->Invoke("BookMenu.BookMenuInstance.CalculatePagination", &result, nullptr, 0); //只能翻一页。
						//bookMenu->bookView->Invoke("BookMenu.BookMenuInstance.UpdatePages", &result, nullptr, 0);
						bookMenu->bookView->Invoke("BookMenu.BookMenuInstance.CalculatePagination", &result, nullptr, 0); //只能翻一页。
						//bookMenu->bookView->Invoke("BookMenu.BookMenuInstance.UpdatePages", &result, nullptr, 0);
						bookMenu->bookView->SetPause(true);
					}
					bookMenu->TurnPage(m_direction);
					if (bookMenu->flags & IMenu::kType_PauseGame)
					{
						mm->numPauseGame -= 1;
						bookMenu->flags &= ~IMenu::kType_PauseGame;
					}
				}
			}
		}

		virtual void Dispose() override
		{
			delete this;
		}

		static void Queue(bool direction)
		{
			UIStringHolder* stringHolder = UIStringHolder::GetSingleton();
			MenuManager* mm = MenuManager::GetSingleton();

			if (mm->IsMenuOpen(stringHolder->bookMenu))
			{
				BookMenu* bookMenu = static_cast<BookMenu*>(mm->GetMenu(stringHolder->bookMenu));
				if (bookMenu != nullptr)
				{
					bool(__cdecl* IsGameRuning)(bool mask) = (bool(__cdecl*)(bool mask))0x006F3570;

					if (!IsGameRuning(true))
					{
						bookMenu->TurnPage(direction);
					}
					else if(!mm->numPauseGame && !(bookMenu->flags & IMenu::kType_PauseGame))
					{
						mm->numPauseGame += 1;
						bookMenu->flags |= IMenu::kType_PauseGame;
						const SKSEPlugin *plugin = SKSEPlugin::GetSingleton();
						const SKSETaskInterface *task = plugin->GetInterface(SKSETaskInterface::Version_2);
						if (task)
						{
							BookContentUpdater *delg = new BookContentUpdater(direction);
							task->AddTask(delg);
						}
					}
				}
			}
		}

	private:
		bool							m_direction;
	};


	typedef UInt32(BookMenu::*FnProcessMessage)(UIMessage*);
	typedef EventResult(BookMenu::*FnReceiveEvent)(BSAnimationGraphEvent*, BSTEventSource<BSAnimationGraphEvent>*);

	static FnProcessMessage					fnProcessMessage;
	static FnReceiveEvent					fnReceiveEvent;



	EventResult	ReceiveEvent_Hook(BSAnimationGraphEvent * evn, BSTEventSource<BSAnimationGraphEvent> * source)
	{
		EventResult result = (this->*fnReceiveEvent)(evn, source);
		if (evn->animName == "PageBackStop" || evn->animName == "PageForwardStop" || evn->animName == "OpenStop")
		{

		}
		return result;
	}

	void TurnPage(bool direction)
	{
		bool animationVariable = false;
		this->GetAnimationVariableBool("bPageFlipping", animationVariable);//是否正在翻页中。
		if (!animationVariable)
		{
			GFxValue result;
			GFxValue argument;

			((void(__stdcall*)(GFxValue&, UInt32, UInt32, void*))0x00402290)(argument, 0x10, 1, (void*)0x0040F490);//构造GFxValue数组,这里只需要一个就够了。

			double turnPageNumber = (this->isNote) ? 1.00f : 2.00f;
			if (!direction)
				turnPageNumber = -turnPageNumber;

			argument.SetNumber(turnPageNumber);
			result.SetBoolean(false);

			bool(__fastcall* Invoke)(GFxMovieView*, void*, char*, GFxValue*, GFxValue*, UInt32) = (bool(__fastcall*)(GFxMovieView*, void*, char*, GFxValue*, GFxValue*, UInt32))(*(*(UInt32**)this->bookView + 0x17));
			if (Invoke(this->bookView, nullptr, "BookMenu.BookMenuInstance.TurnPage", &result, &argument, 1) && result.GetBool())
			{
				if (direction)
					this->SendAnimationEvent("PageForward");
				else
					this->SendAnimationEvent("PageBack");

				if (this->isNote)
				{
					((void(__cdecl*)(char*))0x00899620)("ITMNotePageTurn");
				}
			}
			((void(__stdcall*)(GFxValue&, UInt32, UInt32, void*))0x004022D0)(argument, 0x10, 1, (void*)0x00691680); //GFxValue数组的析构函数。
		}
	}


	UInt32 ProcessMessage_Hook(UIMessage* msg)
	{
		UIStringHolder* holder = UIStringHolder::GetSingleton();

		if (settings.m_menuConfig[holder->bookMenu.c_str()])
		{
			InputStringHolder* input = InputStringHolder::GetSingleton();
			MenuManager* mm = MenuManager::GetSingleton();

			if (msg->type == UIMessage::kMessage_Open)
			{
				this->menuDepth = 4;
			}

			if (msg->type == UIMessage::kMessage_Message)
			{
				BSUIMessageData* msgData = static_cast<BSUIMessageData*>(msg->data);

				if (msgData->unk0C == input->prevPage || msgData->unk0C == input->leftEquip)
				{
					BookContentUpdater::Queue(false);
				}
				else if (msgData->unk0C == input->nextPage || msgData->unk0C == input->rightEquip)
				{
					BookContentUpdater::Queue(true);
				}
				else if (msgData->unk0C == input->accept || msgData->unk0C == input->cancel)
				{
					if (!*(bool*)0x1B3E5D4 || this->disableCloseMsg)
						return 0;
					GlobalBookData* data = GlobalBookData::GetSingleton();
					if (msgData->unk0C == input->accept && data->reference != nullptr)
					{
						g_thePlayer->PickUpItem(data->reference, 1, false, true);
					}
					this->SendAnimationEvent("SoundPlay");
					this->SendAnimationEvent("CloseOut");
					mm->CloseMenu(holder->bookMenu);
				}
				return 1;
			}

			if (msg->type == UIMessage::kMessage_Scaleform)
			{
				if (!*(bool*)0x1B3E5D4)
					return 0;

				BSUIScaleformData* scaleformData = static_cast<BSUIScaleformData*>(msg->data);
				GFxEvent* event = scaleformData->event;

				if (event->type == GFxEvent::KeyDown)
				{
					GFxKeyEvent* key = static_cast<GFxKeyEvent*>(event);
					if (key->keyCode == GFxKey::Code::Left || key->keyCode == GFxKey::Code::Right)
					{
						BookContentUpdater::Queue(key->keyCode == GFxKey::Code::Right);
					}
					else if (this->isNote && (key->keyCode == GFxKey::Code::Up || key->keyCode == GFxKey::Code::Down))
					{
						BookContentUpdater::Queue(key->keyCode == GFxKey::Code::Down);
					}
				}
				return 1;
			}
			return (this->*fnProcessMessage)(msg);
		}
		else
		{
			if (msg->type == UIMessage::kMessage_Open)
			{
				this->menuDepth = 4;
			}
			return (this->*fnProcessMessage)(msg);
		}
	}

	void SetBookText_Hook()
	{
		bool(__cdecl* IsGameRuning)(bool mask) = (bool(__cdecl*)(bool mask))0x006F3570;
		if (!IsGameRuning(true))
		{
			this->SetBookText();

			auto fn = []()->bool {
				UIStringHolder* stringHolder = UIStringHolder::GetSingleton();
				MenuManager* mm = MenuManager::GetSingleton();
				if (mm->IsMenuOpen(stringHolder->bookMenu))
				{
					BookMenu* bookMenu = static_cast<BookMenu*>(mm->GetMenu(stringHolder->bookMenu));
					if (bookMenu != nullptr && (bookMenu->flags & IMenu::kType_PauseGame))
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
						if (mm->numPauseGame)
						{
							bookMenu->bookView->SetPause(true);
							mm->numPauseGame -= 1;
							bookMenu->flags &= ~IMenu::kType_PauseGame;
							if (!mm->numPauseGame)
							{
								MenuModeChangeEvent event;
								event.unk0 = 0;
								event.menuMode = 0;

								mm->BSTEventSource<MenuModeChangeEvent>::SendEvent(&event);
								typedef void(__fastcall * Fn)(void*, void*); //MenuModeChangeEvent
								Fn ReleaseObject = (Fn)0xA511B0;
								ReleaseObject(&event, nullptr);
							}
						}
					}
				}
				return true;
			};

			UIStringHolder* stringHolder = UIStringHolder::GetSingleton();
			if (settings.m_menuConfig[stringHolder->bookMenu.c_str()])
			{
				really_async(fn);
			}
		}
		else
		{
			this->SetBookText();
		}
	}

	static void InitHook()
	{
		fnProcessMessage = SafeWrite32(0x010E3AA4 + 0x04 * 4, &ProcessMessage_Hook);

		//fnReceiveEvent = SafeWrite32(0x010E3A98 + 0x01 * 4, &ReceiveEvent_Hook);

		WriteRelCall(0x008464AC, GetFnAddr(&BookMenu::SetBookText_Hook));

		//WriteRelCall(0x008466D0, GetFnAddr(&BookMenu::CreateBookLayout_Hook));

		//SafeWrite8(0x008466AB, 0x00);

		//SafeWrite8(0x008455E8, 0x00);
	}

	//members:
	UInt32					unk2C;					
	UInt32					unk30;
	UInt32					unk34;
	GFxMovieView*			bookView;				
	NiAVObject*				unk3C;					
	UInt32					unk40;				
	UInt32					unk44;				
	UInt32					unk48;					
	UInt32					unk4C;				
	NiSourceTexture*		unk50;				
	NiTriShape*				unk54;					
	UInt16					unk58;					
	UInt16					refreshTimes;		
	UInt8					disableCloseMsg;		
	bool					isNote;					
	bool					isInit;					
	UInt8					padding;		

	DEFINE_MEMBER_FN(InitBookMenu, void, 0x008451C0);
	DEFINE_MEMBER_FN(CreateBookLayout, void, 0x00845F70);
	DEFINE_MEMBER_FN(SetBookText, void, 0x00845D60);

};
static_assert(sizeof(BookMenu) == 0x60, "sizeof(BookMenu) != 0x60");

BookMenu::FnProcessMessage	BookMenu::fnProcessMessage = nullptr;
BookMenu::FnReceiveEvent	BookMenu::fnReceiveEvent = nullptr;


class LockpickingMenu : public IMenu
{
public:
	typedef UInt32(LockpickingMenu::*FnProcessMessage)(UIMessage*);

	static FnProcessMessage fnProcessMessage;


	UInt32 ProcessMessage_Hook(UIMessage* msg)
	{
		return (this->*fnProcessMessage)(msg);
	}

	static void STDFN Hook_SetLockInfo()
	{
		UIStringHolder* holder = UIStringHolder::GetSingleton();
		MenuManager* manager = MenuManager::GetSingleton();
		if (holder && manager && manager->IsMenuOpen(holder->lockpickingMenu))
		{
			IMenu* lockpickingMenu = manager->GetMenu(holder->lockpickingMenu);
			if (settings.m_menuConfig[holder->lockpickingMenu.c_str()] && (lockpickingMenu->flags & IMenu::kType_PauseGame) == IMenu::kType_PauseGame)
			{
				manager->numPauseGame -= 1;
				lockpickingMenu->flags &= ~IMenu::kType_PauseGame;
				if (!manager->numPauseGame)
				{
					static MenuModeChangeEvent event;
					event.unk0 = 0;
					event.menuMode = 0;
					manager->BSTEventSource<MenuModeChangeEvent>::SendEvent(&event);

					typedef void(__fastcall * Fn)(void*, void*);
					Fn ReleaseObject = (Fn)0xA511B0;
					ReleaseObject(&event, nullptr);
				}
			}
		}
	}

	static void InitHook()
	{
		{
			static const UInt32 kHook_SetLockInfo_Call = 0x0086FE10;

			START_ASM(Hook_SetLockInfo, 0x0087053D, 0x00870549, 1);
				add esp, 0xC
				lea ecx, [esp + 0x38]
				call [kHook_SetLockInfo_Call]
				pushad
				call Hook_SetLockInfo
				popad
			END_ASM(Hook_SetLockInfo);
		}
	}
};
LockpickingMenu::FnProcessMessage	LockpickingMenu::fnProcessMessage = nullptr;


class DialogueMenuEx :public IMenu
{
public:
	typedef UInt32(DialogueMenuEx::*FnProcessMessage)(UIMessage*);

	static FnProcessMessage fnProcessMessage;

	UInt32 ProcessMessage_Hook(UIMessage* msg)
	{
		if (msg->type == UIMessage::kMessage_Open && MenuOpenCloseEventHandler::unpausedCount != 0)
		{
			this->menuDepth = 2;
			//GFxMovieView* view = this->GetMovieView();
		}
		return (this->*fnProcessMessage)(msg);
	}

	static void InitHook()
	{
		fnProcessMessage = SafeWrite32(0x010E4C9C + 0x04 * 4, &ProcessMessage_Hook);
	}
};
DialogueMenuEx::FnProcessMessage	DialogueMenuEx::fnProcessMessage = nullptr;


class InventoryMenuEx : public IMenu
{
public:
	typedef UInt32(InventoryMenuEx::*FnProcessMessage)(UIMessage*);

	static FnProcessMessage fnProcessMessage;

	void Release_Hook()
	{
		if (*(UInt32*)this == 0x010E5B90 && !(this->flags & IMenu::kType_PauseGame))
		{
			if (InterlockedExchangeAdd((volatile long*)((char*)this + 4), -1) == 1)
			{
				void(__fastcall* sub_897160)(void*, void*) = (void(__fastcall*)(void*, void*))0x00897160;//Camera mode.
				sub_897160(*(void**)0x1B2E4C0, nullptr);
				//  sub_897130(dword_1B2E4C0);

				void(__cdecl* sub_861470)(void*) = (void(__cdecl*)(void*))0x861470;  //Enable prompt.
				sub_861470((void*)0x10E3824);

				InputMappingManager* input = InputMappingManager::GetSingleton();
				input->DisableControl(InputMappingManager::ContextType::kContext_Inventory);
				input->DisableControl(InputMappingManager::ContextType::kContext_ItemMenu);
				input->DisableControl(InputMappingManager::ContextType::kContext_MenuMode);

				//A67260  Disable();
				//void(__fastcall* sub_42F240)(void*, void*, bool) = (void(__fastcall*)(void*, void*, bool))0x0042F240;  //???????????
				//sub_42F240(*(void**)0x12E3534, nullptr, 1);

				if (*((UInt32*)this + 0xF)) // Need test.
				{
					void(__fastcall* sub_A49D90)(void*, void*) = (void(__fastcall*)(void*, void*))0x00A49D90;
					sub_A49D90((char*)this + 0x3C, nullptr);
					*((UInt32*)this + 0x11) = 0;
				}
				//Another funciton here.
				GFxValue* root = (GFxValue*)((char*)this + 0x20);
				if ((*(UInt32*)((char*)this + 0x24) >> 6) & 1)
				{
					void(__fastcall* ObjectRelease)(void*, void*, void*, void*) = (void(__fastcall*)(void*, void*, void*, void*))0x922660;
					ObjectRelease(*(void**)root, nullptr, root, *((void**)this + 0xA));
					*(void**)root = nullptr;
				}

				typedef void(__fastcall* Fn)(void*, void*);
				Fn fn = (Fn)0x00A64A10;
				fn(this, nullptr);

				GMemory::Free(this);
			}
			return;
		}
		this->Release();
	}

	UInt32 ProcessMessage_Hook(UIMessage* msg)
	{
		UInt32 result = (this->*fnProcessMessage)(msg);

		if (msg->type == UIMessage::kMessage_Scaleform)
		{
			BSUIScaleformData* scaleformData = static_cast<BSUIScaleformData*>(msg->data);
			GFxEvent* event = scaleformData->event;
			if (event->type == GFxEvent::KeyDown)
			{
				GFxKeyEvent* key = static_cast<GFxKeyEvent*>(event);
				if (key->keyCode == GFxKey::Code::I && !InputMappingManager::GetSingleton()->unk98)
				{
					GFxValue result;
					GFxMovieView* view = this->GetMovieView();
					if (view != nullptr)
					{
						view->Invoke("_root.Menu_mc.onExitMenuRectClick", &result, nullptr, 0);
					}
				}
			}
		}
		return result;
	}

	static void InitHook()
	{
		fnProcessMessage = SafeWrite32(0x010E5B90 + 0x04 * 4, &ProcessMessage_Hook);
		WriteRelCall(0x00A5D686, &Release_Hook);
	}
};
InventoryMenuEx::FnProcessMessage	InventoryMenuEx::fnProcessMessage = nullptr;


class MagicMenuEx : public IMenu
{
public:
	typedef UInt32(MagicMenuEx::*FnProcessMessage)(UIMessage*);

	static FnProcessMessage fnProcessMessage;

	UInt32 ProcessMessage_Hook(UIMessage* msg)
	{
		UInt32 result = (this->*fnProcessMessage)(msg);

		if (msg->type == UIMessage::kMessage_Scaleform)
		{
			BSUIScaleformData* scaleformData = static_cast<BSUIScaleformData*>(msg->data);
			GFxEvent* event = scaleformData->event;
			if (event->type == GFxEvent::KeyDown)
			{
				GFxKeyEvent* key = static_cast<GFxKeyEvent*>(event);
				if (key->keyCode == GFxKey::Code::P && !InputMappingManager::GetSingleton()->unk98)
				{
					GFxValue result;
					GFxMovieView* view = this->GetMovieView();
					if (view != nullptr)
					{
						view->Invoke("_root.Menu_mc.onExitMenuRectClick", &result, nullptr, 0);
					}
				}
			}
		}
		return result;
	}

	static void InitHook()
	{
		fnProcessMessage = SafeWrite32(0x010E6594 + 0x04 * 4, &ProcessMessage_Hook);

		SafeWrite16(0x00873884, 0x9090);
		SafeWrite32(0x00873886, 0x90909090);
		SafeWrite8(0x0087388A, 0x90);
	}
};
MagicMenuEx::FnProcessMessage	MagicMenuEx::fnProcessMessage = nullptr;


class SleepWaitMenu : public IMenu
{
public:
	typedef UInt32(SleepWaitMenu::*FnProcessMessage)(UIMessage*);

	static FnProcessMessage fnProcessMessage;

	UInt32 ProcessMessage_Hook(UIMessage* msg)
	{
		UInt32 result = (this->*fnProcessMessage)(msg);

		if (msg->type == UIMessage::kMessage_Open)
		{
			MenuManager* mm = MenuManager::GetSingleton();
			UIStringHolder* holder = UIStringHolder::GetSingleton();

			if (holder && mm && mm->IsMenuOpen(holder->sleepWaitMenu) && (this->flags & IMenu::kType_PauseGame) && mm->numPauseGame)
			{
				mm->numPauseGame -= 1;
				this->flags &= ~IMenu::kType_PauseGame;
				if (!mm->numPauseGame)
				{
					static MenuModeChangeEvent event;
					event.unk0 = 0;
					event.menuMode = 0;
					mm->BSTEventSource<MenuModeChangeEvent>::SendEvent(&event);

					typedef void(__fastcall * Fn)(void*, void*);
					Fn ReleaseObject = (Fn)0xA511B0;
					ReleaseObject(&event, nullptr);
				}
			}
		}
		return result;
	}

	static void InitHook()
	{
		fnProcessMessage = SafeWrite32(0x010E7754 + 0x04 * 4, &ProcessMessage_Hook);
	}
};
SleepWaitMenu::FnProcessMessage	SleepWaitMenu::fnProcessMessage = nullptr;


void UICallBack_DropItem(FxDelegateArgs* pargs)
{
	InventoryMenu* inventory = reinterpret_cast<InventoryMenu*>(pargs->pThisMenu);
	if (inventory != nullptr && inventory->inventoryData)
	{
		StandardItemData* itemData = inventory->inventoryData->GetSelectedItemData();
		InventoryEntryData* objDesc = nullptr;
		if (itemData)
			objDesc = itemData->objDesc;
		if (objDesc != nullptr)
		{
			void(__cdecl* sub_8997A0)(void*, char*, bool) = (void(__cdecl*)(void*, char*, bool))0x8997A0;
			if (objDesc->IsQuestItem())
			{
				sub_8997A0(*(void**)0x1B1950C, nullptr, true);  //cdecl
			}
			else
			{
				TESForm* form = objDesc->baseForm;
				if (form->formType == FormType::Key)
				{
					sub_8997A0(*(void**)0x1B1953C, nullptr, true);  //cdecl
				}
				else
				{
					GFxValue* args = pargs->args;
					UInt32 count = static_cast<UInt32>(args->GetNumber());
					BaseExtraList* (__cdecl* GetBaseExtraList)(InventoryEntryData*, UInt32, bool) = (BaseExtraList* (__cdecl*)(InventoryEntryData*, UInt32, bool))0x00868950;
					BaseExtraList* extraList = GetBaseExtraList(objDesc, count, false);
					RefHandle handle;
					g_thePlayer->DropItem(&handle, form, extraList, count, 0, 0);

					UInt32(__fastcall * sub_755420)(void*, void*) = (UInt32(__fastcall *)(void*, void*))0x00755420;
					sub_755420(*(void**)0x12E32E8, nullptr);

					inventory->inventoryData->Update(g_thePlayer);

					class PlayerEquipemntUpdater : public TaskDelegate
					{
					public:
						TES_FORMHEAP_REDEFINE_NEW();

						void Run() override
						{
							g_thePlayer->processManager->UpdateEquipment(g_thePlayer);
						}
						void Dispose() override
						{
							delete this;
						}
						static void Register()
						{
							const SKSEPlugin *plugin = SKSEPlugin::GetSingleton();
							const SKSETaskInterface *task = plugin->GetInterface(SKSETaskInterface::Version_2);
							if (task)
							{
								PlayerEquipemntUpdater *delg = new PlayerEquipemntUpdater();
								task->AddTask(delg);
							}
						}
					};
					//PlayerCamera* camera = PlayerCamera::GetSingleton();
					if (inventory->flags & IMenu::kType_PauseGame/* || camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Free]*/)
					{
						g_thePlayer->processManager->UpdateEquipment(g_thePlayer);
					}
					else if (PlayerCamera::GetSingleton()->cameraState == PlayerCamera::GetSingleton()->cameraStates[PlayerCamera::kCameraState_Free])
					{
						PlayerEquipemntUpdater::Register();
					}
				}
			}
		}
	}
}


void UICallBack_CloseTweenMenu(FxDelegateArgs* pargs)
{
	PlayerCamera* camera = PlayerCamera::GetSingleton();//kCameraState_Free
	camera->ResetCamera();

	MenuManager* mm = MenuManager::GetSingleton();
	UIStringHolder* holder = UIStringHolder::GetSingleton();
	mm->CloseMenu(holder->tweenMenu);

	IMenu* inventoryMenu = reinterpret_cast<IMenu*>(pargs->pThisMenu);
	*((bool*)inventoryMenu + 0x51) = 1;
}


void UICallBack_SelectItem(FxDelegateArgs* pargs)
{
	InventoryMenu* inventory = reinterpret_cast<InventoryMenu*>(pargs->pThisMenu);
	GFxValue* args = pargs->args;

	TESForm* form = nullptr;
	BaseExtraList* extraList = nullptr;

	void(__fastcall* EquipItem)(InventoryMenu*, void*, BGSEquipSlot*) = (void(__fastcall*)(InventoryMenu*, void*, BGSEquipSlot*))0x00869DB0;
	if (inventory != nullptr && inventory->bPCControlsReady && pargs->numArgs && args->Type == GFxValue::ValueType::VT_Number)
	{
		if (args->GetNumber() == 0.0f)
		{
			BGSEquipSlot* rightHandSlot = GetRightHandSlot();
			EquipItem(inventory, nullptr, rightHandSlot);
		}
		else if (args->GetNumber() == 1.0f)
		{
			BGSEquipSlot* leftHandSlot = GetLeftHandSlot();
			EquipItem(inventory, nullptr, leftHandSlot);
		}
	}
	else
	{
		EquipItem(inventory, nullptr, nullptr);
	}
	class PlayerEquipemntUpdater : public TaskDelegate
	{
	public:
		TES_FORMHEAP_REDEFINE_NEW();

		void Run() override
		{
			g_thePlayer->processManager->UpdateEquipment(g_thePlayer);
		}
		void Dispose() override
		{
			delete this;
		}
		static void Register()
		{
			const SKSEPlugin *plugin = SKSEPlugin::GetSingleton();
			const SKSETaskInterface *task = plugin->GetInterface(SKSETaskInterface::Version_2);
			if (task)
			{
				PlayerEquipemntUpdater *delg = new PlayerEquipemntUpdater();
				task->AddTask(delg);
			}
		}
	};
	if (inventory->flags & IMenu::kType_PauseGame/* || camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Free]*/)
	{
		g_thePlayer->processManager->UpdateEquipment(g_thePlayer);
	}
	else if (PlayerCamera::GetSingleton()->cameraState == PlayerCamera::GetSingleton()->cameraStates[PlayerCamera::kCameraState_Free])
	{
		PlayerEquipemntUpdater::Register();
	}

	class PlayerInfoUpdater : public UIDelegate
	{
	public:
		TES_FORMHEAP_REDEFINE_NEW();

		void Run() override
		{
			UIStringHolder* stringHolder = UIStringHolder::GetSingleton();
			MenuManager* mm = MenuManager::GetSingleton();
			if (mm->IsMenuOpen(stringHolder->inventoryMenu))
			{
				InventoryMenu* inventory = static_cast<InventoryMenu*>(mm->GetMenu(stringHolder->inventoryMenu));
				inventory->UpdatePlayerInfo();
			}
		}
		void Dispose() override
		{
			delete this;
		}
		static void Register()
		{
			const SKSEPlugin *plugin = SKSEPlugin::GetSingleton();
			const SKSETaskInterface *task = plugin->GetInterface(SKSETaskInterface::Version_2);
			if (task)
			{
				PlayerInfoUpdater *delg = new PlayerInfoUpdater();
				task->AddUITask(delg);
			}
		}
	};
	auto fn = [=](UInt32 time)->bool {std::this_thread::sleep_for(std::chrono::milliseconds(time)); PlayerInfoUpdater::Register(); return true; };
	really_async(fn, 100);
}


void UICallBack_SetSaveDisabled(FxDelegateArgs* pargs)
{
	GFxValue* args = pargs->args; //一个参数数组 GFxVaue[5];

	bool isBool = true;
	if (pargs->numArgs <= 5 || args[5].Type != GFxValue::ValueType::VT_Boolean || !args[5].GetBool())
	{
		isBool = false;
	}
	for (UInt32 i = 0; i < 5 && args[i].GetType() != GFxValue::ValueType::VT_Undefined; ++i)
	{
		GFxValue value(args[i]);
		BGSSaveLoadManager* saveLoadManager = BGSSaveLoadManager::GetSingleton();
		GFxValue arg;
		if (i == 1) //Load
		{
			arg.SetBoolean(false);
		}
		else
		{
			arg.SetBoolean(!saveLoadManager->CanSaveLoadGame(!isBool));
		}
		args[i].pObjectInterface->SetMember(args[i].Value.pData, "disabled", arg, value.GetType() == GFxValue::ValueType::VT_DisplayObject);
		//_MESSAGE("pGFxValue: %p    pObjectInterface: %p    pData:%p", &args[i], args[i].pObjectInterface, args[i].Value.pData);
		if ((arg.GetType() >> 6) & 1)
		{
			arg.pObjectInterface->ObjectRelease(&arg, arg.Value.pData);
		}
		if ((value.GetType() >> 6) & 1)
		{
			value.pObjectInterface->ObjectRelease(&value, value.Value.pData);
		}
	}
}


void RegisterEventHandler()
{
	MenuManager* mm = MenuManager::GetSingleton();
	if (mm)
	{
		BSTEventSource<MenuOpenCloseEvent>* eventDispatcher = mm->GetMenuOpenCloseEventSource();
		eventDispatcher->AddEventSink(MenuOpenCloseEventHandler::GetSingleton());
	}
}


void Hook_Game_Commit()
{

	FavoritesHandler::InitHook();
	BookMenu::InitHook();
	LockpickingMenu::InitHook();
	InventoryMenuEx::InitHook();

	DialogueMenuEx::InitHook();
	MagicMenuEx::InitHook();
	//SleepWaitMenu::InitHook();

	//Fix Inventory.
	SafeWrite16(0x0086BF6F, 0x9090);
	SafeWrite32(0x0086BF71, 0x90909090);

	//Fix Container
	SafeWrite16(0x0084C068, 0x9090);
	SafeWrite32(0x0084C06A, 0x90909090);

	SafeWrite16(0x0084AC57, 0x9090);
	SafeWrite32(0x0084AC59, 0x90909090);

	//Drop item in Inventory menu.
	SafeWrite32(0x0086B437, (UInt32)UICallBack_DropItem);
	//Equip item in Inventory Menu
	SafeWrite32(0x0086B3F4, (UInt32)UICallBack_SelectItem);
	//SetSaveDisabled in Journal Menu
	SafeWrite32(0x008A7F73, (UInt32)UICallBack_SetSaveDisabled);
	//Fix camera in Tweenmenu.
	SafeWrite8(0x008951C4, 0x90);
	SafeWrite32(0x008951C5, 0x90909090);

	//Redefine papyrus function: Utility.IsInMenuMode()
	WriteRelJump(0x00918D90, (UInt32)Hook_IsInMenuMode);

	{
		static const UInt32 kHook_GetFavoritesSpell_Jmp = 0x85BA03;

		START_ASM(Hook_GetFavoritesSpell, 0x0085B919, 0x0085B921, 1);
			push eax
			call Hook_GetFavoritesItem
			test eax, eax
			je InvalidSpell
			JMP_ASM(Hook_GetFavoritesSpell)
			InvalidSpell:
			jmp [kHook_GetFavoritesSpell_Jmp]
		END_ASM(Hook_GetFavoritesSpell);
	}

	{
		static const UInt32 kHook_GetFavoritesItem_Jmp = 0x85BB34;

		START_ASM(Hook_GetFavoritesItem, 0x0085BA52, 0x0085BA5A, 1);
			push eax
			call Hook_GetFavoritesItem
			test eax, eax
			je InvalidItem
			JMP_ASM(Hook_GetFavoritesItem)
			InvalidItem:
			jmp [kHook_GetFavoritesItem_Jmp]
		END_ASM(Hook_GetFavoritesItem);
	}

	{
		START_ASM(Hook_KillActor, 0x006AC3A0, 0x006AC3A9, 1);
			push ecx
			push ecx
			call Hook_KillActor
			pop ecx
			push esi
			mov esi, ecx
			mov ecx, [esi + 0x88]
		END_ASM(Hook_KillActor);
	}

	{
		START_ASM(Hook_AddUIMessage, 0x00431B00, 0x00431B0A, 1);
			push ecx
			mov eax, [esp + 0xC]
			push eax
			mov eax, [esp + 0xC]
			push eax
			call Hook_AddUIMessage
			pop ecx
			test al, al
			jnz Continue
			retn 0xC
			Continue:
			push ecx
			push esi
			mov esi, ecx
			mov eax, [esi + 0x1C8]
		END_ASM(Hook_AddUIMessage);
	}

	{
		START_ASM(Hook_RequstAutoSave, 0x00681770, 0x00681776, 1);
			push ecx
			mov eax, MenuOpenCloseEventHandler::unpausedCount
			cmp eax, 0
			jz	ContinueSave
			pop ecx
			retn
			ContinueSave:
			pop ecx
			sub esp, 0x104
		END_ASM(Hook_RequstAutoSave)
	}
}

