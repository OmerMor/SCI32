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
#include "array.hpp"
#include "audio.hpp"
#ifdef WIN32S
#include "shiftpal.hpp"
#endif

Bool		AudioPause(int);
Bool		AudioResume(int);

const PALSIZE = 1200;

_Packed struct VideoHeader
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
	lastFrameNum = -1;
	status = ROBOT_UNUSED;
	lzScratch = New char[SCRATCHRAM_SIZE];
	cueTimeList = New long[256];
	cueValueList = New ushort[256];
	for (int i = 0; i < 256; i++)  {
		cueTimeList[i] = -1;
		cueValueList[i] = 0;
	}
	hasAudio = False;
}

/*
 *   NOTE:   If robot is not deleted when graphMgr destroyed, 
 *   this destructor gets called during graphmgr destructor, which
 *   will reenter a graphmgr method, causing trouble
 */


Robot::~Robot()
{
	delete[] videoSizeArray;
	delete[] lzScratch;
	delete[] cueTimeList;
	delete[] cueValueList;
	delete hunkPalette;
//	bmHandle.Free();
	if (fd)
		Close(fd);

	if (robotSi)
		graphMgr->DeleteScreenItem(robotSi);

	if (hasAudio)
		StopAudioNow();
}


void
Robot::Pause()
{
	if (status == ROBOT_PLAYING)
		status = ROBOT_PAUSED;
}


void
Robot::Resume()
{	
	if (status == ROBOT_PAUSED  ||  status == ROBOT_FINAL)  {
		status = ROBOT_PLAYING;
///		robotStartTime = timeMgr->GetTickCount() - (frameNum * 6);
	}

	if (frameNum >= frameCount)
		status = ROBOT_FINAL;			
}


void
Robot::DoRobot()
{
	if (status != ROBOT_PLAYING)
		return;

	if (firstFrame)
		frameNum = 0;
	else  {
		frameNum = (timeMgr->GetTickCount() + delayTime.PredictedTicks() - robotStartTime) / 6;
	}

	//  If last frame already shown, set final flag and return
	if (frameNum >= frameCount)  {
		status = ROBOT_FINAL;
		if (hasAudio)  {
			StopAudio();
			hasAudio = False;
		}
		return;
	}

	if (frameNum == lastFrameNum)
		return;

	audioList.SubmitDriverMax();
//	delayTime.StartTiming();

	SeekFrame(frameNum);
	if (version == 2)
		DoVersion2();
	else
		DoVersion3();
	audioList.SubmitDriverMax();

	//  Set flag, return ok
	lastFrameNum = frameNum;
}





void
Robot::DoVersion2()
{
	int size = audioSize + videoSizeArray[frameNum] + sizeof(VideoHeader);
	uchar* scratch = New uchar[size];
	Read(fd, scratch, size);

	//  Submit audio portion
	if (hasAudio)  {
		long audCount = *((long*) &scratch[4]);
		long audPos = *((long*) scratch);
		audioList.AddBlock(audPos, audCount);
		memcpy(audioList.LastBlockAddr(), &scratch[8], audCount);
		audioList.SubmitDriverMax();
	}

	//  Init screenItem with values read from disk
	uchar* video = scratch + audioSize;
	CreateCel(video, True);	// HOOK  firstFrame);
	delete[] scratch;

	if (!robotSi)  {
		robotSi = New ScreenItem(planeId, Bitmap(bmHandle));
		robotSi->pos = SOL_Point(x1, y1);
		robotSi->z = priority;
		graphMgr->AddScreenItem(robotSi);
	}
	else  {
		robotSi->bitmap = bmHandle;
		robotSi->pos = SOL_Point(x1, y1);
		robotSi->z = priority;
		graphMgr->UpdateScreenItem(robotSi);
	}
}


void
Robot::DoVersion3()
{
	long videoSize = videoSizeArray[frameNum] + sizeof(VideoHeader);
	uchar* scratch = New uchar[videoSize];
	Read(fd, scratch, videoSize);
	CreateCel(scratch, True);	// HOOK  firstFrame);

	if (!robotSi)  {
		robotSi = New ScreenItem(planeId, Bitmap(bmHandle));
		robotSi->pos = SOL_Point(x1, y1);
		robotSi->z = priority;
		graphMgr->AddScreenItem(robotSi);
	}
	else  {
		robotSi->bitmap = bmHandle;
		robotSi->pos = SOL_Point(x1, y1);
		robotSi->z = priority;
		graphMgr->UpdateScreenItem(robotSi);
	}

	if (hasAudio)  {
		//  Get audio header
		Read(fd, scratch, 8);
		long audCount = *((long*) &scratch[4]);
		long audPos = *((long*) scratch);

		audioList.AddBlock(audPos, audCount);
		Read(fd, audioList.LastBlockAddr(), audCount);
	}
	delete[] scratch;
}




