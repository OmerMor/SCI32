//***************************************************************************
//                                                                          *
//   Copyright (c)1995,M.D.O.   Programmeur:Mathieu___  Création:21/02/95   *
//                                                                          *
//   Portage :  [X]:PC  [_]:MAC  [_]:CDI  [_]:3DO  [_]:SATURN  [_]:PSX      *
//              [_]:Spécifique                                              *
//                                                                          *
//   Type de source :  [X]:Interne   [_]:Externe   [_]:Grand public         *
//                                                                          *
//   Programme rattaché : [X]:Librairies  [_]:Loader                        *
//                        [_]:Autre : ______________                        *
//                                                                          *
//***************************************************************************


#include "VMDMDO.HPP"
#include "VMD.HPP"

#include <IO.H>
#include <ERRNO.H>
#include <SYS\STAT.H>
#include <STDIO.H>
#include <fcntl.h>
#ifdef WINDOWS
#include "sciwin.hpp"
#endif
#include "sol.hpp"
#include "kernel.hpp"
#include "cursor.hpp"
#include "textid.hpp"
#ifndef WINDOWS
#include "video.hpp"
#else
#include "graphmw.hpp"
#endif
#include "graphmgr.hpp"
#include "pmachine.hpp"
#include "event.hpp"
#include "config.hpp"
#include <dos.h>
#include <stdlib.h>
#include "blob.hpp"

#ifdef WINDOWS
	#include "mousew.hpp"
#endif

#pragma argsused

//***********************************************************************
//                       Various IMD globals
//***********************************************************************

static IMD*	theImd;
static WORD playFlags;
static int 	vmdStatus;
static int 	vmdWidth;
static int 	vmdHeight;
static int 	vmdLeft;
static int 	vmdTop;
static int 	forceStop;
static int 	firstFrame;
static int 	lastFrame;

int 	vmdBlackLines = 0;
int 	vmdDouble = 0;
int 	vmdNoAsync = 0;
int 	vmdSkipOn = 0;
int 	vmdRGB = 100;
#ifdef DEBUG
int 	vmdReport = 0;
static int	framesSkipped;
#endif


extern int blockPaletteUpdate;
extern BOOL COLORCURSOR;
extern	CACHE	*i_Cache;
COK_BITMAP*	theBmp;
static Bool showCursor = False;
static Blob * theBlobs;

unsigned int OpenVMD(char * fileName,int cacheSize = 1024);
unsigned int PutVMD(int left,int top);
unsigned int VMDPlay(int from,int to,int);
unsigned int VMDForceReturn();
unsigned int VMDWaitEvent(int flags,int endFrame);
unsigned int CloseVMD();
unsigned int GetVMDLength();
unsigned int GetVMDPosition();
static void StartBlobs();
static int  AddBlob(int squareSize, int top, int left, int bottom, int right);
static	void DeleteBlob(int blobNo);

#ifdef WINDOWS
#ifndef BITBLT

#define MAXPALETTE	256
_Packed struct {
		BITMAPINFOHEADER	bmih;
		WORD	colorix [MAXPALETTE];
		} biv;

BOOL	firstTime = TRUE;

#endif
#endif


