#pragma once

#include <SKSE/GameForms.h>
#include <SKSE/DebugLog.h>
#include <SKSE/PluginAPI.h>
#include <map>
#include <chrono>

struct ShoutCooldownInfo
{
	typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> record_point;//steady_clock
	typedef std::chrono::milliseconds ms_duration;

	ShoutCooldownInfo(float seconds);

	ShoutCooldownInfo(unsigned long long milliseconds);

	ShoutCooldownInfo& ShoutCooldownInfo::operator=(const ShoutCooldownInfo& rhs);

	ms_duration			cooldownTime;
	record_point		recordPoint;
};


class StorageData : public std::map <TESShout*, ShoutCooldownInfo>
{
public:

	void Save(SKSESerializationInterface * intfc, UInt32 kVersion);
	bool Load(SKSESerializationInterface * intfc, UInt32 kVersion);
	void Revert();
	static UInt64 GetHandleByObject(void * src, UInt32 typeID);
	static void* GetObjectByHandle(UInt64 handle, UInt32 typeID);
	//static StorageData* GetSingleton();
};

extern StorageData storageData;