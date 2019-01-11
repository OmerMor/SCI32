//	cursorw.cpp

#ifndef COLORCUR

#include "sciwin.hpp"
#include "sol.hpp"

#include "celobj.hpp"
#include "cursorw.hpp"
#include "graphmgr.hpp"
#include "mouse.hpp"
#include "msg.hpp"
#include "palmgr.hpp"
#include "ptrdev.hpp"
#include "resource.hpp"
#ifdef WIN32S
#include "shiftpal.hpp"
#endif
#include "mbox.hpp"

uchar	buffAND[128];
uchar	buffXOR[128];

// I put this here instead of in the CursorWin derived class because when
// I did so, it was getting corrupted.  Need to investigate this further.
static	Bool			restrictFlag = False;


CursorWin::CursorWin()
{
SOL_Point	p;

	hideCount = 0;
	fullSize.MakeEmpty();
	restrictFlag = False;
	p.x = 0;
	p.y = 0;
	ForcePos(p);
}


CursorWin::~CursorWin()
{
	ShowCursor (1);
}



void
CursorWin::Set(int v, int l, int c)
{
LPSTR		lpCursor;
uint		i;
int		x, y, xDim, yDim, value, MidRange, HiRange, LoRange;
int		HiVal, LoVal, xHot, yHot;
BYTE		skipColor, theByte;
DWORD		ANDmask, XORmask, XORmask2;
Rgb24		rgb1[256];
Bool		ADDED = False;

uchar *	lpAND;
uchar *	lpXOR;

	CelObjView* cursor = NULL;

	//  Get resource
	if (cursorViewAdd)
		if (resMgr->Check(MemResView,(ResNum)(v + cursorViewAdd))) {
			v += cursorViewAdd;
			ADDED = True;
		}
	cursor = New CelObjView(v, l, c);

	xDim = cursor->Width();
	yDim = cursor->Height();

	if ((xDim > 32) || (yDim > 32))
		if (cursorViewAdd && ADDED) {
			delete cursor;
			v -= cursorViewAdd;
			cursor = New CelObjView(v, l, c);
			xDim = cursor->Width();
			yDim = cursor->Height();
		}

	if ((xDim > 32) || (yDim > 32)) {
		sprintf(mbuff,"cursor too big. view:%d loop:%d cel:%d x:%d y:%d",
				 v, l, c, xDim, yDim);
		MBox(mbuff,"");
		delete cursor;
		return;
	}

	//  erase cursor if not already hidden
	Hide();

	bitmap.SetView(v);
	bitmap.SetLoop(l);
	bitmap.SetCel(c);

	if (hMyCur) {
		SetCursor(NULL);
		if (!DestroyCursor(hMyCur))
			MBox("cursor destroy err","");
	}
	lpAND = buffAND;
	lpXOR = buffXOR;

	//  Extract information, init variables
	imageData = New uchar[xDim * yDim];

	//  Fill imagedata with skip for areas not drawn to
	memset(imageData, 255, xDim * yDim);

	fullSize = SOL_Rect(0, 0, xDim - 1, yDim - 1);
	xHot = cursor->XOrg();
	yHot = cursor->YOrg();
	skipColor = 255;

	//  Convert to non-RLE image, display if it wasn't hidden
	BufferStd dest(xDim, yDim, imageData);
	cursor->Draw(dest, fullSize, SOL_Point(0, 0), False);

	//find the value range (lightest, darkest) so we may adjust the contrast
	lpCursor = (LPSTR)dest.Addr(0,0);
	HiVal = 0;
	LoVal = 65025;
#ifndef WIN32S
	for (i = 0; i < 256; i++) {
	  	rgb1[i] = (Rgb24) graphMgr->GPalette().Source()[i];
	}
#else
	if (!WING) {
		for (i = 0; i < 256; i++)
		  	rgb1[i] = (Rgb24) graphMgr->GPalette().Source()[i];
	} else {
		for (i = 0; i < 246; i++)
		  	rgb1[i+PALSHIFT] = (Rgb24) graphMgr->GPalette().Source()[i];
	}
#endif
	for (i = 0; i < xDim*yDim; i++) {
		theByte = *lpCursor++;
		if (theByte != skipColor) {
			value = (28  * (int)rgb1[theByte].red +
						150 * (int)rgb1[theByte].green +
						28  * (int)rgb1[theByte].blue);
			if (value > HiVal)
				HiVal = value;
			if (value < LoVal)
				LoVal = value;
		}
	}

	MidRange = (HiVal + LoVal) / 2;				/* for 2 value cursor */

	HiRange = HiVal - ((HiVal - LoVal) / 3);	/* for 3 value cursor */
	LoRange = LoVal + HiVal - HiRange; 			/* for 3 value cursor */

	lpCursor = (LPSTR)dest.Addr(0,0);

if ((xDim < 17) && (yDim < 17))				/* can we double the pixels? */
	{
	xHot *= 2;
	yHot *= 2;
	for (y = 0; y < 16; y++)					/* Y run */
		{
		ANDmask = XORmask = XORmask2 = 0;	/* initialize masks */
		for (x = 0; x < 16; x++)				/* X run */
			{
  			ANDmask = ANDmask << 2;				/* shift 2 bits */
  			XORmask = XORmask << 2;
  			XORmask2 = XORmask2 << 2;
			if ((x < xDim) && (y < yDim))		/* if within cursor */
				{
				theByte = *lpCursor++;			/* get next pixel */
				if (theByte == skipColor)
					ANDmask += 3;					/* transparent */
				else
					{
					value = (28  * (int)rgb1[theByte].red +
								150 * (int)rgb1[theByte].green +
								28  * (int)rgb1[theByte].blue);
					if (value > LoRange)
						{
						XORmask += 2;				/* dither for grey */
						XORmask2 += 1;
						}
					if (value > HiRange)
						{
						XORmask += 1;				/* white */
						XORmask2 += 2;
						}
					}
				}
			else
				ANDmask += 3;						/* transparent */
			}

		*lpAND++ = (BYTE)(HIWORD(ANDmask) >> 8);	/* move bytes from dwords */
		*lpAND++ = (BYTE)HIWORD(ANDmask);
		*lpAND++ = (BYTE)(LOWORD(ANDmask) >> 8);
		*lpAND++ = (BYTE)LOWORD(ANDmask);

		*lpAND++ = (BYTE)(HIWORD(ANDmask) >> 8);
		*lpAND++ = (BYTE)HIWORD(ANDmask);
		*lpAND++ = (BYTE)(LOWORD(ANDmask) >> 8);
		*lpAND++ = (BYTE)LOWORD(ANDmask);

		*lpXOR++ = (BYTE)(HIWORD(XORmask) >> 8);
		*lpXOR++ = (BYTE)HIWORD(XORmask);
		*lpXOR++ = (BYTE)(LOWORD(XORmask) >> 8);
		*lpXOR++ = (BYTE)LOWORD(XORmask);

		*lpXOR++ = (BYTE)(HIWORD(XORmask2) >> 8);
		*lpXOR++ = (BYTE)HIWORD(XORmask2);
		*lpXOR++ = (BYTE)(LOWORD(XORmask2) >> 8);
		*lpXOR++ = (BYTE)LOWORD(XORmask2);

		}
	}
else
	{	
	for (y = 0; y < 32; y++)
		{
		ANDmask = XORmask = 0;					/* initialize masks */
		for (x = 0; x < 32; x++)
			{
  			ANDmask = ANDmask << 1;
  			XORmask = XORmask << 1;
			if ((x < xDim) && (y < yDim))
				{
				theByte = *lpCursor++;			/* get next pixel */
				if (theByte == skipColor)
					ANDmask += 1;					/* transparent */
				else
					{
					value = (28  * (int)rgb1[theByte].red +
								150 * (int)rgb1[theByte].green +
								28  * (int)rgb1[theByte].blue);
					if (value > MidRange)
						XORmask += 1;				/* set white (black is 0, 0) */
					}
				}
			else
				ANDmask += 1;						/* transparent */
			}

		*lpAND++ = (BYTE)(HIWORD(ANDmask) >> 8);
		*lpAND++ = (BYTE)HIWORD(ANDmask);
		*lpAND++ = (BYTE)(LOWORD(ANDmask) >> 8);
		*lpAND++ = (BYTE)LOWORD(ANDmask);

		*lpXOR++ = (BYTE)(HIWORD(XORmask) >> 8);
		*lpXOR++ = (BYTE)HIWORD(XORmask);
		*lpXOR++ = (BYTE)(LOWORD(XORmask) >> 8);
		*lpXOR++ = (BYTE)LOWORD(XORmask);
		}
	}

	hMyCur = CreateCursor(MyInstance,xHot,yHot,32,32,
					(const void*)buffAND,(const void*)buffXOR);
	hCurCur = hMyCur;
	SetCursor(hCurCur);
	delete cursor;
	delete imageData;
	UnHide();
}


