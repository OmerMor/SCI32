//	platfrmw.cpp

#include "sol.hpp"

#include	"kernel.hpp"
#include	"pmachine.hpp"

enum PlatformType {
	Macintosh,
	Dos,
	Windows,
	Amiga
};

enum CallType {
	WhatAmI
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
	}
}
