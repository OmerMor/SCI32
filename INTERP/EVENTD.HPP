// eventd.hpp

#ifndef EVENTD_HPP
#define EVENTD_HPP

#ifndef EVENT_HPP
#include	"event.hpp"
#endif

#ifndef KBD_HPP
#include	"kbd.hpp"
#endif

/* struct JoystickEvent {
	ushort type;
	ushort message;
	ushort modifiers;
};*/

// typedef struct JoystickEvent JoystickEvent;

/*
extern "C" {
	Bool JoyInit();
	void PollJoystick(JoystickEvent*);
}
#pragma aux (PushArgs) JoyInit;
#pragma aux (PushArgs) PollJoystick;
*/


struct EventMgrDOS : EventMgr {
	EventMgrDOS();
	~EventMgrDOS();

	Bool		Get(SOL_Event* event, SOL_Event::Type mask);

protected:
	void		FlushKeyboard();
	Kbd::Mod	GetModifiers();
	void		Handle(SOL_Event*);
	Bool		joystickExists;
};

#endif
