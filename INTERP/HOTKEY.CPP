//	integ.cpp

#ifdef DEBUG

#include "sol.hpp"

#include "debug.hpp"
#include "hotkey.hpp"
#include "msg.hpp"
#include "window.hpp"


Bool	hotkeyRequest = False;

void
HotkeyRequest()
{
	debuggerWasOn = debuggerIsOn;
	debuggerIsOn = True;
	hotkeyRequest = True;
}

void
Hotkey()
{
//	msgMgr->Mono("press ENTER to continue; ESC for DEBUG\n");
//	if (WaitOnUser())
//		EnterDebug();
	if (!msgMgr->Alert("press ENTER to continue; ESC for DEBUG\n"))
		EnterDebug();
}

#endif
