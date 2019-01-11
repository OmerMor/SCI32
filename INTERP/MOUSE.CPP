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
#include "event.hpp"
#include "memmgr.hpp"

SOL_Mouse* mouse = NULL;
static SOL_Event event;	// kk - can't create it on the stack!

int			SOL_Mouse::hotOn = 0;
int			SOL_Mouse::inRect = -1;
int			SOL_Mouse::hotRectangleCount = 0;
SCI_Rect*	SOL_Mouse::hotRectangles = NULL;

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

void
KSetHotRectangles(argList)
{
	switch(arg(0)) {		// switch on count 

		case 1:	// Turn on or off
			if(arg(1))
				mouse->SetHotRectangleStatus(True);
			else
				mouse->SetHotRectangleStatus(False);

			break;

		case 2:	// setup rectangles

			mouse->SetHotRectangleStatus(True);
			mouse->SetupHotRectangles(arg(1),arg(2));
	}

}

void
SOL_Mouse::SetHotRectangleStatus(int status)
{
	hotOn = status;
}

void
SOL_Mouse::SetupHotRectangles(int count, MemID handle)
{
	hotRectangleCount = count;
	handle.Lock();
	hotRectangles = (SCI_Rect *) ((char *) (*handle) + 4);	// dereference once - locked memory
	inRect = -1;
}

#pragma off (check_stack)
void
SOL_Mouse::CheckHotRectangles(SOL_Point & pos)
{
	int lastRect = inRect;
	inRect = -1;

	for(int i = 0; i < hotRectangleCount;i++) {
		if(pos.x >= hotRectangles[i].ul.x &&
			pos.x <= hotRectangles[i].lr.x &&
			pos.y >= hotRectangles[i].ul.y &&
			pos.y <= hotRectangles[i].lr.y) {

			inRect = i;
			if(i != lastRect) {
				// it's in a rectangle, create an event
				event.type = SOL_Event::HotRectangle;
				event.message = i;
				eventMgr->Post(&event);
				break;	// no need to continue once hit
			}

			lastRect = inRect;

		}
	}

	// it WAS in a rect, but not now - need to send a message too
	if(lastRect != inRect && lastRect != -1) {	
		inRect = -1;
		event.type = SOL_Event::HotRectangle;
		event.message = -1;
		eventMgr->Post(&event);
	}
}
#pragma on (check_stack)
