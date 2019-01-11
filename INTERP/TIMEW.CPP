// timew.cpp

#define INCLUDE_MMSYSTEM_H
#include "sciwin.hpp"

#include	"sol.hpp"
#include "memmgrw.hpp"
#include "timew.hpp"
#include "winthunk.hpp"
#include "mbox.hpp"

static	Bool	timerEnabled = False;
#ifndef WIN32S
int far			*timeToCheck;
#else
int				*timeToCheck;
#endif

#ifdef WIN32S
static	DWORD			myArgs[2];
#endif


TimeMgrWin::TimeMgrWin()
{
	_disable();

#ifndef WIN32S

	lpTimerCallback = (FARPROC)InvokeIndirectFunction(hInitTimeMgr, DLL_INIT);

	timeToCheck = (int far *)AllocWinMem(8,PAGELOCK);
	SetCheckTime(asyncCheckFreq, timeToCheck);

/* set multimedia timer to 16ms (approx. 1/60 sec).  This is a bit too
   fast, so an adjustment is made in the callback.  The second parameter in
   timeSetEvent is resolution and is set to 1.  Higher values cause the
   clock to be inaccurate in enhanced mode.
*/
	timeBeginPeriod (16);
	EnableTimer();

#else

	timeToCheck = SetCheckTime(asyncCheckFreq);
	timeBeginPeriod (16);
	EnableTimer();

#endif
	_enable();
}

TimeMgrWin::~TimeMgrWin()
{
	_disable();
	DisableTimer();
	timeEndPeriod (15);
#ifndef WIN32S
	FreeWinMem((void far *)timeToCheck,PAGELOCK);
#else
	EndCheckTime(timeToCheck);
#endif
	_enable();
}

ulong
TimeMgrWin::GetTickCount()
{
ulong ticks;

	_disable();
#ifndef WIN32S
	ticks = InvokeIndirectFunction(hTickCount, DLL_TICK);
#else
	ticks = (* pfnUTProc)(NULL, (DWORD)DLL_GET_TICK_COUNT, NULL);
#endif
	_enable();
	return ticks;
}

void
TimeMgrWin::SetTickCount(ulong ticks)
{
	_disable();
#ifndef WIN32S
	InvokeIndirectFunction(hTickSet, ticks, DLL_TICK_SET);
#else
	myArgs[0] = (DWORD)ticks;
	(* pfnUTProc)(myArgs, (DWORD)DLL_SET_TICK_COUNT, NULL);
#endif
	_enable();
}

ulong
TimeMgrWin::GetMilliSec()
{
	return timeGetTime();
}

void
TimeMgrWin::EnableTimer()
{
	if (!timerEnabled) {
#ifndef WIN32S
		timerId = timeSetEvent (16, 1, (LPTIMECALLBACK)lpTimerCallback,
										(DWORD)0, TIME_PERIODIC);
#else
		(* pfnUTProc)(NULL, (DWORD)DLL_START_TIMER, NULL);
#endif
		timerEnabled = True;
	}
}

void
TimeMgrWin::DisableTimer()
{
	if (timerEnabled) {
#ifndef WIN32S
		timeKillEvent (timerId);
#else
		(* pfnUTProc)(NULL, (DWORD)DLL_STOP_TIMER, NULL);
#endif
		timerEnabled = False;
	}
}

#ifndef WIN32S
#pragma warning 579  9         //   Shut off cast warning for FP_OFF
void
TimeMgrWin::SetCheckTime(int freq, int far *timeToCheck)
{
	_disable();
	InvokeIndirectFunction(hSetCheckTime,
									(DWORD)freq,
									(DWORD)FP_OFF(timeToCheck),
									(WORD)FP_SEG(timeToCheck),
									DLL_SET_CHECK_TIME);
	_enable();
}
#pragma warning 579  4         //   Turn on cast warning

#else	//WIN32S

int *
TimeMgrWin::SetCheckTime(int freq)
{
int	*addr;

	_disable();
	myArgs[0] = (DWORD)freq;
	addr = (int *)(* pfnUTProc)(myArgs, (DWORD)DLL_SET_CHECK_TIME, NULL);
	_enable();
	return addr;
}

void
TimeMgrWin::EndCheckTime(int *addr)
{
	_disable();
	myArgs[0] = (DWORD)addr;
	(* pfnUTProc)(myArgs, (DWORD)DLL_END_CHECK_TIME, NULL);
	_enable();
}
#endif
