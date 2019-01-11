// dummy version ov movie.cpp for Windows
#define INCLUDE_MMSYSTEM_H
#include "sciwin.hpp"

#include "sol.hpp"

#include "moview.hpp" 
#include <stdlib.h>
#include <string.h>

#include "mciavi.h"
#include "digitalv.h"
#include "string.h"

#include "audio.hpp"
#include "kernel.hpp"
#include "textid.hpp"
#include "string.h"
#include "msg.hpp"
#include "io.h"
#include "dos.h"
#include "fcntl.h"
#include "intrpt.hpp"
#include "graphmgr.hpp"
#include "time.hpp"
#include "timew.hpp"
#include "palette.hpp"
#include "palmgri.hpp"
#include "config.hpp"
#include "cursor.hpp"
#include "string.h"
#include "stdio.h"
#include "pmachine.hpp"
#include "mbox.hpp"
#include <dos.h>

void	InitMovie(void);
void	closeWaveDev(void);
short movieType;	   
static BOOL	movieInit = FALSE;
static Plane * moviePlane;
static char movieName[80];
static int currentlyPlayingMovie;
static int aviStatus[10];

void
KSetVideoMode(kArgs args)
{
	int short mode = (int short) arg(1);
}

void
KShowMovie(kArgs args)
{
	if (!movieInit)
		InitMovie();

	int movieNo = arg(1);
   switch(arg(2)) {

		case AVI_OPEN:
			 pm.acc = AviOpen(movieNo,(*(TextID)arg(3)));
	 		break;

		case AVI_PUT:

	 		if(arg(0) > 4) {
	    		pm.acc = AviPut(movieNo,arg(3), arg(4), arg(5), arg(6));
	 		} else {
	    		pm.acc = AviPut(movieNo,arg(3), arg(4), NULL, NULL);
	 		}
	 		break;

		case AVI_PUT_DOUBLE:
			AviPutDouble(movieNo,arg(3),arg(4));
			break;

		case AVI_PLAY:

	 		switch (arg(0)) {
	    		case 6:
	       			pm.acc = AviPlay(movieNo,arg(3), arg(4), arg(5), arg(6));
	       			break;

	    		case 5:
	       			pm.acc = AviPlay(movieNo,arg(3), arg(4), arg(5), NULL);
	       			break;

	    		case 4:
	       			pm.acc = AviPlay(movieNo,arg(3), arg(4), SCI_WINDOW, NULL);
	       			break;

	    		case 3:
	       			pm.acc = AviPlay(movieNo,NULL, NULL, SCI_WINDOW, NULL);	// must have from AND to
	       			break;

	    		default:
	       			pm.acc = AviPlay(movieNo,NULL, NULL, SCI_WINDOW, NULL);
	       			break;
	 		}

	 		break;

		case AVI_STOP:
	 		pm.acc = AviStop(movieNo);
	 		break;

		case AVI_PAUSE:
	 		pm.acc = AviPause(movieNo);
	 		break;

		case AVI_CUE:
			pm.acc = AviCue(movieNo,arg(3));
			break;

		case AVI_SEEK:
			pm.acc = AviSeek(movieNo,arg(3));
			break;

		case AVI_RESUME:
	 		pm.acc = AviResume(movieNo);
	 		break;

		case AVI_CLOSE:
	 		pm.acc = AviClose(movieNo);
			break;

		case AVI_SETPALETTE:
			pm.acc = AviSetPalette(movieNo);
			break;

		case AVI_GET_LENGTH:
			pm.acc = AviGetLength(movieNo);
			break;

		case AVI_GET_POSITION:
			pm.acc = AviGetPosition(movieNo);
			break;

		case AVI_GET_STATUS:
			pm.acc = AviGetStatus(movieNo);
			break;

		case AVI_FRAMES_SKIPPED:
			pm.acc = AviGetFramesSkipped(movieNo);
			break;

		case AVI_WAIT_EVENT:
			if(arg(0) == 3)
				pm.acc = AviWaitEvent(movieNo,arg(3));
			else
				pm.acc = AviWaitEvent(movieNo,3);
			break;

	}
}

