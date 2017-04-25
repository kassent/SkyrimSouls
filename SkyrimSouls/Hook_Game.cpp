#include "Hook_Game.h"
#include "Settings.h"
#include "Tools.h"
#include <SKSE/GameMenus.h>
#include <SKSE/SafeWrite.h>
#include <SKSE/GameRTTI.h>
#include <Skyrim/BSDevices/InputDevice.h>
#include <Skyrim/BSDevices/InputManager.h>
#include <Skyrim/BSDevices/KeyCode.h>
#include <Skyrim/Forms/BGSEquipSlot.h>
#include <Skyrim/Forms/PlayerCharacter.h>
#include "Skyrim/BSDevices/InputMappingManager.h"
#include <Skyrim/Camera/PlayerCamera.h>
#include <SKyrim/BSDevices/PlayerControls.h>
#include <Skyrim/NetImmerse/NiAVObject.h>
#include <Skyrim/Forms/Character/Components/ActorProcessManager.h>
#include <Skyrim/Animation/IAnimationGraphManagerHolder.h>

#include <future>
#include <vector>
#include <string>
#include <string>
#include <algorithm>

#define STDFN __stdcall

class EquipManager
{
public:
	virtual ~EquipManager();

	static EquipManager *   GetSingleton(void)
	{
		return *((EquipManager **)0x012E5FAC);
	}

	DEFINE_MEMBER_FN(EquipItem, void, 0x006EF3E0, Actor * actor, TESForm * item, BaseExtraList * extraData, SInt32 count, BGSEquipSlot * equipSlot, int withEquipSound, int preventUnequip, int showMsg, int unk);
	DEFINE_MEMBER_FN(UnequipItem, bool, 0x006EE560, Actor * actor, TESForm * item, BaseExtraList * extraData, SInt32 count, BGSEquipSlot * equipSlot, int unkFlag1, int preventEquip, int unkFlag2, int unkFlag3, int unk);
};



