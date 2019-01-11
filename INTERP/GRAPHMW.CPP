//#include	<stdio.h>

#define INCLUDE_MMSYSTEM_H
#include "sciwin.hpp"
#include "sol.hpp"
#include "bufferw.hpp"
#ifdef WIN32S
#include "shiftpal.hpp"
#include "wingfunc.hpp"
#endif
#include "graphmw.hpp"
#include "mousew.hpp"
#include "msg.hpp"
#include "time.hpp"
#include "pmachine.hpp"
#include "kernel.hpp"
#include "mbox.hpp"

#define MAXPALETTE	256

BMI	bmiHdr;
uint	colorUse;
DWORD	widthMod4;

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

// used to adjust vmap width to mod 4 
#define	M4(c)	((((c) + 3) >> 2) << 2)

GraphicsMgrWin::GraphicsMgrWin(int xdim, int ydim) : GraphicsMgr(xdim, ydim),
	last(xdim, ydim), buffer(M4(vmapWidth), vmapHeight, NULL), pmap(xdim, ydim, NULL)
{
#ifdef WIN32S
	if (!WING) {
#endif
		vmapID.Get(MemBitmap, M4(vmapWidth) * vmapHeight + 1000, VmapHandle);
		buffer.start = (uchar*) *vmapID;
	   buffer.Clear(SOL_BLACK);

		if (TRUECOLOR)
			palmgr = New PaletteMgrTWin();
		else
			palmgr = New PaletteMgrIWin();

		clrCursor = (CursorWin *)New CursorWinClr(buffer);
		winCursor = New CursorWin();

		if (COLORCURSOR)
			cursor = clrCursor;
		else
			cursor = winCursor;

		// Set up the DIB header
		bmiHdr.bmih.biSize =				(DWORD)sizeof(BITMAPINFOHEADER);
		bmiHdr.bmih.biPlanes =			1;
		bmiHdr.bmih.biBitCount =	 	8;
		bmiHdr.bmih.biCompression = 	0;
		bmiHdr.bmih.biSizeImage =		0;
		bmiHdr.bmih.biXPelsPerMeter =	0;
		bmiHdr.bmih.biYPelsPerMeter =	0;
		bmiHdr.bmih.biClrUsed =			MAXPALETTE;
		bmiHdr.bmih.biClrImportant =	MAXPALETTE;

		widthMod4 =			(DWORD)M4(vmapWidth);

		if (TRUECOLOR) {
			colorUse = DIB_RGB_COLORS;
		} else {
			colorUse = DIB_PAL_COLORS;
			// set index values to identity
			for (int i = 0; i < MAXPALETTE; i++)
				bmiHdr.colorix [i] = i;
		}

#ifdef WIN32S
	} else {
		buffer.start = (uchar*)pBits;
	   buffer.Clear(SOL_BLACK);

		if (TRUECOLOR)
			palmgr = New PaletteMgrTWin();
		else
			palmgr = New PaletteMgrIWin();


		clrCursor = (CursorWin *)New CursorWinClr(buffer);
		winCursor = New CursorWin();

		if (COLORCURSOR)
			cursor = clrCursor;
		else
			cursor = winCursor;
	}
#endif
}


GraphicsMgrWin::~GraphicsMgrWin()
{
#ifndef WIN32S
	vmapID.Free();
#else
	if (!WING) {
		vmapID.Free();
	} else {
		WingTerm();
	}
#endif
	delete clrCursor;
	delete winCursor;
	delete palmgr;
}

