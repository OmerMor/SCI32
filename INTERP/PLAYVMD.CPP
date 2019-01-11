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
#include "audio.hpp"
extern	void		AudioRate(int); // temporary (until moved into audio.hpp)
extern	void		AudioBits(int); // temporary (until moved into audio.hpp)
#include "cursor.hpp"
#include "event.hpp"
#include "textid.hpp"
#ifdef WINDOWS
	#include "graphmw.hpp"
#else
	#include "video.hpp"
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

static IMD*		theImd;
static WORD 	playFlags;
static int 		vmdWidth;
static int 		vmdHeight;
static int 		vmdLeft;
static int 		vmdTop;
// The area to be blacked on a movie
static int 		vmdBlack;
static int 		vmdBackgroundLeft;
static int 		vmdBackgroundTop;
static int 		vmdBackgroundRight;
static int 		vmdBackgroundBottom;

static int 		vmdForceStop;
static int 		vmdInterval = 0;
static int 		vmdLastFrame = 0;

int 				vmdBlackLines = False;
int 				vmdDouble = False;
int 				vmdNoAsync = False;
int 				vmdSkipOn = True;
int 				vmdSkipFixed = False;
int 				vmdRGB = 100;
int 				vmdRGBStart = 0;
int 				vmdRGBEnd = 255;
int 				vmdPaletteStart = 0;
int 				vmdPaletteEnd = 255;


int 				vmdLeaveScreenBlack = False;
int 				vmdLeaveLastFrame = False;

int 				vmdStatus;
Plane*			vmdPlane = 0;
int 				vmdReport = 0;
static int		vmdFramesSkipped;
int 				vmdBehind = 200;
int 				vmdAhead = 200;


extern int 		blockPaletteUpdate;
extern BOOL 	COLORCURSOR;
extern CACHE*	i_Cache;
COK_BITMAP*		theBmp;
static Bool 	showCursor = False;
static Blob* 	theBlobs;

static int  	AddBlob(int squareSize, int top, int left, int bottom, int right);
unsigned int 	CloseVMD();
static void 	DeleteBlob(int blobNo);
unsigned int 	GetVMDLength();
unsigned int 	GetVMDPosition();
unsigned int 	OpenVMD(char * fileName,int cacheSize = 1024);
static void 	StartBlobs();
unsigned int 	VMDForceReturn();
unsigned int 	VMDWaitEvent(int flags);

#ifdef WINDOWS
BOOL	vmdShowFullFrame = FALSE;
BOOL	vmdKillFocus = FALSE;
#endif

unsigned int 
OpenVMD(char * fileName,int cacheSize)
{
	IMD*			Imd;
	COK_BITMAP*	Bmp;
	char 			movieName[_MAX_PATH];

	AudioStop(ALL_SAMPLES);

	// get rid of any path or ext.
	_splitpath(fileName,NULL,NULL,movieName,NULL);

	if(vmdStatus != statusNotOpen)	
		// can only play 1 at a time
		return SCIVMD_ERROR;

	int movieFound = 0;
	vmdFramesSkipped = 0;
	vmdInterval = 0;


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

	return 0;
}

unsigned int 
CloseVMD()
{
	if(vmdStatus == statusNotOpen)
		return 0;

	int totalFrames = theImd->NbPict;

	if(!vmdLeaveLastFrame) {
		SOL_Rect dirtyRect(vmdLeft,
							 	 vmdTop,
							 	 vmdLeft + vmdWidth - 1,
							 	 vmdTop + vmdHeight - 1);

		graphMgr->FrameOut(True,dirtyRect);
		graphMgr->DeletePlane(vmdPlane);
		if (!vmdLeaveScreenBlack)
			graphMgr->FrameOut();
	}
	if(!showCursor && vmdLeaveLastFrame) {
		// Copy the bitmap to the frame buffer
		graphMgr->MovieFrameOut((uchar*)	theBmp->Start,
										(int)		theBmp->SizeX,
		  								(int)		0,
		  								(int)		0,
		  								(int)		theBmp->SizeX - 1,
		  								(int)		theBmp->SizeY - 1,
		  								(int)		vmdLeft,
		  								(int)		vmdTop,
													vmdDouble,
													vmdBlackLines);
		graphMgr->DeletePlane(vmdPlane);
	}

	if (!showCursor)
		graphMgr->GCursor().UnHide();
	#ifdef WINDOWS
		else {
			if (COLORCURSOR)
				((GraphicsMgrWin *)graphMgr)->SCursorClr();
		}
	#endif

	CloseIMD(theImd);
	ReleaseMem(theBmp->Start);
	ReleaseMem(theBmp);
	AllocIMD_ptr(NULL);
	CloseCache();
	vmdStatus = statusNotOpen;
	blockPaletteUpdate = 0;

	ReleaseVMDMem();
	AudioRate(22050);  // set the DAC back to its default rate
	AudioBits(16);  // set the DAC back to its default bit setting

	if(vmdReport && vmdSkipOn)
		msgMgr->Alert("Frames:%d Skipped:%d",totalFrames,vmdFramesSkipped/2);

	return 0;	
}