unsigned int 
OpenVMD(char * fileName,int cacheSize)
{
	IMD *Imd;
	COK_BITMAP *Bmp;
	char movieName[_MAX_PATH];

	if(vmdStatus != statusNotOpen)	// can only play 1 at a time
		return SCIVMD_ERROR;

	int movieFound = 0;
#ifdef DEBUG
	framesSkipped = 0;
#endif

	for(int i = 0; i < configMgr->GetNTokens("MovieDir"); i++) {
		char * movieDir = configMgr->Get("MovieDir",i);

		if(movieDir && strlen(movieDir)) {
			char drive[10];
			_splitpath(movieDir,drive,0,0,0);

			// The following findfirst is done to ensure that MSCDEX
			// knows which CD is in the drive
#ifndef WIN32S
			find_t buffer;
			strcat(drive,"\\*.*");
			int result =_dos_findfirst(drive,_A_VOLID,&buffer);
#else
			GetVolumeInformation(drive,NULL,0,NULL,NULL,0,NULL,0);
#endif
			sprintf(movieName,"%s\\%s",movieDir,fileName);
		}
		else
			strcpy(movieName,fileName);
		
		if(!access(movieName,F_OK)) {
			movieFound = TRUE;
			break;	// file was found
		}
	}

	if(!movieFound) {
#ifdef DEBUG
	   msgMgr->Alert(SrcLoc, "VMD: %s Not Found ", movieName);
#endif
		return SCIVMD_ERROR;
	}

	InitCache(1,0,0,0);		// kk changed 10 to 1 (# of vmds?)
	i_LoopingCache= cacheSize;
	i_CountPreload= cacheSize;
	// Imd=OpenIMD(movieName,NULL,VMD_NOWAIT);
	Imd=OpenIMD(movieName,NULL,VMD_NOWAIT+VMD_LOOPCACHE+VMD_PRELOAD);
	if(!Imd) {
		vmdStatus = statusNotOpen;
		return SCIVMD_ERROR;
	}
		
	vmdWidth = Imd->SizeX;	// set static global
	vmdHeight = Imd->SizeY; // set static global
	LoadCache(Imd,0);             // Preload cache in memory
	AllocIMD_ptr(Imd);                            // I specified the speed because it's a VMD without SoundTrack
	Bmp=(COK_BITMAP *) GetMem(sizeof(COK_BITMAP));                        // Create the bitmap in memory
	Bmp->SizeX=Bmp->RSizeX=vmdWidth;
	Bmp->SizeY=vmdHeight;
	Bmp->Video=0x98;                              // Needed!
	Bmp->Start=(UBYTE *) GetMem((unsigned long) vmdWidth * vmdHeight);                  // = 320x200x256c
	Imd->Bmp=Bmp;
	theImd = Imd;
	theBmp = Bmp;
	vmdStatus = statusOpen;

#ifdef WINDOWS
#ifndef BITBLT
	// Set up the DIB header
	biv.bmih.biSize =				(DWORD)sizeof(BITMAPINFOHEADER);
	biv.bmih.biPlanes =			1;
	biv.bmih.biBitCount =	 	8;
	biv.bmih.biCompression = 	0;
	biv.bmih.biSizeImage =		0;
	biv.bmih.biXPelsPerMeter =	0;
	biv.bmih.biYPelsPerMeter =	0;
	biv.bmih.biClrUsed =			MAXPALETTE;
	biv.bmih.biClrImportant =	MAXPALETTE;

	// set index values to identity
	for (i = 0; i < MAXPALETTE; i++)
		biv.colorix [i] = i;
#endif
#endif

	return 0;
}

unsigned int 
CloseVMD()
{
	if(vmdStatus == statusNotOpen)
		return 0;

#ifdef DEBUG
	int totalFrames = theImd->NbPict;
#endif

	CloseIMD(theImd);
	ReleaseMem(theBmp->Start);
	ReleaseMem(theBmp);
	AllocIMD_ptr(NULL);
	CloseCache();
	vmdStatus = statusNotOpen;
	blockPaletteUpdate = 0;

  // Clean up the framebuffer and screen
  int doubleIt = 1;
  if(vmdDouble)
	doubleIt = 2;
		SOL_Rect dirtyRect(vmdLeft,vmdTop,vmdLeft+ vmdWidth * doubleIt - 1,vmdTop+ vmdHeight * doubleIt - 1);
#ifndef WINDOWS
	if (showCursor) {
		graphMgr->FrameOut(True,dirtyRect);
	}
	else {
	  	graphMgr->NextScreen()->ShowList().Add(dirtyRect);
	   	graphMgr->ShowBits();
	}
#else
	graphMgr->NextScreen()->ShowList().Add(dirtyRect);
	graphMgr->ShowBits();
#endif
	if (!showCursor)
		graphMgr->GCursor().UnHide();
#ifdef WINDOWS
	else {
		if (COLORCURSOR)
			((GraphicsMgrWin *)graphMgr)->SCursorClr();
	}
#endif

#ifdef DEBUG
	if(vmdSkipOn && vmdReport && framesSkipped)
		msgMgr->Alert("Frames:%d Skipped:%d",totalFrames,framesSkipped);
#endif

	return 0;	
}

