#include "sol.hpp"
#include "kbdmgr.hpp"
#include "kbd.hpp"
#include "event.hpp"


KbdMgr*	kbdMgr;

void
KbdMgr::MapToDir(SOL_Event* event) const
{
	// How about moving this to MapKeyToDir?

	// mapping of keys to directions
	struct DirMap {
		Kbd::Key		key;
		Kbd::Key		shiftKey;
		Direction	dir;
	};
	
	static DirMap dirMap[] = {
		Kbd::Up,		Kbd::ShiftUp,		dirN,
		Kbd::PgUp,	Kbd::ShiftPgUp,	dirNE,
		Kbd::Right,	Kbd::ShiftRight,	dirE,
		Kbd::PgDn,	Kbd::ShiftPgDn,	dirSE,
		Kbd::Down,	Kbd::ShiftDown,	dirS,
		Kbd::End,	Kbd::ShiftEnd,		dirSW,
		Kbd::Left,	Kbd::ShiftLeft,	dirW,
		Kbd::Home,	Kbd::ShiftHome,	dirNW,
		Kbd::Center,Kbd::ShiftCenter,	dirStop,
		0
	};

	for (int i = 0; dirMap[i].key; i++) {
		Kbd::Key key = event->modifiers & Kbd::Shift ?
			dirMap[i].shiftKey : dirMap[i].key;
		if (key == event->message) {
			event->message = dirMap[i].dir;
			event->type = SOL_Event::Direction;
		}
	}
}
