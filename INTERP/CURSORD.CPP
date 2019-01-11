// cursord.cpp

#include <conio.h>
#include <dos.h>
#include <string.h>

#include "sol.hpp"

#include "celobj.hpp"
#include "cursord.hpp"
#include "intflag.hpp"
#include "msg.hpp"
#include "ptrdev.hpp"
#include "video.hpp"
#include "savegame.hpp"
#include "graphmgr.hpp"

CursorDOS::CursorDOS(const BufferStd& buffer):bitmap(-1,-1,-1)
{
	putCursorInVMAP = False;
	gonnaPaint = False;
	hideCount = 0;

	cursorBack.rect.MakeEmpty();
	cursorBack.buffer = NULL;

	cursorData.rect.MakeEmpty();
	cursorData.buffer = NULL;

	saveVmap.rect.MakeEmpty();
	saveVmap.buffer = NULL;

	drawBuff1.rect.MakeEmpty();
	drawBuff1.buffer = NULL;

	drawBuff2.rect.MakeEmpty();
	drawBuff2.buffer = NULL;

	vmapData.rect.Set(0, 0, buffer.XDim() - 1, buffer.YDim() - 1);
	vmapData.buffer = buffer.start;

	nextPos = SOL_Point(0, 0);
	disableCount = 0;
	restrict = vmapData.rect;
	Set(-1, -1, -1);
}


CursorDOS::~CursorDOS()
{
	delete cursorBack.buffer;
	delete cursorData.buffer;
	delete drawBuff1.buffer;
	delete drawBuff2.buffer;
	delete saveVmap.buffer;
}



void
CursorDOS::Set(int v, int l, int c)
{
	//  Get resource, put information away for savegame
	gonnaPaint = True;
	CelObjView* cursor = NULL;
	if (v != -1)
		cursor = New CelObjView(v, l, c);

	//  Stop interrupts, erase cursor if not already hidden
	PushAndDisableInterrupts();
	Hide();


	bitmap.SetView(v);
	bitmap.SetLoop(l);
	bitmap.SetCel(c);

	int xDim, yDim;

	if (cursor)  {
		xhot = cursor->XOrg();
		yhot = cursor->YOrg();
		xDim = cursor->Width();
		yDim = cursor->Height();
	}
	else  {
		xhot = yhot = 0;
		xDim = yDim = 1;
	}

	//  Extract information, init variables
	delete cursorData.buffer;
	delete cursorBack.buffer;
	delete drawBuff1.buffer;
	delete drawBuff2.buffer;
	delete saveVmap.buffer;

	cursorData.buffer = New uchar[xDim * yDim];
	cursorBack.buffer = New uchar[xDim * yDim];
	drawBuff1.buffer = New uchar[xDim * yDim];
	drawBuff2.buffer = New uchar[xDim * yDim * 4];
	saveVmap.buffer = New uchar[xDim * yDim];

	cursorData.rect.Set(0, 0, xDim - 1, yDim - 1);
	memset(cursorData.buffer, 255, xDim * yDim);
	cursorData.skip = 255;

	//  Convert to non-RLE image, display if it wasn't hidden
	if (cursor)  {
		BufferStd dest(xDim, yDim, cursorData.buffer);
		cursor->Draw(dest, cursorData.rect, SOL_Point(0, 0), False);
	}
	else  {
		*cursorData.buffer = cursorData.skip;
		cursorBack.rect = cursorData.rect;
		cursorBack.rect.Clip(vmapData.rect);
		if (!cursorBack.rect.IsEmpty())
			ReadVideo(cursorBack);
	}

	delete cursor;
	UnHide();
	PopInterrupts();
	gonnaPaint = False;
}




void
CursorDOS::SetDevice(PointerDevice& dev)
{
	SOL_Point p;
	posDevice = &dev;
	posDevice->Setup(*this);
	posDevice->GlobalPos(&p);
	DeviceMovedTo(p);
}




void
CursorDOS::ForcePos(SOL_Point p)
{
	posDevice->SetPos(p);
}