unsigned int 
PutVMD(int left,int top)
{
	vmdLeft = left;
	vmdTop = top;

#ifdef WINDOWS
	if(vmdDouble) {
		// double the vmd
		ReleaseMem(theBmp->Start);
		// = 320x200x256c
		theBmp->Start=(UBYTE *) GetMem((unsigned long) vmdWidth * vmdHeight * 4);
	}
#endif
	return 0;
}

unsigned int 
VMDForceReturn()
{
	forceStop = 1;
	return 0;
}

unsigned int 
VMDWaitEvent(int flags,int endFrame)	// flags
{
	int stopCode = 0;
	LONG ans;
#ifdef WINDOWS
	MSG msg;
#endif
	SOL_Event event;

	// the last frame can also be set by VMDPlay
	// if the endFrame paramenter is -1 (not passed), the lastFrame is 
	// not reset.  If the endFrame is 0, the VMD will play until done

	if(endFrame != -1)
		lastFrame = endFrame;	

	vmdStatus = statusPlaying;

	if (!showCursor && theImd->Pos == 0) 
		graphMgr->GCursor().Hide();
#ifdef WINDOWS
	else {
		if (COLORCURSOR)
			((GraphicsMgrWin *)graphMgr)->SCursorWin();
	}
#endif

	if (ans & VMD_PAL)                // Palette changed !
		AssignColorsPtr(0,255,0,theImd->TempPal);  // Assign the palette

	// Make a copy from Bitmap Bmp to screen, and only for the part who change.
	// The first frame is always black
	MoveBitmap(theBmp,IMD_X1,IMD_Y1,IMD_X2,IMD_Y2,1);

	AsyncEventCheck();


	while(!stopCode) {

		if (!vmdNoAsync)
			AsyncEventCheck();

		if(!forceStop)
  			ans=ViewDelayIMD(theImd);

		if (ans!=VMD_END && ans!=VMD_WAITING && !forceStop) {
			if (ans & VMD_PAL)                // Palette changed !
				AssignColorsPtr(0,255,0,theImd->TempPal);  // Assign the palette

			// Make a copy from Bitmap Bmp to screen, and only for the part who change.
			if(vmdSkipOn && ans & VMD_TIMEOUT) {
#ifdef DEBUG
				++framesSkipped;
#endif
			}
			else
				MoveBitmap(theBmp,IMD_X1,IMD_Y1,IMD_X2,IMD_Y2,0);
		} 

		if(ans == VMD_END) {
			vmdStatus = statusDone;
			if(flags & SCIVMD_END)
				stopCode = SCIVMD_END;
		}

		if(forceStop) {
			stopCode = statusStopped;	// ??
			vmdStatus = statusStopped;
		}

		if(lastFrame && theImd->Pos >= lastFrame) {
			stopCode = SCIVMD_END;
			vmdStatus = statusDone;
		}

#ifdef WINDOWS
		if(flags & SCIVMD_ESC_PRESSED && PeekMessage(&msg,hMyWnd,WM_KEYUP,WM_KEYUP,PM_NOREMOVE)) {
			if(msg.wParam == VK_ESCAPE) {
				stopCode = SCIVMD_ESC_PRESSED;
				vmdStatus = statusPaused;
				break;
			}
		}

		if(flags & SCIVMD_MOUSE_DOWN && PeekMessage(&msg,hMyWnd,WM_LBUTTONDOWN,WM_LBUTTONDOWN,PM_REMOVE)) {
			if (pm.game) {
				SOL_Point mPoint;
				mPoint.x = LOWORD(msg.lParam);
				mPoint.y = HIWORD(msg.lParam);
				if (COLORCURSOR) {
					((MouseWinClr*)mouse)->WinMouseButton(msg.message, mPoint);
				} else
					((MouseWin*)mouse)->WinMouseButton(msg.message, mPoint);
			}

			stopCode = SCIVMD_MOUSE_DOWN;
			vmdStatus = statusPaused;
			break;
		}
#else
		if(flags & SCIVMD_MOUSE_DOWN && eventMgr->IsAvail(NULL, SOL_Event::MouseDown)){
			vmdStatus = statusPaused;
			stopCode = SCIVMD_MOUSE_DOWN;
			break;
		}

		if(flags & SCIVMD_ESC_PRESSED && eventMgr->IsAvail(&event,SOL_Event::KeyUp)) {
			while(eventMgr->Get(&event,SOL_Event::KeyUp)) {
				if(event.message == 27) {	// escape key
					vmdStatus = statusPaused;
					stopCode = SCIVMD_ESC_PRESSED;
					break;
				}
			}
		}
#endif

		if(flags & SCIVMD_HOT_RECTANGLE) {
			if(eventMgr->IsAvail(NULL, 0x400)) {
				vmdStatus = statusPaused;
				stopCode = SCIVMD_HOT_RECTANGLE;
				break;
			}
		}
	}

  return stopCode;	

}