class MenuOpenCloseEventHandler : public BSTEventSink<MenuOpenCloseEvent>
{
public:
	static SInt32							unpausedCount;
	static bool								disableAutoSave;
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
				if (unpausedCount++ == 0)
				{
					if (mm->IsMenuOpen(holder->dialogueMenu) && (evn->menuName == holder->containerMenu || evn->menuName == holder->barterMenu || evn->menuName == holder->giftMenu || evn->menuName == holder->trainingMenu))
					{
						//dialogueTarget = g_thePlayer->GetDialogueTarget();
						//IMenu* dialogueMenu = mm->GetMenu(holder->dialogueMenu);
						//_MESSAGE("menuDepth: %d", dialogueMenu->menuDepth);
						GFxMovieView* view = mm->GetMovieView(holder->dialogueMenu);
						view->SetVisible(false);
					}
					PlayerControls* control = PlayerControls::GetSingleton();
					for (auto element : control->handlers)
					{
						*((UInt32*)element + 1) = false;
						//_MESSAGE("handler name: %s", GetObjectClassName(element));
					}
					//InputManager* inputManager = InputManager::GetSingleton();
					if (evn->menuName == holder->inventoryMenu || evn->menuName == holder->containerMenu || evn->menuName == holder->barterMenu || evn->menuName ==holder->magicMenu || evn->menuName == holder->giftMenu)
					{
						InputMappingManager* input = InputMappingManager::GetSingleton();
						input->DisableControl(0);
					}
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

					if (evn->menuName == holder->inventoryMenu || evn->menuName == holder->containerMenu || evn->menuName == holder->barterMenu || evn->menuName == holder->magicMenu || evn->menuName == holder->giftMenu)
					{
						InputMappingManager* input = InputMappingManager::GetSingleton();
						input->EnableControl(0);
					}
					if (mm->IsMenuOpen(holder->dialogueMenu)/* && (evn->menuName == holder->containerMenu || evn->menuName == holder->barterMenu || evn->menuName == holder->giftMenu || evn->menuName == holder->trainingMenu)*/)
					{
						GFxMovieView* view = mm->GetMovieView(holder->dialogueMenu);
						if(!view->GetVisible())
							view->SetVisible(true);
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
					//IsGamepadEnabled
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

SInt32		MenuOpenCloseEventHandler::unpausedCount = 0;
bool		MenuOpenCloseEventHandler::disableAutoSave = false;


template <typename F, typename... Args>
auto really_async(F&& f, Args&&... args)-> std::future<typename std::result_of<F(Args...)>::type>
{
	using returnValue = typename std::result_of<F(Args...)>::type;
	auto fn = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
	std::packaged_task<returnValue()> task(std::move(fn));
	auto future = task.get_future();
	std::thread thread(std::move(task));
	thread.detach();
	return future;
}



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
		if(isProcessing) Sleep(200);
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
	if (msgID == UIMessage::kMessage_Open && MenuOpenCloseEventHandler::unpausedCount != 0 && (strData == holder->sleepWaitMenu || strData == "Loot Menu"))
	{
		return false;
	}
	return true;
}

void STDFN AttempEquip(PlayerCharacter* actor, InventoryEntryData* objDesc, BGSEquipSlot* slot, bool unk = 1)
{
	//_MESSAGE("> Attemp Equip...");
	TESForm * form = nullptr;
	UInt32 count = NULL;
	BGSEquipSlot *leftEquipSlot = nullptr, *rightEquipSlot = nullptr, *targetEquipSlot = nullptr;
	BaseExtraList *leftExtraList = nullptr, *rightExtraList = nullptr;
	bool condition = false;

	form = objDesc->baseForm;
	leftExtraList = objDesc->GetBaseExtraList(1);//thiscall InventoryEntryData::GetBaseExtraList 
	rightExtraList = objDesc->GetBaseExtraList(0);
	if (form->formType == FormType::Ammo) 
		count = objDesc->GetCount(); //DEFINE_MEMBER_FN_const(GetCount, SInt32, 0x005E8920);
	else
		count = 1;
	if (actor && (UInt32)actor == *(UInt32*)0x1310588 && objDesc->Unk001(61))
	{
		void(__cdecl* sub_8997A0)(void*, char*, bool) = (void(__cdecl*)(void*, char*, bool))0x8997A0;
		if (rightExtraList || leftExtraList)
			sub_8997A0(*(void**)0x1B19584, (char*)0x010D0A38, true);  //cdecl
		else
			sub_8997A0(*(void**)0x1B19560, (char*)0x010D0A38, true);  //cdecl
		return;
	}

	EquipManager* manager = EquipManager::GetSingleton();

	if (rightExtraList && leftExtraList)
	{
		//_MESSAGE("> BothHands...");
		if (unk)
		{
			if (slot)
			{
				//manager->UnequipItem(actor, form, rightExtraList, count, slot, 0, 0, 1, 0, 0);
				if (slot == GetRightHandSlot())
					manager->UnequipItem(actor, form, rightExtraList, count, slot, 0, 0, 1, 0, 0);
				else
					manager->UnequipItem(actor, form, leftExtraList, count, slot, 0, 0, 1, 0, 0);
			}
			else
			{
				leftEquipSlot = GetLeftHandSlot();
				manager->UnequipItem(actor, form, leftExtraList, count, leftEquipSlot, 0, 0, 1, 0, 0);
				rightEquipSlot = GetRightHandSlot();
				manager->UnequipItem(actor, form, rightExtraList, count, rightEquipSlot, 0, 0, 1, 0, 0);
			}
		}
		return;
	}

	if (leftExtraList) //左手已经装备，装备右手。
	{
		//_MESSAGE("> LeftHand...");
		rightExtraList = leftExtraList;
		BGSEquipSlot* leftEquipSlot = GetLeftHandSlot();
		void* (__cdecl * sub_44AA10)(TESForm*) = (void* (__cdecl*)(TESForm*))0x44AA10;
		void* unk0 = sub_44AA10(form);//cdecl
		if (unk0)
		{
			typedef BGSEquipSlot* (__fastcall* Fn)(void*, void*);
			Fn fn = (Fn)(*(UInt32*)(*(UInt32*)unk0 + 0x10));//????
			targetEquipSlot = fn(unk0, nullptr);
		}
		else
			targetEquipSlot = 0;
		bool(__stdcall* sub_6EE190)(void*, void*, void*) = (bool(__stdcall*)(void*, void*, void*))0x6EE190;
		condition = (slot && slot != leftEquipSlot && (targetEquipSlot == GetEitherHandSlot() || sub_6EE190(actor, form, targetEquipSlot)));

		UInt32 itemCount = objDesc->GetCount();  //	DEFINE_MEMBER_FN_const(GetCount, SInt32, 0x005E8920);
		if (itemCount > 1 && condition)
		{
			manager->EquipItem(actor, form, nullptr, count, slot, 0, 0, 1, 0);
		}
		else if (unk)
		{
			BaseExtraList * newEquipList = manager->UnequipItem(actor, form, leftExtraList, count, leftEquipSlot, 0, 0, 1, 0, 0) ? 0 : leftExtraList;
			if (condition)
				manager->EquipItem(actor, form, newEquipList, count, slot, 0, 0, 1, 0);
		}
		return;
	}

	if (rightExtraList) //右手已经装备，装备左手
	{
		//_MESSAGE("> RightHand...");
		leftExtraList = rightExtraList;
		rightEquipSlot = GetRightHandSlot();
		void* (__cdecl * sub_44AA10)(TESForm*) = (void* (__cdecl*)(TESForm*))0x44AA10;
		void* unk1 = sub_44AA10(form);//cdecl
		if (unk1)
		{
			typedef BGSEquipSlot* (__fastcall* Fn)(void*, void*);
			Fn fn = (Fn)(*(UInt32*)(*(UInt32*)unk1 + 0x10));
			targetEquipSlot = fn(unk1, nullptr);
		}
		else
			targetEquipSlot = 0;
		bool(__stdcall* sub_6EE190)(void*, void*, void*) = (bool(__stdcall*)(void*, void*, void*))0x6EE190;
		condition = (slot && slot != rightEquipSlot && (targetEquipSlot == GetEitherHandSlot() || sub_6EE190(actor, form, unk1)));

		UInt32 itemCount = objDesc->GetCount();  //	DEFINE_MEMBER_FN_const(GetCount, SInt32, 0x005E8920);
		if (itemCount > 1 && condition)
		{
			manager->EquipItem(actor, form, nullptr, count, slot, 0, 0, 1, 0);
		}
		else if (unk)
		{
			BaseExtraList * newEquipList = manager->UnequipItem(actor, form, rightExtraList, count, rightEquipSlot, 0, 0, 1, 0, 0) ? 0 : rightExtraList;
			if (condition)
				manager->EquipItem(actor, form, newEquipList, count, slot, 0, 0, 1, 0);
		}
		return;
	}
	//_MESSAGE("> NoHand...");
	BSSimpleList<BaseExtraList *>* simpleList = objDesc->extraList;
	BaseExtraList * equipList = nullptr;
	if (simpleList)
	{
		if (*(BaseExtraList**)simpleList)
			equipList = *(BaseExtraList**)simpleList;
	}
	manager->EquipItem(actor, form, equipList, count, slot, 0, 0, 1, 0);
}



class FavoritesHandler : public MenuEventHandler
{
public:
	typedef bool(FavoritesHandler::*FnCanProcess)(InputEvent *);

	static FnCanProcess fnCanProcess;

	bool CanProcess_Hook(InputEvent *evn)
	{
		bool result = (this->*fnCanProcess)(evn);
		if (evn->deviceType == BSInputDevice::kType_Keyboard && evn->eventType == InputEvent::kEventType_Button)
		{
			ButtonEvent *button = static_cast<ButtonEvent *>(evn);
			if (button->keyMask >= KeyCode::Num1 && button->keyMask <= KeyCode::Num8)
			{
				//_MESSAGE("button->keyMask = %d", button->keyMask);
				result = true;
			}
		}
		return result;
	}

	static void InitHook()
	{
		SafeWrite8(0x0085BE1D, 0x00);
		//fnCanProcess = SafeWrite32(0x010E4DFC + 0x01 * 4, &CanProcess_Hook);
	}
};

FavoritesHandler::FnCanProcess FavoritesHandler::fnCanProcess;


//0x60
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

	typedef UInt32(BookMenu::*FnProcessMessage)(UIMessage*);
	typedef EventResult(BookMenu::*FnReceiveEvent)(BSAnimationGraphEvent*, BSTEventSource<BSAnimationGraphEvent>*);

	static FnProcessMessage fnProcessMessage;
	static FnReceiveEvent	fnReceiveEvent;
	static bool enableMenuControl;
	static bool enableCloseMenu;
	static bool bSlowndownRefreshFreq;

	EventResult	ReceiveEvent_Hook(BSAnimationGraphEvent * evn, BSTEventSource<BSAnimationGraphEvent> * source)
	{
		//animatinName: OpenStop
		//animatinName: PageForwardStop
		//animatinName: PageBackStop
		//animatinName: SoundPlay
		//animatinName: CloseOut

		EventResult result = (this->*fnReceiveEvent)(evn, source);

		if(evn->animName == "PageBackStop" || evn->animName == "PageForwardStop" || evn->animName == "OpenStop") //animatonName: PageBackStop animatonName: PageForwardStop
			bSlowndownRefreshFreq = false;
		return result;
	}

	UInt32 ProcessMessage_Hook(UIMessage* msg)
	{
		if (settings.m_menuConfig["Book Menu"])
		{
			static MenuManager* mm = MenuManager::GetSingleton();
			static UIStringHolder* holder = UIStringHolder::GetSingleton();

			if (msg->type == UIMessage::kMessage_Open)
			{
				this->menuDepth = 4;
				bSlowndownRefreshFreq = true;
			}

			if (msg->type == UIMessage::kMessage_Refresh)
			{
				if (this->refreshTimes > 5 && !*(bool*)0x1B3E5D4)
				{
					float(__fastcall* sub_844D60)(void*, void*) = (float(__fastcall*)(void*, void*))0x00844D60; //update *(bool)0x1B3E5D4;
					sub_844D60((void*)0x1B3E580, nullptr);

					if (!*(bool*)0x1B3E5D4)
					{
						float(__fastcall* sub_844FD0)(void*) = (float(__fastcall*)(void*))0x00844FD0;
						float unk1 = sub_844FD0((void*)0x1B3E580);

						void(__fastcall* sub_420630)(void*, void*, float) = (void(__fastcall*)(void*, void*, float))0x00420630;
						sub_420630(this->unk3C, nullptr,unk1);

						struct
						{
							float unk0;
							float unk1;
							float unk2;
							float unk3;
						} unk2;

						void(__fastcall* sub_844F60)(void*, void*, void*) = (void(__fastcall*)(void*, void*, void*))0x00844F60;
						sub_844F60((void*)0x1B3E580, nullptr, &unk2);

						void(__fastcall* sub_4719A0)(void*, void*, void*) = (void(__fastcall*)(void*, void*, void*))0x004719A0;
						sub_4719A0(&unk2, nullptr, &(this->unk3C->m_localTransform));

						NiPoint3 unk3;
						void(__fastcall* sub_844ED0)(void*, void*, void*) = (void(__fastcall*)(void*, void*, void*))0x00844ED0;
						sub_844ED0((void*)0x1B3E580, nullptr, &unk3);

						if (this->isNote)
						{
							this->unk3C->m_localTransform.pos = unk3;
						}
						else
						{
							NiPoint3 unk4;
							NiPoint3*(__fastcall* sub_420360)(void*, void*, void*, void*) = (NiPoint3*(__fastcall*)(void*, void*, void*, void*))0x00420360;
							NiPoint3* unk5 = sub_420360(&this->unk3C->m_unkPoint, nullptr, &unk4, &this->unk3C->m_localTransform.pos);

							float(__fastcall* sub_844F80)(void*) = (float(__fastcall*)(void*))0x00844F80;
							float unk6 = sub_844F80((void*)0x1B3E580);

							NiPoint3 unk7;

							NiPoint3*(__cdecl* sub_420430)(void*, float, void*) = (NiPoint3*(__cdecl*)(void*, float, void*))0x00420430;
							NiPoint3* var5 = sub_420430(&unk7, unk6, unk5);

							NiPoint3 unk8;

							NiPoint3* unk9 = sub_420360(&unk3, nullptr, &unk8, var5);

							this->unk3C->m_localTransform.pos = *unk9;
						}

						void(__fastcall* sub_6504A0)(void*, void*, void*) = (void(__fastcall*)(void*, void*, void*))0x006504A0;
						sub_6504A0((char*)this + 0x1C, nullptr, *(void**)0x1B4ADE0);

						if (this->refreshTimes == 6)
						{
							if (holder && mm && mm->IsMenuOpen(holder->bookMenu) && (this->flags & IMenu::kType_PauseGame) == IMenu::kType_PauseGame && mm->numPauseGame)
							{
								mm->numPauseGame -= 1;
								this->flags &= ~IMenu::kType_PauseGame;
								if (!mm->numPauseGame)
								{
									static MenuModeChangeEvent event;
									event.unk0 = 0;
									event.menuMode = 0;

									mm->BSTEventSource<MenuModeChangeEvent>::SendEvent(&event);
									typedef void(__fastcall * Fn)(void*, void*); //MenuModeChangeEvent
									Fn fn = (Fn)0xA511B0;
									fn(&event, nullptr);
								}
							}
							this->refreshTimes += 1;
						}
					}

					if (bSlowndownRefreshFreq)
						Sleep(settings.m_waitTime);  //Slow down the refresh frequency, otherwise updating maybe cause CTD.

					static NiAVObject::ControllerUpdateContext ctx;
					ctx.delta = 0.0f;
					ctx.flags = 0;
					this->unk3C->UpdateNode(&ctx);

					return 0;
				}
			}

			if (msg->type == UIMessage::kMessage_Message)
			{
				InputStringHolder* input = InputStringHolder::GetSingleton();
				BSUIMessageData* msgData = static_cast<BSUIMessageData*>(msg->data);

				if (msgData->unk0C == input->prevPage || msgData->unk0C == input->nextPage || msgData->unk0C == input->leftEquip || msgData->unk0C == input->rightEquip)
				{
					//double iPageSetIndex = this->bookView->GetVariableDouble("BookMenu.BookMenuInstance.iPageSetIndex");
					//bSlowndownRefreshFreq = true;

					mm->numPauseGame += 1;
					Sleep(settings.m_delayTime);

					UInt32 result = (this->*fnProcessMessage)(msg);

					mm->numPauseGame -= 1;

					return result;

					//bSlowndownRefreshFreq

				}

				else if (msgData->unk0C == input->accept)
				{
					if (!*(bool*)0x1B3E5D4 || this->disableCloseMsg)
						return 0;
					GlobalBookData* data = GlobalBookData::GetSingleton();
					if (data->reference != nullptr)
					{
						bSlowndownRefreshFreq = false;
						g_thePlayer->PickUpItem(data->reference, 1, false, true);
						this->SendAnimationEvent("SoundPlay");
						this->SendAnimationEvent("CloseOut");
						mm->CloseMenu(holder->bookMenu);

						return 1;
					}
				}
				else if (msgData->unk0C == input->cancel)
				{
					if (!*(bool*)0x1B3E5D4 || this->disableCloseMsg)
						return 0;
					bSlowndownRefreshFreq = false;
					this->SendAnimationEvent("SoundPlay");
					this->SendAnimationEvent("CloseOut");
					mm->CloseMenu(holder->bookMenu);

					return 1;
				}
			}
			return (this->*fnProcessMessage)(msg);
		}
		else
		{
			return (this->*fnProcessMessage)(msg);
		}

	}

	static void STDFN Hook_SetBookText()
	{
		auto fn = []()->bool {
			std::this_thread::sleep_for(std::chrono::milliseconds(settings.m_waitTime));
			MenuManager* mm = MenuManager::GetSingleton();
			UIStringHolder* holder = UIStringHolder::GetSingleton();
			if (holder && mm && mm->IsMenuOpen(holder->bookMenu))
			{
				IMenu* bookMenu = mm->GetMenu(holder->bookMenu);
				if ((bookMenu->flags & IMenu::kType_PauseGame) == IMenu::kType_PauseGame && settings.m_menuConfig["Book Menu"])
				{
					mm->numPauseGame -= 1;
					bookMenu->flags &= ~IMenu::kType_PauseGame;
					if (!mm->numPauseGame)
					{
						static MenuModeChangeEvent event;
						event.unk0 = 0;
						event.menuMode = 0;
						mm->BSTEventSource<MenuModeChangeEvent>::SendEvent(&event);
						typedef void(__fastcall * Fn)(void*, void*); //MenuModeChangeEvent
						Fn fn = (Fn)0xA511B0;
						fn(&event, nullptr);
					}
				}
			}
			return true;
		};
		really_async(fn);
	}

	static void InitHook()
	{
		fnProcessMessage = SafeWrite32(0x010E3AA4 + 0x04 * 4, &ProcessMessage_Hook);

		fnReceiveEvent = SafeWrite32(0x010E3A98 + 0x01 * 4, &ReceiveEvent_Hook);

		//SafeWrite16(0x846C73, 0x00EB);
		//{
		//	static const UInt32 kHook_SetBookText_Call = 0x00841A90;

		//	START_ASM(Hook_SetBookText, 0x00845EFE, 0x00845F0A, 1)

		//		add esp, 0xC
		//		lea ecx, [esp + 0x2C]
		//		call [kHook_SetBookText_Call]
		//		pushad
		//		call Hook_SetBookText
		//		popad

		//	END_ASM(Hook_SetBookText)
		//}

	}

//members:
	UInt32					unk2C;					// ini'd 0   char*  bookContent?
	UInt32					unk30;
	UInt32					unk34;
	GFxMovieView*			bookView;				// ini'd 0   "Book" swf name?   //__cdecl sub_926020(unk38, "BookMenu.BookMenuInstance.PrepForClose", 0);  //invoke???
	NiAVObject*				unk3C;					// ini'd 0   !=0 can process message.  //bookContent
	UInt32					unk40;					// ini'd 0
	UInt32					unk44;					// ini'd 0
	UInt32					unk48;					// ini'd 0
	UInt32					unk4C;					// ini'd 0
	NiSourceTexture*		unk50;					// ini'd 0
	NiTriShape*				unk54;					// ini'd 0
	UInt16					unk58;					// ini'd 0
	UInt16					refreshTimes;			// ini'd 0  0 ~ 6
	UInt8					disableCloseMsg;		// ini'd 0  ==1 can't process close msg. after process a close msg, this will be set to 1;
	bool					isNote;					// ini'd 0  0x5D
	bool					isInit;					// ini'd 0  bookConten initial?
	UInt8					padding;				// ini'd 0

	DEFINE_MEMBER_FN(InitBookMenu, void, 0x008451C0);
	//DEFINE_MEMBER_FN(CreateBookContent, void, 0x00845D60);
	DEFINE_MEMBER_FN(CreateBookContent, void, 0x00845F70);
	//void __cdecl sub_C746A0(int a1, int a2, int a3) sub_C746A0(*(_DWORD *)(v2 + 0x3C), 1, 0);

};
static_assert(sizeof(BookMenu) == 0x60, "BOOKMENU");

BookMenu::FnProcessMessage	BookMenu::fnProcessMessage = nullptr;
BookMenu::FnReceiveEvent	BookMenu::fnReceiveEvent = nullptr;
bool						BookMenu::enableMenuControl = false;
bool						BookMenu::enableCloseMenu = false;
bool						BookMenu::bSlowndownRefreshFreq = false;




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
		MenuManager* manager = MenuManager::GetSingleton();
		UIStringHolder* holder = UIStringHolder::GetSingleton();

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
					Fn fn = (Fn)0xA511B0;
					fn(&event, nullptr);
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
			GFxMovieView* view = this->GetMovieView();
			if (view != nullptr)
			{
				//view->SetVisible(false);
			}
		}
		return (this->*fnProcessMessage)(msg);
	}

	static void InitHook()
	{
		fnProcessMessage = SafeWrite32(0x010E4C9C + 0x04 * 4, &ProcessMessage_Hook);
	}
};

