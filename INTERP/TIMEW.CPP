// timew.cpp

#include <dos.h>
#define INCLUDE_MMSYSTEM_H
#include "sciwin.hpp"

#ifndef WIN32S
#include "scidll.hpp"
#else
#define	W32SUT_32
#include "scidllws.h"
#include "winclass.hpp"
#include "w32sut.h"
#endif

#include "mbox.hpp"
#include	"sol.hpp"
#include "memmgrw.hpp"
#include "timew.hpp"

static	Bool	timerEnabled = False;
#ifndef WIN32S
int far			*timeToCheck;
HANDLE			hMylib32;			// kludge
#else
int				*timeToCheck;
#endif

#ifndef WIN32S
FARPROC	fp;
#else
typedef BOOL (WINAPI * PUTREGISTER) ( HANDLE     hModule,
                                      LPCSTR     lpsz16BitDLL,
                                      LPCSTR     lpszInitName,
                                      LPCSTR     lpszProcName,
                                      UT32PROC * ppfn32Thunk,
                                      FARPROC    pfnUT32Callback,
                                      LPVOID     lpBuff
                                    );

typedef VOID (WINAPI * PUTUNREGISTER) (HANDLE hModule);

PUTREGISTER		pUTRegister = NULL;
PUTUNREGISTER	pUTUnRegister = NULL;
UT32PROC			pfnUTProc = NULL;
HMODULE			hKernel32 = 0;
static	DWORD			myArgs[2];
#endif


TimeMgrWin::TimeMgrWin()
{
#ifndef WIN32S
HINDIR	hInitTimeMgr;

	_disable();
  hlib32 = LoadLibrary( "scidll.dll" );
  hMylib32 = hlib32;		// kludge
  fp = GetProcAddress( hlib32, "Win386LibEntry" );

  hInitTimeMgr = GetIndirectFunctionHandle( fp, INDIR_WORD, INDIR_CDECL, INDIR_ENDLIST );
  hTermTimeMgr = GetIndirectFunctionHandle( fp, INDIR_WORD, INDIR_CDECL, INDIR_ENDLIST );
  hTickCount =   GetIndirectFunctionHandle( fp, INDIR_WORD, INDIR_CDECL, INDIR_ENDLIST );
  hTickSet =     GetIndirectFunctionHandle( fp, INDIR_DWORD, INDIR_WORD, INDIR_ENDLIST );
  hSetCheckTime = GetIndirectFunctionHandle( fp, INDIR_DWORD, INDIR_DWORD, INDIR_WORD, INDIR_WORD, INDIR_ENDLIST );

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
	_enable();
#else
BOOL	bResult;

	_disable();
	hKernel32 = LoadLibrary("Kernel32.dll");
	pUTRegister = (PUTREGISTER)GetProcAddress(hKernel32, "UTRegister");
	pUTUnRegister = (PUTUNREGISTER) GetProcAddress( hKernel32, "UTUnRegister" );
	bResult = (*pUTRegister)(Main::hInstance,
								"scidllws.dll",
								NULL,
								"DLLProc",
								&pfnUTProc,
								NULL, NULL);
	if (!bResult)
		MBox("DLL not initialized","");

	timeToCheck = SetCheckTime(asyncCheckFreq);
	timeBeginPeriod (16);
	EnableTimer();
	_enable();
#endif
}

TimeMgrWin::~TimeMgrWin()
{
	_disable();
	DisableTimer();
	timeEndPeriod (15);
#ifndef WIN32S
	FreeWinMem((void far *)timeToCheck,PAGELOCK);
//	FreeLibrary( hlib32 );
#else
	EndCheckTime(timeToCheck);
//	(*pUTUnRegister) (Main::hInstance);
//	FreeLibrary(hKernel32);
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

void UnloadDll(void)
{
// All this Thunk stuff should be put in a separate module
// No time right now. TM 11/8/94
#ifndef WIN32S
	FreeLibrary( hMylib32 );
#else
	(*pUTUnRegister) (Main::hInstance);
	FreeLibrary(hKernel32);
#endif
}
