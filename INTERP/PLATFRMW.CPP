//	platfrmw.cpp

#include <windows.h>

#include "sol.hpp"

#include "config.hpp"
#include	"kernel.hpp"
#include	"pmachine.hpp"
#include	"textid.hpp"

enum PlatformType {
	Macintosh,
	Dos,
	Windows,
	Amiga
};

enum CallType {
	WhatAmI,
	CDspeed
};

void
KPlatform(argList)
{
	pm.acc = 0;

	if (argCount < 1) {
		pm.acc = (int) Windows;
		return;
   }

	switch ((CallType) arg(1)) {

		//	Return the platform type that we are
		case WhatAmI:
			pm.acc = (int) Windows;
			break;
		case CDspeed:
			pm.acc = configMgr->GetNum("CDspeed",0);
			break;
	}
}

void
KGetSierraProfileInt(argList)
{
	pm.acc = GetPrivateProfileInt(
		*(TextID) arg(1),		//	section
		*(TextID) arg(2),		//	entry
		arg(3),					// default
		"SIERRA.INI"
	);
}

void
KGetSierraProfileString(argList)
{
	pm.acc = GetPrivateProfileString(
		*(TextID) arg(1),		//	section
		*(TextID) arg(2),		//	entry
		*(TextID) arg(3),		// default
		*(TextID) arg(4),		//	dest
		arg(5),					//	dest size
		"SIERRA.INI"
	);
}