DialogueMenuEx::FnProcessMessage	DialogueMenuEx::fnProcessMessage = nullptr;




class TrainingMenu : public IMenu
{
public:
	typedef UInt32(TrainingMenu::*FnProcessMessage)(UIMessage*);

	static FnProcessMessage fnProcessMessage;


	UInt32 ProcessMessage_Hook(UIMessage* msg)
	{
		UInt32 result = (this->*fnProcessMessage)(msg);
		if (msg->type == UIMessage::kMessage_Message)
		{
			InputStringHolder* input = InputStringHolder::GetSingleton();
			BSUIMessageData* msgData = static_cast<BSUIMessageData*>(msg->data);
			_MESSAGE("msgData: %s", msgData->unk0C.c_str());
		}

		return result;
	}

	static void InitHook()
	{
		fnProcessMessage = SafeWrite32(0x10E8004 + 0x04 * 4, &ProcessMessage_Hook);
	}
};

TrainingMenu::FnProcessMessage	TrainingMenu::fnProcessMessage = nullptr;




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

				void(__cdecl* sub_861470)(void*) = (void(__cdecl*)(void*))0x861470;  //Enable prompt.
				sub_861470((void*)0x10E3824);

				InputMappingManager* input = InputMappingManager::GetSingleton();
				input->DisableControl(4);
				input->DisableControl(3);
				input->DisableControl(1);

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
		}
		else
			this->Release();
	}

	UInt32 ProcessMessage_Hook(UIMessage* msg)
	{
		UInt32 result = (this->*fnProcessMessage)(msg);
		if (msg->type == UIMessage::kMessage_Message)
		{
			InputStringHolder* input = InputStringHolder::GetSingleton();
			BSUIMessageData* msgData = static_cast<BSUIMessageData*>(msg->data);
			_MESSAGE("msgData: %s", msgData->unk0C.c_str());
		}
		return result;
	}

	static void InitHook()
	{
		//fnProcessMessage = SafeWrite32(0x010E5B90 + 0x04 * 4, &ProcessMessage_Hook);
		WriteRelCall(0x00A5D686, &Release_Hook);
	}
};

