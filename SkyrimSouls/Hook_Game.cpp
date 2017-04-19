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
#include <Skyrim/Forms/Character/Components/ActorProcessManager.h>
#include <Skyrim/Animation/IAnimationGraphManagerHolder.h>
#include <future>

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
		form = nullptr;
	return form;
}


void STDFN Hook_KillActor(Actor* actor)
{
	if (DYNAMIC_CAST<PlayerCharacter*>(actor) != nullptr)
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
		if(isProcessing)
			Sleep(200);
		_MESSAGE("> Player was killed by something, try to close all menus to avoid issues...");
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
		fnCanProcess = SafeWrite32(0x010E4DFC + 0x01 * 4, &CanProcess_Hook);
	}
};

FavoritesHandler::FnCanProcess FavoritesHandler::fnCanProcess;

#include <Skyrim\NetImmerse\NiAVObject.h>

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

	EventResult	ReceiveEvent_Hook(BSAnimationGraphEvent * evn, BSTEventSource<BSAnimationGraphEvent> * source)
	{
		//animatinName: OpenStop
		//animatinName: PageForward
		//animatinName: PageBack
		//animatinName: SoundPlay
		//animatinName: CloseOut
		EventResult result = (this->*fnReceiveEvent)(evn, source);
		MenuManager* manager = MenuManager::GetSingleton();
		UIStringHolder* holder = UIStringHolder::GetSingleton();
		if (evn->animName == "OpenStop" && settings.m_menuConfig["Book Menu"])
		{
			if (holder && manager && manager->IsMenuOpen(holder->bookMenu))
			{
				auto fn = []()->bool {
					std::this_thread::sleep_for(std::chrono::milliseconds(settings.m_disableTime));///
					enableCloseMenu = true;
					return true;
				};
				really_async(fn);
			}
		}
		return result;
	}

	UInt32 ProcessMessage_Hook(UIMessage* msg)
	{
		if (settings.m_menuConfig["Book Menu"])
		{
			static MenuManager* mm = MenuManager::GetSingleton();
			static UIStringHolder* holder = UIStringHolder::GetSingleton();

			static bool bSlowndownRefreshFreq = true;

			if (msg->type == UIMessage::kMessage_Open)
			{
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
									Fn sendEvent = (Fn)0xA511B0;
									sendEvent(&event, nullptr);
								}
							}
							this->refreshTimes += 1;
						}
						if 	(bSlowndownRefreshFreq)
							Sleep(settings.m_waitTime);  //Slow down the refresh frequency, otherwise updating maybe cause CTD.
					}

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
					mm->numPauseGame += 1;
					Sleep(settings.m_delayTime);

					UInt32 result = (this->*fnProcessMessage)(msg);

					mm->numPauseGame -= 1;

					return result;

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
						Fn callback = (Fn)0xA511B0;
						callback(&event, nullptr);
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

		//fnReceiveEvent = SafeWrite32(0x010E3A98 + 0x01 * 4, &ReceiveEvent_Hook);

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
			if (settings.m_menuConfig["Lockpicking Menu"] && (lockpickingMenu->flags & IMenu::kType_PauseGame) == IMenu::kType_PauseGame)
			{
				manager->numPauseGame -= 1;
				lockpickingMenu->flags &= ~IMenu::kType_PauseGame;
				if (!manager->numPauseGame)
				{
					static MenuModeChangeEvent event;
					event.unk0 = 0;
					event.menuMode = 0;
					manager->BSTEventSource<MenuModeChangeEvent>::SendEvent(&event);
					typedef void(__fastcall * Fn)(void*, void*); //MenuModeChangeEvent
					Fn fn = (Fn)0xA511B0;
					fn(&event, nullptr);
				}
			}
		}
	}

	static void InitHook()
	{
		//fnProcessMessage = SafeWrite32(0x010E6308 + 0x04 * 4, &ProcessMessage_Hook);

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




void Hook_Game_Commit()
{
	FavoritesHandler::InitHook();
	BookMenu::InitHook();
	LockpickingMenu::InitHook();
	//ConsoleMenu::InitHook();
	//WriteRelCall(0x00869E0B, (UInt32)AttempEquip);

	//Fix inventory
	SafeWrite16(0x0086BF6F, 0x9090);
	SafeWrite32(0x0086BF71, 0x90909090);

	//Fix container
	SafeWrite16(0x0084C068, 0x9090);
	SafeWrite32(0x0084C06A, 0x90909090);

	SafeWrite16(0x0084AC57, 0x9090);
	SafeWrite32(0x0084AC59, 0x90909090);

	//Redefine papyrus function: IsInMenuMode
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
}

