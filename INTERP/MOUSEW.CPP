#include <dos.h>
#include <string.h>

#include "sciwin.hpp"
#include "sol.hpp"

#include "cursor.hpp" 
#include "cursorw.hpp"
#include "event.hpp"
#include "graphmgr.hpp"
#include "kbdmgr.hpp"
#include "kernel.hpp"
#include "mousew.hpp"
#include "pmachine.hpp"
#include "ratio.hpp"
#include "savegame.hpp"
#include "mbox.hpp"

SOL_Mouse* winMouse = NULL;
SOL_Mouse* clrMouse = NULL;

SOL_Cursor*	MouseWin::cursor = NULL;
Bool			MouseWin::restrictFlag = False;
Bool			MouseWin::inCage = False;

// Made these global so they are common to both mouse types.
// There's probably a better way to do this such as an intermediate
// common class.  TM 5/8/95
static	Bool			exists = False;
static	SOL_Rect		restrict;
static	SOL_Point	pos;
static	int			buttonState = 0;
static	SOL_Event	event;		//  Watcom 9.5 insisted that the interrupt
											//  routine couldn't create one locally on
											//  the stack.  DF
static	Bool			outOfGame = False;

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
Bool	prevInCage;
Bool	outSide;

	// See if we are outside the game window
	if ((p.x < gameBorder) 	||
		 (p.y < gameBorder)	||
		 (p.x > (gameWidth  + gameBorder)) ||
		 (p.y > (gameHeight + gameBorder)))
			outSide = True;
	else
			outSide = False;

	// if we just left the game window, then do a ReleaseCapture so we don't
	// get mouse move messages.  If the Windows cursor was hidden, unhide it.
	if (outSide && !outOfGame) {
		outOfGame = True;
		if (graphMgr->GCursor().GetHideCount())
			ShowCursor (1);
		if (!buttonState)
			ReleaseCapture();
	}

	// if we just entered the game window, then capture the the mouse so
	// that the user can't sneak out without us knowing about it.  If we
	// are hiding the cursor, than hide it.
	if (!outSide && outOfGame) {
		outOfGame = False;
		if (graphMgr->GCursor().GetHideCount())
			ShowCursor (0);
		SetCapture(hMyWnd);
	}

	// scale to sci dimensions
	tmp = p;
	tmp.x = (tmp.x < gameBorder)? 0: tmp.x - gameBorder; 
	tmp.y = (tmp.y < gameBorder)? 0: tmp.y - gameBorder; 

	tmp.x = (tmp.x * SCIRESX) / gameWidth;
	tmp.y = (tmp.y * SCIRESY) / gameHeight;
	tmp.x = (tmp.x > SCIRESX-1)? SCIRESX-1: tmp.x;
	tmp.y = (tmp.y > SCIRESY-1)? SCIRESY-1: tmp.y;

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

	if(hotOn)
		CheckHotRectangles(pos);

	// Must do a SetCursor on every move even if it hasn't changed so that
	// Windows will change it when entering client area.
	SetCursor(hCurCur);

	// If we moved out of the cage, show the cursor
	// If we move into the cage, hide the cursor if hideCount is > 0
	if (inCage != prevInCage) {
		if (!inCage) {
			ShowCursor (1);
		} else {
			if (graphMgr->GCursor().GetHideCount())
				ShowCursor (0);
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
//	tmp.x = (tmp.x < gameBorder)? 0: tmp.x - gameBorder; 
//	tmp.y = (tmp.y < gameBorder)? 0: tmp.y - gameBorder; 
	tmp.x -= gameBorder;
	tmp.y -= gameBorder;
	tmp.x = (tmp.x * SCIRESX) / gameWidth;
	tmp.y = (tmp.y * SCIRESY) / gameHeight;
//	tmp.x = (tmp.x > SCIRESX-1)? SCIRESX-1: tmp.x;
//	tmp.y = (tmp.y > SCIRESY-1)? SCIRESY-1: tmp.y;

	// ignore if not within restrict rect
	if (restrictFlag & !restrict.Intersect(tmp))
		return;

	SetPos(p);

//	event.where = pos;
	event.where = tmp;

	if ((iMessage == WM_LBUTTONDOWN) ||
		 (iMessage == WM_MBUTTONDOWN) ||
		 (iMessage == WM_RBUTTONDOWN)) {
		event.type = SOL_Event::MouseDown;
		buttonState = 1;
	} else {
		if (outOfGame)
			ReleaseCapture();
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

SOL_Cursor*	MouseWinClr::cursor = NULL;
//SOL_Rect		MouseWinClr::restrict;
Bool			MouseWinClr::useMouse = True;
Bool			MouseWinClr::isSwift = False;
//Bool			MouseWinClr::exists = False;
uchar*		MouseWinClr::stackBuff = NULL;
//SOL_Point	MouseWinClr::pos;
short			MouseWinClr::zaxis = 0;
short			MouseWinClr::pitch = 0;
short			MouseWinClr::roll = 0;
short			MouseWinClr::yaw = 0;
Bool			MouseWinClr::paintFlag = False;
//int			MouseWinClr::buttonState = 0;
//SOL_Event	MouseWinClr::event;		//  Watcom 9.5 insisted that the interrupt
											//  routine couldn't create one locally on
											//  the stack.  DF

#pragma warning 579  9				//   Shut off cast warning for mouse handler


MouseWinClr::MouseWinClr()
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
		msgMgr->Fatal("\nIn MS-DOS this game requires a mouse driver.\n"
 						  "This program could not find a driver that was loaded.\n"
						  "Please load a mouse driver before launching the game.");

}


MouseWinClr::~MouseWinClr()
{
	if (useMouse)  { // Exists())  {
//		ReleaseMouseInterrupt();
		delete stackBuff;
	}
}


int
MouseWinClr::GlobalPos(SOL_Point* pt) const
{
	pt->x = pos.x;
	pt->y = pos.y;
	return buttonState;
}


int
MouseWinClr::GlobalPosSci(SOL_Point* pt) const
{
	pt->x = pos.x * Ratio(SCIRESX, graphMgr->Xdim());
	pt->y = pos.y * Ratio(SCIRESY, graphMgr->Ydim());
	return buttonState;
}


int
MouseWinClr::GetSwiftInfo(int* z, int* p, int* r, int* y) const
{
	// Return interrupt level position in global coords of mouse in the point
	*z = zaxis;
	*p = pitch;
	*r = roll;
	*y = yaw;
	return buttonState;
}


Bool
MouseWinClr::Exists() const
{
	return exists;
}


void
MouseWinClr::SetRestrictRect(const SOL_Rect& r1)
{
	Ratio ratiox(graphMgr->Xdim(), SCIRESX);
	Ratio ratioy(graphMgr->Ydim(), SCIRESY);

	restrict.A.x = r1.A.x * ratiox;
	restrict.A.y = r1.A.y * ratioy;
	restrict.B.x = r1.B.x * ratiox;
	restrict.B.y = r1.B.y * ratioy;

//	SOL_Point p = MouseWinClr::pos;
	SOL_Point p = pos;
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
MouseWinClr::ClearRestrictRect()
{
	restrict = SOL_Rect(0, 0, graphMgr->Xdim() - 1, graphMgr->Ydim() - 1);
}


void
MouseWinClr::Setup(SOL_Cursor& cur)
{
	cursor = &cur;
}


void
MouseWinClr::SetPos(const SOL_Point& p)
{
Bool	outSide;

	// Must do a SetCursor on every move even if it hasn't changed so that
	// Windows will change it when entering client area.
	SetCursor(hCurCur);

	// See if we are outside the game window
	if ((p.x < gameBorder) 	||
		 (p.y < gameBorder)	||
		 (p.x > (gameWidth  + gameBorder)) ||
		 (p.y > (gameHeight + gameBorder)))
			outSide = True;
	else
			outSide = False;

	pos = p;
	pos.x -= gameBorder;
	pos.y -= gameBorder;

	// If in the game window, move the cursor
	if (!outSide && useMouse && cursor) {
//		pos = p;
//		pos.x -= gameBorder;
//		pos.y -= gameBorder;
		cursor->DeviceMovedTo(pos);
		// The following bit of kludge code is here because in Phantas, if
		// the CD drawer is opened causing a "cannot read file, cancel, retry"
		// message and retry is selected, then the game continues, but the
		// Windows cursor is left on the screen.  I don't know why this is,
		// but changing the cursor twice seems to fix the problem.  I do this
		// on a Windows PAINT message since we always get this when the
		// Windows dialog box disappears.
		if (paintFlag) {
			hCurCur = hAltCur;
			SetCursor(hCurCur);
			hCurCur = hNullCur;
			SetCursor(hCurCur);
		}
	}
	paintFlag = False;

	// if we just left the game window, then hide the color cursor, show
	// the Windows cursor, and do a ReleaseCapture so we don't get mouse
	// move messages.
	if (outSide && !outOfGame) {
		outOfGame = True;
		graphMgr->GCursor().Hide();
		hCurCur = hAltCur;
		SetCursor(hCurCur);
		if (!buttonState)
			ReleaseCapture();
	}

	// if we just entered the game window, then show the color cursor,
	// hide the Windows cursor, and capture the the mouse so that the
	// user can't sneak out without us knowing about it.
	if (!outSide && outOfGame) {
		outOfGame = False;
		graphMgr->GCursor().UnHide();
		hCurCur = hNullCur;
		SetCursor(hCurCur);
		SetCapture(hMyWnd);
	}
} 



Bool
MouseWinClr::IsSwift() const
{
	return isSwift;
} 

void
MouseWinClr::WinMouseButton(unsigned iMessage, const SOL_Point& p)
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
		if (outOfGame)
			ReleaseCapture();
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


