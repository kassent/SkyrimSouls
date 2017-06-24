#include "Skyrim/Misc/InventoryChanges.h"
#include "Skyrim/Forms/TESForm.h"
#include "Skyrim/Forms/PlayerCharacter.h"

InventoryEntryData::InventoryEntryData(TESForm *item, SInt32 count)		// 004750C0
{
	baseForm = item;
	//extraList = new BSSimpleList<BaseExtraList *>;
	extraList = nullptr;
	countDelta = count;
}

InventoryEntryData::~InventoryEntryData()
{
	if (extraList)
	{
		delete extraList;
	}
}

void InventoryEntryData::AddEntryList(BaseExtraList *extra)
{
	if (!extra)
		return;

	if (!extraList)
		extraList = new BSSimpleList<BaseExtraList *>;
	if (extraList)
		extraList->push_back(extra);
}

void InventoryChanges::GetEntry(TESForm * item, BSTArray<InventoryEntryData*>& collector)
{
	if (item->Is(FormType::Ammo))
	{
		if (entryList)
		{
			for (InventoryEntryData *entry : *entryList)
			{
				if (entry->baseForm == item)
				{
					collector.push_back(entry);
					return;
				}
			}
		}
	}
	else
	{
		size_t index = 0;
		InventoryEntryData * pEntryData = GetEntryDataByIndex(index);
		while (pEntryData != nullptr)
		{
			if (pEntryData->baseForm == item)
			{
				collector.push_back(pEntryData);
				return;
			}
			else
			{
				pEntryData = GetEntryDataByIndex(++index);
			}
		}
	}
}

InventoryEntryData * InventoryChanges::FindEntry(TESForm *item) const
{
	if (entryList)
	{
		for (InventoryEntryData *entry : *entryList)
		{
			if (entry->baseForm == item)
				return entry;
		}
	}

	return nullptr;
}
