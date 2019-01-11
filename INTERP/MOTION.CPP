// motion.cpp
//		kernel procedures for dealing with Actor motion and cycling.

#include <string.h>

#include "sol.hpp"

#include "graph.hpp"
#include "kernel.hpp"
#include "math.hpp"
#include "msg.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include "rect.hpp"
#include "resource.hpp"
#include "selector.hpp"
#include "si.hpp"

// states of actor's "s_signal" property 
const uint	CANUPD		= 0x0001;	// init "stopped condition
const uint	HIDDEN		= 0x0008;	// an actor to HIDE has been hidden 
const uint	setBaseRect = 0x0020;   // 
const uint	blocked	   = 0x0400;   // 
const uint	fixedLoop	= 0x0800;	// loop is fixed 
const uint	skipCheck	= 0x1000;	//	cel fixed
const uint	ignrHrz		= 0x2000;	// can ignore horizon
const uint	ignrAct		= 0x4000;	// can ignore other actors
const uint	setNSRect   = 0x8000;

static void	DirLoop(ObjectID, int);
static void	GetCelRect(int view,int loop,int cel,int x,int y,int z,SOL_Rect& rect);

void
KBaseSetter(argList)
{
	ObjectID	actor = arg(1);
	int scale = actor.GetIndexedProperty(actScaleSignal);

   SOL_Rect rect;

   // Set the base rect based on the cel dimensions
	if (!(scale & SCALE_ON_BIT)) {
		GetCelRect(actor.GetIndexedProperty(actView),
					  actor.GetIndexedProperty(actLoop),
					  actor.GetIndexedProperty(actCel),
					  actor.GetIndexedProperty(actX),
					  actor.GetIndexedProperty(actY),
					  actor.GetIndexedProperty(actZ),
                 rect);

		actor.SetIndexedProperty(actBRLeft, rect.A.x);
		actor.SetIndexedProperty(actBRRight, rect.B.x);
   }
	else {
		GetCelRect(actor.GetIndexedProperty(actView),
					  actor.GetIndexedProperty(actLoop),
					  actor.GetIndexedProperty(actCel),
					  actor.GetIndexedProperty(actX),
					  actor.GetIndexedProperty(actY),
					  actor.GetIndexedProperty(actZ),
                 rect);

		actor.SetIndexedProperty(actBRLeft,  rect.A.x);
		actor.SetIndexedProperty(actBRRight, rect.B.x);
//
// The NSLeft and NSRight were not updated and so the BRLeft,Right was always
// off..just estimate now until a proper fix is done.
//
//		actor.SetIndexedProperty(actBRLeft, actor.GetIndexedProperty(actNSLeft));
//		actor.SetIndexedProperty(actBRRight, actor.GetIndexedProperty(actNSRight));
   }

	int y = actor.GetIndexedProperty(actY) +1;
	actor.SetIndexedProperty(actBRBottom, y);
	actor.SetIndexedProperty(actBRTop, y - actor.GetIndexedProperty(actYStep));
}

static void
GetCelRect(int view,int loop,int cel,int x,int y,int z,SOL_Rect& rect)
{

   CelObjView celObj(view,loop,cel);

	Ratio scaleX(SCIRESX,celObj.ResX());
	Ratio scaleY(SCIRESY,celObj.ResY());

   if (celObj.Mirrored()) {
		rect.A.x = x - (celObj.Width() - celObj.XOrg())*scaleX;
	}
	else {
		rect.A.x = x - celObj.XOrg()*scaleX;
	}
	rect.B.x = rect.A.x + celObj.Width()*scaleX - 1;
	rect.A.y = y - z - celObj.YOrg()*scaleY + 1;
	rect.B.y = rect.A.y + celObj.Height()*scaleY - 1;
}

static void
GetCelRectScaled(int view,int loop,int cel,int x,int y,int z,int sX, int sY, SOL_Rect& rect)
{

   CelObjView celObj(view,loop,cel);

	Ratio scaleX(sX, 128); 
	Ratio scaleY(sY, 128);

   if (celObj.Mirrored()) {
		rect.A.x = x - (celObj.Width() - celObj.XOrg())*scaleX;
	}
	else {
		rect.A.x = x - celObj.XOrg()*scaleX;
	}
	rect.B.x = rect.A.x + celObj.Width()*scaleX - 1;
	rect.A.y = y - z - celObj.YOrg()*scaleY + 1;
	rect.B.y = rect.A.y + celObj.Height()*scaleY - 1;
}