void
GraphicsMgrWin::AlterVMAP(const SOLPalette* myPal, const SOLPalette* newPal,int palIndex, char* palArray)
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

		if ((palIndex == 1) && (palArray[i] == 0)) {
			oldColor = (*myPal)[i];
			oldVal = 65535 * 4; // get a number larger than we generate

			for(j=0;j<236;j++) {
					newColor = (*newPal)[j];
					newVal   = (oldColor.red-newColor.red)*(oldColor.red-newColor.red) + 
								  (oldColor.blue-newColor.blue)*(oldColor.blue-newColor.blue) +
								  (oldColor.green-newColor.green)*(oldColor.green-newColor.green);
								  // sum of squares difference
					if (newVal < oldVal)	{
						oldVal = newVal;
						index  = j;
					}
		
			}
			pixels[i] = index;		
		}
	}

	for(i=0;i<next.Xdim()*next.Ydim();i++)	{
#ifdef WIN32S
		if (WING) {
		if ((palArray[buffer.start[i]-10] == palIndex)&&(palIndex==-1)) 	 // found odd pixel
				buffer.start[i] = (uchar) pixels[buffer.start[i] - 10] + 10;	
		if (((palArray[buffer.start[i]-10] == palIndex)&&(palIndex==1)) ||	 // found odd pixel
			((palArray[buffer.start[i]-10] == 0)&&(palIndex==1)))				
				buffer.start[i] = (uchar) pixels[buffer.start[i] - 10] + 10;	
				 
		}	else {
#endif

		if ((palArray[buffer.start[i]] == palIndex)&&(palIndex==-1)) 	 // found odd pixel
				buffer.start[i] = (uchar) pixels[buffer.start[i]];	

		if (((palArray[buffer.start[i]] == palIndex)&&(palIndex==1))||
			  ((palArray[buffer.start[i]] == 0)&&(palIndex==1)))	 	 // found odd pixel
				buffer.start[i] = (uchar) pixels[buffer.start[i]];	

#ifdef WIN32S
		}
#endif
	}

}

void
GraphicsMgrWin::ShowBits()
{
int	i;

	int showListSize = next.ShowList().Length();
	if (!showListSize)
		return;

	if (cursor == clrCursor) {
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
	} else {
		for (i = 0; i < showListSize; i++)
			ShowRect(&next.ShowList()[i], buffer.start);
		next.ShowList().Clear();
	}
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
		bmiHdr.bmih.biWidth =			(DWORD)widthMod4;
		bmiHdr.bmih.biHeight =			(DWORD)vmapHeight;

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
								(LPBITMAPINFO)&bmiHdr.bmih,	//pointer to header
								colorUse);					//indexed color table or RGB
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
	pm.acc = (graphMgr->Xdim() >= 640  &&  graphMgr->Ydim() >= 400);
}

void
GraphicsMgrWin::SCursorClr()
{
int	v, l, c;

	if (cursor != clrCursor) {
		v = GCursor().Get().View();
		l = GCursor().Get().Loop();
		c = GCursor().Get().Cel();
	 	ShowCursor (0);
	 	hCurCur = hNullCur;
	 	SetCursor(hCurCur);
	 	ShowCursor (1);
		mouse = clrMouse;
		cursor = clrCursor;
		GCursor().Set(v, l, c);
		GCursor().Show();
	}
}

void
GraphicsMgrWin::SCursorWin()
{
int	v, l, c;

	if (cursor != winCursor) {
		v = GCursor().Get().View();
		l = GCursor().Get().Loop();
		c = GCursor().Get().Cel();
		GCursor().Hide();
		mouse = winMouse;
		cursor = winCursor;
		GCursor().Set(v, l, c);
//		ShowCursor (0);
//	 	hCurCur = hMyCur;
//	 	SetCursor(hCurCur);
//		ShowCursor (1);
	}
}

