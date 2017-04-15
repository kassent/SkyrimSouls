#pragma execution_character_set("utf-8")

#include "Tools.h"
#include <SKSE.h>
#include <SKSE/GameRTTI.h>
#include <SKSE/GameEvents.h>
#include <Skyrim/Forms/TESShout.h>
#include <Skyrim/Forms/PlayerCharacter.h>
#include <Skyrim/Forms/BGSEquipSlot.h>
#include <Skyrim/FormComponents/TESFullName.h>
#include "Serialization.h"

//冷却时间只有在被卸下的时候更新
//#define DEBUG_LOG 1

EventResult ReceiveEvent_LoadGameEvent(TESLoadGameEvent * evn, BSTEventSource<TESLoadGameEvent> * dispatcher)
{
#ifdef DEBUG_LOG
	_MESSAGE("> Game Loaded...");
#endif
	for (auto& pair : storageData)
	{
		pair.second.recordPoint = std::chrono::time_point_cast<ShoutCooldownInfo::ms_duration>(std::chrono::system_clock::now());
	}
	if (g_thePlayer != nullptr)
	{
		TESForm* form = g_thePlayer->equippedShout;
		TESShout* shout = DYNAMIC_CAST<TESShout*>(form);
		if(shout != nullptr)
		{
			auto it = storageData.find(shout);
			if (it != storageData.end())
			{
				signed long long count = it->second.cooldownTime.count();
				if (count <= 0)
					g_thePlayer->SetVoiceRecoveryTime(0.0f);
				else
					g_thePlayer->SetVoiceRecoveryTime(static_cast<float>(count) / 1000);
			}
			else
				g_thePlayer->SetVoiceRecoveryTime(0.0f);
		}
		else
			g_thePlayer->SetVoiceRecoveryTime(0.0f);
	}
	return kEvent_Continue;
}

EventResult ReceiveEvent_WaitStartEvent(TESWaitStartEvent * evn, BSTEventSource<TESWaitStartEvent> * dispatcher)
{
	if (g_thePlayer != nullptr)
	{
		g_thePlayer->SetVoiceRecoveryTime(0.0f);
		ShoutCooldownInfo::ms_duration duration(0);
		for (auto& pair : storageData)
			pair.second.cooldownTime = duration;
	}
#ifdef DEBUG_LOG
	_MESSAGE("> WaitStartEvent: current: %.2f desired:%.2f", evn->current, evn->desired);
#endif
	return kEvent_Continue;
}

EventResult ReceiveEvent_SleepStartEvent(TESSleepStartEvent * evn, BSTEventSource<TESSleepStartEvent> * dispatcher)
{
	if (g_thePlayer != nullptr)
	{
		g_thePlayer->SetVoiceRecoveryTime(0.0f);
		ShoutCooldownInfo::ms_duration duration(0);
		for (auto& pair : storageData)
			pair.second.cooldownTime = duration;
	}
#ifdef DEBUG_LOG
	_MESSAGE("> SleepStartEvent: current: %.2f desired:%.2f", evn->startTime, evn->endTime);
#endif
	return kEvent_Continue;
}


void __stdcall OnEquipShout(Actor* actor, TESForm* form)
{
#ifdef DEBUG_LOG
	_MESSAGE(__FUNCTION__);
#endif
	PlayerCharacter* player = DYNAMIC_CAST<PlayerCharacter*>(actor);

	if (player != nullptr)
	{
		TESFullName *fullName = nullptr;
		TESForm* oldForm = player->equippedShout;
		TESShout* oldShout = DYNAMIC_CAST<TESShout*>(oldForm);
		if (oldShout != nullptr)
		{
#ifdef DEBUG_LOG
			fullName = DYNAMIC_CAST<TESFullName*>(oldShout);
			_MESSAGE("oldShoutName: %s", fullName->GetFullName());
#endif
			float recoveryTime = player->GetVoiceRecoveryTime();
			auto it = storageData.find(oldShout);
			if (it != storageData.end())
			{
				ShoutCooldownInfo info(recoveryTime);
				it->second = info;
			}
			else
				storageData.insert({ oldShout , ShoutCooldownInfo(recoveryTime) });
		}
		TESShout* newShout = DYNAMIC_CAST<TESShout*>(form);
		if (newShout != nullptr)
		{
#ifdef DEBUG_LOG
			TESFullName *fullName = DYNAMIC_CAST<TESFullName*>(newShout);
			_MESSAGE("newShoutName: %s", fullName->GetFullName());
			_MESSAGE("recoveryTime: %.2f", player->GetVoiceRecoveryTime());
#endif
			auto it = storageData.find(newShout);
			if (it != storageData.end())
			{
				signed long long count = it->second.cooldownTime.count();
				if(!count)
					player->SetVoiceRecoveryTime(0.0f);
				ShoutCooldownInfo::ms_duration duration = std::chrono::time_point_cast<ShoutCooldownInfo::ms_duration>(std::chrono::system_clock::now()) - it->second.recordPoint;
				count = it->second.cooldownTime.count() - duration.count();
				if (count <= 0)
					player->SetVoiceRecoveryTime(0.0f);
				else
					player->SetVoiceRecoveryTime(static_cast<float>(count) / 1000);
			}
			else
				player->SetVoiceRecoveryTime(0.0f);
		}
	}
}

