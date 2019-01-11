#include "sol.hpp"


#include "kernel.hpp"
#include "textid.hpp"
#include "string.h"
#include "msg.hpp"
#include "io.h"
#include "dos.h"
#include "fcntl.h"
#include "intrpt.hpp"
#include "graphmgr.hpp"
#include "graphmd.hpp"
#include "time.hpp"
#include "timed.hpp"
#include "palette.hpp"
#include "palmgri.hpp"
#include "config.hpp"
#include "cursor.hpp"
#include "string.h"
#include "stdio.h"
#include "movie.hpp"
#include "event.hpp"

#define VACANT 0
#define FILLED 1
#define IN_USE 2

extern "C" {uchar currentVideoMode;};
#pragma aux currentVideoMode "*";

extern "C" {void WaitForBlank();}
#pragma aux (PushArgs) WaitForBlank;

static int short currPage = 0;
static long startTick;
static char pageStatus[4];
static char currentPage;
static char movieOn;
static long frameRate;
static MemID paletteHandle;
static SOLPalette * solPalette;
static movieXOffset;
static movieYOffset;

/* cel structure */
_Packed struct	aCel{
	unsigned short		xDim;			/* x dimension of cel */
	unsigned short		yDim;			/* y dimension of cel */
	unsigned short		xOff;			/* x axis offset */
	unsigned short		yOff;			/* y axis offset */
	unsigned char	skipColor;		/* skip color for cel */
	unsigned char	compressType;	
	unsigned short		compressRemapCount;
	long		compressSize;
	long		controlSize;
	long		paletteOffset;
	long		dataOff;			/* byte data for the cel */
};


void
KSetVideoMode(kArgs args) 
{

	int short mode = (int short) arg(1);

	char lastMode = currentVideoMode;

	if(currentVideoMode == (char) mode || currentVideoMode == 0x80)
		return;

	switch(mode) {

		case 1:			// Set Video Mode to ModeX

			graphMgr->SetVideoForMovies();
			currentVideoMode = (char) mode;

			break;

		case 0:

			currentVideoMode = 0;
			graphMgr->SetVideoForSpeed();
			break;

		default:
			msgMgr->Alert(SrcLoc,Msg_InvalidVideoMode,"Invalid Video Mode Request");

	}
}

void
KShowMovie(kArgs args)
{
	// Disabled!!!!
	if(argCount == 10000) {
		char * fileName;
		fileName = *(TextID)arg(1);
		if(argCount == 4)
			graphMgr->PlayMovie(fileName,arg(2),arg(3),arg(4));
		else
			graphMgr->PlayMovie(fileName,arg(2));
	}
}

#ifdef DEBUG
void
KPlayBack(kArgs)
{
}
#endif


static ServerFunc MovieServer;
static void RemoveMovieServer();
static void SetupMovieServer();
void SetMoviePalette();



_Packed struct {
		long	pageOffset;
		uchar *	buffer;		
		uchar *	dataOffset;
		int short	xOffset;
		int short 	yOffset;
		int short 	xDim;
		int short	yDim;
		char  compressType;
	} xCel ;


void RunMovie(char * fileName, int speed,int xOff, int yOff)
{
	MemID buffer;
	int handle;
	char movieName[80];
	unsigned result;

	movieXOffset = xOff;
	movieYOffset = yOff;

	char * movieDir = configMgr->Get("MovieDir");
	if(movieDir && strlen(movieDir)) {
		strcpy(movieName,movieDir);
		strcat(movieName,"\\");
		strcat(movieName,fileName);
		}
	else
		strcpy(movieName,fileName);

	buffer.Get(MemMovieBuffer, 64200);
	result = _dos_open(movieName,O_RDONLY,&handle);

	if(result) {
		msgMgr->Alert("Unable to Open Movie File");	// want the customer to see this
		return;
	}


	/* Main loop for displaying reading special format cels from an
		opened file.  Cels are displayed with "speed" ticks between them
	*/

	if(currentVideoMode == 1)
		RunMovieX(buffer,handle,speed);
	else if(currentVideoMode == 0x80)
		RunMovieVesa(buffer,handle,speed);

	buffer.Free();
	_dos_close(handle);

}

void RunMovieX(MemID & buffer, int handle, int speed)
{
	int short i;
	int short done;
	int short lastPage;
	int short nextPage;
	unsigned short celCount;


	currentPage = 0;
	frameRate = speed;

	celCount = LoadCelCount(handle);
	LoadMoviePalette(handle);

	CopyPage(3 * 0x4000,0);
	ShowPage(3 * 0x4000);		// point to unused page

	// Fill video pages for initial pass
	for(i = 0; i < 4; i++) {

		if(i == 3) {		// Now Filling Page 3 - switch to page 0
			SetMoviePalette();
			ShowPage(0);
			startTick = timeMgr->GetTickCount();
			pageStatus[0] = IN_USE;
			currPage = 1;
		}

		if(i > 0) 
			CopyPage(i * 0x4000,(i - 1) * 0x4000);

		LoadFrame((char) i,handle,buffer);

		pageStatus[i] = FILLED;

	}

	SetupMovieServer();
	movieOn = True;
	currentPage = 0;

	while (i < celCount) {
		if (pageStatus[currentPage] == VACANT) {

			if(currentPage == 0)
				lastPage = 3;
			else
				lastPage = currentPage - 1;

			// if the next frame is one we want to fill,
			// let's wait until a blank to make sure the hardware
			// really did switch to it

			nextPage = currentPage + 1;
			if(nextPage > 3)
				nextPage = 0;

			if(pageStatus[nextPage] == IN_USE)
				WaitForBlank();
				

			CopyPage(currentPage * 0x4000,lastPage * 0x4000);

			LoadFrame((char) currentPage,handle,buffer);

			pageStatus[currentPage] = FILLED;
			i++;
			++currentPage;
			if (currentPage > 3)
				currentPage = 0;
		}
	}


	/* wait for all our filled buffers to be displayed */

	do  {

		done = True;
		for (i = 0; i < 4; i++) {
			if (pageStatus[i] == FILLED) {
				done = False;
				break;
			}
		}
	}
	while (!done);


	// We need to copy the last page filled to page 3 so there are no glitches
	// because the next movie (if any) will start by showing page 3)
	if(currentPage == 0)
		currentPage = 3;
	else
		currentPage--;

	movieOn = False;
	RemoveMovieServer();

	for(i = 3; i >= 0; i--) {
		if(i != currentPage)
			CopyPage(i * 0x4000,currentPage * 0x4000);
	}
	
	ShowPage(0);

	
}

