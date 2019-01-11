#ifndef KBDW_HPP
#define KBDW_HPP

#ifndef KBD_HPP
#include "kbd.hpp"
#endif

struct KbdWin : Kbd {
	Key	Get();
	Mod	Modifiers();
	Lock	LockStatus() { return 0; }
	void	Reset();
};

#endif