unsigned int 
GetVMDLength()
{
	return theImd->NbPict;
}

unsigned int 
GetVMDPosition()
{
	return theImd->Pos;
}

void
MoveBitmap(COK_BITMAP* Bmp,
			  WORD bitmapX1,
			  WORD bitmapY1,
			  WORD bitmapX2,
			  WORD bitmapY2,
			  int blackLines)
{
	// Bmp is the pointer to the in memory bitmap
	// bitmapX1,bitmapY1,bitmapX2,bitmapY2 is the dirty rectangle of the bitmap

	if (bitmapX1 == -1)
		// A dirty rectangle was not submitted
		return;
#ifdef WINDOWS
#ifdef BITBLT
	WORD		width,height;
	WORD		mapWidth,mapHeight;
	WORD		destX, destY;
	HDC		hMem;
	HBITMAP	hbmp,hOld;

	if(theBlobs)
		theBlobs->Process((char *) Bmp->Start);

	blackLines = blackLines;			//allow compile under Windows
	mapWidth=Bmp->SizeX;
	mapHeight=Bmp->SizeY;
	width=bitmapX2 - bitmapX1;
	height=bitmapY2 - bitmapY1;

	hMem=CreateCompatibleDC(hMyDC);

	hbmp=CreateBitmap(mapWidth,mapHeight,1,8,NULL);
	hOld=SelectObject(hMem,hbmp);
	SetBitmapBits(hbmp,(long)mapWidth*(long)mapHeight,Bmp->Start);

	destX = bitmapX1 + gameBorder + vmdLeft;
	destY = bitmapY1 + gameBorder + vmdTop;
	if(!vmdDouble)
		BitBlt(hMyDC,destX,destY,width,height,hMem,bitmapX1,bitmapY1,SRCCOPY);
	else {
		StretchBlt(hMyDC,destX,destY,vmdWidth << 1,vmdHeight << 1,hMem,
		bitmapX1,bitmapY1,width,height,SRCCOPY);

	}

	SelectObject(hMem,hOld);
	DeleteObject(hbmp);

	DeleteDC(hMem);
#else
	WORD		width,height;
	WORD		mapWidth,mapHeight;
	WORD		destX, destY;

	blackLines = blackLines;			//allow compile under Windows
	mapWidth=Bmp->SizeX;
	mapHeight=Bmp->SizeY;
	width=bitmapX2 - bitmapX1;
	height=bitmapY2 - bitmapY1;

	if (vmdDouble) {
		mapWidth*=2;
		mapHeight*=2;
	}

	destX = bitmapX1 + gameBorder + vmdLeft;
	destY = bitmapY1 + gameBorder + vmdTop;

	biv.bmih.biWidth =		 	(DWORD)mapWidth;
	biv.bmih.biHeight =			(DWORD)mapHeight;

	if(theBlobs) 
		theBlobs->Process((char *) Bmp->Start);

	SetDIBitsToDevice (hMyDC,							//device context
							destX,							//destination X
							destY,							//destination Y
							width + 1,						//X-extent
							height + 1,						//Y-extent
							bitmapX1,						//source X
							mapHeight - bitmapY2 - 1,	//source Y
							0,									//first scan-line number
							mapHeight,						//number of scan lines
							(LPSTR)Bmp->Start,			//pointer to DIB
							(LPBITMAPINFO)&biv.bmih,	//pointer to header
							DIB_PAL_COLORS);				//indexed color table
#endif

#else
	WORD		mapWidth;

	mapWidth=Bmp->SizeX;

	if(theBlobs)
		theBlobs->Process((char *) Bmp->Start);

	if (showCursor)
	{
		// Update the frame buffer and do a frameout
		if (vmdDouble)
			graphMgr->MovieFrameOutDbl((uchar*)	Bmp->Start,
												(int)		mapWidth,
			  									(int)		bitmapX1,
			  									(int)		bitmapY1,
			  									(int)		bitmapX2,
			  									(int)		bitmapY2,
			  									(int)		vmdLeft,
			  									(int)		vmdTop);
		else
			graphMgr->MovieFrameOut((uchar*)	Bmp->Start,
											(int)		mapWidth,
			  								(int)		bitmapX1,
			  								(int)		bitmapY1,
			  								(int)		bitmapX2,
			  								(int)		bitmapY2,
			  								(int)		vmdLeft,
			  								(int)		vmdTop);
	}
	else
	{
		// Update the hardware
		SOL_Rect bitMapRect(bitmapX1,bitmapY1,bitmapX2,bitmapY2);
		SOL_Point screenPosition(vmdLeft,vmdTop);

		if (vmdDouble)	{
			if (vmdBlackLines)
				ShowMovieBlackDbl(&bitMapRect,
						  	  			(uchar*)Bmp->Start,
						  	  			(int)mapWidth,
						  	  			&screenPosition,
							  			blackLines);
			else
				ShowMovieRectDbl(&bitMapRect,
						  	  		  (uchar*)Bmp->Start,
						  	  		  (int)mapWidth,
						  	  		  &screenPosition);
	  }
	  else {
			if (vmdBlackLines)
				ShowMovieBlack(&bitMapRect,
						  	  	(uchar*)Bmp->Start,
						  	  	(int)mapWidth,
						  	  	&screenPosition,
							  	blackLines);
			else
				ShowMovieRect(&bitMapRect,
						  	  	(uchar*)Bmp->Start,
						  	  	(int)mapWidth,
						  	  	&screenPosition);
	  }
	}
#endif
}

