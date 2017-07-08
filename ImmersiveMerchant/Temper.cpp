#include "Temper.h"
#include <Skyrim/ExtraData/ExtraContainerChanges.h>
#include <Skyrim/Forms/TESForm.h>
#include <Skyrim/Forms/PlayerCharacter.h>
#include <SKSE/PluginAPI.h>

//0x20
struct TemperItemData
{
	InventoryEntryData*			entryData;
	UInt32						unk04;
	TESForm*					form;
	float						unk0C;  //current
	float						unk10;  //desired
	float						unk14;
	float						unk18;
	bool						unk1C;
	bool						unk1D;
	bool						unk1E;
	bool						padding;
	DEFINE_MEMBER_FN(Create, TemperItemData*, 0x00856490, InventoryEntryData* entryData, TESForm* form);   //Calculte perks...
};

//0x0C
struct TemperProcessor
{
	struct ExtraDataNode
	{
		BaseExtraList*     extraList;
		ExtraDataNode*	   nextNode;
	};

	TESForm*					baseForm;
	ExtraDataNode*				dataNode;		//extraData;
	SInt32						countDelta;		// 08

	DEFINE_MEMBER_FN(InsertNode, void, 0x00476BA0, BaseExtraList* extraList, bool arg);
	DEFINE_MEMBER_FN(Create, TemperProcessor*, 0x00476AB0, InventoryEntryData* entryData);
};


bool TemperInventoryItem(TESForm* form)
{
	if (form != nullptr && (form->IsArmor() || form->IsWeapon()))
	{
		UInt32 numItems = g_thePlayer->GetNumItems();
		InventoryChanges*(__cdecl* GetInventoryChanges)(TESObjectREFR* ref) = (InventoryChanges*(__cdecl*)(TESObjectREFR* actor))0x00476800;
		InventoryChanges* pChanges = GetInventoryChanges(g_thePlayer);
		InventoryEntryData* pEntry = pChanges->FindEntry(form);
		if (pEntry != nullptr)
		{
			TemperItemData* pTemperItemData = new (FormHeap_Allocate(sizeof(TemperItemData))) TemperItemData();
			pTemperItemData->Create(pEntry, form);
			if (pTemperItemData->unk1E && (pTemperItemData->unk18 - pTemperItemData->unk14) >= 1.0f)
			{
				TemperProcessor* pTemperProcessor = new (FormHeap_Allocate(sizeof(TemperProcessor))) TemperProcessor();
				pTemperProcessor->Create(pEntry);
				if (!pTemperProcessor->dataNode || !pTemperProcessor->dataNode->extraList)
				{
					BaseExtraList* pExtraList = new (FormHeap_Allocate(sizeof(BaseExtraList))) BaseExtraList();
					pExtraList->Init();
					pTemperProcessor->InsertNode(pExtraList, true);
				}
				if (pTemperProcessor->dataNode && pTemperProcessor->dataNode->extraList)
				{
					pTemperProcessor->dataNode->extraList->SetTemperHealth(((pTemperItemData->unk0C > pTemperItemData->unk10) ? pTemperItemData->unk0C : pTemperItemData->unk10));
				}
			}
		}
	}
}


namespace InventoryUtil
{

}