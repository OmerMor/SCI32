// eventw.hpp

#ifndef EVENTW_HPP
#define EVENTW_HPP

#ifndef EVENT_HPP
#include	"event.hpp"
#endif

#ifndef KBD_HPP
#include	"kbd.hpp"
#endif

struct EventMgrWin : EventMgr {
	EventMgrWin();
	~EventMgrWin() {};

	Bool		Get(SOL_Event* event, SOL_Event::Type mask);

protected:
	void		FlushKeyboard() {};
	Kbd::Mod	GetModifiers();
	void		Handle(SOL_Event*);
};

#endif
