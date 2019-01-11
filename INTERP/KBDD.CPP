//	kbd.cpp

#include <bios.h>
#include <conio.h>
#include <dos.h>
#include <string.h>

#include "sol.hpp"
#include "kbdd.hpp"


///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

Kbd::Key
KbdDOS::Get()
{
	Key key;

	_disable();

	if (_bios_keybrd(_KEYBRD_READY))
		key = _bios_keybrd(_KEYBRD_READ);
	else
		key = 0;

	_enable();

	return key;
}

Kbd::Lock
KbdDOS::LockStatus()
{
	//	this is not the cleanest way to implement this, but I wanted to leave
	//	Modifiers() alone to avoid changing other code --MW

	return Kbd::Lock(_bios_keybrd(_KEYBRD_SHIFTSTATUS) & 0xF0);
}

Kbd::Mod
KbdDOS::Modifiers()
{
	return Kbd::Mod(_bios_keybrd(_KEYBRD_SHIFTSTATUS) & 0x0F);
}

void
KbdDOS::Reset()
{
	const IntStatReg	= 0x20;
	const EOI			= 0x20;

	// reset the keyboard
	int inByte = inp(CtrlPort);		//get current control value
	inByte |= 0x80;
	outp(CtrlPort, inByte);			//do reset
	inByte &= 0x7F;
	outp(CtrlPort, inByte);			//restore port to original condition

	// signal the end of the interrupt to the interrupt controller
	outp(IntStatReg, EOI);
}

