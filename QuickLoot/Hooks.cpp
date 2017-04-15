#include <SKSE.h>
#include <SKSE/DebugLog.h>
#include <SKSE/GameForms.h>
#include <SKSE/GameObjects.h>
#include <SKSE/GameEvents.h>
#include <SKSE/GameMenus.h>
#include <SKSE/GameInput.h>
#include <SKSE/GameRTTI.h>
#include <SKSE/SafeWrite.h>

#include "Hooks.h"
#include "LootMenu.h"

//========================================================
// input patches
//========================================================

// ルートメニューが開いてる場合は、マウスホイールでカメラが移動しないよう書き換える
template <UInt32 VTBL>
class CameraStateHandler : public PlayerInputHandler
{
public:
	typedef bool(CameraStateHandler::*FnCanProcess)(InputEvent* evn);

	static FnCanProcess fnCanProcessOld;

	bool CanProcess_Hook(InputEvent* evn)
	{
		bool result = (this->*fnCanProcessOld)(evn);
		LootMenu *lootMenu = LootMenu::GetSingleton();

		if (result && lootMenu && lootMenu->IsVisible())
		{
			result = (evn->GetControlID() == InputStringHolder::GetSingleton()->togglePOV);
		}

		return result;
	}

	static void InitHook()
	{
		fnCanProcessOld = SafeWrite32(VTBL + 4, &CanProcess_Hook);
	}
};

template <UInt32 VTBL> typename CameraStateHandler<VTBL>::FnCanProcess CameraStateHandler<VTBL>::fnCanProcessOld;

typedef CameraStateHandler<0x010E29D4> ThirdPersonStateHandler;
typedef CameraStateHandler<0x010E30B0> FirstPersonStateHandler;


class FavoritesHandler : public MenuEventHandler
{
public:
	typedef bool(FavoritesHandler::*FnCanProcess)(InputEvent *);

	static FnCanProcess fnCanProcess;

	bool CanProcess_Hook(InputEvent *evn)
	{
		bool result = (this->*fnCanProcess)(evn);
		LootMenu *lootMenu = LootMenu::GetSingleton();

		if (result && lootMenu && lootMenu->IsVisible())
		{
			if (evn->deviceType == BSInputDevice::kType_Gamepad && evn->eventType == InputEvent::kEventType_Button)
			{
				ButtonEvent *button = static_cast<ButtonEvent *>(evn);
				result = (button->keyMask != KeyCode::Gamepad_Up && button->keyMask != KeyCode::Gamepad_Down);
			}
		}

		return result;
	}

	static void InitHook()
	{
		fnCanProcess = SafeWrite32(0x010E6A38 + 0x01 * 4, &CanProcess_Hook);
	}
};

FavoritesHandler::FnCanProcess FavoritesHandler::fnCanProcess;



#ifdef USE_JUMP_KEY
// ルートメニューが開いている場合は、ジャンプしないよう書き換える
class JumpHandlerEx : public JumpHandler
{
public:
	typedef bool(JumpHandlerEx::*FnCanProcess)(InputEvent* evn);

	static FnCanProcess fnCanProcessOld;

	bool CanProcess_Hook(InputEvent* evn)
	{
		LootMenu *lootMenu = LootMenu::GetSingleton();
		return (lootMenu && lootMenu->IsVisible()) ? false : (this->*fnCanProcessOld)(evn);
	}

	static void InitHook()
	{
		fnCanProcessOld = SafeWrite32(0x010D460C + 4, &CanProcess_Hook);
	}
};

JumpHandlerEx::FnCanProcess JumpHandlerEx::fnCanProcessOld;
#endif


// ルートメニューが開いている場合は、武器構えボタンでコンテナが開くよう書き換える
class ReadyWeaponHandlerEx : public ReadyWeaponHandler
{
public:
	typedef void(ReadyWeaponHandlerEx::*FnProcessButton)(ButtonEvent* evn, PlayerControls::Data14 *data);

	static FnProcessButton fnProcessButtonOld;

	void ProcessButton_Hook(ButtonEvent* evn, PlayerControls::Data14 *data)
	{
		LootMenu *lootMenu = LootMenu::GetSingleton();

		static bool bProcessLongTap = false;

		if (!evn->IsDown())
		{
			if (bProcessLongTap && evn->timer > 2.0f)
			{
				bProcessLongTap = false;

				if (lootMenu && !lootMenu->IsVisible())
				{
					bool disabled = !lootMenu->IsDisabled();
					lootMenu->SetDisabled(disabled);
					
					typedef void(*FnDebug_Notification)(const char *, bool, bool);
					const FnDebug_Notification fnDebug_Notification = (FnDebug_Notification)0x008997A0;

					fnDebug_Notification((disabled ? "QuickLoot has been disabled." : "QuickLoot has been enabled."), false, true);
				}
			}

			return;
		}

		bProcessLongTap = true;

		if (lootMenu && lootMenu->IsVisible())
		{
			g_thePlayer->StartActivation();
		}
		else
		{
			(this->*fnProcessButtonOld)(evn, data);
		}
	}

	static void InitHook()
	{
		fnProcessButtonOld = SafeWrite32(0x010D45C4 + 0x04 * 4, &ProcessButton_Hook);
	}
};

ReadyWeaponHandlerEx::FnProcessButton ReadyWeaponHandlerEx::fnProcessButtonOld;


// アクティベート時の動作を書き換える
class PlayerCharacterEx : public PlayerCharacter
{
public:
	typedef void(PlayerCharacterEx::*FnStartActivation)();

