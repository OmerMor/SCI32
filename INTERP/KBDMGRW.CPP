#include "sciwin.hpp"
#include "sol.hpp"
#include "kbdmgrw.hpp"
#include "kbd.hpp"
#include "event.hpp"


Bool
KbdMgrWin::HandleKey(unsigned iMessage, WORD wParam, LONG lParam)
// Return True if we will use this key, False to pass it to WINDOWS
{
Bool	charUsed;

	charUsed = True;

	switch (iMessage)
		{
		case WM_CHAR:					/* standard character entered */
			KeyEvent(wParam);			/* virtual key code */
			break;

		case WM_KEYDOWN:				/* for non-standard characters */
			switch (wParam)
				{
				case VK_CLEAR:
				case VK_PRIOR:
				case VK_NEXT:
				case VK_END:
				case VK_HOME:
				case VK_LEFT:
				case VK_UP:
				case VK_RIGHT:
				case VK_DOWN:
				case VK_DELETE:
				case VK_SUBTRACT:
				case VK_F1:
				case VK_F2:
				case VK_F3:
				case VK_F4:
				case VK_F5:
				case VK_F6:
				case VK_F7:
				case VK_F8:
				case VK_F9:
				case VK_F10:
					KeyEvent((HIWORD (lParam) & 0xff) << 8);	/* scan code */
					break;

				case VK_PAUSE:
					KeyEvent(0x7000);
					break;

				default:					/* ignore all others */
					charUsed = False;
					break;
				}
			break;

		case WM_SYSKEYDOWN:				/* ALT key combinations */
			if (wParam == VK_F10)
				KeyEvent((HIWORD (lParam) & 0xff) << 8);		/* scan code */
			else
				charUsed = False;
			break;

		case WM_SYSCHAR:					/* ALT character entered */
			KeyEvent((HIWORD (lParam) & 0xff) << 8);		/* scan code */
			break;

		default:
			charUsed = False;
			break;
		}
	return charUsed;
}



void
KbdMgrWin::KeyEvent (WORD charCode)
/************************************************************************
	Process a Windows keyboard event and post it to the SCI event queue.
*************************************************************************/
{
SOL_Event	event;

	event.type			= SOL_Event::KeyDown;
	event.message		= charCode;
	eventMgr->Handle(&event);
}
