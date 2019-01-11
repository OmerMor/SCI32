#ifndef KBDMGR_HPP
#define KBDMGR_HPP

class SOL_Event;

#ifndef KBD_HPP
#include "kbd.hpp"
#endif

class KbdMgr
{
	//	a manager that gets keys for SCI
 public:
	virtual ~KbdMgr() {};

	virtual	void	MapToDir(SOL_Event*) const;
	virtual	Bool	Get(SOL_Event*) = 0;
	virtual	Kbd::Mod	Modifiers() = 0;
};
extern KbdMgr * kbdMgr;


#endif
