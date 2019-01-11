//	platfrmw.cpp

#include "sol.hpp"

#include "sciwin.hpp"
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
	CDspeed,
	ColorDepth,
	CDdrive
};

enum ColorTypes {
	Error,
	Colors16,
	Colors256,
	High16Bit,
	True32Bit
};

enum WindowsOptions {
	TitleBar
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
		case ColorDepth:
			if (nColors == 16) {
				pm.acc = Colors16;
				break;
			} else if (nColors == 256) {
				pm.acc = Colors256;
				break;
			} else if (nColors == 65536) {
				pm.acc = High16Bit;
				break;
			} else if (nColors == 16777216) {
				pm.acc = True32Bit;
				break;
			} else {
				pm.acc = Error;
				break;
			}
		case CDdrive:
			pm.acc = *configMgr->Get("CDdrive",0);
			if (!pm.acc)
				pm.acc = *configMgr->Get("resCDisc",0);
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

void
KSetWindowsOption(argList)
{

	switch((WindowsOptions) arg(1)) {
		case TitleBar:
			if (arg(2))
				pm.acc = WritePrivateProfileString(szInfo,"TitleBar","true",szIniFile);
			else
				pm.acc = WritePrivateProfileString(szInfo,"TitleBar","false",szIniFile);
			break;

		default:
			pm.acc = False;
			break;
	}
}

void
KGetWindowsOption(argList)
{

	switch((WindowsOptions) arg(1)) {
		case TitleBar:
			pm.acc = GetTFOption("titlebar",TRUE);
			break;

		default:
			pm.acc = False;
			break;
	}
}