void
CursorWin::ForcePos(SOL_Point p)
{
	DeviceMovedTo(p);
}


void
CursorWin::DeviceMovedTo(const SOL_Point& p)
{
POINT	myPoint;

	if (!restrictFlag) {
		myPoint.x = p.x;
		myPoint.y = p.y;
	} else {
	  	myPoint.x = Min(Max(p.x,restrict.A.x),restrict.B.x);
	  	myPoint.y = Min(Max(p.y,restrict.A.y),restrict.B.y);
	}

	// scale to client coords
	myPoint.x = (((int)myPoint.x * gameWidth)  / LOWRESX) + gameBorder;
	myPoint.y = (((int)myPoint.y * gameHeight) / LOWRESY) + gameBorder;

	ClientToScreen (hMyWnd, (LPPOINT)&myPoint);
	ShowCursor (0);
	SetCursorPos (myPoint.x, myPoint.y);
	ShowCursor (1);
}


void
CursorWin::Hide()
{
	if (!hideCount)
		while (ShowCursor (0) > -1);
	hideCount++;
}


void
CursorWin::UnHide()
{
	if (hideCount)  {
		hideCount--;

		//  If it became visible, show it
		if (!hideCount)
			ShowCursor (1);
	}
	else
		msgMgr->Fatal(SrcLoc, Msg_CursorUnHide);
}

