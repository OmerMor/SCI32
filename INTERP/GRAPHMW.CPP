//#include	<stdio.h>

#define INCLUDE_MMSYSTEM_H
#include "sciwin.hpp"
#include "sol.hpp"
#ifdef COLORCUR
#include "bufferw.hpp"
#endif
#ifdef WIN32S
#include "shiftpal.hpp"
#include "wingfunc.hpp"
#endif
#include "graphmw.hpp"
#include "msg.hpp"
#include "time.hpp"
#include "pmachine.hpp"
#include "kernel.hpp"
#include "mbox.hpp"

#define MAXPALETTE	256
_Packed struct {
		BITMAPINFOHEADER	bmih;
		WORD	colorix [MAXPALETTE];
		} bi;

static	HDC		hdcMem;

#ifdef WIN32S
// WinG stuff

extern	Bool	paletteUpdated;

extern	_Packed struct {
		WORD					palVersion;
		WORD					palNumEntries;
		union {
			PALETTEENTRY		palPalEntry[256];
			DWORD					dwEntry[256];
			};
		} lPal;
#endif

GraphicsMgrWin::GraphicsMgrWin(int xdim, int ydim) : GraphicsMgr(xdim, ydim),
	last(xdim, ydim), buffer(vmapWidth, vmapHeight, NULL), pmap(xdim, ydim, NULL)
{
#ifdef WIN32S
	if (!WING) {
#endif
		vmapID.Get(MemBitmap, vmapWidth * vmapHeight + 1000, VmapHandle);
		buffer.start = (uchar*) *vmapID;
	   buffer.Clear(SOL_BLACK);

		if (TRUECOLOR)
			palmgr = New PaletteMgrTWin();
		else
			palmgr = New PaletteMgrIWin();

#ifndef COLORCUR
		cursor = New CursorWin();
#else
		cursor = New CursorWin(buffer);
#endif

		// Set up the DIB header
		bi.bmih.biSize =				(DWORD)sizeof(BITMAPINFOHEADER);
		bi.bmih.biWidth =				(DWORD)vmapWidth;
		bi.bmih.biHeight =			(DWORD)vmapHeight;
		bi.bmih.biPlanes =			1;
		bi.bmih.biBitCount =			8;
		bi.bmih.biCompression =		0;
		bi.bmih.biSizeImage =		0;
		bi.bmih.biXPelsPerMeter =	0;
		bi.bmih.biYPelsPerMeter =	0;
		bi.bmih.biClrUsed =			MAXPALETTE;
		bi.bmih.biClrImportant =	MAXPALETTE;

		// set index values to identity
		for (int i = 0; i < MAXPALETTE; i++)
			bi.colorix [i] = i;

		hdcMem = CreateCompatibleDC (hMyDC);

#ifdef WIN32S
	} else {
		buffer.start = (uchar*)pBits;
	   buffer.Clear(SOL_BLACK);

		if (TRUECOLOR)
			palmgr = New PaletteMgrTWin();
		else
			palmgr = New PaletteMgrIWin();

		cursor = New CursorWin();
	}
#endif
}


GraphicsMgrWin::~GraphicsMgrWin()
{
#ifndef WIN32S
	vmapID.Free();
	delete cursor;
	DeleteDC(hdcMem);
#else
	if (!WING) {
		vmapID.Free();
		delete cursor;
		DeleteDC(hdcMem);
	} else {
		delete cursor;
		WingTerm();
	}
#endif
}

void
GraphicsMgrWin::AlterVMAP(SOLPalette* myPal,int palIndex, char* palArray)
{
	int test	= palIndex;

	Rgb24 oldColor,newColor;
	uchar pixels[256];
	int i,j,index;
	long	oldVal, newVal;

	for(i=0;i<256;i++) {
		pixels[i] = i;
		oldColor = (*myPal)[i];

		if (palArray[i] == palIndex) {
			oldVal = 65535 * 4; // get a number larger than we generate

			for(j=0;j<236;j++) {
				if ((palArray[j] != palIndex) && (palArray[j] != 0)) {
					newColor = (*myPal)[j];
					newVal   = (oldColor.red-newColor.red)*(oldColor.red-newColor.red) + 
								  (oldColor.blue-newColor.blue)*(oldColor.blue-newColor.blue) +
								  (oldColor.green-newColor.green)*(oldColor.green-newColor.green);
								  // sum of squares difference
					if (newVal < oldVal)	{
						oldVal = newVal;
						index  = j;
					}
				}			
			}
			pixels[i] = index;		
		}
	}

	for(i=0;i<next.Xdim()*next.Ydim();i++)	{
#ifdef WIN32S
		if (WING) {
		if (palArray[buffer.start[i]-10] == palIndex) {	 // found odd pixel
							buffer.start[i] = (uchar) pixels[buffer.start[i] - 10] + 10;	
		} 
		}	else
#endif
		if (palArray[buffer.start[i]] == palIndex) 	 // found odd pixel
				buffer.start[i] = (uchar) pixels[buffer.start[i]];	
		

	}

}

void
GraphicsMgrWin::ShowBits()
{
int	i;

	int showListSize = next.ShowList().Length();
	if (!showListSize)
		return;

#ifdef COLORCUR
	for (i = 0;i < showListSize; i++)  {
		SOL_Rect rect = next.ShowList()[i];
		rect.A.x &= 0x7FFE;
		rect.B.x |= 1;
		GCursor().GonnaPaint(rect);
	}
	GCursor().PaintStarting();
	for (i = 0; i < showListSize; i++)  {
		SOL_Rect rect = next.ShowList()[i];
		rect.A.x &= 0x7FFE;
		rect.B.x |= 1;
		ShowRect(&rect, buffer.start);
	}
	GCursor().DonePainting();
	next.ShowList().Clear();
#else
	for (i = 0; i < showListSize; i++)
		ShowRect(&next.ShowList()[i], buffer.start);
	next.ShowList().Clear();
#endif
}


