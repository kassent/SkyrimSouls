#pragma once

#include "../BSCore/BSTEvent.h"
#include "../BSCore/BSFixedString.h"

// 08
class MenuOpenCloseEvent
{
public:
	BSFixedString	menuName;	// 00
	bool			opening;	// 04
	char			pad[3];
};

class MenuModeChangeEvent
{
public:
	UInt32 		unk0;
	UInt8 		menuMode;
};
static_assert(sizeof(MenuModeChangeEvent) == 0x8, "Error");

class UserEventEnabledEvent
{
};