unsigned int 
VMDPlay(int from,int to,int)
{
	firstFrame = from;
	/*
	if(firstFrame) {
		theImd->Pos = firstFrame;
    	i_Cache[0].Loop->LastSector=-1;
    	i_Cache[0].Loop->NextSector=-1;
    	i_Cache[0].Loop->UsedSector= 0;
    	memset(i_Cache[0].Loop->Sector,-1,(UWORD)(2 * i_Cache[0].Loop->NbSector * LOOP_BASE));
		i_Cache[0].Offset = theImd->PictInfo[firstFrame].Offset;
	} */

	lastFrame = to;

	return 0;
}

static void 
StartBlobs()
{
	if(vmdStatus == statusNotOpen)	
		return;

	if(theBlobs)
		delete theBlobs;

#ifdef WINDOWS
	if(vmdDouble)
		theBlobs = New Blob(vmdWidth << 1,vmdHeight << 1);
	else
#endif
		theBlobs = New Blob(vmdWidth,vmdHeight);
}

static int
AddBlob(int squareSize, int top, int left, int bottom, int right)
{
#ifdef WINDOWS
	if(vmdDouble) {
		squareSize <<= 1;
		left <<= 1;
		top <<= 1;
		bottom <<= 1;
		right <<= 1;
	}
#endif
	if(squareSize < 2)
		squareSize = 2;
	else if(squareSize > 32)
		squareSize = 32;

#ifdef WINDOWS
	int tmp = top;
	if(vmdDouble) {
		top = vmdHeight << 1 - bottom;
		bottom = vmdHeight << 1 - tmp;
	}
	else {
		top = vmdHeight - bottom;
		bottom = vmdHeight - tmp;
	}
#endif

	return theBlobs->AddRect(squareSize, top,left,bottom,right);
}