void
CursorDOS::GonnaPaint(SOL_Rect rect)
{
	if (!hideCount  &&  !putCursorInVMAP  &&  !cursorBack.rect.IsEmpty())  {
		rect.A.x &= 0x7FFC;
		rect.B.x |= 3;
		if (cursorBack.rect.Intersect(rect)) {
			putCursorInVMAP = True;
		}
	}
	gonnaPaint = True;
}


void
CursorDOS::PaintStarting()
{
	if (putCursorInVMAP)  {
		//  Save Vmap, put cursor in vmap
		saveVmap.rect = cursorData.rect;
		Copy(saveVmap, vmapData);
		Paint(vmapData, cursorData);
	}
}

void
CursorDOS::DonePainting()
{
	if (gonnaPaint)  {

		//  If cursor was put in vmap, take it out now
		if (putCursorInVMAP)  {
			Copy(vmapData, saveVmap);
			saveVmap.rect.MakeEmpty();
			putCursorInVMAP = False;
		}

		//  Update cursor background
		if (!hideCount  &&  !cursorBack.rect.IsEmpty())
			Copy(cursorBack, vmapData);

		gonnaPaint = False;
	}
	else
		msgMgr->Fatal(SrcLoc, Msg_DonePainting);
}



/*
 *
 *  Called from interrupts and normal code!
 *
 */

void
CursorDOS::DeviceMovedTo(const SOL_Point& p)
{
	if (!gonnaPaint)  {
		PushAndDisableInterrupts();
		nextPos.x = p.x;
		nextPos.y = p.y;
		Move();
		PopInterrupts();
	}
}



/*
 *
 *  Called from interrupts and normal code!
 *
 */

void
CursorDOS::Hide()
{
	PushAndDisableInterrupts();

	if (!hideCount)  {
		if (!cursorBack.rect.IsEmpty())  {
			DrawToHardware(cursorBack);
		}
	}
	hideCount++;

	PopInterrupts();
}


/*
 *
 *  Called from interrupts and normal code!
 *
 */

void
CursorDOS::UnHide()
{
	//  Disable further interrrupts
	PushAndDisableInterrupts();

	if (hideCount)  {
		hideCount--;

		//  If it became visible, calc draw area, save bkg, paint
		if (!hideCount)  {
			cursorData.rect.MoveTo(nextPos.x - xhot, nextPos.y - yhot);
			RevealCursor();
		}
	}
	else
		msgMgr->Fatal(SrcLoc, Msg_CursorUnHide);

	PopInterrupts();
}

/*
 *
 *  Called from interrupts and normal code!
 *
 */

void
CursorDOS::Show()
{
	//  Disable further interrrupts
	PushAndDisableInterrupts();

	if (hideCount)  {
		hideCount = 0;

		//  If it became visible, calc draw area, save bkg, paint
		cursorData.rect.MoveTo(nextPos.x - xhot, nextPos.y - yhot);
		RevealCursor();
	}

	PopInterrupts();
}



void
CursorDOS::RevealCursor()
{
	cursorBack.rect = cursorData.rect;
	cursorBack.rect.Clip(vmapData.rect);
	if (cursorBack.rect.IsEmpty())
		return;

	ReadVideo(cursorBack);
	drawBuff1.rect = cursorData.rect;
	Copy(drawBuff1, cursorBack);

	Paint(drawBuff1, cursorData);
	DrawToHardware(drawBuff1);
}



/*
 *
 *  Called from interrupts and normal code!
 *
 */