void
Robot::DisplayFrame(int frame, int newXOff, int newYOff, int newPri)
{
	if (frame >= frameCount)
		return;

	//  Update xoff, yoff, priority if override wanted
	if (newXOff != 50000)
		xoff = newXOff;
	if (newYOff != 50000)
		yoff = newYOff;
	if (newPri != 50000)
		priority = newPri;


	frameNum = frame;
	Pause();

	if (frame != lastFrameNum)  {
		SeekFrame(frameNum);
		if (version == 2)
			DoVersion2();
		else
			DoVersion3();
	}
	else  {
		//  In case xoff, yoff was changed this call
		x1 = fileX1 + xoff;
		y1 = fileY1 + yoff;

		if (!robotSi)  {
			robotSi = New ScreenItem(planeId, Bitmap(bmHandle));
			robotSi->pos = SOL_Point(x1, y1);
			robotSi->z = priority;
			graphMgr->AddScreenItem(robotSi);
		}
		else  {
			robotSi->bitmap = bmHandle;
			robotSi->pos = SOL_Point(x1, y1);
			robotSi->z = priority;
			graphMgr->UpdateScreenItem(robotSi);
		}
	}

	lastFrameNum = frameNum;
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

	if (hasAudio)  {
		StopAudioNow();
		hasAudio = False;
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
	if (firstFrame  &&  status == ROBOT_PLAYING)  {
//		robotStartTime = timeMgr->GetMilliSec();
		robotStartTime = timeMgr->GetTickCount();
		AudioResume(-1);
		firstFrame = False;
	}

	if (status == ROBOT_PLAYING)  {
//		delayTime.EndTiming();

		audioList.SubmitDriverMax();
		long ticks = timeMgr->GetTickCount() + 3;
		while (timeMgr->GetTickCount() < ticks) {}
//		msgMgr->Mono("Frame: %d \n", frameNum / 10);
//		msgMgr->Mono("Frame: %d  Time: %d\n", frameNum, (timeMgr->GetTickCount() - robotStartTime) / 6);
//		msgMgr->Dump("Frame: %d  Time: %d\n", frameNum, (robotStartTime) / 6);
	}

}

long
Robot::GetCue()
{
	//  If we are waiting for handshake, return -1
	if (graphMgr->RobotStatus() == ROBOT_FINAL)
		return -1;

	//  Search for cue time <= current frame number
	for (int i = 0; i < 256; i++)  {
		if (cueTimeList[i] != -1  &&  cueTimeList[i] <= frameNum)  {
			cueTimeList[i] = -1;
			return cueValueList[i];
		}
	}

	//  No cues found for this time
	return 0;
}

void
Robot::CreateCel(uchar* video, Bool usePalette)
{
	VideoHeader* vh = (VideoHeader*) video;
	compressType = vh->compressionType;
	vertScaleFactor = vh->vertScaleFactor;
	fileX1 = vh->x1;
	fileY1 = vh->y1;
	x1 = fileX1 + xoff;
	y1 = fileY1 + yoff;
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
#ifdef WIN32S
	if (WING)
		OffsetData(ptr, PALSHIFT);
#endif
	if (vertScaleFactor != 100)  {
		ExpandData((uchar*) *bmHandle + CELHEADERSIZE, ptr);
		temp.Free();
	}

	if (usePalette)  {
		memcpy((uchar*) *bmHandle + CELHEADERSIZE + (xsize * ysize), hunkPalette, PALSIZE);
	}
}



void
Robot::Init()
{
	uchar type, scrap;
	char buffer[4];
	char junk[8];
	short fcount, asize, vsize1, vsize2;
	ushort palSize, audPrimerSize;
	char palFlag, audFlag;

_Packed	struct AudPrimerHdr
	{
		long	totalSampleCount;
		short	compressionType;
		long	evenSize;
		long	oddSize;
	};

	fd = Open(resNum);
	if (critErrHandler->Tripped() ||  fd < 1)
		msgMgr->Fatal("Could not open Robot resource %d\n", resNum);

	firstFrame = True;
	lastFrameNum = -1;
	frameNum = 0;
	status = ROBOT_PAUSED;

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
	hasAudio = audFlag;
	vPadSize[0] = vsize1;
	vPadSize[1] = vsize2;
	frameCount = fcount;

	if (type != 0x3d  ||  strcmp(buffer, "SOL"))	
		msgMgr->Fatal("Resource is not Robot type!");

	if (!(version == 2  ||  version == 3))
		msgMgr->Fatal("Unsupported version of Robot resource");

	if (!hasAudio)  {
		LSeek(fd, audPrimerSize, SEEK_CUR);
	}
	else  {
		if (audPrimerSize)  {
			AudPrimerHdr audPrimerHdr;
			Read(fd, &audPrimerHdr, 14);

			if (audPrimerHdr.compressionType)
				msgMgr->Fatal("Unknown audio header compression type");

			if (audPrimerHdr.totalSampleCount)  {
				audioList.AddBlock(0, audPrimerHdr.evenSize);
				Read(fd, audioList.LastBlockAddr(), audPrimerHdr.evenSize);

				audioList.AddBlock(1, audPrimerHdr.oddSize);
				Read(fd, audioList.LastBlockAddr(), audPrimerHdr.oddSize);
			}
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

	Read(fd, videoSizeArray, sizeof(*videoSizeArray) * frameCount);
	LSeek(fd, 8 * frameCount, SEEK_CUR);
	if (version == 3)  {
		Read(fd, cueTimeList, sizeof(*cueTimeList) * 256);
		Read(fd, cueValueList, sizeof(*cueValueList) * 256);
	}

	long pos = LSeek(fd, 0, SEEK_CUR);
	int remain = pos % 2048;
	if (remain)
		LSeek(fd, 2048 - remain, SEEK_CUR);
	fileDataStart = LSeek(fd, 0, SEEK_CUR);
	AudioPause(-1);
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
Robot::OffsetData(uchar* ptr, char offset)
{
	int ydim = (ysize * vertScaleFactor) / 100;
	for (int y = 0; y < ydim; y++)  {
		for (int x = 0; x < xsize; x++) {
			if (*ptr < 246)
				*ptr += offset;
			ptr++;
		}
	}
}


int
Robot::Info(SOL_Rect* rect)
{
	if (robotSi)  {
		long i = graphMgr->NextScreen()->Planes().Search(robotSi->PlaneId());
		robotSi->NowSeen(graphMgr->NextScreen()->Planes()[i], *rect);
	}
	
	return frameCount;
}



void
KRobot(argList)
{
	// Robot functions
	//      arg1-----function code
	//					
	enum {
		rOpen,
		rDisplayFrame,
		rFrameInfo,
		rSaveOffset,
		rPlay,
		rHasEnded,	//  true only if waiting for handshake
		rExists,		//  true anytime robot started but not terminated
		rTerminate,	//  Deletes final cel from graphMgr, clears robot state
						//  Does NOT call frameout.
      rGetCue     //  returns cues to Robot SCI object
	};

	// The arguments vary from function to function
	// The return value varys from function to function

	int function = arg(1);
	SOL_Rect rect;
	SCIWord* data;

	switch (function) {
		case rOpen:
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

		case rDisplayFrame:
			if (argCount < 3)
				graphMgr->GRobot().DisplayFrame(arg(2));
			else
				graphMgr->GRobot().DisplayFrame(arg(2), arg(3), arg(4));
			break;

		case rFrameInfo:
			// Input -
			//              arg(1) = ArrayID to array for rect

			pm.acc = graphMgr->GRobot().Info(&rect);
			data = (SCIWord*) ((ArrayID) arg(2)).Data();
			data[0] = rect.A.x;
			data[1] = rect.A.y;
			data[2] = rect.B.x;
			data[3] = rect.B.y;
			break;


		case rPlay:
			graphMgr->GRobot().Resume();
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

		case rGetCue: {
			// Input
			//      arg2-----robot object
			// Output
         //    cue values returned
         //       0  no ques and not at end of play
         //       -1 play over last frame still on screen
         //       que value of oldest cue encountered that has not been returned 

	      ObjectID robotObject = (ObjectID) arg(2);
			robotObject.SetIndexedProperty(sndSignal, (Property) graphMgr->GRobot().GetCue());
			break;
		}

		default:
			msgMgr->Alert("Robot Kernel function not implemented");
			break;
	}
}

void
Robot::SeekFrame(int frame)
{
	//  Calculate where frame is in file and seek there
	int oddCount = frame >> 1;
	int evenCount = oddCount;
	if (frame & 1)
		evenCount++;

	int evenSize = (audioSize + vPadSize[0]) * evenCount;
	int oddSize = (audioSize + vPadSize[1]) * oddCount;
	long posStart = fileDataStart + oddSize + evenSize;
	LSeek(fd, posStart, SEEK_SET);
}

int
Robot::Open(ResNum num)
{
	int	fd;
	char	nameBuf[MaxPath + 1];

	// check resource-config file path(s)...
	if (!configMgr->GetNTokens("robot"))  {
		MakeName(nameBuf, ".\\", num);
		return ::Open(nameBuf, O_BINARY | O_RDONLY);
	}
	else  {
		for (int i = 0; i < configMgr->GetNTokens("robot"); i++)  {
			MakeName(nameBuf, configMgr->Get("robot", i), num);
			if ((fd = ::Open(nameBuf, O_BINARY | O_RDONLY)) != -1)
				return fd;
		}
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

void
Robot::StopAudio()
{
	AudRobot(RobotAudInfo(NULL, -1, -1));
}

void
Robot::StopAudioNow()
{
	AudRobot(RobotAudInfo(NULL, 0, 0));
}

Robot::AudioList::AudioBlock::AudioBlock(int pos, int size) : pos(pos), size(size)
{
	memid.Get(MemResAudio, size);
	memid.SetNotDiscardable();
}


Robot::AudioList::AudioBlock::~AudioBlock()
{
	memid.Free();
}

Bool
Robot::AudioList::AudioBlock::Submit()
{
//	msgMgr->Mono("pos: %d  len: %d\n", pos*2, size);
	return AudRobot(RobotAudInfo((char*) *memid, size, pos*2));
}
		
uchar*
Robot::AudioList::AudioBlock::operator*()
{
	return (uchar*) *memid;
}


Robot::AudioList::AudioList()
{
	for (int i = 0; i < 10; i++)
		array[i] = NULL;
	oldestBlockIndex = 0;
	newestBlockIndex = 0;
	blockCount = 0;
}


Robot::AudioList::~AudioList()
{
	for (int i = 0; i < 10; i++)
		delete array[i];
}



Bool
Robot::AudioList::IsEmpty() const
{
	return blockCount;
}


void
Robot::AudioList::AddBlock(int pos, int size)
{
	if (blockCount == 10)  {
		msgMgr->Mono("*"); //Fatal("Robot AudioList full!");
		delete array[oldestBlockIndex];
		array[oldestBlockIndex] = NULL;
		oldestBlockIndex++;
		if (oldestBlockIndex == 10)
			oldestBlockIndex = 0;
		blockCount--;
	}

	msgMgr->Mono("+");

	//  If first block, init variables properly
	if (!blockCount)
		oldestBlockIndex = newestBlockIndex = 0;
	else  {
		newestBlockIndex++;
		if (newestBlockIndex == 10)
			newestBlockIndex = 0;
	}

	array[newestBlockIndex] = New AudioBlock(pos, size);
	blockCount++;
}


uchar*
Robot::AudioList::LastBlockAddr()
{
	return **array[newestBlockIndex];
}



void
Robot::AudioList::SubmitDriverMax()
{
	while (blockCount)  {
		if (!array[oldestBlockIndex]->Submit())  {
			msgMgr->Mono(".");
			return;
		}

		delete array[oldestBlockIndex];
		array[oldestBlockIndex] = NULL;
		oldestBlockIndex++;
		if (oldestBlockIndex == 10)
			oldestBlockIndex = 0;

		blockCount--;
		msgMgr->Mono("_");
	}
}

Robot::DelayTime::DelayTime()
{
	for (int i = 0; i < 10; i++)  {
		timeStamp[i] = i;
		delays[i] = 0;
	}

	earliestTimeStamp = 0;
	latestTimeStamp = 9;
	startTime = -1;
}

void
Robot::DelayTime::StartTiming()
{
	startTime = timeMgr->GetTickCount();
}

void
Robot::DelayTime::EndTiming()
{
	if (startTime == -1)
		return;
		
	int time = timeMgr->GetTickCount() - startTime;
	for (int i = 0; i < 10; i++)  {
		if (timeStamp[i] == earliestTimeStamp)  {
			timeStamp[i] = ++latestTimeStamp;
			delays[i] = time;
			break;
		}
	}
	earliestTimeStamp++;
	SortList();
	startTime = -1;
//	msgMgr->Mono("Time: %d\n", PredictedTicks());
}


void
Robot::DelayTime::SortList()
{
	for (int i = 0; i < 9; i++)  {
		ulong smallDelay = delays[i];
		int smallIndex = i;

		//  Search for smallest remaining in list
		for (int j = i + 1; j < 10; j++)  {
			if (delays[j] < smallDelay)  {
				smallDelay = delays[j];
				smallIndex = j;
			}
		}

		//  If smaller found, swap it to the top of the list
		if (smallIndex != i)  {
			ulong tmpDelay = delays[i];
			int tmpTimeStamp = timeStamp[i];

			delays[i] = smallDelay;
			timeStamp[i] = timeStamp[smallIndex];

			delays[smallIndex] = tmpDelay;
			timeStamp[smallIndex] = tmpTimeStamp;
		}
	}
}


ulong
Robot::DelayTime::PredictedTicks()
{
	return delays[5];
}
