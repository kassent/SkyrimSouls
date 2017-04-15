#pragma once

struct Settings
{
	static bool	bDisableInCombat;
	static bool	bDisableTheft;
	static bool	bDisablePickpocketing;
	static bool	bDisableIfEmpty;
	static bool	bCloseIfEmpty;
	static bool bUseConsole;
	static bool bForceAnsi;
	static bool bDisableLootSingle;
	static int	iOpacity;
	static int	iScale;
	static int	iPositionX;
	static int	iPositionY;
	static int	iItemLimit;

	static bool SetBool(const char *name, bool val);
	static bool SetInt(const char *name, int val);
	static bool Set(const char *name, int val);
	static void Eval(const char *line);

	static void Load();
};
