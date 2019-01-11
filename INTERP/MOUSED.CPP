#include <dos.h>
#include <string.h>

#include "sol.hpp"

#include "cursor.hpp" 
#include "event.hpp"
#include "graphmd.hpp" 
#include "intflag.hpp"
#include "kbdmgr.hpp"
#include "kernel.hpp"
#include "moused.hpp"
#include "pmachine.hpp"
#include "ratio.hpp"
#include "savegame.hpp"
#include "vga.hpp"

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

SOL_Cursor*	MouseDOS::cursor = NULL;
SOL_Rect		MouseDOS::restrict;
Bool			MouseDOS::useMouse = True;
Bool			MouseDOS::isSwift = False;
Bool			MouseDOS::exists = False;
Bool			MouseDOS::entered = False;
uchar*		MouseDOS::stackBuff = NULL;
SOL_Point	MouseDOS::pos;
short			MouseDOS::zaxis = 0;
short			MouseDOS::pitch = 0;
short			MouseDOS::roll = 0;
short			MouseDOS::yaw = 0;
int			MouseDOS::buttonState = 0;
SOL_Event	MouseDOS::event;		//  Watcom 9.5 insisted that the interrupt
											//  routine couldn't create one locally on
											//  the stack.  DF

#pragma warning 579  9				//   Shut off cast warning for mouse handler


MouseDOS::MouseDOS(Bool)
{
	cursor = NULL;
	useMouse = True;

	InitMouse();
	if (Exists())  {
		stackBuff = New uchar[15000];
		InstallMouseInterrupt();
		ClearRestrictRect();
	}
	else
		msgMgr->Fatal("\nNo mouse drivers was found in DOS.\n"
						  "Please rerun INSTALL and read the README file for more help.");
}


MouseDOS::~MouseDOS()
{
	if (useMouse)  { // Exists())  {
		ReleaseMouseInterrupt();
		delete stackBuff;
	}
}


int
MouseDOS::GlobalPos(SOL_Point* pt) const
{
	pt->x = pos.x;
	pt->y = pos.y;
	return buttonState;
}


int
MouseDOS::GlobalPosSci(SOL_Point* pt) const
{
	pt->x = pos.x * Ratio(SCIRESX, graphMgr->Xdim());
	pt->y = pos.y * Ratio(SCIRESY, graphMgr->Ydim());
	return buttonState;
}


int
MouseDOS::GetSwiftInfo(int* z, int* p, int* r, int* y) const
{
	// Return interrupt level position in global coords of mouse in the point
	*z = zaxis;
	*p = pitch;
	*r = roll;
	*y = yaw;
	return buttonState;
}


Bool
MouseDOS::Exists() const
{
	return exists;
}

void
MouseDOS::InstallMouseInterrupt()
{
	union REGPACK	regs;
	struct SREGS	sregs;

	memset(&regs, 0, sizeof(regs));
	segread(&sregs);
	regs.x.es = FP_SEG(HndlMouse);
   if (isSwift)
      regs.w.cx = 0x7fff;
   else
      regs.w.cx = 0x7f;

	regs.x.edx = FP_OFF(HndlMouse);
	regs.w.ax = mouseSetInt;
	intr(mouseInt, &regs);
}

void
MouseDOS::ReleaseMouseInterrupt()
{
	union REGPACK regs;

	memset(&regs, 0, sizeof(regs));
	regs.w.ax = mouseInstalled;
	intr(mouseInt, &regs);
}


void
MouseDOS::InitMouse()
{
	union REGPACK regs;

	//  Reset mouse, set existance flag
	memset(&regs, 0, sizeof(regs));
	regs.w.ax = mouseInstalled;
	intr(mouseInt, &regs);
	exists = regs.w.ax;
	if (!exists)
		return;

// HOOK
// This code can not work with Rational's 4gw DOS extender
// There is supposed to be an address passed in ES:DX
// Check the documentation

//	//  Is mouse Swift
//	memset(&regs, 0, sizeof(regs));
//	regs.w.ax = swiftInstalled;
//	intr(mouseInt, &regs);
//	isSwift = (regs.w.ax == 1); // cool we got us a CYBER MOUSE - them special effects is fantastic

	pos.x = 0;
	pos.y = 0;
}

//
//  Uses mouse driver coordinate system
//

void
MouseDOS::SetMouseDriverPos(const SOL_Point& p)
{
	union REGPACK regs;

	memset(&regs, 0, sizeof regs);
	regs.w.cx = p.x;
	regs.w.dx = p.y;

	// call the mouse
	regs.w.ax = mouseSet;
	intr(mouseInt, &regs);
}


void
MouseDOS::SetRestrictRect(const SOL_Rect& r1)
{
	Ratio ratiox(graphMgr->Xdim(), SCIRESX);
	Ratio ratioy(graphMgr->Ydim(), SCIRESY);

	restrict.A.x = r1.A.x * ratiox;
	restrict.A.y = r1.A.y * ratioy;
	restrict.B.x = r1.B.x * ratiox;
	restrict.B.y = r1.B.y * ratioy;

	SOL_Point p = MouseDOS::pos;
	if (p.x < restrict.A.x)
		p.x = restrict.A.x;
	if (p.x > restrict.B.x)
		p.x = restrict.B.x;

	if (p.y < restrict.A.y)
		p.y = restrict.A.y;
	if (p.y > restrict.B.y)
		p.y = restrict.B.y;

	SetMouseDriverPos(p);
}