unsigned int 
VMDForceReturn()
{
	vmdForceStop = 1;
	return 0;
}

unsigned int 
VMDWaitEvent(int flags)
{
	int stopCode = 0;
	LONG ans;
	#ifdef WINDOWS
		MSG msg;
	#endif
	SOL_Event event;

	// If the lastFrame is 0, the VMD will play until done.

	vmdStatus = statusPlaying;

	if (theImd->Pos == 0) {
		if (!showCursor) 
			graphMgr->GCursor().Hide();
		#ifdef WINDOWS
			else {
				if (COLORCURSOR)
					((GraphicsMgrWin *)graphMgr)->SCursorWin();
			}
		#endif

		// Set the blackout area defaults
		if (!vmdBlack) {
			vmdBackgroundLeft = vmdLeft;
			vmdBackgroundTop = vmdTop;
			vmdBackgroundRight = vmdLeft + vmdWidth - 1;
			vmdBackgroundBottom = vmdTop + vmdHeight - 1;
			if (vmdDouble)	{
				vmdBackgroundRight += vmdWidth;
				vmdBackgroundBottom += vmdHeight;
			}
			// Scale dimensions down to low resolution
			Ratio scaleX(SCIRESX,graphMgr->Xdim());
			Ratio scaleY(SCIRESY,graphMgr->Ydim());
			vmdBackgroundLeft = vmdBackgroundLeft * scaleX;
			vmdBackgroundTop = vmdBackgroundTop * scaleY;
			vmdBackgroundRight = ((vmdBackgroundRight + 1) * scaleX) - 1;
			vmdBackgroundBottom = ((vmdBackgroundBottom + 1) * scaleY) - 1;
		}

		// The first frame is always black
		SOL_Rect dirtyRect(vmdBackgroundLeft,
							  	 vmdBackgroundTop,
							  	 vmdBackgroundRight,
							  	 vmdBackgroundBottom);
		vmdPlane = New Plane(dirtyRect);
		graphMgr->AddPlane(vmdPlane);
		graphMgr->FrameOut();
	}

	AsyncEventCheck();

	while(!stopCode) {

		AsyncEventCheck();

		if (!vmdForceStop)
  			ans=ViewDelayIMD(theImd);

		if (ans!=VMD_END && ans!=VMD_WAITING && !vmdForceStop) {
			if (ans & VMD_PAL)                // Palette changed !
				AssignColorsPtr(0,255,0,theImd->TempPal);  // Assign the palette

			// Make a copy from Bitmap Bmp to screen
			// and only for the part that changed.
			if(vmdSkipOn && (ans & VMD_BEHIND)){
				++vmdFramesSkipped;
				if (vmdFramesSkipped & 1)
					// Skip every other skip
					MoveBitmap(theBmp,IMD_X1,IMD_Y1,IMD_X2,IMD_Y2);
			}
			else
				if (!(ans & VMD_TIMEOUT))
					MoveBitmap(theBmp,IMD_X1,IMD_Y1,IMD_X2,IMD_Y2);
		} 

		if (ans == VMD_END) {
			vmdStatus = statusDone;
			if(flags & SCIVMD_END)
				stopCode = SCIVMD_END;
		}

		if (vmdForceStop) {
			stopCode = statusStopped;
			vmdStatus = statusStopped;
		}

		if (vmdLastFrame && theImd->Pos >= vmdLastFrame) {
			stopCode = SCIVMD_END;
			vmdStatus = statusDone;
		}

		if (vmdInterval && theImd->Pos && ((theImd->Pos % vmdInterval) == 0)) {
			stopCode = SCIVMD_RETURN_INTERVAL;
			vmdStatus = statusPaused;
		}

		if (!vmdNoAsync)
		{

			#ifdef WINDOWS
				if(flags & SCIVMD_ESC_PRESSED && eventMgr->IsAvail(&event,SOL_Event::KeyDown)) {
					while(eventMgr->Get(&event,SOL_Event::KeyDown)) {
						if(event.message == 27) {	// escape key
							vmdStatus = statusPaused;
							stopCode = SCIVMD_ESC_PRESSED;
							break;
						}
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

				if (vmdKillFocus) {
					vmdKillFocus = FALSE;
					vmdStatus = statusPaused;
					stopCode = SCIVMD_KILLFOCUS;
					break;
				}

			#else
				if(flags & SCIVMD_MOUSE_DOWN && eventMgr->IsAvail(NULL, SOL_Event::MouseDown)) {
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
			  WORD bitmapY2)
{
	// Bmp is the pointer to the in memory bitmap
	// bitmapX1,bitmapY1,bitmapX2,bitmapY2 is the dirty rectangle of the bitmap

	if (bitmapX1 == -1)
		// A dirty rectangle was not submitted
		return;
#ifdef WINDOWS
		WORD		width,height;
		WORD		mapWidth,mapHeight;
		WORD		destX, destY;

		mapWidth=Bmp->SizeX;
		mapHeight=Bmp->SizeY;

		if (vmdDouble) {
			mapWidth*=2;
			mapHeight*=2;
		}

		// Windows says to repaint the whole frame
		if (vmdShowFullFrame) {
			vmdShowFullFrame = FALSE;
			bitmapX1 = 0;
			bitmapY1 = 0;
			bitmapX2 = mapWidth + 1;
			bitmapY2 = mapHeight + 1;
		}

		width=bitmapX2 - bitmapX1;
		height=bitmapY2 - bitmapY1;

		destX = bitmapX1 + gameBorder + vmdLeft;
		destY = bitmapY1 + gameBorder + vmdTop;

		bmiHdr.bmih.biWidth = 	 	(DWORD)mapWidth;
		bmiHdr.bmih.biHeight = 		(DWORD)mapHeight;

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
								(LPBITMAPINFO)&bmiHdr.bmih,//pointer to header
								colorUse);						//color table
#else
		WORD		mapWidth;

		mapWidth=Bmp->SizeX;

		if(theBlobs)
			theBlobs->Process((char *) Bmp->Start);

		if (showCursor) {
			// Update the frame buffer and do a ShowBits
				graphMgr->MovieFrameOut((uchar*)	Bmp->Start,
												(int)		mapWidth,
			  									(int)		bitmapX1,
			  									(int)		bitmapY1,
			  									(int)		bitmapX2,
			  									(int)		bitmapY2,
			  									(int)		vmdLeft,
			  									(int)		vmdTop,
															vmdDouble,
															vmdBlackLines);
		}
		else {
			// Update the hardware
			SOL_Rect bitMapRect(bitmapX1,bitmapY1,bitmapX2,bitmapY2);
			SOL_Point screenPosition(vmdLeft,vmdTop);

			if (vmdDouble)	{
				if (vmdBlackLines) {
					ShowMovieBlackDbl(&bitMapRect,
						  	  				(uchar*)Bmp->Start,
						  	  				(int)mapWidth,
						  	  				&screenPosition);
				}
				else {
					ShowMovieRectDbl(&bitMapRect,
						  	  		  	(uchar*)Bmp->Start,
						  	  		  	(int)mapWidth,
						  	  		  	&screenPosition);
				}
	  		}
	  		else {
				if (vmdBlackLines) {
					ShowMovieBlack(&bitMapRect,
						  	  		(uchar*)Bmp->Start,
						  	  		(int)mapWidth,
						  	  		&screenPosition);
				}
				else {
					ShowMovieRect(&bitMapRect,
						  	  		(uchar*)Bmp->Start,
						  	  		(int)mapWidth,
						  	  		&screenPosition);
				}
	  		}
		}
#endif
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
		// Flip the rectangle for upside down bitmap
		int saveTop = top;
		if(vmdDouble) {
			top = (vmdHeight << 1) - bottom;
			bottom = (vmdHeight << 1) - saveTop;
		}
		else {
			top = vmdHeight - bottom;
			bottom = vmdHeight - saveTop;
		}
	#endif


	if(squareSize < 2)
		squareSize = 2;
	else if(squareSize > 32)
		squareSize = 32;

	return theBlobs->AddRect(squareSize,top,left,bottom,right);
}

static void
DeleteBlob(int blobNo)
{
	theBlobs->DeleteRect(blobNo);
}

void
KPlayVMD(kArgs args)
{
	switch((int) arg(1)) {

		case SCIVMD_OPEN:
			{
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

				vmdLastFrame = 0;
				vmdBlack = False;
				if (configMgr->Get(configMgr->vmdReport))
					vmdReport = True;


				// Check for vmdSkip in the resource.cfg/win
				char* skipOrNot = configMgr->Get("vmdSkip");
				if (skipOrNot 					&& 
				 	*skipOrNot 				&& 
				 	(!stricmp(skipOrNot,"yes") || !stricmp(skipOrNot,"true"))) {
					vmdSkipOn = True;
					vmdSkipFixed = True;
				}

				if (skipOrNot 					&& 
				 	*skipOrNot 				&& 
				 	(!stricmp(skipOrNot,"no") || !stricmp(skipOrNot,"false"))) {
					vmdSkipOn = False;
					vmdSkipFixed = True;
				}
			}
			break;

		case SCIVMD_PUT: 
			//	 get postion
			vmdLeft = arg(2);
			vmdTop = arg(3);
			// force to a 2 byte boundry
			vmdLeft &= 0xFFFFFFFE;

			// Set defaults
			vmdLeaveScreenBlack = False;
			vmdLeaveLastFrame = False;
			vmdDouble = False;
 			vmdBlackLines = False;
 			vmdNoAsync = False;
 			vmdRGB = 100;

			if(argCount >= 4)	{
				// get play options
				int options = arg(4);
				if (options & SCIVMD_LEAVE_SCREEN_BLACK)
					vmdLeaveScreenBlack = True;
				if (options & SCIVMD_LEAVE_LAST_FRAME)
					vmdLeaveLastFrame = True;
				if (options & SCIVMD_DOUBLE)
					vmdDouble = True;
				if (options & SCIVMD_BLACK_LINES)
					vmdBlackLines = True;
				if (options & SCIVMD_NO_AYSNC_CHECKS)
					vmdNoAsync = True;
				if ((options & SCIVMD_RGB) && (argCount >= 7)) {
 					vmdRGB = 100 + arg(5);
					vmdRGBStart = arg(6);
					vmdRGBEnd = arg(7);
				}
			}

			#ifdef WINDOWS
				if(vmdDouble) {
					// double the vmd bitmap allocation
					ReleaseMem(theBmp->Start);
					theBmp->Start=(UBYTE *) GetMem((unsigned long) 4*vmdWidth*vmdHeight);
					if (vmdBlackLines == True) {
						// Start out the bitmap black!!
						memset(theBmp->Start,0,4*vmdWidth*vmdHeight);
					}
				}
			#endif
			pm.acc = 0;
			break;

		case SCIVMD_PUT_DOUBLE: 
			#ifdef DEBUG
				msgMgr->Fatal(SrcLoc,"PlayVMD no longer supports SCIVMD_PUT_DOUBLE\n");
			#endif
			break;

		case SCIVMD_PLAY:
			#ifdef DEBUG
				msgMgr->Alert("PlayVMD no longer supports SCIVMD_PLAY\n");
			#endif
			break;

		case SCIVMD_WAIT_EVENT:
			if ((int) arg(2) & SCIVMD_TO_FRAME) {
				// Ending frame number given
				if(argCount >= 3) {
					vmdLastFrame = (int) arg(3);
				}
				#ifdef DEBUG
					else
						msgMgr->Alert(SrcLoc,"VMD return frame number not given\n");
				#endif
			}
			if ((int) arg(2) & SCIVMD_RETURN_INTERVAL) {
				// Intervals on which to return to SCI given
				vmdInterval = 3;
				if((argCount == 3) && !((int) arg(2) & SCIVMD_TO_FRAME)) {
					vmdInterval = (int) arg(3);
				}
				else {
					if (argCount > 3)
						vmdInterval = (int) arg(4);
					#ifdef DEBUG
						else
							msgMgr->Alert(SrcLoc,"VMD return interval not given\n");
					#endif
				}
			}

			pm.acc = VMDWaitEvent((int) arg(2));
			break;

		case SCIVMD_STOP:
		case SCIVMD_PAUSE:
			pm.acc = VMDForceReturn();
			break;

		case SCIVMD_RESUME:
			// playFlags set on last VMDWaitEvent
			pm.acc = VMDWaitEvent(playFlags);
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
			pm.acc = vmdFramesSkipped;
			break;

		case SCIVMD_SETPALETTE:
			blockPaletteUpdate = 1;
			break;

		case SCIVMD_RESTRICT_PALETTE:
			// Arg 1 = start of valid palette range
			// Arg 2 = end of valid palette range

			vmdPaletteStart = (int) arg(2);
			vmdPaletteEnd = (int) arg(3);
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

		case SCIVMD_BLACK:
			#ifdef DEBUG
				if (argCount < 5)
					msgMgr->Fatal(SrcLoc,"Invalid number of arguments to SCIVMD_BLACK");
			#endif
			// Set the blackout area defaults
			vmdBackgroundLeft = Max(0,(int) arg(2));
			vmdBackgroundTop = Max(0,(int) arg(3));
			vmdBackgroundRight = Min(SCIRESX-1,(int) arg(4));
			vmdBackgroundBottom = Min(SCIRESY-1,(int) arg(5));
			vmdBlack = True;
			break;
		case SCIVMD_SKIP:
			if (!vmdSkipFixed) {
				#ifdef DEBUG
					if (argCount < 2)
						msgMgr->Fatal(SrcLoc,"Invalid number of arguments to SCIVMD_SKIP");
				#endif
				vmdSkipOn = (int) arg(2);
			}
			break;

		#ifdef DEBUG
		default:
			msgMgr->Fatal(SrcLoc,"PlayVMD called with invalid option");
			break;
		#endif
	}
}
