//	event.cpp

#include "sol.hpp"

#include "debug.hpp"
#include "event.hpp"
#include "eventw.hpp"
#include "hotkey.hpp"
#include "integ.hpp"
#include "kbdmgrw.hpp"
#include "mouse.hpp"
#include "sciwin.hpp"

EventMgrWin::EventMgrWin()
{
	kbdMgr = New KbdMgrWin;
}

Bool
EventMgrWin::Get(SOL_Event* event, SOL_Event::Type mask)
{
	//	(poll joystick)
	
	// first, see if anything in Windows' event queue
	AsyncEventCheck();
	return EventMgr::Get(event, mask);
}

Kbd::Mod
EventMgrWin::GetModifiers()
{
	return ((GetKeyState(VK_MENU)    & 0x8000) ? 8 : 0) |
	       ((GetKeyState(VK_CONTROL) & 0x8000) ? 4 : 0) |
	       ((GetKeyState(VK_SHIFT)   & 0x8000) ? 3 : 0);
}

void
EventMgrWin::Handle(SOL_Event* event)
{
	EventMgr::Handle(event);

	if (event->type == SOL_Event::Null)
		return;

	if(mouse) {
		mouse->GlobalPosSci(&(event->where));
		mouse->GetSwiftInfo(
						&(event->zaxis),
						&(event->pitch),
						&(event->roll),
						&(event->yaw)
      );
	} else
		event->where = SOL_Point(0,0);

	Kbd::Mod mods = GetModifiers();
	event->modifiers |= mods;

#ifdef DEBUG
	if (event->type == SOL_Event::KeyDown && event->message == '`') {
		ToggleDebug();
		return;
	}

	if (event->type == SOL_Event::KeyDown && event->message == '~') {
		CheckIntegrityRequest();
		return;
	}

	if (event->type == SOL_Event::KeyDown && event->message == '@') {
		HotkeyRequest();
		return;
	}
#endif

	Post(event);
}