InventoryMenuEx::FnProcessMessage	InventoryMenuEx::fnProcessMessage = nullptr;





class PlayerControlsEx : public BSTEventSink<MenuModeChangeEvent>
{
public:	
	typedef EventResult(PlayerControlsEx::*FnReceiveEvent)(MenuModeChangeEvent *, BSTEventSource<MenuModeChangeEvent> *);
	static FnReceiveEvent fnReceiveEvent;

	EventResult ReceiveEvent_Hook(MenuModeChangeEvent *evn, BSTEventSource<MenuModeChangeEvent> *source)// override;	// 00772B10
	{
		EventResult result = (this->*fnReceiveEvent)(evn, source);
		_MESSAGE("menuMode: %d, unk:%d", evn->menuMode, *((UInt8*)this + 54));
		return result;
	}

	static void InitHook()
	{
		fnReceiveEvent = SafeWrite32(0x010D4668 + 0x01 * 4, &ReceiveEvent_Hook);
	}
};

PlayerControlsEx::FnReceiveEvent	PlayerControlsEx::fnReceiveEvent = nullptr;



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
				}
			}
		}
	}
}


void UICallBack_CloseTweenMenu(FxDelegateArgs* pargs)
{
	PlayerCamera* camera = PlayerCamera::GetSingleton();
	camera->ResetCamera();
	MenuManager* mm = MenuManager::GetSingleton();
	UIStringHolder* holder = UIStringHolder::GetSingleton();
	mm->CloseMenu(holder->tweenMenu);

	IMenu* inventoryMenu = reinterpret_cast<IMenu*>(pargs->pThisMenu);
	*((bool*)inventoryMenu + 0x51) = 1;
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

	//fix inventory
	SafeWrite16(0x0086BF6F, 0x9090);
	SafeWrite32(0x0086BF71, 0x90909090);

	//fix container
	SafeWrite16(0x0084C068, 0x9090);
	SafeWrite32(0x0084C06A, 0x90909090);

	SafeWrite16(0x0084AC57, 0x9090);
	SafeWrite32(0x0084AC59, 0x90909090);

	//drop item in inventory
	SafeWrite32(0x0086B437, (UInt32)UICallBack_DropItem);

	//fix camera in tweenmenu:
	SafeWrite8(0x008951C4, 0x90);
	SafeWrite32(0x008951C5, 0x90909090);

	//redefine papyrus function: Utility.IsInMenuMode()
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
			jmp	[kHook_GetFavoritesItem_Jmp]

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