void __stdcall OnUnequipShout(Actor* actor, TESForm* form)
{
#ifdef DEBUG_LOG
	_MESSAGE(__FUNCTION__);
#endif
	PlayerCharacter* player = DYNAMIC_CAST<PlayerCharacter*>(actor);
	TESShout* shout = DYNAMIC_CAST<TESShout*>(form);
	if (player != nullptr && shout != nullptr)
	{
#ifdef DEBUG_LOG
		TESFullName *fullName = DYNAMIC_CAST<TESFullName*>(shout);
		_MESSAGE("unequipedShoutName: %s", fullName->GetFullName());
#endif
		float recoveryTime = player->GetVoiceRecoveryTime();
#ifdef DEBUG_LOG
		_MESSAGE("recoveryTime: %.2f", recoveryTime);
#endif
		auto it = storageData.find(shout);
		if (it != storageData.end())
		{
			ShoutCooldownInfo info(recoveryTime);
			it->second = info;
		}
		else
			storageData.insert({ shout , ShoutCooldownInfo(recoveryTime) });
		player->SetVoiceRecoveryTime(0.0f);
	}
}

void __stdcall OnEquipSpell(Actor* actor, TESForm*  form, BGSEquipSlot* slot)
{
#ifdef DEBUG_LOG
	_MESSAGE(__FUNCTION__);
#endif
	BGSEquipSlot* targetEquipSlot = nullptr;
	void* (__cdecl * sub_44AA10)(TESForm*) = (void* (__cdecl*)(TESForm*))0x44AA10;
	void* unk0 = sub_44AA10(form);//cdecl
	if (unk0)
	{
		typedef BGSEquipSlot* (__fastcall* Fn)(void*, void*);
		Fn fn = (Fn)(*(UInt32*)(*(UInt32*)unk0 + 0x10));//????
		targetEquipSlot = fn(unk0, nullptr);
	}
	else
		targetEquipSlot = nullptr;

	typedef BGSEquipSlot * (*_GetVoiceSlot)();
	const _GetVoiceSlot GetVoiceSlot = (_GetVoiceSlot)0x0054C8A0;

	if (targetEquipSlot == GetVoiceSlot())
	{
		PlayerCharacter* player = DYNAMIC_CAST<PlayerCharacter*>(actor);
		SpellItem* spell = DYNAMIC_CAST<SpellItem*>(form);
		if (player != nullptr && spell != nullptr)
		{
			TESForm* oldForm = player->equippedShout;
			TESShout* oldShout = DYNAMIC_CAST<TESShout*>(oldForm);
			if (oldShout != nullptr)
			{
				TESFullName* fullName = DYNAMIC_CAST<TESFullName*>(oldShout);
#ifdef DEBUG_LOG
				_MESSAGE("oldShoutName: %s", fullName->GetFullName());
#endif
				float recoveryTime = player->GetVoiceRecoveryTime();
				auto it = storageData.find(oldShout);
				if (it != storageData.end())
				{
					ShoutCooldownInfo info(recoveryTime);
					it->second = info;
				}
				else
					storageData.insert({ oldShout , ShoutCooldownInfo(recoveryTime) });
			}
			player->SetVoiceRecoveryTime(0.0f);
		}
	}
}


void Hook_Shouts_Commit()
{

	AddEventSink<TESLoadGameEvent>(ReceiveEvent_LoadGameEvent);

	AddEventSink<TESWaitStartEvent>(ReceiveEvent_WaitStartEvent);

	AddEventSink<TESSleepStartEvent>(ReceiveEvent_SleepStartEvent);

	{
		START_ASM(Hook_EquipShout, 0x006EFC20, 0x006EFC25, 1);
			push ecx
			mov eax, [esp + 0xC]
			push eax
			mov eax, [esp + 0xC]
			push eax
			call OnEquipShout
			pop ecx
			sub esp, 0x10
			push ebx
			push ebp
		END_ASM(Hook_EquipShout);
	}

	{
		START_ASM(Hook_UnequipShout, 0x006EFEB0, 0x006EFEB5, 1);
			push ecx
			mov eax, [esp + 0xC]
			push eax
			mov eax, [esp + 0xC]
			push eax
			call OnUnequipShout
			pop ecx
			push ebx
			mov ebx, [esp + 0xC]
		END_ASM(Hook_UnequipShout);
	}

	{
		START_ASM(Hook_EquipSpell, 0x006EF4B0, 0x006EF4B8, 1)
			push ecx
			mov eax, [esp + 0x10]
			push eax
			mov eax, [esp + 0x10]
			push eax
			mov eax, [esp + 0x10]
			push eax
			call OnEquipSpell
			pop ecx
			sub esp, 8
			push ebx
			mov ebx, [esp + 0x10]
		END_ASM(Hook_EquipSpell)
	}
}