void
CursorWin::Show()
{
	if (hideCount)  {
		hideCount = 0;
		ShowCursor (1);
	}
}

void
CursorWin::SetRestrictRect(const SOL_Rect& r)
{
SOL_Point	p;
SOL_Rect		screen;

	restrictFlag = True;
	restrict = r;
	screen.Set(0,0,LOWRESX,LOWRESY);
	restrict.Clip(screen);
	mouse->SetRestrictRect(restrict);
	mouse->GlobalPos(&p);
	ForcePos(p);
}


void
CursorWin::ClearRestrictRect()
{
	restrictFlag = False;
	mouse->ClearRestrictRect();
}

#else		//COLORCUR

#include <conio.h>
#include <dos.h>
#include <string.h>

#define INCLUDE_MMSYSTEM_H
#include "sciwin.hpp"
#include "sol.hpp"

#include "celobj.hpp"
#include "cursorw.hpp"
#include "intflag.hpp"
#include "msg.hpp"
#include "ptrdev.hpp"
//#include "video.hpp"
#include "savegame.hpp"
#include "graphmgr.hpp"
#include "mbox.hpp"

//	Windows cursor stuff
#define MAXPALETTE	256
//#define CMAPSIZE		5000

uchar *cursorBuffAddr;
//uchar	winCurBuff[CMAPSIZE];
uchar	*winCurBuff;

struct {
		BITMAPINFOHEADER	bmih;
		WORD	colorix [MAXPALETTE];
		} bic;


CursorWin::CursorWin(const BufferWin& buffer):bitmap(-1,-1,-1),
	cursorData(False),
	cursorBack(False),
	saveVmap(False),
	vmapData(True),
	drawBuff1(False),
	drawBuff2(False)
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
//	Set(-1, -1, -1);

	// set up work buffer pointers
	cursorData.buffer = cursorBuffAddr;
	cursorBack.buffer = cursorBuffAddr + maxCursorSize;
	drawBuff1.buffer =  cursorBuffAddr + 2 * maxCursorSize;
	saveVmap.buffer =   cursorBuffAddr + 3 * maxCursorSize;
	drawBuff2.buffer =  cursorBuffAddr + 4 * maxCursorSize;
	winCurBuff =        cursorBuffAddr + 8 * maxCursorSize;

	// set up DIB header for the Windows cursor buff
	bic.bmih.biSize =				(DWORD)sizeof(BITMAPINFOHEADER);
//	bic.bmih.biWidth =			(DWORD)CMAPWIDTH;
//	bic.bmih.biHeight =			(DWORD)CMAPHEIGHT;
	bic.bmih.biPlanes =			1;
	bic.bmih.biBitCount =	 	8;
	bic.bmih.biCompression = 	0;
	bic.bmih.biSizeImage =		0;
	bic.bmih.biXPelsPerMeter =	0;
	bic.bmih.biYPelsPerMeter =	0;
	bic.bmih.biClrUsed =			MAXPALETTE;
	bic.bmih.biClrImportant =	MAXPALETTE;

	// set index values to identity
	for (int i = 0; i < MAXPALETTE; i++)
		bic.colorix [i] = i;
	bic.colorix[255] = 7;						/* defined as white - this is close */

	ShowCursor(0);

	Set(-1, -1, -1);
}