static void
DeleteBlob(int blobNo)
{
	theBlobs->DeleteRect(blobNo);
}

void
KPlayVMD(kArgs args)
{

	switch(arg(1)) {
		
		case SCIVMD_OPEN:
			// force a FrameOut to Make sure pending stuff has been done
//			graphMgr->FrameOut(TRUE);	
			if(argCount >= 3)	{
				int cacheSize = arg(3);
				if (cacheSize > 1024)
					cacheSize = 1024;
				if (cacheSize <= 0)
					cacheSize = 16;
				pm.acc = OpenVMD((*(TextID)arg(2)),cacheSize);

			}
			else
				pm.acc = OpenVMD((*(TextID)arg(2)));
			break;

		case SCIVMD_PUT: 
			{
				int left = arg(2);
				int right = arg(3);
				// force to a 2 byte boundry
				left &= 0xFFFFFFFE;
				vmdDouble = False;

				pm.acc = PutVMD(left,right);
				break;
			}

		case SCIVMD_PUT_DOUBLE: 
			{
				int left = arg(2);
				int right = arg(3);
				// force to a 2 byte boundry
				left &= 0xFFFFFFFE;
				vmdDouble = True;

				pm.acc = PutVMD(left,right);
				break;
			}

		case SCIVMD_PLAY:

			switch(argCount) {
				case 1:
					pm.acc = VMDPlay(0,0,0);
					break;
				case 3:
					pm.acc = VMDPlay(arg(2),arg(3),0);
					break;
				case 4:
					pm.acc = VMDPlay(arg(2),arg(3),0);
					break;
				default:
					pm.acc = SCIVMD_ERROR;
					break;
			}

			break;

		case SCIVMD_WAIT_EVENT:
			if(argCount == 2)
				pm.acc = VMDWaitEvent(arg(2),-1);	// not given - 0 doesn't change anything
			else if(argCount == 3)
				pm.acc = VMDWaitEvent(arg(2),arg(3));	// pass the ending frame number
			break;

		case SCIVMD_STOP:
		case SCIVMD_PAUSE:
			pm.acc = VMDForceReturn();
			break;

		case SCIVMD_RESUME:
			pm.acc = VMDWaitEvent(playFlags,0);	// playFlags set on last VMDWaitEvent
			break;

		case SCIVMD_CLOSE:
			pm.acc = CloseVMD();
			break;

		case SCIVMD_GET_LENGTH:
			pm.acc = GetVMDLength();
			break;

		case SCIVMD_GET_POSITION:
			pm.acc = GetVMDPosition();
			break;

		case SCIVMD_CUE:
		case SCIVMD_SEEK:
			pm.acc = SCIVMD_ERROR;
			break;

		case SCIVMD_GET_STATUS:
			pm.acc = vmdStatus;
			break;

		case SCIVMD_FRAMES_SKIPPED:
			pm.acc = 0;
			break;

		case SCIVMD_SETPALETTE:
			blockPaletteUpdate = 1;
			break;

		case SCIVMD_CURSOR:
			showCursor = arg(2);
			break;

		case SCIVMD_START_BLOB:
			StartBlobs();
			pm.acc = 0;
			break;

		case SCIVMD_STOP_BLOBS:
			delete theBlobs;
			theBlobs = NULL;
			pm.acc = 0;
			break;

		case SCIVMD_ADD_BLOB:
			pm.acc = AddBlob(arg(2), arg(3),arg(4),arg(5),arg(6));
			break;

		case SCIVMD_DELETE_BLOB:
			DeleteBlob(arg(2));
			pm.acc = 0;
			break;
			

	}
}