void RunMovieVesa(MemID & buffer,int handle, int speed)
{

	int short i;
	unsigned short celCount;


	currentPage = 0;
	frameRate = speed;

	celCount = LoadCelCount(handle);
	LoadMoviePalette(handle);
	SetMoviePalette();

	movieOn = True;
	currentPage = 0;

	i = 0;
	unsigned long tick;
	startTick = 0;

	while (i < celCount) {

		tick = timeMgr->GetTickCount();
		if(tick >= startTick + frameRate) {		// time for display
			LoadFrame(0, handle,buffer);
			startTick = tick;
			i++;
		}
	}



	movieOn = False;

}


void LoadFrame(char page, int handle, MemID & buffer)

{

	/* Display frame first reads the cel header from the curent file position
		and determines how much expanded cel data needs to be read and where
		it is to be positioned.  It then reads the data and sends the data
		a line at a time to the video driver for display */


	aCel theCel;
	uchar * theBuffer;
	unsigned short headerSize;
	unsigned count;
	unsigned result;

	headerSize = sizeof(aCel);
	theBuffer = (uchar *) *buffer;

	result = _dos_read(handle,(uchar *) &theCel,headerSize,&count);

	if(result) {
		msgMgr->Alert(SrcLoc,Msg_MovieError);
		return;
	}

	// set the file position to the begining of the cel's actual data

	lseek(handle,theCel.dataOff,SEEK_SET);

	AsyncEventCheck();
	result = _dos_read(handle,theBuffer,(unsigned ) theCel.compressSize,&count);
	AsyncEventCheck();

	if(result) {
		msgMgr->Alert(SrcLoc,Msg_MovieError);
		return;
	}

	// Now the buffer is filled with an uncompressed, cel of 
	// Display Block (cels.s) only sets up registers for the driver call

	xCel.pageOffset = (int short) page * 0x4000;
	xCel.buffer = theBuffer;
	xCel.dataOffset = theBuffer + (unsigned ) theCel.controlSize;
	xCel.xOffset = theCel.xOff + movieXOffset;
	xCel.yOffset = theCel.yOff + movieYOffset;
	xCel.xDim = theCel.xDim;
	xCel.yDim = theCel.yDim;
	xCel.compressType = theCel.compressType;


	FillVideoPage(&xCel,theBuffer + theCel.controlSize, theBuffer + count);
	AsyncEventCheck();

}


/*
 *
 *
 *
 *
 *  Server section
 *  ==============
 *
 *
 */
 

static int short lastPage = -1;


static void SetupMovieServer()
{
	int short i;

	currPage = 0;
	lastPage = -1;

	for (i = 0; i < 4; i++)  {
		pageStatus[i] = VACANT;
	}
	InstallServer(MovieServer, 1);
}


static void RemoveMovieServer()
{
	DisposeServer(MovieServer);
}


static void __pragma("ServerFunc")
MovieServer()
{
	_disable();
	long tick;

	//  If nothing playing, ignore interrupt
	if (!movieOn)
		return;

	tick = timeMgr->GetTickCount();
	if (tick >= startTick + frameRate) {		// time for display

		//  If desired page available, display
		if (pageStatus[currPage] == FILLED)  {
			ShowPage(currPage * 0x4000);
			pageStatus[currPage] = IN_USE;

			//  Mark last displayed page as vacant (if any)
			if (lastPage != -1)
				pageStatus[lastPage] = VACANT;
			lastPage = currPage;

			//  Advance currPage to next one
			currPage++;
			if (currPage == 4)
				currPage = 0;

			startTick = tick;
		}
	}

	_enable();

}


/* LoadMoviePalette */

void LoadMoviePalette(int handle)
{

	long length;
	unsigned result;
	unsigned count;

	result = _dos_read(handle,(uchar *) &length,4,&count);
	if(result) {
		msgMgr->Alert(SrcLoc,Msg_MovieError);
		return;
	}


	paletteHandle.Get(MemMovie, length);
	AsyncEventCheck();
	result = _dos_read(handle,*paletteHandle,(int short) length,&count);
	if(result) {
		msgMgr->Alert(SrcLoc,Msg_MovieError);
		return;
  	}

	HunkPalette hunkPalette;
	hunkPalette.Init(*paletteHandle);
	solPalette = New SOLPalette(hunkPalette);
	paletteHandle.Free();

}

void SetMoviePalette()

{
	graphMgr->GPalette().Submit(*solPalette);
	graphMgr->FrameOut();
}

/* LoadCelCount */

unsigned short LoadCelCount(int  handle)
{

	unsigned short celCount;
	unsigned result;
	unsigned count;

	result = _dos_read(handle,(uchar *) &celCount,2,&count);
	if(result) {
		msgMgr->Alert(SrcLoc,Msg_MovieError);
		return 0;
	}

	return celCount;


}