#ifdef DEBUG
void
KPlayBack(kArgs)
{
}
#endif

unsigned int AviOpen(int movieNo,char * filename)
{
	int i;
	int movieFound = 0;

	for(i = 0; i < configMgr->GetNTokens("MovieDir"); i++) {
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
			sprintf(movieName,"%s\\%s",movieDir,filename);
		}
		else
			strcpy(movieName,filename);
		
		if(!access(movieName,F_OK)) {
			movieFound = TRUE;
			break;	// file was found
		}
	}
	
	if(!movieFound) {
#ifdef DEBUG
	   msgMgr->Alert(SrcLoc, "AVI: %s Not Found ", movieName);
#endif
		return AVI_OPEN_FAIL;
	}

	// OK, the AVI was found - try to open it
	char str[255];
	char resultStr[255];

	sprintf(str,"open %s alias sciMovie%-1d wait",movieName,movieNo);
	DWORD errorCode = mciSendString(str,resultStr,80,0L);

	if(errorCode) {
#ifdef DEBUG
		mciGetErrorString(errorCode,str,200);
		msgMgr->Fatal(SrcLoc, "AVI Open Failed.  Return Value: %s", str);
#endif
//		msgMgr->Fatal(SrcLoc,Msg_AVIOpen);
		return AVI_OPEN_FAIL;
	}
	aviStatus[movieNo] = statusOpen;
	return AVI_NO_ERROR;
}

unsigned int AviSetPalette(int movieNo)
{
	char str[80];
	char resultStr[80];
	
	graphMgr->GPalette().UpdateForFrame();
	graphMgr->GPalette().UpdateHardware();
  	sprintf(str,"setvideo sciMovie%-1d palette handle to %u wait",movieNo,hPal);
	DWORD errorCode = mciSendString(str,resultStr,80,0L);
	if(errorCode) {
#ifdef DEBUG
	   msgMgr->Alert(SrcLoc, "AVI Set Palette Failed.  Return Value: %d", errorCode);
#endif
		msgMgr->Fatal(SrcLoc,Msg_AVISetPalette);

		return AVI_PALETTE_FAIL;
	}

	return AVI_NO_ERROR;
}

   
unsigned int AviClose(int movieNo)
{
	char str[30];

	if(aviStatus[movieNo] == statusNotOpen)
		return 0;

	sprintf(str,"close sciMovie%-1d",movieNo);
	mciSendString(str,NULL,0,0);

	hOldPal = SelectPalette (hMyDC, hPal, 0);
	RealizePalette (hMyDC);

	aviStatus[movieNo] = statusNotOpen;	
	graphMgr->DeletePlane(moviePlane);

	return AVI_NO_ERROR;
}

