// time.cpp

#include <time.h>

#include	"sol.hpp"

#include "kernel.hpp"
#include "msg.hpp"
#include "pmachine.hpp"
#include "savegame.hpp"
#include "time.hpp"

TimeMgr*	timeMgr;

uint
TimeMgr::SysTime(int func)
{
	struct tm*	newtime;
	time_t		long_time;
	int			fmtTime;

	time(&long_time);
	newtime = localtime(&long_time);

	switch (func) {

		case 1:
			//return packed time
			//HHHH|MMMM|MMSS|SSSS - (hour 1-12)
			fmtTime = newtime->tm_hour;
			if (fmtTime == 0)
				fmtTime = 12;
			if (fmtTime > 12)
				fmtTime -= 12;
			fmtTime = (fmtTime << 12) |
						 (newtime->tm_min << 6) |
						 (newtime->tm_sec);
			break;

		case 2:
			//return packed time
			//HHHH|HMMM|MMMS|SSSS - (hour 0-24)
			//* note loss of SECOND resolution in this form
			fmtTime = (newtime->tm_hour << 11) |
						 (newtime->tm_min << 5) |
						 (newtime->tm_sec >> 1);
			break;

		case 3:
			//return packed calendar date - (years since 1980)
			//YYYY|YYYM|MMMD|DDDD
			fmtTime = ((newtime->tm_year - 80) << 9) |
						 ((newtime->tm_mon + 1) << 5) |
						 (newtime->tm_mday);
			break;

		default:
			fmtTime = 0;
			break;
	}

	return fmtTime;
}

void
KGetTime(argList)
{
	// return low word of system ticks OR packed real time

	if (argCount == 1)
		pm.acc = (Acc) timeMgr->SysTime(arg(1));
	else
		pm.acc = (Acc) timeMgr->GetTickCount();
}

void
KWait(argList)
{
	//	wait the specified number of ticks before returning

	static ulong	lastTick;

	ulong ticks = arg(1);

	if (ticks)
		while (ticks + lastTick > timeMgr->GetTickCount())
			;

	pm.acc = (Acc) (timeMgr->GetTickCount() - lastTick);
	lastTick = timeMgr->GetTickCount();
}

void
TimeMgr::SaveRestore(SaveGame* saveGame, Bool save)
{
	ulong t;
	
	if (save)
		t = GetTickCount();
	saveGame->SaveRestore(&t, sizeof t, save);
	if (!save)
		SetTickCount(t);
}
