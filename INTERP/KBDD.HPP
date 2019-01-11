#ifndef KBDD_HPP
#define KBDD_HPP

#ifndef KBD_HPP
#include "kbd.hpp"
#endif

struct KbdDOS : Kbd {
	Key	Get();
	Mod	Modifiers();
	Lock	LockStatus();
	void	Reset();
};

#endif
