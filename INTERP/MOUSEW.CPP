#ifndef COLORCUR

#include "sciwin.hpp"
#include "sol.hpp"

#include "cursorw.hpp"
#include "event.hpp"
#include "graphmgr.hpp"
#include "kbdmgr.hpp"
#include "kernel.hpp"
#include "mousew.hpp"
#include "pmachine.hpp"
#include "savegame.hpp"
//#include "mbox.hpp"

SOL_Cursor*	MouseWin::cursor = NULL;
Bool			MouseWin::restrictFlag = False;
Bool			MouseWin::exists = False;
SOL_Rect		MouseWin::restrict;
SOL_Point	MouseWin::pos;
int			MouseWin::buttonState = 0;
SOL_Event	MouseWin::event;		//  Watcom 9.5 insisted that the interrupt
											//  routine couldn't create one locally on
											//  the stack.  DF
Bool			MouseWin::inCage = False;


MouseWin::MouseWin()
{
//POINT			winPoint;
SOL_Point	solPoint;

	cursor = NULL;

	//  set existance flag
	exists = GetSystemMetrics (SM_MOUSEPRESENT);

	//  Set initial mouse position to 0, 0
	solPoint.x = 0;
	solPoint.y = 0;
	SetPos(solPoint);
}


MouseWin::~MouseWin()
{
}

int
MouseWin::GlobalPos(SOL_Point* pt) const
{
	// Return interrupt level position in global coords of mouse in the point
	*pt = pos;
	return buttonState;
}


int
MouseWin::GlobalPosSci(SOL_Point* pt) const
{
	// Return interrupt level position in global coords of mouse in the point  
	if (graphMgr) {
//		pt->x = pos.x / Ratio(graphMgr->Xdim(), LOWRESX);
//		pt->y = pos.y / Ratio(graphMgr->Ydim(), LOWRESY);
		// already scaled under Windows
		*pt = pos;
	}
	else
		*pt = SOL_Point(0,0);
	return buttonState;
}


Bool
MouseWin::Exists() const
{
	return exists;
}

void
MouseWin::SetPos(const SOL_Point& p)
{
SOL_Point	tmp;
Bool			prevInCage;

	tmp = p;

	// scale to sci dimensions
	tmp.x = (tmp.x < gameBorder)? 0: tmp.x - gameBorder; 
	tmp.y = (tmp.y < gameBorder)? 0: tmp.y - gameBorder; 
	tmp.x = (tmp.x * LOWRESX) / gameWidth;
	tmp.y = (tmp.y * LOWRESY) / gameHeight;
	tmp.x = (tmp.x > LOWRESX-1)? LOWRESX-1: tmp.x;
	tmp.y = (tmp.y > LOWRESY-1)? LOWRESY-1: tmp.y;

	// change cursor if we have moved into or out of the restrict rect
	prevInCage = inCage;
	if (restrictFlag) {
		if (restrict.Intersect(tmp)) {
			if (!inCage) {
				inCage = True;
				hCurCur = hMyCur;
			}
		} else {
			if (inCage) {
				inCage = False;
				hCurCur = hAltCur;
			}
			tmp = pos;		//out of cage - don't change x, y
		}
	}
	pos = tmp;

	// Must do a SetCursor on every move even if it hasn't changed so that
	// Windows will change it when entering client area.
	SetCursor(hCurCur);

	// If we moved out of the cage, show the cursor
	// If we move into the cage, hide the cursor if hideCount is > 0
	if (inCage != prevInCage) {
		if (!inCage) {
//			MBox("went out","");
			ShowCursor (1);
		} else {
//			MBox("went in","");
			if (graphMgr->GCursor().GetHideCount())
				ShowCursor (0);
//			MBox("went in2","");
		}
	}
}


void
MouseWin::SetRestrictRect(const SOL_Rect& r)
{
	restrict = r;
	restrictFlag = True;
	inCage = True;
}

void
MouseWin::ClearRestrictRect()
{
	restrictFlag = False;
	inCage = False;
	hCurCur = hMyCur;
	SetCursor(hCurCur);
}


