#include <io.h>
#include "sol.hpp"
#include "robot.hpp"
#include "dos.hpp"
#include "criterr.hpp"
#include "graphmgr.hpp"
#include "graph.hpp"
#include "dcomp.hpp"
#include "time.hpp"
#include "kernel.hpp"
#include "pmachine.hpp"
#include "config.hpp"
#include "audio.hpp"


const PALSIZE = 1200;

struct VideoHeader
{
	short	celCount;
	uchar	compressionType;
	uchar	vertScaleFactor;
	short	xsize, ysize;
	short	xoff, yoff;
	short	x1, y1;
	short	size;
};

enum CompType {LZ_COMP, RLE_COMP, UNCOMP};

int	Robot::spinRate = 150;
int	Robot::cacheSize = 0;
static int testFramesDone = 0;


Robot::Robot(int resNum, long planeId, int priority, int xoff, int yoff) :
	priority(priority),
	resNum(resNum),
	planeId(planeId), 
	xoff(xoff),
	yoff(yoff)
{
	hunkPalette = NULL;
	robotSi = NULL;
	videoSizeArray = NULL;
	firstFrame = True;
	lzScratch = New char[SCRATCHRAM_SIZE];
}

/*
 *   NOTE:   If robot is not deleted when graphMgr destroyed, 
 *   this destructor gets called during graphmgr destructor, which
 *   will reenter a graphmgr method, causing trouble
 */


Robot::~Robot()
{
//	msgMgr->Alert("Frames output: %d\n", testFramesDone);
	delete robotSi;
	delete[] videoSizeArray;
	delete[] lzScratch;
	delete hunkPalette;
	bmHandle.Free();
	if (fd)
		Close(fd);

	if (robotSi)
		graphMgr->DeleteScreenItem(robotSi);
}


int
Robot::DoRobot()
{
	static int lastFrameNum;

	//  If this is first call, initialize properly
	if (firstFrame)  {
		fd = Open(resNum);
		if (critErrHandler->Tripped() ||  fd < 1)
			msgMgr->Fatal("Could not open Robot resource %d\n", resNum);

		Init();
		lastFrameNum = -1;
		frameNum = 0;
	}
	else  {
		//  Figure out which frame timer indicates to do
		//  We must predict read, draw and display time.
//		frameNum = (timeMgr->GetTickCount() - robotStartTime) / 6;
		frameNum++;
	}


	//  If last frame already shown, close file and return done
	if (frameNum >= frameCount)  {
		Close(fd);
		fd = 0;
		while (!AudRobot(RobotAudInfo(NULL, 0, 0))) {}
		return ROBOT_DONE;
	}

	if (frameNum == lastFrameNum)  {
		return ROBOT_OK;
	}

	//  Read data, submit audio
	int oddCount = frameNum >> 1;
	int evenCount = oddCount;
	if (frameNum & 1)
		evenCount++;

	int evenSize = (audioSize + vPadSize[0]) * evenCount;
	int oddSize = (audioSize + vPadSize[1]) * oddCount;

	int time1Start = timeMgr->GetMilliSec();
	LSeek(fd, fileDataStart + oddSize + evenSize, SEEK_SET);
	long posStart = LSeek(fd, 0, SEEK_CUR);
	int size = audioSize + videoSizeArray[frameNum] + sizeof(VideoHeader);
	uchar* scratch = New uchar[size];
	Read(fd, scratch, size);

	long audCount = *((long*) &scratch[4]);
	long audPos = *((long*) scratch);

	while (!AudRobot(RobotAudInfo((char*) &scratch[8], audCount, audPos << 1))) {}

	int time1End = timeMgr->GetMilliSec();

	//  Init screenItem with values read from disk
	//  Seek to beginning of next record
	if (frameNum + 2 < frameCount)  {
		int nextFrameNum = frameNum + 2;
		oddCount = nextFrameNum >> 1;
		evenCount = oddCount;
		if (nextFrameNum & 1)
			evenCount++;

		evenSize = (audioSize + vPadSize[0]) * evenCount;
		oddSize = (audioSize + vPadSize[1]) * oddCount;

		LSeek(fd, fileDataStart + oddSize + evenSize, SEEK_SET);
	}
	uchar* video = scratch + audioSize;
	CreateCel(video, firstFrame);
	delete[] scratch;

	if (firstFrame)  {
//		robotSi = New ScreenItem(planeId, Bitmap(bmHandle));
		robotSi->pos = SOL_Point(x1, y1);
		robotSi->z = priority;
//		graphMgr->AddScreenItem(robotSi);
	}
	else  {
		robotSi->bitmap = bmHandle;
		robotSi->pos = SOL_Point(x1, y1);
		robotSi->z = priority;
//		graphMgr->UpdateScreenItem(robotSi);
	}

	//  Set flag, return ok
	lastFrameNum = frameNum;
	testFramesDone++;
	return ROBOT_OK;
}


void
Robot::KillRobot()
{
	if (fd)  {
		Close(fd);
		fd = 0;
	}

	if (robotSi)  {
		graphMgr->DeleteScreenItem(robotSi);
		robotSi = NULL;
	}
}


