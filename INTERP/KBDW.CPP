//	kbdw.cpp

#include "sciwin.hpp"
#include "sol.hpp"
#include "kbdw.hpp"


Kbd::Key
KbdWin::Get()
{
	return 0;
}

Kbd::Mod
KbdWin::Modifiers()
{
//	return Kbd::Mod(_bios_keybrd(_KEYBRD_SHIFTSTATUS) & 0x0F);
//	return 0;
	return ((GetKeyState(VK_MENU)    & 0x8000) ? 8 : 0) |
	      ((GetKeyState(VK_CONTROL) & 0x8000) ? 4 : 0) |
	      ((GetKeyState(VK_SHIFT)   & 0x8000) ? 3 : 0);
}

void
KbdWin::Reset()
{
}