void
KCelRect(argList)
{
	SOL_Rect r;
	
	ObjectID saveRect = arg(1);

	if (argCount > 7)
		GetCelRectScaled(arg(2), arg(3), arg(4), arg(5), arg(6), arg(7), arg(8), arg(9), r);
	else
		GetCelRect(arg(2), arg(3), arg(4), arg(5), arg(6), arg(7), r);

	saveRect.SetProperty(s_left,  r.A.x);
	saveRect.SetProperty(s_top,   r.A.y);
	saveRect.SetProperty(s_right, r.B.x);
	saveRect.SetProperty(s_bottom,r.B.y);
}

void
KDirLoop(argList)
{
	DirLoop(arg(1), arg(2));
}

void
KInitBresen(argList)
{
	//	Initialize internal state of a motion class for a modified Bresenham line
	// algorithm (see 'bresen.doc' for derivation).

	ObjectID	motion = arg(1);
	ObjectID	client = motion.GetIndexedProperty(motClient);
	int		dx, dy, incr, i1, i2, xAxis, di;
	int		watchDog;
	int		skipFactor = argCount >= 2 ? arg(2) : 1;
	int		toX = motion.GetIndexedProperty(motX);
	int		toY = motion.GetIndexedProperty(motY);
	int		tdx = client.GetIndexedProperty(actXStep) * skipFactor;
	int		tdy = client.GetIndexedProperty(actYStep) * skipFactor;

	if (tdx > tdy)
		watchDog = tdx;
	else
		watchDog = tdy;

	watchDog *= 2;

	// Get distances to be moved.
	int DX = toX - client.GetIndexedProperty(actX);
	int DY = toY - client.GetIndexedProperty(actY);

	// Compute basic step sizes.
	while (1) {
		dx = tdx;
		dy = tdy;

		if (Abs(DX) >= Abs(DY)) {
			// Then motion will be along the x-axis.
			xAxis = True;
			if (DX < 0)
				dx = -dx;
			dy = DX ? dx * DY / DX : 0;

		} else {
			// Our major motion is along the y-axis.
			xAxis = False;
			if (DY < 0)
				dy = -dy;
			dx = DY ? dy * DX / DY : 0;
		}
	
		// Compute increments and decision variable.
		i1 = xAxis ? 2 * (dx * DY - dy * DX) : 2 * (dy * DX - dx * DY);
		incr = 1;
		if ((xAxis && DY < 0) || (!xAxis && DX < 0)) {
			i1 = -i1;
			incr = -1;
		}
	
		i2 = i1 - 2 * (xAxis ? DX : DY);
		di = i1 - (xAxis ? DX : DY);
	
		if ((xAxis && DX < 0) || (!xAxis && DY < 0)) {
			i1 = -i1;
			i2 = -i2;
			di = -di;
		}

		// limit x step to avoid over stepping Y step size
		if (xAxis && tdx > tdy) {
			if (tdx && Abs(dy + incr) > tdy){
				if (!(--watchDog))
					msgMgr->Fatal(SrcLoc, Msg_Bresen);
				--tdx;
				continue;
			}
		}

		break;
	
	}
	// Set the various variables we've computed.
	motion.SetIndexedProperty(motDX, (Property) dx);
	motion.SetIndexedProperty(motDY, (Property) dy);
	motion.SetIndexedProperty(motI1, (Property) i1);
	motion.SetIndexedProperty(motI2, (Property) i2);
	motion.SetIndexedProperty(motDI, (Property) di);
	motion.SetIndexedProperty(motIncr, (Property) incr);
	motion.SetIndexedProperty(motXAxis, (Property) xAxis);
}

