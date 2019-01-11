//	platfrmd.cpp

#include "sol.hpp"

#include "config.hpp"
#include	"kernel.hpp"
#include	"pmachine.hpp"

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

void
KPlatform(argList)
{
	pm.acc = 0;

	if (argCount < 1) {
		pm.acc = (int) Dos;
		return;
   }

	switch ((CallType) arg(1)) {

		//	Return the platform type that we are
		case WhatAmI:
			pm.acc = (int) Dos;
			break;
		case CDspeed:
			pm.acc = configMgr->GetNum("CDspeed",0);
			break;
		case ColorDepth:
			pm.acc = Colors256;
			break;
		case CDdrive:
			pm.acc = *configMgr->Get("CDdrive",0);
			if (!pm.acc)
				pm.acc = *configMgr->Get("resCDisc",0);
			break;
	}
}
