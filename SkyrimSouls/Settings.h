#pragma once

#include <map>
#include <string>

//#define DEBUG_LOG 1

struct Settings
{
	Settings()
	{
		m_menuConfig["Console"] = 1;
		m_menuConfig["Tutorial Menu"] = 1;
		m_menuConfig["MessageBoxMenu"] = 1;
		m_menuConfig["TweenMenu"] = 1;
		m_menuConfig["InventoryMenu"] = 1;
		m_menuConfig["MagicMenu"] = 1;
		m_menuConfig["ContainerMenu"] = 1;
		m_menuConfig["FavoritesMenu"] = 1;
#ifdef DEBUG_LOG
		m_menuConfig["Crafting Menu"] = 1;
#endif
		m_menuConfig["BarterMenu"] = 1;
		m_menuConfig["Training Menu"] = 1;
		m_menuConfig["Lockpicking Menu"] = 1;
		m_menuConfig["Book Menu"] = 1;
#ifdef DEBUG_LOG
		m_menuConfig["MapMenu"] = 1;
		m_menuConfig["StatsMenu"] = 1;
		m_menuConfig["Sleep/Wait Menu"] = 0;
#endif
		m_menuConfig["GiftMenu"] = 1;
		m_menuConfig["Journal Menu"] = 1;
		//Sleep/Wait Menu
		m_menuConfig["CustomMenu"] = 1;
		m_waitTime = 30;
		m_disableTime = 30;
		m_delayTime = 50;
	}

	std::map<std::string, int>		m_menuConfig;
	int								m_waitTime;
	int								m_delayTime;
	int								m_disableTime;

	bool Set(const char *name, int val);
	void Eval(const char *line);

	void Load();
};


extern Settings settings;