void
KDoBresen(argList)
{
	// Move an actor one step

	ObjectID motion = arg(1);
	ObjectID client = motion.GetIndexedProperty(motClient);
	int		i1, i2, di, oldX, oldY;
//	char		aniState[1000];
	SCIWord	blocker;

	pm.acc = 0;

//	client.SetIndexedProperty(actSignal,
//		Property(client.GetIndexedProperty(actSignal) & ~blocked));

	// Get properties in variables for speed and convenience
	int x			= client.GetIndexedProperty(actX);
	int y			= client.GetIndexedProperty(actY);
	int toX		= motion.GetIndexedProperty(motX);
	int toY		= motion.GetIndexedProperty(motY);
	int xAxis	= motion.GetIndexedProperty(motXAxis);
	int dx		= motion.GetIndexedProperty(motDX);
	int dy		= motion.GetIndexedProperty(motDY);
	int incr		= motion.GetIndexedProperty(motIncr);
//	uint cSignal = client.GetIndexedProperty(actSignal);

//	if (cSignal & setBaseRect){
//		oldBrTop =	client.GetIndexedProperty(actBRTop);
//		oldBrLeft = client.GetIndexedProperty(actBRLeft);
//		oldBrRight = client.GetIndexedProperty(actBRRight);
//		oldBrBottom =	client.GetIndexedProperty(actBRBottom);
//	}

	i1	  = motion.GetIndexedProperty(motI1);
	i2	  = motion.GetIndexedProperty(motI2);
	di	  = motion.GetIndexedProperty(motDI);
	oldX = x;
	oldY = y;
	motion.SetIndexedProperty(motXLast, (Property) x);
	motion.SetIndexedProperty(motYLast, (Property) y);

	// Save the current animation state before moving the client
	//	this will be replaced!
//	memcpy(aniState, *client, client->Size() * sizeof(Property));

	if ((xAxis && (Abs(toX - x) <= Abs(dx))) ||
		 (!xAxis && (Abs(toY - y) <= Abs(dy)))) {
		/* We're within a step size of the destination -- set
			client's x & y to it.
		*/
		x = toX;
		y = toY;

	} else {
		// Move one step.
		x += dx;
		y += dy;
		if (di < 0)
			di += i1;
		else {
			di += i2;
			if (xAxis)
				y += incr;
			else
				x += incr;
		}
	}

	// Update client's properties.
	client.SetIndexedProperty(actX, (Property) x);
	client.SetIndexedProperty(actY, (Property) y);

	// Check position validity for this cel.
	if (blocker = pm.InvokeMethod(client, s_cantBeHere)) {
		/* Client can't be here -- restore the original state and
			mark the client as blocked.
		*/
//		memcpy(*client, aniState, client->Size() * sizeof(Property));
//		i1 = si1;
//		i2 = si2;
//		di = sdi;
		// reset client's position.
  		client.SetIndexedProperty(actX, (Property) oldX);
		client.SetIndexedProperty(actY, (Property) oldY);
//		if (cSignal & setBaseRect){
//			client.SetIndexedProperty(actBRTop, (Property) oldBrTop);
//			client.SetIndexedProperty(actBRLeft, (Property) oldBrLeft);
//			client.SetIndexedProperty(actBRRight, (Property) oldBrRight);
//			client.SetIndexedProperty(actBRBottom, (Property) oldBrBottom);
//		}

		client.SetIndexedProperty(actSignal,
			Property(client.GetIndexedProperty(actSignal) | blocked));
	}
	else {
		client.SetIndexedProperty(actSignal,
			Property(client.GetIndexedProperty(actSignal) & ~blocked));
		motion.SetIndexedProperty(motI1, (Property) i1);
		motion.SetIndexedProperty(motI2, (Property) i2);
		motion.SetIndexedProperty(motDI, (Property) di);
	}
	if (x == toX && y == toY)
		pm.InvokeMethod(motion, s_moveDone);
	pm.acc = blocker;
}