CursorWin::~CursorWin()
{
	ShowCursor(1);
//	delete cursorBack.buffer;
//	delete cursorData.buffer;
//	delete drawBuff1.buffer;
//	delete drawBuff2.buffer;
//	delete saveVmap.buffer;
}



void
CursorWin::Set(int v, int l, int c)
{
//LPSTR		lpCursor, lpWinBuff;

	//  Get resource, put information away for savegame
	gonnaPaint = True;
	CelObjView* cursor = NULL;
	if (v != -1)
		cursor = New CelObjView(v, l, c);

	//  Stop interrupts, erase cursor if not already hidden
//	PushAndDisableInterrupts();
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

	int xDimMod4 = ((xDim + 3) >> 2) << 2;
	if (xDimMod4 * yDim > maxCursorSize) {
		sprintf(mbuff, "Cursor too large for buffer (%d X %d).\n"
							"Set maxCursorSize option to %d", xDimMod4, yDim, xDimMod4 * yDim);
		MBox(mbuff,"");
		exit(255);
	}

	//  Extract information, init variables
//	delete cursorData.buffer;
//	delete cursorBack.buffer;
//	delete drawBuff1.buffer;
//	delete drawBuff2.buffer;
//	delete saveVmap.buffer;

//	cursorData.buffer = New uchar[xDim * yDim];
//	cursorBack.buffer = New uchar[xDim * yDim];
//	drawBuff1.buffer = New uchar[xDim * yDim];
//	drawBuff2.buffer = New uchar[xDim * yDim * 4];
//	saveVmap.buffer = New uchar[xDim * yDim];

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
//	PopInterrupts();
	gonnaPaint = False;
}




void
CursorWin::SetDevice(PointerDevice& dev)
{
	SOL_Point p;
	posDevice = &dev;
	posDevice->Setup(*this);
	posDevice->GlobalPos(&p);
	DeviceMovedTo(p);
}




void
CursorWin::ForcePos(SOL_Point p)
{
	SetCursorPos(p.x, p.y);
//	posDevice->SetPos(p);
}



void
CursorWin::GonnaPaint(SOL_Rect rect)
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
CursorWin::PaintStarting()
{
	if (putCursorInVMAP)  {
		//  Save Vmap, put cursor in vmap
		saveVmap.rect = cursorData.rect;
		Copy(saveVmap, vmapData);
		Paint(vmapData, cursorData);
	}
}

void
CursorWin::DonePainting()
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
CursorWin::DeviceMovedTo(const SOL_Point& p)
{
	if (!gonnaPaint)  {
//		PushAndDisableInterrupts();
		nextPos.x = p.x;
		nextPos.y = p.y;
		Move();
//		PopInterrupts();
	}
}



/*
 *
 *  Called from interrupts and normal code!
 *
 */

void
CursorWin::Hide()
{
//	PushAndDisableInterrupts();

	if (!hideCount)  {
		if (!cursorBack.rect.IsEmpty())  {
			DrawToHardware(cursorBack);
		}
	}
	hideCount++;

//	PopInterrupts();
}


/*
 *
 *  Called from interrupts and normal code!
 *
 */

void
CursorWin::UnHide()
{
	//  Disable further interrrupts
//	PushAndDisableInterrupts();

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

//	PopInterrupts();
}

/*
 *
 *  Called from interrupts and normal code!
 *
 */

void
CursorWin::Show()
{
	//  Disable further interrrupts
//	PushAndDisableInterrupts();

	if (hideCount)  {
		hideCount = 0;

		//  If it became visible, calc draw area, save bkg, paint
		cursorData.rect.MoveTo(nextPos.x - xhot, nextPos.y - yhot);
		RevealCursor();
	}

//	PopInterrupts();
}