void
CursorDOS::Move()
{
	if (!hideCount)  {

		//  If it was off the screen, just show it and return
		cursorData.rect.MoveTo(nextPos.x - xhot, nextPos.y - yhot);
		if (cursorBack.rect.IsEmpty())  {
			RevealCursor();
			return;
		}

		//  If we just moved entirely off screen, remove background & return
		if (!cursorData.rect.Intersect(vmapData.rect))  {
			DrawToHardware(cursorBack);
			return;
		}


		if (!cursorData.rect.Intersect(cursorBack.rect))  {
			//  Do two rectangles

			//  Read new rectangles background
			drawBuff1.rect = cursorData.rect;
			drawBuff1.rect.Clip(vmapData.rect);
			ReadVideo(drawBuff1);

			//  Ensure that data from there has been erased

			//  Make duplicate to 2nd buffer
			drawBuff2.rect = drawBuff1.rect;
			Copy(drawBuff2, drawBuff1);

			//  Drawing cursor to buffer 1, display on hardware
			Paint(drawBuff1, cursorData);
			DrawToHardware(drawBuff1);

			//  Display old background to hardware (erase)
			DrawToHardware(cursorBack);

			//  Copy buffer 2 to background
			cursorBack.rect = cursorData.rect;
			cursorBack.rect.Clip(vmapData.rect);
			Copy(cursorBack, drawBuff2);
		}
		else  {
			//  Do one rectangle

			SOL_Rect mergedRect = cursorBack.rect;
			mergedRect.Both(cursorData.rect);
			mergedRect.Clip(vmapData.rect);

			//  Set up overlapping rectangle, read background from video
			drawBuff2.rect = mergedRect;
			ReadVideo(drawBuff2);

			//  Erase old cursor from new background rect
			Copy(drawBuff2, cursorBack);

			//  Update current cursor background
			cursorBack.rect = cursorData.rect;
			cursorBack.rect.Clip(vmapData.rect);
			Copy(cursorBack, drawBuff2);

			//  Draw cursor in background buffer, update hardware
			Paint(drawBuff2, cursorData);
			DrawToHardware(drawBuff2);
		}
	}
}




/*
 *
 *  Called from interrupts (Unhide)!
 *
 */

void
CursorDOS::Paint(const Info& dest, const Info& src)
{
	if (src.rect.IsEmpty())
		return;

	SOL_Rect srcRect = src.rect;
	srcRect.Clip(dest.rect);
	if (srcRect.IsEmpty())
		return;

	int sxo = srcRect.A.x - src.rect.A.x;
	int syo = srcRect.A.y - src.rect.A.y;
	int xsize = srcRect.Xlen();
	int ysize = srcRect.Ylen();

	uchar* destPtr = dest.buffer + ((srcRect.A.y - dest.rect.A.y) * dest.rect.Xlen()) + (srcRect.A.x - dest.rect.A.x);
	const uchar* srcPtr = src.buffer + (syo * src.rect.Xlen()) + sxo;
	uchar skip = src.skip;

	int srcNextLine = src.rect.Xlen() - xsize;
	int destNextLine = dest.rect.Xlen() - xsize;

	for (int y = 0; y < ysize; y++)
	{
		for (int x = 0; x < xsize; x++)
		{
			if (*srcPtr != skip)
				*destPtr = *srcPtr;
			destPtr++;
			srcPtr++;
		}
		srcPtr += srcNextLine;
		destPtr += destNextLine;
	}
}

/*
 *
 *  Called from interrupts (Unhide)!
 *
 */

void
CursorDOS::Copy(const Info& dest, const Info& src)
{
	if (src.rect.IsEmpty())
		return;

	SOL_Rect srcRect = src.rect;
	srcRect.Clip(dest.rect);
	if (srcRect.IsEmpty())
		return;


	int sxo = srcRect.A.x - src.rect.A.x;
	int syo = srcRect.A.y - src.rect.A.y;
	int xsize = srcRect.Xlen();
	int ysize = srcRect.Ylen();

	uchar* destPtr = dest.buffer + ((srcRect.A.y - dest.rect.A.y) * dest.rect.Xlen()) + (srcRect.A.x - dest.rect.A.x);
	const uchar* srcPtr = src.buffer + (syo * src.rect.Xlen()) + sxo;

	int srcNextLine = src.rect.Xlen();
	int destNextLine = dest.rect.Xlen();

	for (int y = 0; y < ysize; y++)  {
		memcpy(destPtr, srcPtr, xsize);
		destPtr += destNextLine;
		srcPtr += srcNextLine;
	}
}


