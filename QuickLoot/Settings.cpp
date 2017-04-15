#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Settings.h"
#include <SKSE.h>
#include <SKSE/DebugLog.h>
#include <SKSE/GameStreams.h>
#include <string>

bool	Settings::bDisableInCombat = false;
bool	Settings::bDisableTheft = false;
bool	Settings::bDisablePickpocketing = false;
bool	Settings::bDisableIfEmpty = false;
bool	Settings::bCloseIfEmpty = false;
bool	Settings::bUseConsole = false;
bool	Settings::bForceAnsi = false;
bool	Settings::bDisableLootSingle = false;
int		Settings::iOpacity = 25;
int		Settings::iScale = 100;
int		Settings::iPositionX = -1;
int		Settings::iPositionY = -1;
int		Settings::iItemLimit = -1;

bool Settings::SetBool(const char *name, bool val)
{
	if (_stricmp(name, "bDisableInCombat") == 0)
	{
		bDisableInCombat = val;
	}
	else if (_stricmp(name, "bDisableTheft") == 0)
	{
		bDisableTheft = val;
	}
	else if (_stricmp(name, "bDisablePickpocketing") == 0)
	{
		bDisablePickpocketing = val;
	}
	else if (_stricmp(name, "bDisableIfEmpty") == 0)
	{
		bDisableIfEmpty = val;
	}
	else if (_stricmp(name, "bCloseIfEmpty") == 0)
	{
		bCloseIfEmpty = val;
	}
	else if (_stricmp(name, "bUseConsole") == 0)
	{
		bUseConsole = val;
	}
	else if (_stricmp(name, "bForceAnsi") == 0)
	{
		bForceAnsi = val;
	}
	else if (_stricmp(name, "bDisableLootSingle") == 0)
	{
		bDisableLootSingle = val;
	}
	else
	{
		return false;
	}

	_MESSAGE("  %s = %d", name, val);

	return true;
}


bool Settings::SetInt(const char *name, int val)
{
	if (_stricmp(name, "iOpacity") == 0)
	{
		iOpacity = val;
	}
	else if (_stricmp(name, "iScale") == 0)
	{
		iScale = val;
	}
	else if (_stricmp(name, "iPositionX") == 0)
	{
		iPositionX = val;
	}
	else if (_stricmp(name, "iPositionY") == 0)
	{
		iPositionY = val;
	}
	else if (_stricmp(name, "iItemLimit") == 0)
	{
		iItemLimit = val;
	}
	else
	{
		return false;
	}

	_MESSAGE("  %s = %d", name, val);

	return true;
}


bool Settings::Set(const char *name, int val)
{
	if (!name || !name[0])
		return false;

	if (name[0] == 'b')
		return SetBool(name, (val != 0));
	if (name[0] == 'i')
		return SetInt(name, val);

	return false;
}


void Settings::Eval(const char *line)
{
	char buf[256];
	strcpy_s(buf, line);

	char *ctx;
	char *name = strtok_s(buf, "=", &ctx);
	if (name && name[0])
	{
		char *val = strtok_s(nullptr, "", &ctx);
		if (val && val[0])
		{
			if (val[0] == '-')
			{
				Set(name, -atoi(val+1));
			}
			else
			{
				Set(name, atoi(val));
			}
		}
	}
}


void Settings::Load()
{
	BSResourceNiBinaryStream iniStream("SKSE\\QuickLoot.ini");
	if (!iniStream)
		return;

	char buf[256];

	_MESSAGE("Settings::Load()");
	while (true)
	{
		UInt32 len = iniStream.ReadLine(buf, 250);
		if (len == 0)
			break;
		buf[len] = 0;

		Eval(buf);
	}
	_MESSAGE("");
}