unsigned int AviPut(int movieNo,int x, int y, int width, int height)
{ 

	// This may need some beefing up in cases where the user does not
	// wish to play to the SCI window, but a seperate child window.

	RECT rect;
	// BOOL mciRet;
	char str[255];				  
	char retStr[255];
	DWORD errorCode;
	int aviWidth,aviHeight;

	// Get the AVIs actual width and height;
		/* get the original size of the movie */

	sprintf(str,"where sciMovie%-1d source max wait",movieNo);
	errorCode = mciSendString(str,retStr,255,0L);
	if(errorCode) {
#ifdef DEBUG
   msgMgr->Alert(SrcLoc, "AVI Where Failed.  Return Value: %d", errorCode);
#endif

		msgMgr->Fatal(SrcLoc,Msg_AVIPut);
		return AVI_WHERE_FAIL;
	}

	// parse the returned string
	char * start;
	char * ptr = retStr;
	// get the left - always 0
	while(*ptr++ != ' ');
	// get the top - always 0
	while(*ptr++ != ' ');
	
	// get the width
	start = ptr;
	while(*ptr != ' ') {
		ptr++;
	}

	*ptr++ = '\0';
	aviWidth = atoi(start);
	aviHeight = atoi(ptr);


	if (width && height) {

		rect.left = x * Ratio(gameWidth, SCIRESX) + gameBorder;
		rect.top = y * Ratio(gameHeight, SCIRESY) + gameBorder;
		rect.right = width * Ratio(gameWidth, SCIRESX);
		rect.bottom = height * Ratio(gameHeight, SCIRESY);

#ifdef DEBUG
		if(rect.right != aviWidth || rect.bottom != aviHeight) {
			if(rect.right == aviWidth * 2 && rect.bottom == aviHeight * 2){}
			else 
				msgMgr->Alert("Your AVI width and/or height do not match the AVI");
		}
#endif
   } else {
		rect.left = x * Ratio(gameWidth, SCIRESX) + gameBorder;
		rect.top = y * Ratio(gameHeight, SCIRESY) + gameBorder;
		rect.right = aviWidth;
		rect.bottom = aviHeight;
		// setup width and height for the plane creation
		width = rect.right * Ratio(SCIRESX,gameWidth);
		height = rect.bottom * Ratio(SCIRESY,gameHeight);
	}

	sprintf(str,"put sciMovie%-1d destination at %u %u %u %u wait",movieNo,rect.left,
		rect.top,rect.right,rect.bottom);

	errorCode = mciSendString(str,retStr,80,0L);
	if(errorCode) {
#ifdef DEBUG
	   msgMgr->Alert(SrcLoc, "AVI Put Failed.  Return Value: %d", errorCode);
#endif

		msgMgr->Fatal(SrcLoc,Msg_AVIPut);
		return AVI_PUT_FAIL;
	}

	// create a plane for the movie to play in (protect it from SCI)
	// First, create an SOL_RECT with lo-res coordinates and offset by top/left
	SOL_Rect sRect;
	int sRight = x + width - 1;
	int sBottom = y + height - 1;
	sRect.Set(x,y,sRight,sBottom);
	moviePlane = new Plane(sRect);
	graphMgr->AddPlane(moviePlane);
	graphMgr->FrameOut(TRUE);

	return AVI_NO_ERROR;
}

unsigned int AviPutDouble(int movieNo,int x, int y)
{ 

	RECT rect;
	// BOOL mciRet;
	char str[255];				  
	char retStr[255];
	DWORD errorCode;
	int aviWidth,aviHeight;

	// Get the AVIs actual width and height;
		/* get the original size of the movie */

	sprintf(str,"where sciMovie%-1d source max wait",movieNo);
	errorCode = mciSendString(str,retStr,255,0L);
	if(errorCode) {
#ifdef DEBUG
   msgMgr->Alert(SrcLoc, "AVI Where Failed.  Return Value: %d", errorCode);
#endif

		msgMgr->Fatal(SrcLoc,Msg_AVIPut);
		return AVI_WHERE_FAIL;
	}

	// parse the returned string
	char * start;
	char * ptr = retStr;
	// get the left - always 0
	while(*ptr++ != ' ');
	// get the top - always 0
	while(*ptr++ != ' ');
	
	// get the width
	start = ptr;
	while(*ptr != ' ') {
		ptr++;
	}

	*ptr++ = '\0';
	aviWidth = atoi(start);
	aviHeight = atoi(ptr);


	rect.left = x * Ratio(gameWidth, SCIRESX) + gameBorder;
	rect.top = y * Ratio(gameHeight, SCIRESY) + gameBorder;
	rect.right = aviWidth * 2;
	rect.bottom = aviHeight * 2;

	sprintf(str,"put sciMovie%-1d destination at %u %u %u %u wait",movieNo,rect.left,
		rect.top,rect.right,rect.bottom);

	errorCode = mciSendString(str,retStr,80,0L);
	if(errorCode) {
#ifdef DEBUG
	   msgMgr->Alert(SrcLoc, "AVI Put Failed.  Return Value: %d", errorCode);
#else
		msgMgr->Fatal(SrcLoc,Msg_AVIPut);
		return AVI_PUT_FAIL;
#endif

	}

	// create a plane for the movie to play in (protect it from SCI)
	// First, create an SOL_RECT with lo-res coordinates and offset by top/left
	SOL_Rect sRect;
	int width = rect.right * Ratio(SCIRESX,gameWidth);
	int height = rect.bottom * Ratio(SCIRESY,gameHeight);
	int sRight = x + width - 1;
	int sBottom = y + height - 1;
	sRect.Set(x,y,sRight,sBottom);
	moviePlane = new Plane(sRect);
	graphMgr->AddPlane(moviePlane);
	graphMgr->FrameOut(TRUE);

	return AVI_NO_ERROR;
}

