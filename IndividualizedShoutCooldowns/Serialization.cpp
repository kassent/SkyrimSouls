#include "Serialization.h"
#include <Skyrim/VMState.h>
#include <Skyrim/SkyrimVM.h>

ShoutCooldownInfo::ShoutCooldownInfo(float seconds)
{
	//_MESSAGE("recovery miliseconds: %d", static_cast<UInt32>(seconds * 1000));
	ms_duration duration(static_cast<long long>(seconds * 1000));
	cooldownTime = duration;
	recordPoint = std::chrono::time_point_cast<ms_duration>(std::chrono::system_clock::now());
}

ShoutCooldownInfo::ShoutCooldownInfo(unsigned long long milliseconds)
{
	ms_duration duration(milliseconds);
	cooldownTime = duration;
	recordPoint = std::chrono::time_point_cast<ms_duration>(std::chrono::system_clock::now());
}

ShoutCooldownInfo& ShoutCooldownInfo::ShoutCooldownInfo::operator=(const ShoutCooldownInfo& rhs)
{
	if (this != &rhs)
	{
		this->cooldownTime = rhs.cooldownTime;
		this->recordPoint = rhs.recordPoint;
	}
	return *this;
}



void StorageData::Save(SKSESerializationInterface * intfc, UInt32 kVersion)
{
	intfc->OpenRecord('ISCD', kVersion);

	UInt32 totalRecords = this->size();
	intfc->WriteRecordData(&totalRecords, sizeof(totalRecords));

	for (auto& pair : *this)
	{
		UInt64 shoutHandle = GetHandleByObject(pair.first, TESShout::kTypeID);
		_MESSAGE("Saving shout record:%016I64X", shoutHandle);
		intfc->WriteRecordData(&shoutHandle, sizeof(shoutHandle));
		signed long long count = pair.second.cooldownTime.count();
		if (count == NULL)
		{
			intfc->WriteRecordData(&count, sizeof(count));
			continue;
		}
		ShoutCooldownInfo::ms_duration duration = std::chrono::time_point_cast<ShoutCooldownInfo::ms_duration>(std::chrono::system_clock::now()) - pair.second.recordPoint;
		count = pair.second.cooldownTime.count() - duration.count();
		if (count < 0)
			count = 0;
		intfc->WriteRecordData(&count, sizeof(count));
	}
}

bool StorageData::Load(SKSESerializationInterface* intfc, UInt32 kVersion)
{
	bool error = false;
	UInt32 totalRecord = 0;

	if (!intfc->ReadRecordData(&totalRecord, sizeof(totalRecord)))
	{
		_MESSAGE("%s - Error loading total records from table", __FUNCTION__);
		error = true;
		return error;
	}
	for (UInt32 i = 0; i < totalRecord; i++)
	{
		UInt64 shoutHandle = 0;
		unsigned long long count = 0;
		if (!intfc->ReadRecordData(&shoutHandle, sizeof(shoutHandle)))
		{
			_MESSAGE("%s - Error loading shout form ptr", __FUNCTION__);
			error = true;
			return error;
		}
		TESShout* shout = (TESShout*)GetObjectByHandle(shoutHandle, TESShout::kTypeID);
		if (!shout)
		{
			if (!intfc->ReadRecordData(&count, sizeof(count)))
			{
				_MESSAGE("%s - Error loading time left data", __FUNCTION__);
				error = true;
				return error;
			}
			continue;
		}

		if (!intfc->ReadRecordData(&count, sizeof(count)))
		{
			_MESSAGE("%s - Error loading time left data", __FUNCTION__);
			error = true;
			return error;
		}
		if (count)
			this->insert({ shout, ShoutCooldownInfo(count) });

		_MESSAGE("Load shout record:%016X", shoutHandle);
	}
	return error;
}

void StorageData::Revert()
{
	this->clear();
}

UInt64 StorageData::GetHandleByObject(void * src, UInt32 typeID)
{
	VMState	* registry = g_skyrimVM->GetState();
	BSScript::IObjectHandlePolicy	* policy = registry->GetHandlePolicy2();

	return policy->Create(typeID, (void*)src);
}

void * StorageData::GetObjectByHandle(UInt64 handle, UInt32 typeID)
{
	VMState	* registry = g_skyrimVM->GetState();
	BSScript::IObjectHandlePolicy	* policy = registry->GetHandlePolicy2();

	if (handle == policy->GetInvalidHandle()) {
		return nullptr;
	}
	return policy->Resolve(typeID, handle);
}


StorageData storageData;