void
CursorWin::RevealCursor()
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
CursorWin::Move()
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
//			Copy(drawBuff2, cursorBack);

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
CursorWin::Paint(const Info& dest, const Info& src)
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

	if (dest.invertedFlag) {
		destPtr = dest.buffer + ((dest.rect.B.y - srcRect.A.y) * dest.rect.Xlen()) + (srcRect.A.x - dest.rect.A.x);
		destNextLine = - dest.rect.Xlen() - xsize;
	}

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
CursorWin::Copy(const Info& dest, const Info& src)
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

	if (src.invertedFlag) {
		syo = src.rect.A.y + src.rect.B.y - srcRect.A.y - dest.rect.Ylen() + srcRect.Ylen();
		srcPtr = src.buffer + (syo * src.rect.Xlen()) + sxo;
  		srcNextLine = -src.rect.Xlen();
	}
	if (dest.invertedFlag) {
		destPtr = dest.buffer + ((dest.rect.B.y - srcRect.A.y) * dest.rect.Xlen()) + (srcRect.A.x - dest.rect.A.x);
		destNextLine = -dest.rect.Xlen();
	}

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

//void
//CursorWin::PushAndDisableInterrupts()
//{
//	if (!disableCount)
//		interruptState = ReadInterruptAndClear();
//	disableCount++;
//}



/*
 *
 *  Called from interrupts!
 *
 */

//void
//CursorWin::PopInterrupts()
//{
//	if (!disableCount) {
//		msgMgr->Fatal(SrcLoc, Msg_StackUnderflow);
//		getch();
//	}
//	disableCount--;
//	if (!disableCount)
//		SetInterruptFlag(interruptState);
//}


void
CursorWin::DrawToHardware(const Info& info)

{
WORD top, left, bottom, right, width, widthMod4, height;

	SOL_Rect rect = info.rect;
	rect.Clip(vmapData.rect);
	int sxo = rect.A.x - info.rect.A.x;
	int syo = rect.A.y - info.rect.A.y;
	uchar* src = info.buffer + (syo * info.rect.Xlen()) + sxo;

	// need both srcXOff and srcYOff because we don't know here
	// what video mdeo we're in and which we'll need
	int srcXOff = info.rect.Xlen() - rect.Xlen();

	left =	rect.A.x;
	top =		rect.A.y;
	right =	rect.B.x;
	bottom =	rect.B.y;
	width =	right - left + 1;
	height =	bottom - top + 1;
	widthMod4 = (((width + 3) >> 2) << 2);

	if (widthMod4 * height > 4 * maxCursorSize) {
		sprintf(mbuff, "xCursor too large for buffer.\n"
							"Set maxCursorSize option to %d", (widthMod4 * height) / 4);
		MBox(mbuff,"");
		exit(255);
	}

	// write it to the windows buffer
	uchar* srcPtr = src;
	int srcNextLine = width;

	uchar* destPtr = winCurBuff + (height - 1) * widthMod4;
	int destNextLine = widthMod4;

	for (int y = 0; y < height; y++) {
		memcpy(destPtr, srcPtr, width);
		destPtr -= destNextLine;
		srcPtr += srcNextLine;
	}

	bic.bmih.biWidth =			(DWORD)widthMod4;
	bic.bmih.biHeight =			(DWORD)height;

	SetDIBitsToDevice (hMyDC,						//device context
							left + gameBorder,		//destination X
							top + gameBorder,			//destination Y
//							0,								//destination X
//							0,								//destination Y
							width,						//X-extent
							height,						//Y-extent
//							CMAPWIDTH,					//X-extent
//							CMAPHEIGHT,					//Y-extent
							0,								//source X
							0,								//source Y
							0,								//first scan-line number
							height,						//number of scan lines
							(LPSTR)winCurBuff, 		//pointer to DIB
							(LPBITMAPINFO)&bic.bmih, //pointer to header
							DIB_PAL_COLORS);			//indexed color table
}


void
CursorWin::ReadVideo(const Info& info)
{
//	if (graphMgr->vmapValid)
		ReadVideoFromVMAP(info);
//	else
//		SciDisplay("not valid");
//		ReadVideoFromHardware(info);
}

//void
//CursorWin::ReadVideoFromHardware(const Info& info)
//{
//	uchar* dest = info.buffer;
//	ReadVideoRect(&info.rect, dest);
//}

void
CursorWin::ReadVideoFromVMAP(const Info& info)
{
	Copy(info, vmapData);
}



void
CursorWin::SetRestrictRect(const SOL_Rect& r)
{
	restrict = r;
	restrict.Clip(vmapData.rect);
	posDevice->SetRestrictRect(restrict);
	ForcePos(nextPos);
}


void
CursorWin::ClearRestrictRect()
{
	restrict = vmapData.rect;
	posDevice->ClearRestrictRect();
}


void
CursorWin::SaveRestore(SaveGame* saveGame, Bool save)
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
CursorWin::SaveRestoreAfter(SaveGame*, Bool save)
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

#endif	//COLORCUR