void
PutMovieRect(uchar* screen,uchar* bitmap,int bitmapWidth,SOL_Rect* rect,SOL_Point* screenPos)
{
	// Copy vmd movie bitmap into framebuffer

	// The screen is the address of the framebuffer
	// The bitmap is the address of the bitmap
	// The bitmapWidth is the width of the bitmap
	// The rect is the bitmap relative rectangle to blit
	// The screenPos is the position of the bitmap within the framebuffer	

	int hRun = rect->Xlen();
	int vRun = rect->Ylen();
	int screenOffset = vmapWidth - hRun;
	int bitmapOffset = bitmapWidth - hRun;

	// Calculate the screen start
	screen += (screenPos->y + rect->A.y)*vmapWidth + screenPos->x + rect->A.y;
	// Calculate the bitmap start
	bitmap += rect->A.y*bitmapWidth + rect->A.x;

	while (vRun--) {
		memcpy(screen,bitmap,hRun);
		screen += screenOffset;
		bitmap += bitmapOffset;
	}
}
void
PutMovieRectBlack(uchar* screen,uchar* bitmap,int bitmapWidth,SOL_Rect* rect,SOL_Point* screenPos)
{
	// Copy vmd movie bitmap into framebuffer

	// The screen is the address of the framebuffer
	// The bitmap is the address of the bitmap
	// The bitmapWidth is the width of the bitmap
	// The rect is the bitmap relative rectangle to blit
	// The screenPos is the position of the bitmap within the framebuffer	

	// Only do the even numbered lines
	rect->A.y = rect->A.y & -2;

	int hRun = rect->Xlen();
	int vRun = rect->Ylen() >> 1;
	int screenOffset = (vmapWidth << 1) - hRun;
	int bitmapOffset = (bitmapWidth << 1) - hRun;

	// Calculate the screen start
	screen += (screenPos->y + rect->A.y)*vmapWidth + screenPos->x + rect->A.y;
	// Calculate the bitmap start
	bitmap += rect->A.y*bitmapWidth + rect->A.x;

	while (vRun--) {
		memcpy(screen,bitmap,hRun);
		screen += screenOffset;
		bitmap += bitmapOffset;
	}
}
void
PutMovieRectDbl(uchar* screen,uchar* bitmap,int bitmapWidth,SOL_Rect* rect,SOL_Point* screenPos)
{
	// Copy vmd movie bitmap into framebuffer

	// The screen is the address of the framebuffer
	// The bitmap is the address of the bitmap
	// The bitmapWidth is the width of the bitmap
	// The rect is the bitmap relative rectangle to blit
	// The screenPos is the position of the bitmap within the framebuffer	

// HOOK
// The following code will not work
	int hRun = rect->Xlen();
	int vRun = rect->Ylen();
	int screenOffset = vmapWidth - hRun;
	int bitmapOffset = bitmapWidth - hRun;

	// Calculate the screen start
	screen += (screenPos->y + rect->A.y)*vmapWidth + screenPos->x + rect->A.y;
	// Calculate the bitmap start
	bitmap += rect->A.y*bitmapWidth + rect->A.x;

	while (vRun--) {
		memcpy(screen,bitmap,hRun);
		screen += screenOffset;
		bitmap += bitmapOffset;
	}
}
void
PutMovieRectDblBlack(uchar* screen,uchar* bitmap,int bitmapWidth,SOL_Rect* rect,SOL_Point* screenPos)
{
	// Copy vmd movie bitmap into framebuffer

	// The screen is the address of the framebuffer
	// The bitmap is the address of the bitmap
	// The bitmapWidth is the width of the bitmap
	// The rect is the bitmap relative rectangle to blit
	// The screenPos is the position of the bitmap within the framebuffer	

// HOOK
// The following code will not work
	// Only do the even numbered lines
	rect->A.y = rect->A.y & -2;

	int hRun = rect->Xlen();
	int vRun = rect->Ylen() >> 1;
	int screenOffset = (vmapWidth << 1) - hRun;
	int bitmapOffset = (bitmapWidth << 1) - hRun;

	// Calculate the screen start
	screen += (screenPos->y + rect->A.y)*vmapWidth + screenPos->x + rect->A.y;
	// Calculate the bitmap start
	bitmap += rect->A.y*bitmapWidth + rect->A.x;

	while (vRun--) {
		memcpy(screen,bitmap,hRun);
		screen += screenOffset;
		bitmap += bitmapOffset;
	}
}