unsigned int AviPlay(int movieNo,int from, int to, int showStyle, int cue)
{
	DWORD dwPlayFlags = 0;
	DWORD errorCode;
	// BOOL mciRet;

	char str[80];
	char retStr[80];

	sprintf(str,"Movie: %d  From: %d  To: %d  ShowStyle: %d\n",movieNo,from,to,showStyle);
	if(showStyle == SCI_WINDOW) {
		sprintf(str,"window sciMovie%-1d handle %u wait",movieNo,hMyWnd);
		errorCode = mciSendString(str,retStr,80,0);
		if(errorCode) {
#ifdef DEBUG
	   msgMgr->Alert(SrcLoc, "AVI Window Command Failed.  Return Value: %d", errorCode);
#endif

			msgMgr->Fatal(SrcLoc,Msg_AVIPlay);
		}
	}

	// Turn off the SCI Timer interupts

	if(to && from >= 0 && from <= to) {
		sprintf(retStr,"play sciMovie%-1d from %d to %d",movieNo,from,to);
	}
	else {
		sprintf(str,"play sciMovie%-1d",movieNo);
		strcpy(retStr,str);
	}

	if(cue)	{
		sprintf(str,"%s notify",retStr);
		currentlyPlayingMovie = movieNo;

#ifndef WIN32S
		errorCode = mciSendString(str,retStr,80,hMyWnd);
#else
		errorCode = mciSendString(str,retStr,80,(void *)hMyWnd);
#endif
#ifdef DEBUG
		if(errorCode)
	   		msgMgr->Alert(SrcLoc, "AVI Play Failed.  Return Value: %d", errorCode);
#endif
	}
	else {
		sprintf(str,"%s wait",retStr);
		((TimeMgrWin*)timeMgr)->DisableTimer();
		errorCode = mciSendString(str,retStr,80,0L);
		((TimeMgrWin*)timeMgr)->EnableTimer();
	}

	if(errorCode) {
#ifdef DEBUG
	   msgMgr->Alert(SrcLoc, "AVI Play Failed.  Return Value: %d", errorCode);
#endif

		msgMgr->Fatal(SrcLoc,Msg_AVIPlay);
		return AVI_PLAY_FAIL;
	}

	aviStatus[movieNo] = statusPlaying;
	return AVI_NO_ERROR;
}
unsigned int AviWaitEvent(int,int flags)
{
	MSG msg; 
	WORD retValue;

	while (1) {
		
		if(flags & AVI_COMPLETED && PeekMessage(&msg, hMyWnd, MM_MCINOTIFY, MM_MCINOTIFY, PM_NOREMOVE)) {
			retValue = AVI_COMPLETED;
			break;
		}

		if(flags & AVI_ESC_PRESSED && PeekMessage(&msg,hMyWnd,WM_KEYUP,WM_KEYUP,PM_NOREMOVE)) {
			if(msg.wParam == VK_ESCAPE) {
				retValue = AVI_ESC_PRESSED;
				break;
			}
		}

		if(flags & AVI_MOUSE_DOWN && PeekMessage(&msg,hMyWnd,WM_LBUTTONDOWN,WM_LBUTTONDOWN,PM_NOREMOVE)) {
			retValue = AVI_MOUSE_DOWN;
			break;
		}
	}

	return retValue;
}