void
Robot::FrameAlmostVisible()
{
	if (!firstFrame)  {
		//  Do delay if we are ahead of schedule
	}
}


void
Robot::FrameNowVisible()
{
	if (firstFrame)  {
//		robotStartTime = timeMgr->GetMilliSec();
		robotStartTime = timeMgr->GetTickCount();
		firstFrame = False;
	}
}


void
Robot::CreateCel(uchar* video, Bool usePalette)
{
	VideoHeader* vh = (VideoHeader*) video;
	compressType = vh->compressionType;
	vertScaleFactor = vh->vertScaleFactor;
	x1 = vh->x1 + xoff;
	y1 = vh->y1 + yoff;
	xsize = vh->xsize;
	ysize = vh->ysize;

	if (vh->celCount != 1)
		msgMgr->Fatal("Robot has %d cels in record!\n", (int) vh->celCount);

	bmHandle.Free();
	bmHandle.Get(MemBitmap, xsize * ysize + CELHEADERSIZE + PALSIZE);
	BuildHeader(bmHandle, xsize, ysize, 255, vh->xoff, vh->yoff, xRes, yRes, usePalette);

	MemID temp;
	uchar* ptr = NULL;
	if (vertScaleFactor == 100)
		ptr = (uchar*) *bmHandle + CELHEADERSIZE;
	else  {
		temp.Get(MemBitmap, xsize * ((ysize * vertScaleFactor) / 100));
 		ptr = (uchar*) *temp;
	}

	switch (compressType)  {
		case RLE_COMP:
			RLEDecode(ptr, video + sizeof(VideoHeader));
			break;
	
		case LZ_COMP:
			LZDecode(ptr, video + sizeof(VideoHeader), videoSizeArray[frameNum]);
			break;

		case UNCOMP:
			memcpy(ptr, video + sizeof(VideoHeader), vh->xsize * vh->ysize);
			break;
	}

	if (vertScaleFactor != 100)  {
		ExpandData((uchar*) *bmHandle + CELHEADERSIZE, ptr);
		temp.Free();
	}

	if (usePalette)  {
		memcpy((uchar*) *bmHandle + CELHEADERSIZE + (xsize * ysize), hunkPalette, PALSIZE);
	}
}


struct AudPrimerHdr
{
	long	totalSampleCount;
	short	compressionType;
	long	evenSize;
	long	oddSize;
};

void
Robot::Init()
{
	uchar type, scrap;
	char buffer[4];
	char junk[8];
	short fcount, asize, vsize1, vsize2, version;
	short palSize, audPrimerSize;
	char palFlag, audFlag;


	Read(fd, &type, 1);
	Read(fd, &scrap, 1);
	Read(fd, buffer, 4);
	Read(fd, &version, 2);
	Read(fd, &asize, 2);
	Read(fd, &vsize1, 2);
	Read(fd, &vsize2, 2);
	Read(fd, &fcount, 2);
	Read(fd, &palSize, 2);
	Read(fd, &audPrimerSize, 2);
	Read(fd, &xRes, 2);
	Read(fd, &yRes, 2);
	Read(fd, &palFlag, 1);
	Read(fd, &audFlag, 1);
	Read(fd, junk, 8);

	if (!xRes)
		xRes = graphMgr->NextScreen()->Xdim();
	if (!yRes)
		yRes = graphMgr->NextScreen()->Ydim();

	audioSize = asize;
	vPadSize[0] = vsize1;
	vPadSize[1] = vsize2;
	frameCount = fcount;

	if (type != 0x3d  ||  strcmp(buffer, "SOL"))	
		msgMgr->Fatal("Resource is not Robot type!");

	if (version != 2)
		msgMgr->Fatal("Unsupported version of Robot resource");


//	LSeek(fd, audPrimerSize, SEEK_CUR);
	if (audPrimerSize)  {
		AudPrimerHdr audPrimerHdr;
		Read(fd, &audPrimerHdr, 14);

		if (audPrimerHdr.totalSampleCount)  {
			long size = audPrimerHdr.evenSize;
			char* audBuff1 = New char[size];
			Read(fd, audBuff1, size);
			while (!AudRobot(RobotAudInfo(audBuff1, size, 0))) {}

			size = audPrimerHdr.oddSize;
			Read(fd, audBuff1, size);
			while (!AudRobot(RobotAudInfo(audBuff1, size, 2))) {}
			delete audBuff1;
		}
	}

	//  Read in palette if flag indicates one in file
	if (palFlag)  {
		hunkPalette = (void*) New uchar[PALSIZE];
		Read(fd, hunkPalette, palSize);
	}
	else
		LSeek(fd, palSize, SEEK_CUR);

	delete[] videoSizeArray;
	videoSizeArray = New ushort[frameCount];

	Read(fd, videoSizeArray, sizeof(ushort) * frameCount);
	LSeek(fd, 8 * frameCount, SEEK_CUR);

	long pos = LSeek(fd, 0, SEEK_CUR);
	int remain = pos % 2048;
	if (remain)
		LSeek(fd, 2048 - remain, SEEK_CUR);
	fileDataStart = LSeek(fd, 0, SEEK_CUR);
}