void
KSetJump(argList)
{
	/* Compute the initial xStep for a motion of class Jump based on the
	 * x and y differences of the start and end points and the force of
	 * gravity.  This was downcoded from Script to use longs to avoid
	 * overflow errors.
	 */

	long	denom;
	int	n, xStep, yStep;

	ObjectID theJump = arg(1);
	long DX = arg(2);
	long DY = arg(3);
	int gy = arg(4);
	
	/* For  most motion (increasing y or x motion comparable to or greater
	 * than y motion), we pick equal x & y velocities.  For motion which
	 * is mainly upward, we pick a y velocity which is n times that of x.
	 */
	n = 1;
	if (DX && (DY + Abs(DX)) < 0)
		n = (int) ((2 * Abs(DY)) / Abs(DX));
		
	while (1) {
		denom = DY + n * Abs(DX);
		if (Abs(2 * denom) > Abs(DX))
			break;
		++n;
	   }
	
	xStep = denom ? sqrt(gy * DX * DX / (2 * denom)) : 0;
	
	/* Scale the y velocity, make sure that its sign is negative and that
	 * the x velocity is of the same sign as the x distance.
	 */
	yStep = n * xStep;
	if (yStep > 0)
		yStep = -yStep;
	if (DX < 0)
		xStep = -xStep;

	/* If we're supposed to move up and the y velocity is 0, recompute
	 * y based on no x movement.
	 */
	if (DY < 0 && yStep == 0)
		yStep = -1 - sqrt(-(2 * gy * DY));

	// Set the jump properties.
	theJump.SetIndexedProperty(jmpXStep, (Property) xStep);
	theJump.SetIndexedProperty(jmpYStep, (Property) yStep);
}

void
KCantBeHere(argList)
{
	// determine and return legality of actors position
	// This code checks base rect intersection

	ObjectID		him = arg(1);
	SOL_ListID	cast = arg(2);
	ObjectID		me;
	SOL_Rect		r;
	SOL_Rect		chkR;
	const SCIWord		*adr = him.GetIndexedPropAddr(actBRLeft);
	r.A.x = *adr;
	r.A.y = *(adr + 1);
	r.B.x = *(adr + 2);
	r.B.y = *(adr + 3);
	
	/* (s_illegalBits) are the bits that the object cannot be on.
		Anding this with the bits the object is on tells us which
		bits the object is on but shouldn't be on.  If this is zero,
		the position is valid.
	 */
	pm.acc = 0;
	// if I am hidden or ignoring actors my position is legal
	if (!(him.GetIndexedProperty(actSignal) & (ignrAct | HIDDEN))) {
		// default to no hits
		pm.acc = 0;
		// now the last thing we care about is our rectangles
		for (KNodeID node = cast->First(); (int) node; node = node->Next()) {
			me = node->val;
			// Can't hit myself
			if (him == me)
				continue;

			// can't hit if I'm as below
			if (me.GetIndexedProperty(actSignal) & (ignrAct | HIDDEN))
				continue;

			// if our rectangles intersect we are done
			adr = me.GetIndexedPropAddr(actBRLeft);
			chkR.A.x = *adr;
			chkR.A.y = *(adr + 1);
			chkR.B.x = *(adr + 2);
			chkR.B.y = *(adr + 3);
			if (r.A.x >= chkR.B.x || r.B.x <= chkR.A.x ||
				 r.A.y >= chkR.B.y || r.B.y <= chkR.A.y) {
				continue;
			} else {
				pm.acc = (Acc) me;
				break;
			}
		}
	}
}

static void
DirLoop(ObjectID actor, int angle)
{
	//	This code correlates the desired direction to the proper loop.

	// Only set loop if loop is not fixed.
	if (!(fixedLoop & actor.GetIndexedProperty(actSignal))) {
		int loop;
		int nLoops = GetNumLoops(resMgr->Get(MemResView,
			actor.GetIndexedProperty(actView)));

		// Set the loop for the actor based on how many loops it has.
		if (angle > 315 || angle < 45)
			loop = (nLoops >= 4)? 3 : -1;
		else if (angle > 135 && angle < 225)
			loop = (nLoops >= 4)? 2 : -1;
		else if ((angle < 180))
			loop = 0;
		else
			loop = 1;

		// If the loop is not 'same' (-1), set it.
		if (loop != -1)
			actor.SetIndexedProperty(actLoop, (Property) loop);
	}
}