void
GraphicsMgrWin::ShowRect(const SOL_Rect * dims, uchar * Buffer)
{
WORD top, left, bottom, right;

	left = dims->A.x;
	top = dims->A.y;
	right = dims->B.x;
	bottom = dims->B.y;

	// adjust left to DWORD boundary
	while (left & 3)
		left--;
	// adjust right+1 to DWORD boundary
//	while ((right + 1) & 3)
//		right++;


#ifdef WIN32S
  	if (!WING) {
#endif

		SetDIBitsToDevice (hMyDC,						//device context
								left + gameBorder,		//destination X
								top + gameBorder,			//destination Y
								right - left + 1,			//X-extent
								bottom - top + 1,			//Y-extent
								left,							//source X
								vmapHeight - bottom - 1,	//source Y
								0,								//first scan-line number
								vmapHeight,					//number of scan lines
								(LPSTR)Buffer,				//pointer to DIB
								(LPBITMAPINFO)&bi.bmih,	//pointer to header
								DIB_PAL_COLORS);			//indexed color table
#ifdef WIN32S
	} else {

		if (paletteUpdated) {
			for(int i = 0; i < 256; i++) {
				winGbi.ColorTable[i].rgbRed = lPal.palPalEntry[i].peRed;
				winGbi.ColorTable[i].rgbGreen = lPal.palPalEntry[i].peGreen;
				winGbi.ColorTable[i].rgbBlue = lPal.palPalEntry[i].peBlue;
			}
			if (!(*fpWinGSetDIBColorTable)(hWinGDC, 0, 256, winGbi.ColorTable))
				MBox("color table update fail","");
			paletteUpdated = False;
		}

		(*fpWinGBitBlt)(
			hMyDC,							//destination DC
			left + gameBorder,			//destination X
			top + gameBorder,				//destination Y
			right - left + 1,				//width
			bottom - top + 1,				//height
			hWinGDC,							//source DC
			left,								//source X
			top);							 	//source Y
	}
#endif
}


void
GraphicsMgrWin::PlayMovie(char* fileName, int tickCount,int , int )
{
	char* x = fileName;
	int y = tickCount;
//	FrameOut();
//	SetVideoForMovies();
//	RunMovie(fileName,tickCount);
	//  do movie stuff
}	


void
GraphicsMgrWin::SetVideoForMovies()
{
//	if (!modeX)  {
//		SetVideoMode(1);
//		modeX = True;
//	}
}


void
GraphicsMgrWin::SetVideoForSpeed()
{
//	if (modeX)  {
//		SetVideoMode(0);
//		modeX = False;
		//////   Change this to frameout later
//		ShowRect(&SOL_Rect(0, 0, 319, 199), buffer.start);
//	}
}


void
GraphicsMgrWin::TimingTest()
{
long time1, time2;
POINT	myPoint;

	pixelsPerCall = 0;
	time1 = 0;
	time2 = 0;
	const TIMERTRYS = 100;
	if (!forcePPC) {
		if (timeMgr)  {
			myPoint.x = 200;
			myPoint.y = 200;
			ClientToScreen (hMyWnd, (LPPOINT)&myPoint);
			SetCursorPos (myPoint.x, myPoint.y);
			SOL_Rect rect(0, 0, 100, 50);
			timingFlag = True;
			ulong timeStart = timeGetTime();
			int offx = 10;
			for (int i = 0;i < TIMERTRYS;i++) {
				ShowRect(&rect, buffer.start);
				rect.Offset(offx, 0);
				offx *= -1;
			}
			time1 = timeGetTime() - timeStart;
			long area1 = rect.Area();


			rect.Set(0, 0, 100, 100);
			offx = 10;
			timeStart = timeGetTime();
			for (i = 0; i < TIMERTRYS; i++) {
				ShowRect(&rect, buffer.start);
				rect.Offset(offx, 0);
				offx *= -1;
			}
			time2 = timeGetTime() - timeStart;
			long area2 = rect.Area();

			if (time2 > time1)  {
				pixelsPerCall = (((area2 - area1) * time1) / (time2 - time1)) - area1;
				if (pixelsPerCall < 0)
					pixelsPerCall = 0;
			}
			else
				pixelsPerCall = 0;
			myPoint.x = 0;
			myPoint.y = 0;
			ClientToScreen (hMyWnd, (LPPOINT)&myPoint);
			SetCursorPos (myPoint.x, myPoint.y);
		}
	} else {
		pixelsPerCall = forcePPC;
	}

	if (SHOWMEM) {
		sprintf(mbuff,"ppc:%d t1:%d t2:%d", pixelsPerCall, time1, time2);
		MBox(mbuff,"");
	}
}

void
GraphicsMgrWin::Repaint(const SOL_Rect& theRect)
{
	// Add this rectangle to the showList
	next.ShowList().Add(theRect);
	ShowBits();
}

void
GraphicsMgrWin::ResizeVmap(int x, int y)
{
// do-nothing for now - map is 640 x 480
	int ax = x;
	int ay = y;
}


void
KIsHiRes(kArgs)
{
	pm.acc = HIRES;
}