const int REPEAT = 0x80;


void
Robot::RLEDecode(uchar* dest, uchar* src)
{
	uchar run, dbyte;
	int xDim = xsize;
	int yDim = (ysize * vertScaleFactor) / 100;

	for (int y = 0; y < yDim; y++)
	{
		for (int count = xDim; count; )
		{
			dbyte = *src++;
			run = dbyte & 0x3f;

			if (dbyte & REPEAT)
			{
				if (dbyte & REPSKIP)
					dbyte = 255;
				else
					dbyte = *src++;

				count -= run;
				memset(dest, dbyte, run);
				dest += run;
			}
			else
			{
				count -= run;
				memcpy(dest, src, run);
				dest += run;
				src += run;
			}
			if (count < 0)
				msgMgr->Fatal("Robot::RLEDecode - RLE data corrupt");
		}
	}
}

void
Robot::LZDecode(uchar* dest, uchar* src, ulong srcCount)
{
	int xdim = xsize;
	int ydim = (ysize * vertScaleFactor) / 100;
	ulong destCount = xdim * ydim;
	Initcompress(lzScratch);

	int result = Decompress((char**) &src, (char**) &dest, &srcCount, &destCount, lzScratch);

//	if (srcCount  ||  result != 1)
//		msgMgr->Fatal(Msg_CompressSource);

	srcCount = 0;
	result = Decompress((char**) &src, (char**) &dest, &srcCount, &destCount, lzScratch);

	if (destCount > 1)
		msgMgr->Fatal(Msg_CompressDestination);
}


void
Robot::ExpandData(uchar* dest, const uchar* src)
{
	int ydim = (ysize * vertScaleFactor) / 100;

	long total = 0;
	int numer = ysize;
	int denom = ydim;

	for (int y = ydim - 1; y >= 0; y--)  {
		total += numer;
		int count = total / denom;
		total %= denom;

		while (count--)  {
			memcpy(dest, src, xsize);
			dest += xsize;
		}
		src += xsize;
	}
}






void
KRobot(argList)
{
	// Robot functions
	//      arg1-----function code
	//					
	enum {
		rStart,		//
		rHasEnded,	//  true only if waiting for handshake
		rExists,		//  true anytime robot started but not terminated
		rTerminate	//  Deletes final cel from graphMgr, clears robot state
						//  Does NOT call frameout.
	};

	// The arguments vary from function to function
	// The return value varys from function to function

	int function = arg(1);

	switch (function) {
		case rStart:
			// Start Robot
			//	-----------
			// Input
			//      arg2-----robot resource number
			//      arg3-----plane ID
			//      arg4-----priority
			//      arg5-----x Offset
			//      arg6-----y Offset
			// Output
			//		  pm.acc = ?

			graphMgr->InitRobot(arg(2), arg(3), arg(4), arg(5), arg(6));
			pm.acc = False;
			break;

		case rHasEnded:
			// return true if waiting for terminating handshake
			//	------------------------------------------------
			// Input
			// 	  None
			// Output
			//		  pm.acc = True / False

			pm.acc = (graphMgr->RobotStatus() == ROBOT_FINAL);
			break;

		case rExists:
			// return whether robot sequence in progress (visible)
			// ---------------------------------------------------
			// Input
			//      None
			// Output
			//      pm.acc = True / False

			pm.acc = (graphMgr->RobotStatus() != ROBOT_UNUSED);
			break;

		case rTerminate:
			// Input
			//      None
			// Output

			graphMgr->KillRobot();
			break;
			
	}
}


int
Robot::Open(ResNum num)
{
	int	fd;
	char	nameBuf[MaxPath + 1];

	// check resource-config file path(s)...
	for (int i = 0; i < configMgr->GetNTokens("robot"); i++)  {
		MakeName(nameBuf, configMgr->Get("robot", i), num);
		if ((fd = ::Open(nameBuf, O_BINARY | O_RDONLY)) != -1)
			return fd;
	}
	return -1;
}


void
Robot::MakeName(char *buf, char* fileSpec, ResNum num)
{
	char	rootName[MaxFName + 1];

	sprintf(rootName, "%u", num);
	MakeName(buf, fileSpec, rootName);
}

void
Robot::MakeName(char* buf, char* fileSpec, char* rootName)
{
	char	drive[MaxDrive + 1];
	char	dir[MaxDir + 1];
	char	ext[MaxExt + 1];
	char	fullName[MaxPath + 1];
	
	//	if there are no wildcard characters in fileSpec, assume it's a dir and
	// slap on a backslash so _splitpath knows it, unless it's just a drive
	//	or the last char is already a backslash
	if (!strchr(fileSpec, '*') && fileSpec[strlen(fileSpec) - 1] != ':' &&
	    fileSpec[strlen(fileSpec) - 1] != '\\') {
		strcpy(fullName, fileSpec);
		strcat(fullName, "\\");
		fileSpec = fullName;
	}
	
	//	get the drive, directory and extension
	_splitpath(fileSpec, drive, dir, 0, ext);
	
	//	and add the name and extension
	_makepath(buf, drive, dir, rootName, "rbt");
}
