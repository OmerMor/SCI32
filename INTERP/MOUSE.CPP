#include <dos.h>
#include <string.h>

#include "sol.hpp"

#include "cursor.hpp"
#include "graphmgr.hpp"
#include "kbdmgr.hpp"
#include "kernel.hpp"
#include "mouse.hpp"
#include "pmachine.hpp"
#include "savegame.hpp"

SOL_Mouse* mouse = NULL;


void
KHaveMouse(kArgs)
{
	if (mouse)
		pm.acc = mouse->Exists();		//  Really means exists, irregardless if
	else									//  user wants it or not
		pm.acc = False;
}


void
KSetCursor(argList)
{
	switch (argCount) {
		case 1:
         // show or hide cursor 
         // disable special effects cursor
         // restrict cursor to port
			switch (arg(1)) {
				case -1:
					break;

				case -2:
					graphMgr->GCursor().ClearRestrictRect();
					break;

				default:
			  		if (arg(1))
						graphMgr->GCursor().Show();
				   else
						graphMgr->GCursor().Hide();
					break;
			}
			break;

		case 2:
         // move cursor
         graphMgr->GCursor().ForcePos(SOL_Point(arg(1), arg(2)));
			break;

		case 3:
         // set cursor
			graphMgr->GCursor().Set(arg(1), arg(2), arg(3));
			break;

		case 4:
			//  restrict mouse to a given rectangle
			//  Top left bottom right
			graphMgr->GCursor().SetRestrictRect(SOL_Rect(arg(1), arg(2), arg(3), arg(4)));
			break;

		case 10:
			// special magnifier cursor
			break;
	}
}


void
KVibrateMouse(argList)
{
	if(argCount < 3)
		mouse->Vibrate(arg(1));
	else
		mouse->Vibrate(arg(1), arg(2), arg(3));
}