void
MouseWin::WinMouseButton(unsigned iMessage, const SOL_Point& p)
{
SOL_Point	tmp;

	tmp = p;

	// scale to sci dimensions
	tmp.x = (tmp.x < gameBorder)? 0: tmp.x - gameBorder; 
	tmp.y = (tmp.y < gameBorder)? 0: tmp.y - gameBorder; 
	tmp.x = (tmp.x * LOWRESX) / gameWidth;
	tmp.y = (tmp.y * LOWRESY) / gameHeight;
	tmp.x = (tmp.x > LOWRESX-1)? LOWRESX-1: tmp.x;
	tmp.y = (tmp.y > LOWRESY-1)? LOWRESY-1: tmp.y;

	// ignore if not within restrict rect
	if (restrictFlag & !restrict.Intersect(tmp))
		return;

	SetPos(p);

	event.where = pos;

	if ((iMessage == WM_LBUTTONDOWN) ||
		 (iMessage == WM_MBUTTONDOWN) ||
		 (iMessage == WM_RBUTTONDOWN)) {
		event.type = SOL_Event::MouseDown;
		buttonState = 1;
	} else {
		event.type = SOL_Event::MouseUp;
		buttonState = 0;
	}

	event.modifiers = kbdMgr->Modifiers();

	// Center button events are treated as ctrl-left button events.
	// Set the modifiers accordingly.
	if ((iMessage == WM_MBUTTONDOWN) ||
		 (iMessage == WM_MBUTTONUP))
		event.modifiers |= Kbd::Ctrl;

	// Right button events are treated as shift-left button events.
	// Set the modifiers accordingly.
	if ((iMessage == WM_RBUTTONDOWN) ||
		 (iMessage == WM_RBUTTONUP))
		event.modifiers |= Kbd::Shift;

		// Post the event.
	eventMgr->Post(&event);
}

#else		//COLORCUR
#include <dos.h>
#include <string.h>

#include "sciwin.hpp"
#include "sol.hpp"

#include "cursor.hpp" 
#include "event.hpp"
//#include "graphmw.hpp" 
#include "graphmgr.hpp" 
//#include "intflag.hpp"
#include "kbdmgr.hpp"
#include "kernel.hpp"
#include "mousew.hpp"
#include "pmachine.hpp"
#include "ratio.hpp"
#include "savegame.hpp"
#include "mbox.hpp"
//#include "vga.hpp"

const mouseInt	= 0x33;
const short mouseInstalled = 0;
const short mouseShow = 1;
const short mouseHide = 2;
const short mouseGet = 3;
const short mouseSet =	4;
const short mouseHLimit = 7;
const short mouseVLimit = 8;
const short mouseCursor = 9;
const short mouseSetInt = 12;
const short swiftVibrate = 0x5330;
const short swiftInstalled = 0x53C1;
const short mouseMoved	= 0x01;

SOL_Cursor*	MouseWin::cursor = NULL;
SOL_Rect		MouseWin::restrict;
Bool			MouseWin::useMouse = True;
Bool			MouseWin::isSwift = False;
Bool			MouseWin::exists = False;
uchar*		MouseWin::stackBuff = NULL;
SOL_Point	MouseWin::pos;
short			MouseWin::zaxis = 0;
short			MouseWin::pitch = 0;
short			MouseWin::roll = 0;
short			MouseWin::yaw = 0;
int			MouseWin::buttonState = 0;
SOL_Event	MouseWin::event;		//  Watcom 9.5 insisted that the interrupt
											//  routine couldn't create one locally on
											//  the stack.  DF

#pragma warning 579  9				//   Shut off cast warning for mouse handler


MouseWin::MouseWin()
{
	cursor = NULL;
	useMouse = True;

//	InitMouse();
	//  set existance flag
	exists = GetSystemMetrics (SM_MOUSEPRESENT);

	if (Exists())  {
		stackBuff = New uchar[15000];
//		InstallMouseInterrupt();
		ClearRestrictRect();
	}
	else
		msgMgr->Fatal(Msg_NoMouseDriver);
}