/*
 *
 *  Called from interrupts!
 *
 */

void
CursorDOS::PushAndDisableInterrupts()
{
	if (!disableCount)
		interruptState = ReadInterruptAndClear();
	disableCount++;
}



/*
 *
 *  Called from interrupts!
 *
 */

void
CursorDOS::PopInterrupts()
{
	if (!disableCount) {
		msgMgr->Fatal(SrcLoc, Msg_StackUnderflow);
		getch();
	}
	disableCount--;
	if (!disableCount)
		SetInterruptFlag(interruptState);
}


void
CursorDOS::DrawToHardware(const Info& info)

{
	SOL_Rect rect = info.rect;
	rect.Clip(vmapData.rect);
	int sxo = rect.A.x - info.rect.A.x;
	int syo = rect.A.y - info.rect.A.y;
	uchar* src = info.buffer + (syo * info.rect.Xlen()) + sxo;

	// need both srcXOff and srcYOff because we don't know here
	// what video mdeo we're in and which we'll need
	int srcXOff = info.rect.Xlen() - rect.Xlen();

	WriteVideoRect(&rect,src,srcXOff);
}


void
CursorDOS::ReadVideo(const Info& info)
{
	if (graphMgr->vmapValid)
		ReadVideoFromVMAP(info);
	else
		ReadVideoFromHardware(info);
}

void
CursorDOS::ReadVideoFromHardware(const Info& info)
{
	uchar* dest = info.buffer;
	ReadVideoRect(&info.rect, dest);
}

void
CursorDOS::ReadVideoFromVMAP(const Info& info)
{
	Copy(info, vmapData);
}



void
CursorDOS::SetRestrictRect(const SOL_Rect& r)
{
	restrict = r;
	restrict.Clip(vmapData.rect);
	posDevice->SetRestrictRect(restrict);
	ForcePos(nextPos);
}


void
CursorDOS::ClearRestrictRect()
{
	restrict = vmapData.rect;
	posDevice->ClearRestrictRect();
}


void
CursorDOS::SaveRestore(SaveGame* saveGame, Bool save)
{
	int view = bitmap.View();
	int loop = bitmap.Loop();
	int cel = bitmap.Cel();

	saveGame->SaveRestore(&view, sizeof(int), save);
	saveGame->SaveRestore(&loop, sizeof(int), save);
	saveGame->SaveRestore(&cel, sizeof(int), save);

	if (save)  {
		saveGame->SaveRestore(&hideCount, sizeof(hideCount), save);
		saveGame->SaveRestore(&restrict.A.x, sizeof(restrict.A.x), save);
		saveGame->SaveRestore(&restrict.A.y, sizeof(restrict.A.y), save);
		saveGame->SaveRestore(&restrict.B.x, sizeof(restrict.B.x), save);
		saveGame->SaveRestore(&restrict.B.y, sizeof(restrict.B.y), save);
	}
	else  {
		saveGame->SaveRestore(&restoreHideCount, sizeof(restoreHideCount), save);
		saveGame->SaveRestore(&restoreRestrict.A.x, sizeof(restoreRestrict.A.x), save);
		saveGame->SaveRestore(&restoreRestrict.A.y, sizeof(restoreRestrict.A.y), save);
		saveGame->SaveRestore(&restoreRestrict.B.x, sizeof(restoreRestrict.B.x), save);
		saveGame->SaveRestore(&restoreRestrict.B.y, sizeof(restoreRestrict.B.y), save);
	}
}

void
CursorDOS::SaveRestoreAfter(SaveGame*, Bool save)
{
	if (!save)  {
		//  Turn off interrupts  ******
		restrict = restoreRestrict;
		Hide();
		Set(bitmap.View(), bitmap.Loop(), bitmap.Cel());

		if (!restoreHideCount)
			Show();

		hideCount = restoreHideCount;
		//  Turn on interrupts  *******
	}
}