unsigned int AviStop(int movieNo)
{
	char str[30];
	sprintf(str,"stop sciMovie%-1d",movieNo);
	DWORD errorCode = mciSendString(str,NULL,0,0);
	if(errorCode) {
		char str[80];
		mciGetErrorString(errorCode,str,80);
		return AVI_STOP_FAIL;
	}

	aviStatus[movieNo] = statusStopped;
	return AVI_NO_ERROR;
}

unsigned int AviPause(int movieNo)
{

	if(aviStatus[movieNo] != statusPlaying)
		return AVI_PAUSE_FAIL;

	char str[30];
	sprintf(str,"pause sciMovie%-1d",movieNo);
	DWORD errorCode = mciSendString(str,NULL,0,0);
	if(errorCode) {
		return AVI_PAUSE_FAIL;
	}

	aviStatus[movieNo] = statusPaused;
	return AVI_NO_ERROR;

}

unsigned int AviCue(int movieNo, int frameNo)
{

	char str[30];
	sprintf(str,"cue sciMovie%-1d output to %d",movieNo,frameNo);
	DWORD errorCode = mciSendString(str,NULL,0,0);
	if(errorCode) {
		return AVI_CUE_FAIL;
	}

	aviStatus[movieNo] = statusPaused;
	return AVI_NO_ERROR;

}
unsigned int AviSeek(int movieNo, int frameNo)
{
	if(aviStatus[movieNo] == statusNotOpen)
		return AVI_PAUSE_FAIL;

	char str[30];
	sprintf(str,"seek sciMovie%-1d  to %d",movieNo,frameNo);
	DWORD errorCode = mciSendString(str,NULL,0,0);
	if(errorCode) {
		return AVI_CUE_FAIL;
	}

	aviStatus[movieNo] = statusPaused;
	return AVI_NO_ERROR;

}



void AviNotify(WPARAM ,LPARAM)
{
	// Notify is used only for AviPlay with cue, so
	// the only notify we should be getting is from a play
	// which has been completed. The only problem is that I
	// can't get (I think) which movie it was if more than one
	// is open.  I'm storing the playing movie number in a static variable

	aviStatus[currentlyPlayingMovie] = statusDone;

}


unsigned int AviResume(int movieNo)
{
	if(aviStatus[movieNo] != statusPaused)
		return AVI_RESUME_FAIL;

	char str[30];
	sprintf(str,"resume sciMovie%-1d",movieNo);
	DWORD errorCode = mciSendString(str,NULL,0,0);
	if(errorCode) {
		return AVI_RESUME_FAIL;
	}

	aviStatus[movieNo] = statusPlaying;
	return AVI_NO_ERROR;

}

void	InitMovie(void)
{
	// No longer necessary, but...
}

void	TermMovie(void)
{
	// No longer necessary, but...
}

unsigned int AviGetLength(int movieNo)
{
	if(aviStatus[movieNo] == statusNotOpen)
		return 0;

	char str[80];
	char resultStr[80];
	sprintf(str,"status sciMovie%-1d length wait",movieNo);
	DWORD errorCode = mciSendString(str,resultStr,80,0L);
	if(!errorCode)
		return atoi(resultStr);
	else
		return 0;
}

unsigned int AviGetPosition(int movieNo)
{

	char str[80];
	char resultStr[80];
	sprintf(str,"status sciMovie%-1d position wait",movieNo);
	DWORD errorCode = mciSendString(str,resultStr,80,0L);
	return atoi(resultStr);

}
unsigned int AviGetFramesSkipped(int movieNo)
{

	char str[80];
	char resultStr[80];
	sprintf(str,"status sciMovie%-1d frames skipped wait",movieNo);
	DWORD errorCode = mciSendString(str,resultStr,80,0L);
	return atoi(resultStr);

}


unsigned int AviGetStatus(int movieNo)
{
	return aviStatus[movieNo];
}