MouseWin::~MouseWin()
{
	if (useMouse)  { // Exists())  {
//		ReleaseMouseInterrupt();
		delete stackBuff;
	}
}


int
MouseWin::GlobalPos(SOL_Point* pt) const
{
	pt->x = pos.x;
	pt->y = pos.y;
	return buttonState;
}


int
MouseWin::GlobalPosSci(SOL_Point* pt) const
{
	pt->x = pos.x * Ratio(LOWRESX, graphMgr->Xdim());
	pt->y = pos.y * Ratio(LOWRESY, graphMgr->Ydim());
	return buttonState;
}


int
MouseWin::GetSwiftInfo(int* z, int* p, int* r, int* y) const
{
	// Return interrupt level position in global coords of mouse in the point
	*z = zaxis;
	*p = pitch;
	*r = roll;
	*y = yaw;
	return buttonState;
}


Bool
MouseWin::Exists() const
{
	return exists;
}


void
MouseWin::SetRestrictRect(const SOL_Rect& r1)
{
	Ratio ratiox(graphMgr->Xdim(), LOWRESX);
	Ratio ratioy(graphMgr->Ydim(), LOWRESY);

	restrict.A.x = r1.A.x * ratiox;
	restrict.A.y = r1.A.y * ratioy;
	restrict.B.x = r1.B.x * ratiox;
	restrict.B.y = r1.B.y * ratioy;

	SOL_Point p = MouseWin::pos;
	if (p.x < restrict.A.x)
		p.x = restrict.A.x;
	if (p.x > restrict.B.x)
		p.x = restrict.B.x;

	if (p.y < restrict.A.y)
		p.y = restrict.A.y;
	if (p.y > restrict.B.y)
		p.y = restrict.B.y;

//	SetMouseDriverPos(p);
}

void
MouseWin::ClearRestrictRect()
{
	restrict = SOL_Rect(0, 0, graphMgr->Xdim() - 1, graphMgr->Ydim() - 1);
}


void
MouseWin::Setup(SOL_Cursor& cur)
{
	cursor = &cur;
}


void
MouseWin::SetPos(const SOL_Point& p)
{

	pos = p;
//	pos.x = pos.x * Ratio(graphMgr->Xdim(), LOWRESX);
//	pos.y = pos.y * Ratio(graphMgr->Ydim(), LOWRESY);

//sprintf(mbuff,"x:%d y:%d", pos.x, pos.y);
//SciDisplay(mbuff);

	if (useMouse)  {
//		SetMouseDriverPos(pos);
		if (cursor)
			cursor->DeviceMovedTo(pos);
	}
} 



Bool
MouseWin::IsSwift() const
{
	return isSwift;
} 

void
MouseWin::WinMouseButton(unsigned iMessage, const SOL_Point& p)
{

	SetPos(p);

//	event.where = pos;
//	mouse->GlobalPosSci(&(MouseDOS::event.where));
	mouse->GlobalPosSci(&event.where);

	if ((iMessage == WM_LBUTTONDOWN) ||
		 (iMessage == WM_MBUTTONDOWN) ||
		 (iMessage == WM_RBUTTONDOWN)) {
		event.type = SOL_Event::MouseDown;
		buttonState = 1;
	} else {
		event.type = SOL_Event::MouseUp;
		buttonState = 0;
	}

	event.modifiers = kbdMgr->Modifiers();

	// Center button events are treated as ctrl-left button events.
	// Set the modifiers accordingly.
	if ((iMessage == WM_MBUTTONDOWN) ||
		 (iMessage == WM_MBUTTONUP))
		event.modifiers |= Kbd::Ctrl;

	// Right button events are treated as shift-left button events.
	// Set the modifiers accordingly.
	if ((iMessage == WM_RBUTTONDOWN) ||
		 (iMessage == WM_RBUTTONUP))
		event.modifiers |= Kbd::Shift;

		// Post the event.
	eventMgr->Post(&event);
}

#endif	//COLORCUR

