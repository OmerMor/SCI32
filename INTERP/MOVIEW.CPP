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
#include "timed.hpp"
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
	   
WORD  AVIDeviceID = NULL;
WORD    avi_cue_who = NULL;
BOOL    avi_playing = FALSE;
BOOL  avi_paused = FALSE;
BOOL    avi_fullscreen;
BOOL	movieInit = FALSE;
short movieType;
HBRUSH hBrush;
HBRUSH hOldBrush;

char	aviVideo[] = "AVIVideo";
char movieName[80];


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

		case AVI_RESUME:
	 pm.acc = AviResume(movieNo);
	 break;

		case AVI_CLOSE:
	 pm.acc = AviClose(movieNo);
			break;

		case AVI_SETPALETTE:
		pm.acc = AviSetPalette(movieNo);
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
  

	char * movieDir = configMgr->Get("MovieDir");
	if(movieDir && strlen(movieDir)) {
		strcpy(movieName,movieDir);
		strcat(movieName,"\\");
		strcat(movieName,filename);
		}
	else
		strcpy(movieName,filename);

	char str[255];
	char resultStr[255];

	// The following findfirst is done to ensure that MSCDEX
	// knows which CD is in the drive

	char drive[10];
	_splitpath(movieName,drive,0,0,0);
#ifndef WIN32S
	find_t buffer;
	strcat(drive,"\\*.*");
	int result =_dos_findfirst(drive,_A_VOLID,&buffer);
#else
	GetVolumeInformation(drive,NULL,0,NULL,NULL,0,NULL,0);
#endif

	sprintf(str,"open %s alias sciMovie%-1d wait",movieName,movieNo);
	int retryCount = 0;
	DWORD errorCode = 1;
	while(errorCode && retryCount < 5) {
		errorCode = mciSendString(str,resultStr,80,0L);
		++retryCount;
	}


	if(errorCode) {
#ifdef DEBUG
		mciGetErrorString(errorCode,str,80);
	   msgMgr->Alert(SrcLoc, "AVI Open Failed.  Return Value: %s", str);
#endif
		msgMgr->Fatal(SrcLoc,Msg_AVIOpen);
		return AVI_OPEN_FAIL;

	}
	else
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


	// set window was here

	return AVI_NO_ERROR;

}

   
unsigned int AviClose(int movieNo)
{

	char str[30];
	sprintf(str,"close sciMovie%-1d",movieNo);
	mciSendString(str,NULL,0,0);

	hOldPal = SelectPalette (hMyDC, hPal, 0);
	RealizePalette (hMyDC);
	
	// hBrush = SelectObject(hMyDC, hOldBrush);
	//InvalidateRect(hMyWnd,NULL,FALSE);
	

	avi_playing = FALSE;
   avi_paused = FALSE;
	
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
  
	if (width && height) {

		rect.left = x * Ratio(gameWidth, LOWRESX) + gameBorder;
		rect.top = y * Ratio(gameHeight, LOWRESY) + gameBorder;
		rect.right = width * Ratio(gameWidth, LOWRESX);
		rect.bottom = height * Ratio(gameHeight, LOWRESY);

   } else {

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
		rect.right = atoi(start);
		rect.bottom = atoi(ptr);

		rect.left = x * Ratio(gameWidth, LOWRESX) + gameBorder;
		rect.top = y * Ratio(gameHeight, LOWRESY) + gameBorder;
		//rect.right = mciRect.rc.right; // rect.left + 
		//rect.bottom = mciRect.rc.bottom; // rect.top + 

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

	return AVI_NO_ERROR;
}

unsigned int AviPlay(int movieNo,int from, int to, int showStyle, int cue)
{
	DWORD dwPlayFlags = 0;
	DWORD errorCode;
	MSG msg; 
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


	if(to && from >= 0 && from <= to) {
		sprintf(retStr,"play sciMovie%-1d from %d to %d",movieNo,from,to);
	}
	else {
		sprintf(str,"play sciMovie%-1d",movieNo);
		strcpy(retStr,str);
	}

	if(cue)	{
		sprintf(str,"%s notify",retStr);
#ifndef WIN32S
		errorCode = mciSendString(str,retStr,80,hMyWnd);
#else
		errorCode = mciSendString(str,retStr,80,(void *)hMyWnd);
#endif
#ifdef DEBUG
	   msgMgr->Alert(SrcLoc, "AVI Play Failed.  Return Value: %d", errorCode);
#endif
		while (1) {
			if(PeekMessage(&msg, hMyWnd, MM_MCINOTIFY, MM_MCINOTIFY, PM_NOREMOVE))
			break;

			if(PeekMessage(&msg,hMyWnd,WM_KEYUP,WM_KEYUP,PM_REMOVE)) {
				if(msg.wParam == VK_ESCAPE) {
					break;
				}
			}
		}
	}
	else {
		sprintf(str,"%s wait",retStr);
		errorCode = mciSendString(str,retStr,80,0L);
	}

	if(errorCode) {
#ifdef DEBUG
	   msgMgr->Alert(SrcLoc, "AVI Play Failed.  Return Value: %d", errorCode);
#endif

		msgMgr->Fatal(SrcLoc,Msg_AVIPlay);
		return AVI_PLAY_FAIL;
	}

	return AVI_NO_ERROR;
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

   avi_paused = FALSE;
	avi_playing = FALSE;
	return AVI_NO_ERROR;
}

unsigned int AviPause(int movieNo)
{


	char str[30];
	sprintf(str,"pause sciMovie%-1d",movieNo);
	DWORD errorCode = mciSendString(str,NULL,0,0);
	if(errorCode) {
		return AVI_PAUSE_FAIL;
	}

	avi_paused = TRUE;
	return AVI_NO_ERROR;

}



unsigned int AviResume(int movieNo)
{

	char str[30];
	sprintf(str,"resume sciMovie%-1d",movieNo);
	DWORD errorCode = mciSendString(str,NULL,0,0);
	if(errorCode) {
		return AVI_RESUME_FAIL;
	}

	avi_paused = FALSE;
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