void
MouseDOS::ClearRestrictRect()
{
	restrict = SOL_Rect(0, 0, graphMgr->Xdim() - 1, graphMgr->Ydim() - 1);
}


void
MouseDOS::Setup(SOL_Cursor& cur)
{
	cursor = &cur;
}


void
MouseDOS::SetPos(const SOL_Point& p)
{
	entered = True;
	pos = p;
	pos.x = pos.x * Ratio(graphMgr->Xdim(), SCIRESX);
	pos.y = pos.y * Ratio(graphMgr->Ydim(), SCIRESY);

	if (useMouse)  {
		SetMouseDriverPos(pos);
		if (cursor)
			cursor->DeviceMovedTo(pos);
	}
	entered = False;
} 



Bool
MouseDOS::IsSwift() const
{
	return isSwift;
} 

void
MouseDOS::Vibrate(int duration, int on, int off) const
{
	if (!isSwift)
		return;

	union REGPACK regs;
	memset(&regs, 0, sizeof(union REGPACK));
	regs.w.ax = swiftVibrate;
	regs.w.bx = on << 8 + off;
	regs.w.cx = duration;
	intr(mouseInt, &regs);
}



//Mouse interupt handler. build an event record and post it
#pragma off (check_stack)
void _loadds __far
HndlMouse(int eax, int ebx, int, int, int esi, int edi)
{

#pragma aux HndlMouse parm [EAX] [EBX] [ECX] [EDX] [ESI] [EDI]
	static int status;
	static SOL_Point p1;
	static short rawx, rawy, lastrawx, lastrawy, lastposx, lastposy;

	if (MouseDOS::entered)
		return;

	MouseDOS::entered = True;
	status = eax;
	MouseDOS::buttonState = ebx;
	rawx = (short) esi;
	rawy = (short) edi;
	MouseDOS::pos.x += rawx - lastrawx;
	MouseDOS::pos.y += rawy - lastrawy;

	if (MouseDOS::pos.x > MouseDOS::restrict.B.x)
		MouseDOS::pos.x = MouseDOS::restrict.B.x;
	else if (MouseDOS::pos.x < MouseDOS::restrict.A.x)
		MouseDOS::pos.x = MouseDOS::restrict.A.x;

	if (MouseDOS::pos.y > MouseDOS::restrict.B.y)
		MouseDOS::pos.y = MouseDOS::restrict.B.y;
	else if (MouseDOS::pos.y < MouseDOS::restrict.A.y)
		MouseDOS::pos.y = MouseDOS::restrict.A.y;

		static SWIFT_3DStatus *swiftData;
		swiftData = (SWIFT_3DStatus *) ((esi & 0xffff) << 4);    // static to get it off the stack

	short es = SetESToDS();
	SetStack(MouseDOS::stackBuff + 10000, 0);

	if(mouse->HotOn()) {
		SOL_Point pnt;
		mouse->GlobalPosSci(&pnt);
		mouse->CheckHotRectangles(pnt);
	}

	if (MouseDOS::isSwift && (status & swiftFunc)) {
		MouseDOS::zaxis = swiftData->z;
		MouseDOS::pitch = swiftData->pitch;
		MouseDOS::roll = swiftData->roll;
		MouseDOS::yaw = swiftData->yaw;
	}

	mouse->GlobalPosSci(&(MouseDOS::event.where));
	MouseDOS::event.zaxis = MouseDOS::zaxis;
	MouseDOS::event.pitch = MouseDOS::pitch;
	MouseDOS::event.roll = MouseDOS::roll;
	MouseDOS::event.yaw = MouseDOS::yaw;

   // Note here two event could theoretically be posted if both a button press
   // and swift event happen in the same interupt.

   // swift event posted
	if(MouseDOS::isSwift && (status & swiftFunc)) {
		MouseDOS::event.type = SOL_Event::MouseExt;
		MouseDOS::event.modifiers = kbdMgr->Modifiers();
		eventMgr->Post(&MouseDOS::event);
	}
		
	// Only create events for non-move incidents
	if (status & (butUp | butDown)) {

		// See whether a button went up or down.
		if (status & butDown)
			MouseDOS::event.type = SOL_Event::MouseDown;
		else
			MouseDOS::event.type = SOL_Event::MouseUp;

		// Center button events are treated as ctrl-left button events.
		// Set the modifiers accordingly.
		if (status & cButton)
			MouseDOS::event.modifiers = Kbd::Ctrl;
		else
			MouseDOS::event.modifiers = 0;

		// Right button events are treated as shift-left button events.
		// Set the modifiers accordingly.
		if (status & rButton)
			MouseDOS::event.modifiers = Kbd::Shift;

		MouseDOS::event.modifiers |= kbdMgr->Modifiers();

		// Post the event.
		eventMgr->Post(&MouseDOS::event);
	}

	// If the mouse moved, we must move cursor to a new location.
	if (MouseDOS::pos.x != lastposx  ||  MouseDOS::pos.y != lastposy)  {
		if (MouseDOS::cursor)  {
			MouseDOS::cursor->DeviceMovedTo(MouseDOS::pos);
		}
	}

	lastposx = MouseDOS::pos.x;
	lastposy = MouseDOS::pos.y;
	lastrawx = rawx;
	lastrawy = rawy;

	RestoreStack(0);
	SetES(es);
	MouseDOS::entered = False;
}
#pragma on (check_stack)


