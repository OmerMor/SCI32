//	event.cpp

#include <dos.h>
#include <string.h>

#include "sol.hpp"

#include "config.hpp"
#include "debug.hpp"
#include "event.hpp"
#include "eventd.hpp"
#include "hotkey.hpp"
#include "integ.hpp"
#include "kbdmgrd.hpp"
#include "kernel.hpp"
#include "intrpt.hpp"
#include "mouse.hpp"
#include "selector.hpp"
#include "stdio.h"
#include "mono.hpp"

static ServerFunc PollKeyboard;

EventMgrDOS::EventMgrDOS()
{
	/* char* joyStick = configMgr->Get("joyStick");
	if (joyStick &&
			(!*joyStick || !stricmp(joyStick,"on") || !stricmp(joyStick,"yes")))
		joystickExists = JoyInit();
	else
		joystickExists = False;
	*/

#ifdef DEBUG 
	kbdMgr = configMgr->Get(ConfigMgr::WVIDEO) ? New KbdMgrBIOS : New KbdMgrISR;
#else
	kbdMgr = New KbdMgrISR;
#endif
	InstallServer(PollKeyboard, 1);
}

EventMgrDOS::~EventMgrDOS()
{
	DisposeServer(PollKeyboard);
	delete kbdMgr;
}

void
EventMgrDOS::FlushKeyboard()
{
	SOL_Event event;
	while (kbdMgr->Get(&event))
		;
}

Bool
EventMgrDOS::Get(SOL_Event* event, SOL_Event::Type mask)
{
	AsyncEventCheck();

	//	(poll joystick)
	/* if(joystickExists) {
		JoystickEvent forJoy;
		PollJoystick(&forJoy);
		if(forJoy.type) {

			switch(forJoy.type) {
				case 64:
					event->type = SOL_Event::Direction;
					event->message = forJoy.message;
					event->modifiers = forJoy.modifiers;
					break;

				case 256:
					event->type = SOL_Event::JoyDown;
					event->message = forJoy.message;
					event->modifiers = forJoy.modifiers;
					break;

				case 512:
					event->type = SOL_Event::JoyUp;
					event->message = forJoy.message;
					event->modifiers = forJoy.modifiers;
					break;
			}

			Post(event);

		}
	}

	*/
	
	return EventMgr::Get(event, mask);
}

Kbd::Mod
EventMgrDOS::GetModifiers()
{
	return kbdMgr->Modifiers();
}

void
EventMgrDOS::Handle(SOL_Event* event)
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

	Kbd::Mod mods = kbdMgr->Modifiers();
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

static void __pragma("ServerFunc")
PollKeyboard()
{
	static SOL_Event event;
	
	event.modifiers = 0;
	kbdMgr->Get(&event);
	eventMgr->Handle(&event);
}