	static FnStartActivation fnStartActivationOld;

	void StartActivation_Hook()
	{
		LootMenu *lootMenu = LootMenu::GetSingleton();

		if (lootMenu && lootMenu->IsVisible())
		{
			lootMenu->TakeItem();
		}
		else
		{
			(this->*fnStartActivationOld)();
		}
	}

	static void InitHook()
	{
		fnStartActivationOld = WriteRelCall(0x0077265A, &StartActivation_Hook);
	}
};

PlayerCharacterEx::FnStartActivation PlayerCharacterEx::fnStartActivationOld;


//========================================================
// crosshair text patches
//========================================================

template <UInt32 VTBL>
class TESBoundObjectEx : public TESObjectCONT
{
public:
	typedef bool(TESBoundObjectEx::*FnGetCrosshairText)(TESObjectREFR *ref, BSString * dst, bool arg3);

	static FnGetCrosshairText fnGetCrosshairText;

	bool GetCrosshairText_Hook(TESObjectREFR *ref, BSString * dst, bool unk)
	{
		UInt32 retnAddr = *(UInt32*)(&ref - 1);

		if (retnAddr == 0x0073A136 && LootMenu::CanOpen(ref))
			return false;

		return (this->*fnGetCrosshairText)(ref, dst, unk);
	}

	static void InitHook()
	{
		fnGetCrosshairText = SafeWrite32(VTBL + 0x4D * 4, &GetCrosshairText_Hook);
	}
};

template <UInt32 VTBL>
typename TESBoundObjectEx<VTBL>::FnGetCrosshairText TESBoundObjectEx<VTBL>::fnGetCrosshairText;

typedef TESBoundObjectEx<0x01084D44> TESObjectCONTEx;
typedef TESBoundObjectEx<0x010841B4> TESObjectACTIEx;	// for ash pile
typedef TESBoundObjectEx<0x010A5964> TESNPCEx;


//========================================================
// HDT patch
//========================================================

#include <map>

class FreezeEventHandler : public BSTEventSink<MenuOpenCloseEvent>
{
public:
	typedef EventResult(FreezeEventHandler::*FnReceiveEvent)(MenuOpenCloseEvent *evn, BSTEventSource<MenuOpenCloseEvent> *src);

	static std::map<UInt32, FnReceiveEvent> ms_handlerMap;

	UInt32 GetVPtr() const
	{
		return *(UInt32*)this;
	}

	EventResult ReceiveEvent_Hook(MenuOpenCloseEvent *evn, BSTEventSource<MenuOpenCloseEvent> *src)
	{
		static BSFixedString menuName = "Loot Menu";

		if (evn->menuName == menuName)
		{
			_MESSAGE("* hdt-pe sink is skipped");
			return kEvent_Continue;
		}

		auto it = ms_handlerMap.find(GetVPtr());
		if (it == ms_handlerMap.end())
			return kEvent_Continue;

		FnReceiveEvent handler = it->second;
		return (this->*handler)(evn, src);
	}

	void InstallHook()
	{
		UInt32 vptr = GetVPtr();
		FreezeEventHandler::FnReceiveEvent pFn = SafeWrite32(vptr + 4, &FreezeEventHandler::ReceiveEvent_Hook);
		ms_handlerMap[vptr] = pFn;
	}
};

std::map<UInt32, FreezeEventHandler::FnReceiveEvent> FreezeEventHandler::ms_handlerMap;


class MenuOpenCloseEventSource : public BSTEventSource<MenuOpenCloseEvent>
{
public:
	void ProcessHook()
	{
		lock.Lock();

		BSTEventSink<MenuOpenCloseEvent> *sink;
		UInt32 idx = 0;
		while (eventSinks.GetAt(idx, sink))
		{
			const char * className = GetObjectClassName(sink);
			if (strcmp(className, "class FreezeEventHandler") == 0)
			{
				FreezeEventHandler *freezeEventHandler = static_cast<FreezeEventHandler *>(sink);
				_MESSAGE("<< hdtPhysicsExtensions detected %p>>", freezeEventHandler->GetVPtr());

				freezeEventHandler->InstallHook();
			}

			++idx;
		}

		lock.Unlock();
	}


	static void InitHook()
	{
		MenuManager *mm = MenuManager::GetSingleton();
		if (mm)
		{
			MenuOpenCloseEventSource *pThis = static_cast<MenuOpenCloseEventSource*>(mm->GetMenuOpenCloseEventSource());
			pThis->ProcessHook();
		}
	}
};

//========================================================

namespace Hooks
{
	void Install()
	{
		// input patches
		if (InputManager::GetSingleton()->IsGamepadEnabled())
		{
			FavoritesHandler::InitHook();			// directional pad
		}
		else
		{
			ThirdPersonStateHandler::InitHook();	// mouse wheel
			FirstPersonStateHandler::InitHook();	// mouse wheel
		}
#ifdef USE_JUMP_KEY
		JumpHandlerEx::InitHook();					// [Space] or (Y)
#endif
		ReadyWeaponHandlerEx::InitHook();			// [R] or (X)
		PlayerCharacterEx::InitHook();				// [E] or (A)

		// crosshair text patches
		TESObjectCONTEx::InitHook();
		TESObjectACTIEx::InitHook();
		TESNPCEx::InitHook();

		// HDT patch
		MenuOpenCloseEventSource::InitHook();
	}
}
