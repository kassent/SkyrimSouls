#include "Settings.h"
#include <Windows.h>
#include <SKSE/GameStreams.h>

Settings settings;

bool Settings::Set(const char *name, int val)
{
	if (!name || !name[0])
		return false;

	if (_stricmp(name, "Console") == 0)
	{
		m_menuConfig["Console"] = val;
	}
	else if (_stricmp(name, "TutorialMenu") == 0)
	{
		m_menuConfig["Tutorial Menu"] = val;
	}
	else if (_stricmp(name, "MessageBoxMenu") == 0)
	{
		m_menuConfig["MessageBoxMenu"] = val;
	}
	else if (_stricmp(name, "TweenMenu") == 0)
	{
		m_menuConfig["TweenMenu"] = val;
	}
	else if (_stricmp(name, "InventoryMenu") == 0)
	{
		m_menuConfig["InventoryMenu"] = val;
	}
	else if (_stricmp(name, "MagicMenu") == 0)
	{
		m_menuConfig["MagicMenu"] = val;
	}
	else if (_stricmp(name, "ContainerMenu") == 0)
	{
		m_menuConfig["ContainerMenu"] = val;
	}
	else if (_stricmp(name, "FavoritesMenu") == 0)
	{
		m_menuConfig["FavoritesMenu"] = val;
	}
	else if (_stricmp(name, "BarterMenu") == 0)
	{
		m_menuConfig["BarterMenu"] = val;
	}
	else if (_stricmp(name, "TrainingMenu") == 0)
	{
		m_menuConfig["Training Menu"] = val;
	}
	else if (_stricmp(name, "LockpickingMenu") == 0)
	{
		m_menuConfig["Lockpicking Menu"] = val;
	}
	else if (_stricmp(name, "BookMenu") == 0)
	{
		m_menuConfig["Book Menu"] = val;
	}
	else if (_stricmp(name, "GiftMenu") == 0)
	{
		m_menuConfig["GiftMenu"] = val;
	}
	else if (_stricmp(name, "JournalMenu") == 0)
	{
		m_menuConfig["Journal Menu"] = val;
	}
	else if (_stricmp(name, "CustomMenu") == 0)
	{
		m_menuConfig["CustomMenu"] = val;
	}
	else if (_stricmp(name, "WaitTime") == 0)
	{
		m_waitTime = val;
	}
	else if (_stricmp(name, "DisableTime") == 0)
	{
		m_disableTime = val;
	}
	else if (_stricmp(name, "DelayTime") == 0)
	{
		m_delayTime = val;
	}
	else
	{
		return false;
	}
	_MESSAGE("  %s = %d", name, val);

	return true;
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
				Set(name, -atoi(val + 1));
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
	BSResourceNiBinaryStream iniStream("SKSE\\Plugins\\SkyrimSouls.ini");
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
