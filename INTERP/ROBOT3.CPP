#include "sol.hpp"
#include "robot.hpp"

#include "array.hpp"
#include "audio.hpp"
#include "config.hpp"
#include "criterr.hpp"
#include "dcomp.hpp"
#include "dos.hpp"
#include "graph.hpp"
#include "graphmgr.hpp"
#include "kernel.hpp"
#include "pmachine.hpp"
#include "resource.hpp"
#include "shiftpal.hpp"
#include "time.hpp"
#include "event.hpp"


// For now - move to MSG.CPP when available

void AssertMsgFail(const char* filename, int line, const char* str);
#define DebugPrintf

#ifdef DEBUG
#define	assertMsg(exp, str) \
	if (exp) {}				\
	else						\
	AssertMsgFail(__FILE__, __LINE__, str)

#else
#define	assertMsg(exp, str)
#endif



//#define DANTEST1

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

_Packed struct VideoHeader4
{
	uchar	compressionType;
	uchar	vertScaleFactor;
	short	xsize, ysize;
	short	xoff, yoff;
	short	x1, y1;
	short	size;
	short	checksum;
	long	unused;
};

enum CompType {LZ_COMP, RLE_COMP, UNCOMP};


Robot::Robot(int resNum, long planeId, int pri, int xoff, int yoff) :
	resNum(resNum),
	planeId(planeId), 
	xoff(xoff),
	yoff(yoff)
{
	for (int i = 0; i < SiArraySize; i++)  {
		robotSi[i] = NULL;
		priority[i] = pri;
	}
	siCount = 0;
	framesSkipped = 0;
	maxFramesSeqSkipped = 0;
	fd = 0;
	cueForceShowFrame = -1;

	hunkPalette = NULL;
	videoSizeArray = NULL;
	recordPosArray = NULL;
	firstFrame = True;
	lastFrameNum = -1;
	status = ROBOT_UNUSED;
	lzScratch = New char[SCRATCHRAM_SIZE];
	cueTimeList = New long[256];
	cueValueList = New ushort[256];
	for (i = 0; i < 256; i++)  {
		cueTimeList[i] = -1;
		cueValueList[i] = 0;
	}
	hasAudio = False;
	displayFrameCalled = False;
}

/*
 *   NOTE:   If robot is not deleted when graphMgr destroyed, 
 *   this destructor gets called during graphmgr destructor, which
 *   will reenter a graphmgr method, causing trouble
 */


Robot::~Robot()
{
	delete[] videoSizeArray;
	delete[]	recordPosArray;
	delete[] lzScratch;
	delete[] cueTimeList;
	delete[] cueValueList;
	delete hunkPalette;
	for (int i = 0; i < siCount; i++)
		bmHandle[i].Free();
	status = ROBOT_UNUSED;					//  in case calls made to deleted robot
	if (fd)
		resMgr->Close(fd);

	for (i = 0; i < siCount; i++)
		graphMgr->DeleteScreenItem(robotSi[i]);

	if (hasAudio)
		audioList.StopAudioNow();
}





void
Robot::Reset(int resNum1, long planeId1, int pri, int xoff1, int yoff1)
{
	resNum = resNum1,
	planeId = planeId1; 
	xoff = xoff1;
	yoff = yoff1;

	delete[] videoSizeArray;
	delete[]	recordPosArray;
	delete[] lzScratch;
	delete[] cueTimeList;
	delete[] cueValueList;
	delete hunkPalette;

	for (int i = 0; i < siCount; i++)
		bmHandle[i].Free();
	if (fd)
		resMgr->Close(fd);

	for (i = 0; i < siCount; i++)
		graphMgr->DeleteScreenItem(robotSi[i]);

	if (hasAudio)
		audioList.StopAudioNow();

	for (i = 0; i < SiArraySize; i++)  {
		robotSi[i] = NULL;
		priority[i] = pri;
	}
	siCount = 0;
	framesSkipped = 0;
	maxFramesSeqSkipped = 0;
	fd = 0;
	cueForceShowFrame = -1;

	hunkPalette = NULL;
	videoSizeArray = NULL;
	recordPosArray = NULL;
	firstFrame = True;
	lastFrameNum = -1;
	status = ROBOT_UNUSED;
	lzScratch = New char[SCRATCHRAM_SIZE];
	cueTimeList = New long[256];
	cueValueList = New ushort[256];
	for (i = 0; i < 256; i++)  {
		cueTimeList[i] = -1;
		cueValueList[i] = 0;
	}
	hasAudio = False;
	displayFrameCalled = False;

	delayTime.Reset();
	audioList.Reset();
	Init();
}




/*
 *   NOTE:   If robot is not deleted when graphMgr destroyed, 
 *   this destructor gets called during graphmgr destructor, which
 *   will reenter a graphmgr method, causing trouble
 */










long
Robot::TickCount() const
{
//	return (timeMgr->GetTickCount() * 11025) / 11198;
	return timeMgr->GetTickCount();
}


//
//  Note that pause/resume will not do anything if we are at FINAL
//
void
Robot::Pause()
{
	assertMsg(status != ROBOT_UNUSED, "Robot::Pause - Called when robot uninitialized");

	if (status == ROBOT_PLAYING)  {
		audioList.StopAudioNow();
		status = ROBOT_PAUSED;
	}
}



//
//  Note that resume/pause will not do anything if we are at FINAL
//

void
Robot::Resume()
{	
	assertMsg(status != ROBOT_UNUSED, "Robot::Resume - Called when robot uninitialized");

	if (status == ROBOT_PAUSED)  {
		status = ROBOT_PLAYING;

		//  NOTE:  even though robotStartTime will be updated to when the
		//  robot shows up on the screen, I must have this set to a reasonable
		//  value while the status == ROBOT_PLAYING (for cues).

		if (!firstFrame)
			robotStartTime = TickCount() - ToTicks(frameNum);
	}
}


void
Robot::DoRobot()
{
	assertMsg(status != ROBOT_UNUSED, "Robot::DoRobot - Called when robot uninitialized");
	AsyncEventCheck();

	if (status != ROBOT_PLAYING)
		return;

	if (firstFrame)
		frameNum = 0;
	else if (cueForceShowFrame != -1)  {
		frameNum = cueForceShowFrame;
		cueForceShowFrame = -1;
	}
	else  {
		frameNum = ToFrame(TickCount() + delayTime.PredictedTicks() - robotStartTime);
//		frameNum++;
	}

	//  If last frame already shown, set final flag and return
	if (frameNum >= frameCount)  {
		if (lastFrameNum == frameCount - 1)  {
			status = ROBOT_FINAL;
			if (hasAudio)  {
				audioList.StopAudio();
				hasAudio = False;
			}
			return;
		}
		else
			frameNum = frameCount - 1;
	}

	//  If desired frame already being animated, return
	if (frameNum == lastFrameNum)
		return;

	if (version >= 4  &&  hasAudio)  {
		uchar scratch[20];
		for (long tmp = lastFrameNum + canSkip + 1; tmp < frameNum; tmp += canSkip + 1)  {
			LSeek(fd, recordPosArray[tmp] + videoSizeArray[tmp], SEEK_SET);
			audioList.SubmitDriverMax();
			Read(fd, scratch, 8);
			long audCount = *((long*) &scratch[4]);
			long audPos = *((long*) scratch);

			audioList.AddBlock(audPos, audCount);
			Read(fd, audioList.LastBlockAddr(), audCount);
		}
	}

	audioList.SubmitDriverMax();
	delayTime.StartTiming();
	SeekFrame(frameNum);
	if (version == 2)
		DoVersion2();
	else if (version == 3)
		DoVersion3();
	else if (version == 4)
		DoVersion4();
	else if (version == 5)
		DoVersion5();
	else
		msgMgr->Fatal("Robot Version %d not supported in this interpreter", version);

	audioList.SubmitDriverMax();

	//  Set flag, return ok
	if (frameNum != lastFrameNum + 1)  {
		int skipped = frameNum - lastFrameNum - 1;
		framesSkipped += skipped;
		if (skipped > maxFramesSeqSkipped)
			maxFramesSeqSkipped = skipped;
	}

	lastFrameNum = frameNum;
}




void
Robot::DoVersion2()
{
	int size = audioSize + videoSizeArray[frameNum] + sizeof(VideoHeader);
	uchar* scratch = New uchar[size];
	Read(fd, scratch, size);
	siCount = 1;

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

	if (!robotSi[0])  {
		robotSi[0] = New ScreenItem(planeId, Bitmap(bmHandle[0]));
		robotSi[0]->pos = SOL_Point(x1[0], y1[0]);
		if (priority[0] == -1)
			robotSi[0]->SetFloatPriority();
		else
			robotSi[0]->SetPriority(priority[0]);
			
		graphMgr->AddScreenItem(robotSi[0]);
	}
	else  {
		robotSi[0]->bitmap = bmHandle[0];
		robotSi[0]->pos = SOL_Point(x1[0], y1[0]);
		if (priority[0] == -1)
			robotSi[0]->SetFloatPriority();
		else
			robotSi[0]->SetPriority(priority[0]);
		graphMgr->UpdateScreenItem(robotSi[0]);
	}
}


void
Robot::DoVersion3()
{
	long videoSize = videoSizeArray[frameNum] + sizeof(VideoHeader);
	uchar* scratch = New uchar[videoSize];
	Read(fd, scratch, videoSize);
	siCount = 1;
	CreateCel(scratch, True);	// HOOK  firstFrame);


	if (!robotSi[0])  {
		robotSi[0] = New ScreenItem(planeId, Bitmap(bmHandle[0]));
		robotSi[0]->pos = SOL_Point(x1[0], y1[0]);
		if (priority[0] == -1)
			robotSi[0]->SetFloatPriority();
		else
			robotSi[0]->SetPriority(priority[0]);
		graphMgr->AddScreenItem(robotSi[0]);
	}
	else  {
		robotSi[0]->bitmap = bmHandle[0];
		robotSi[0]->pos = SOL_Point(x1[0], y1[0]);
		if (priority[0] == -1)
			robotSi[0]->SetFloatPriority();
		else
			robotSi[0]->SetPriority(priority[0]);
		graphMgr->UpdateScreenItem(robotSi[0]);
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
Robot::DoVersion4()
{
	int oldSiCount = siCount;
	ushort videoSize = videoSizeArray[frameNum];
	uchar* scratch = New uchar[videoSize];
	Read(fd, scratch, videoSize);
	siCount = * (short*) scratch;
	CreateCel4(scratch + 2, siCount, True);	// HOOK  firstFrame);

	for (int i = 0; i < siCount; i++)  {
		if (!robotSi[i])  {
			robotSi[i] = New ScreenItem(planeId, Bitmap(bmHandle[i]));
			robotSi[i]->pos = SOL_Point(x1[i], y1[i]);
			if (priority[i] == -1)
				robotSi[i]->SetFloatPriority();
			else
				robotSi[i]->SetPriority(priority[i]);
			graphMgr->AddScreenItem(robotSi[i]);
		}
		else  {
			robotSi[i]->bitmap = bmHandle[i];
			robotSi[i]->pos = SOL_Point(x1[i], y1[i]);
			if (priority[i] == -1)
				robotSi[i]->SetFloatPriority();
			else
				robotSi[i]->SetPriority(priority[i]);
			graphMgr->UpdateScreenItem(robotSi[i]);
		}
	}

	for (i = siCount; i < oldSiCount; i++)  {
		graphMgr->DeleteScreenItem(robotSi[i]);
		robotSi[i] = NULL;
		bmHandle[i].Free();
	}

	if (hasAudio)  {
		//  Get audio header
		//  Audio pad **************************************************
		Read(fd, scratch, 8);
		long audCount = *((long*) &scratch[4]);
		long audPos = *((long*) scratch);

		audioList.AddBlock(audPos, audCount);
		Read(fd, audioList.LastBlockAddr(), audCount);
	}
	delete[] scratch;
}




void
Robot::DoVersion5()
{
	int oldSiCount = siCount;
	ushort videoSize = videoSizeArray[frameNum];
	uchar* scratch = New uchar[videoSize];
	Read(fd, scratch, videoSize);
	siCount = * (short*) scratch;

	if (hasAudio)  {
		//  Get audio header
		uchar audioTmp[16];
		Read(fd, audioTmp, 8);
		long audCount = *((long*) &audioTmp[4]);
		long audPos = *((long*) audioTmp);

		audioList.AddBlock(audPos, audCount);
		Read(fd, audioList.LastBlockAddr(), audCount);
	}

	CreateCel5(scratch + 2, siCount, True);	// HOOK  firstFrame);

	for (int i = 0; i < siCount; i++)  {
		if (!robotSi[i])  {
			robotSi[i] = New ScreenItem(planeId, Bitmap(bmHandle[i]));
			robotSi[i]->pos = SOL_Point(x1[i], y1[i]);
			if (priority[i] == -1)
				robotSi[i]->SetFloatPriority();
			else
				robotSi[i]->SetPriority(priority[i]);
			graphMgr->AddScreenItem(robotSi[i]);
		}
		else  {
			robotSi[i]->bitmap = bmHandle[i];
			robotSi[i]->pos = SOL_Point(x1[i], y1[i]);
			if (priority[i] == -1)
				robotSi[i]->SetFloatPriority();
			else
				robotSi[i]->SetPriority(priority[i]);
			graphMgr->UpdateScreenItem(robotSi[i]);
		}
	}

	for (i = siCount; i < oldSiCount; i++)  {
		graphMgr->DeleteScreenItem(robotSi[i]);
		robotSi[i] = NULL;
		bmHandle[i].Free();
	}

	delete[] scratch;
}


void
Robot::DisplayFrame(int frame, int newXOff, int newYOff, int newPri)
{
	assertMsg(status != ROBOT_UNUSED, "Robot::DisplayFrame - Called when robot uninitialized");
	assertMsg(frame < frameCount, "Robot::DisplayFrame - Frame out of range!\n");

	displayFrameCalled = True;

	//  Update xoff, yoff, priority if override wanted
	if (newXOff != 50000)
		xoff = newXOff;
	if (newYOff != 50000)
		yoff = newYOff;
	if (newPri != 50000)  {
		for (int i = 0; i < SiArraySize; i++)
			priority[i] = newPri;
	}

	frameNum = frame;
	Pause();

	if (frame != lastFrameNum)  {
		SeekFrame(frameNum);
		if (version == 2)
			DoVersion2();
		else if (version == 3)
			DoVersion3();
		else if (version == 4)
			DoVersion4();
		else
			DoVersion5();
	}
	else  {
		//  In case xoff, yoff was changed this call
		for (int i = 0; i < siCount; i++)  {
			if (!accessType)  {
		 		x1[i] = fileX1[i] + xoff;
				y1[i] = fileY1[i] + yoff;
			}
			else  {
				CelHeader* celHeader = (CelHeader*) *bmHandle[i];
				xsize = celHeader->xDim;
				ysize = celHeader->yDim;

				int gameResX = graphMgr->Xdim();
				int gameResY = graphMgr->Ydim();
				int highResX1 = fileX1[i] + ((xoff * gameResX) / LOWRESX);
				int highResY1 = fileY1[i] + ((yoff * gameResY) / LOWRESY);
				int highResY2 = highResY1 + ysize - 1;

				int lowResX1 = (highResX1 * LOWRESX) / gameResX;
				int lowResY1 = (highResY1 * LOWRESY) / gameResY;
				int lowResY2 = (highResY2 * LOWRESY) / gameResY;

				int xorg = (highResX1 - ((lowResX1 * 63) / 32)) * -1;
				int yorg = ((lowResY2 * 9) / 4) - highResY1;
				celHeader->xHot = xorg;
				celHeader->yHot = yorg;
				x1[i] = lowResX1;
				y1[i] = lowResY2;
			}


			if (!robotSi[i])  {
				robotSi[i] = New ScreenItem(planeId, Bitmap(bmHandle[i]));
				robotSi[i]->pos = SOL_Point(x1[i], y1[i]);
				if (priority[i] == -1)
					robotSi[i]->SetFloatPriority();
				else
					robotSi[i]->SetPriority(priority[i]);
				graphMgr->AddScreenItem(robotSi[i]);
			}
			else  {
				robotSi[i]->bitmap = bmHandle[i];
				robotSi[i]->pos = SOL_Point(x1[i], y1[i]);
				if (priority[i] == -1)
					robotSi[i]->SetFloatPriority();
				else
					robotSi[i]->SetPriority(priority[i]);
				graphMgr->UpdateScreenItem(robotSi[i]);
			}
		}
	}

	lastFrameNum = frameNum;
}


void
Robot::KillRobot()
{
	if (fd)  {
		resMgr->Close(fd);
		fd = 0;
	}

	for (int i = 0; i < siCount; i++)  {
		graphMgr->DeleteScreenItem(robotSi[i]);
		robotSi[i] = NULL;
	}
	siCount = 0;

	if (hasAudio)  {
		audioList.StopAudioNow();
		hasAudio = False;
	}
}


void
Robot::FrameAlmostVisible()
{
	AsyncEventCheck();
	if (!firstFrame)  {
		//  Do delay if we are ahead of schedule
	}
}


void
Robot::FrameNowVisible()
{
	if (firstFrame  &&  status == ROBOT_PLAYING)  {

		//  BUGBUG - DF - 5-16-95  - Helps to avoid audio bug after pic loaded
		char scratch;							//	b
		Read(fd, &scratch, 1);				//	u
		LSeek(fd, -1L, SEEK_CUR);			//	g


		robotStartTime = TickCount();
		AudioResume(-3);						//  Robot audio resume
		firstFrame = False;
	}

	if (status == ROBOT_PLAYING)  {
		if (delayTime.TimingInProgress())
			delayTime.EndTiming();

		audioList.SubmitDriverMax();
		
		long ticks = TickCount() + 3;
		while (TickCount() < ticks) {}
	}

	AsyncEventCheck();
}

long
Robot::GetCue()
{
	//  BUGBUG - DF - 4/27/95
	//  This if statement compensates for bad timing code in Phantasmagoria#1
	//  It should become a Warning or Fatal message that stops game play

	if (graphMgr->RobotStatus() == ROBOT_UNUSED)
		return 0;

	//  BUGBUG - DF - 4/27/95
	//  Same as above - wait till Phantasmagoria ships
	if (graphMgr->RobotStatus() == ROBOT_PAUSED)
		return 0;

	//  If we are waiting for handshake, return -1
	if (graphMgr->RobotStatus() == ROBOT_FINAL)
		return -1;

	//  Nothing displayed yet, so ignore cue request
	if (firstFrame)
		return 0;

	//  Search for cue time <= estimated next current frame number
	int timexx = TickCount();
	int estNextFrameNum = ToFrame(timexx + delayTime.PredictedTicks() - robotStartTime);

	assert(estNextFrameNum >= 0);
	assert(frameCount > 0);

	if (estNextFrameNum >= frameCount)
		estNextFrameNum = frameCount - 1;

	//  Cannot have a cue on the last frame, because we force the next frame
	//  to show.  This condition should be screened during init.
	//  Also, how does cueTimeList work on non-10-per-sec robots?

	for (int i = 0; i < 256; i++)  {
		if (cueTimeList[i] != -1  &&  cueTimeList[i] <= estNextFrameNum)  {
			if (cueTimeList[i] >= lastFrameNum)
				cueForceShowFrame = cueTimeList[i] + 1;

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
	fileX1[0] = vh->x1;
	fileY1[0] = vh->y1;
	x1[0] = fileX1[0] + xoff;
	y1[0] = fileY1[0] + yoff;
	xsize = vh->xsize;
	ysize = vh->ysize;

	if (vh->celCount != 1)
		msgMgr->Fatal("Robot has %d cels in record!\n", (int) vh->celCount);

	bmHandle[0].Free();
	bmHandle[0].Get(MemBitmap, xsize * ysize + CELHEADERSIZE + PALSIZE);
	BuildHeader(bmHandle[0], xsize, ysize, 255, vh->xoff, vh->yoff, xRes, yRes, usePalette);

	MemID temp;
	uchar* ptr = NULL;
	if (vertScaleFactor == 100)
		ptr = (uchar*) *bmHandle[0] + CELHEADERSIZE;
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
	if (WING)
		OffsetData(ptr, PALSHIFT);

	if (vertScaleFactor != 100)  {
		ExpandData((uchar*) *bmHandle[0] + CELHEADERSIZE, ptr, 0);
		temp.Free();
	}

	if (usePalette)  {
		memcpy((uchar*) *bmHandle[0] + CELHEADERSIZE + (xsize * ysize), hunkPalette, PALSIZE);
	}
}

void
Robot::CreateCel4(uchar* video, int celCount, Bool usePalette)
{
	for (int i = 0; i < celCount; i++)  {
		Create1Cel4(video, i, usePalette);
		VideoHeader4* vh = (VideoHeader4*) video;
		int size = vh->size;
		video += sizeof(VideoHeader4);
		video += size;
	}
}


void
Robot::Create1Cel4(uchar* video, int index, Bool usePalette)
{
	assert(video);

	VideoHeader4* vh = (VideoHeader4*) video;
	compressType = vh->compressionType;
	vertScaleFactor = vh->vertScaleFactor;

	xsize = vh->xsize;
	ysize = vh->ysize;
	int xorg, yorg;

	if (accessType == 1)  {
		int gameResX = graphMgr->Xdim();
		int gameResY = graphMgr->Ydim();
		int highResX1 = vh->x1 + ((xoff * gameResX) / LOWRESX);
		int highResY1 = vh->y1 + ((yoff * gameResY) / LOWRESY);
		int highResY2 = highResY1 + ysize - 1;
	
		int lowResX1 = (highResX1 * LOWRESX) / gameResX;
		int lowResY1 = (highResY1 * LOWRESY) / gameResY;
		int lowResY2 = (highResY2 * LOWRESY) / gameResY;

		xorg = (highResX1 - ((lowResX1 * gameResX) / LOWRESX)) * -1;
		yorg = ((lowResY2 * gameResY) / LOWRESY) - highResY1;
		x1[index] = lowResX1;
		y1[index] = lowResY2;
		fileX1[index] = vh->x1;
		fileY1[index] = vh->y1;
	}
	else  {
		xorg = vh->xoff;
		yorg = vh->yoff;
		fileX1[index] = vh->x1;
		fileY1[index] = vh->y1;
		x1[index] = fileX1[index] + xoff;
		y1[index] = fileY1[index] + yoff;
	}
 

	//  Do checksum to verify data integrity
	uchar* vidptr = video + sizeof(VideoHeader4);
	short checksum = 0;
	for (int i = 0; i < vh->size; i++)
		checksum ^= *vidptr++;
	if (checksum != vh->checksum)
		msgMgr->Fatal("Checksum error!");	// ***************************************************************

	bmHandle[index].Free();
	bmHandle[index].Get(MemBitmap, xsize * ysize + CELHEADERSIZE + PALSIZE);
	BuildHeader(bmHandle[index], xsize, ysize, 255, xorg, yorg, xRes, yRes, usePalette);

	MemID temp;
	uchar* ptr = NULL;
	if (vertScaleFactor == 100)
		ptr = (uchar*) *bmHandle[index] + CELHEADERSIZE;
	else  {
		temp.Get(MemBitmap, xsize * ((ysize * vertScaleFactor) / 100));
 		ptr = (uchar*) *temp;
	}

	switch (compressType)  {
		case LZ_COMP:
			LZDecode(ptr, video + sizeof(VideoHeader4), vh->size);
			break;

		case UNCOMP:
			memcpy(ptr, video + sizeof(VideoHeader4), vh->xsize * vh->ysize);
			break;

		default:
			msgMgr->Fatal("Unknown compression type: %d!\n", compressType);
	}

	if (WING)
		OffsetData(ptr, PALSHIFT);

	if (vertScaleFactor != 100)  {
		ExpandData((uchar*) *bmHandle[index] + CELHEADERSIZE, ptr, 0);
		temp.Free();
	}

	if (usePalette)  {
		memcpy((uchar*) *bmHandle[index] + CELHEADERSIZE + (xsize * ysize), hunkPalette, PALSIZE);
	}
}

void
Robot::CreateCel5(uchar* video, int celCount, Bool usePalette)
{
	for (int i = 0; i < celCount; i++)  {
		Create1Cel5(video, i, usePalette);
		VideoHeader4* vh = (VideoHeader4*) video;
		int size = vh->size;
		video += sizeof(VideoHeader4);
		video += size;
	}
}


void
Robot::Create1Cel5(uchar* video, int index, Bool usePalette)
{
	assert(video);

	VideoHeader4* vh = (VideoHeader4*) video;
	video += sizeof(VideoHeader4);
	vertScaleFactor = vh->vertScaleFactor;

	xsize = vh->xsize;
	ysize = vh->ysize;
	int xorg, yorg;

	int gameResX = graphMgr->Xdim();
	int gameResY = graphMgr->Ydim();
	int highResX1 = vh->x1 + ((xoff * gameResX) / LOWRESX);
	int highResY1 = vh->y1 + ((yoff * gameResY) / LOWRESY);
	int highResY2 = highResY1 + ysize - 1;

	int lowResX1 = (highResX1 * LOWRESX) / gameResX;
	int lowResY1 = (highResY1 * LOWRESY) / gameResY;
	int lowResY2 = (highResY2 * LOWRESY) / gameResY;

	xorg = (highResX1 - ((lowResX1 * gameResX) / LOWRESX)) * -1;
	yorg = ((lowResY2 * gameResY) / LOWRESY) - highResY1;
	x1[index] = lowResX1;
	y1[index] = lowResY2;
	fileX1[index] = vh->x1;
	fileY1[index] = vh->y1;

	bmHandle[index].Free();
	bmHandle[index].Get(MemBitmap, xsize * ysize + CELHEADERSIZE + PALSIZE);
	BuildHeader(bmHandle[index], xsize, ysize, 255, xorg, yorg, xRes, yRes, usePalette);

	MemID temp;
	uchar* destBuff = NULL;
	if (vertScaleFactor == 100)
		destBuff = (uchar*) *bmHandle[index] + CELHEADERSIZE;
	else  {
		temp.Get(MemBitmap, xsize * ((ysize * vertScaleFactor) / 100));
 		destBuff = (uchar*) *temp;
	}

	//  Loop through proper number of sub-blocks
	uchar* ptr = destBuff;
	for (int i = 0; i < vh->checksum; i++)  {
		long	cmpSize = * (long*) &video[0];
		long	destSize = * (long*) &video[4];
		short compType = * (short*) &video[8];

	#ifdef DANTEST1
		short checksum2 = * (short*) &video[10];
		short checksum = 0;
		video += 12;
		uchar* vidptr = video;
		for (int i = 0; i < cmpSize; i++)
			checksum ^= *vidptr++;
		if (checksum != checksum2)
			msgMgr->Fatal("Checksum error!");
	#else
		video += 10;
	#endif


		switch (compType)  {
			case LZ_COMP:
				LZDecode(destBuff, video, cmpSize, destSize);
				break;

			case UNCOMP:
				memcpy(destBuff, video, destSize);
				break;
				
			default:
				msgMgr->Fatal("Unknown compression type!");
				break;
		}
		
		video += cmpSize;
		destBuff += destSize;
	}


	if (WING)
		OffsetData(ptr, PALSHIFT);

	if (vertScaleFactor != 100)  {
		ExpandData((uchar*) *bmHandle[index] + CELHEADERSIZE, ptr, 0);
		temp.Free();
	}

	if (usePalette)  {
		memcpy((uchar*) *bmHandle[index] + CELHEADERSIZE + (xsize * ysize), hunkPalette, PALSIZE);
	}
}


void
Robot::Init()
{
	char buffer[4];
	char junk[32];
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

	assertMsg(fd == 0, "Robot::Init called when already open!");

	fd = Open(resNum);
	if (critErrHandler->Tripped() ||  fd < 1)
		msgMgr->Fatal("Could not open Robot resource %d\n", resNum);

	firstFrame = True;
	lastFrameNum = -1;
	cueForceShowFrame = -1;
	frameNum = 0;
	status = ROBOT_PAUSED;

	Read(fd, buffer, 4);
	Read(fd, &version, 2);
	if (version < 4)  {
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
	}
	else  {
		Read(fd, &asize, 2);
		Read(fd, &audPadSize, 2);
		Read(fd, junk, 2);
		Read(fd, &fcount, 2);
		Read(fd, &palSize, 2);
		Read(fd, &audPrimerSize, 2);
		Read(fd, &xRes, 2);
		Read(fd, &yRes, 2);
		Read(fd, &palFlag, 1);
		Read(fd, &audFlag, 1);
		Read(fd, junk, 2);			//  cueCount, frameRate
		Read(fd, &frameRate, 2);
		Read(fd, &accessType, 2);
		Read(fd, &canSkip, 2);
		Read(fd, &maxCelsPerFrame, 2);
		Read(fd, &maxsize0, 4);
		Read(fd, &maxsize1, 4);
		Read(fd, &maxsize2, 4);
		Read(fd, &maxsize3, 4);
		Read(fd, junk, 8);
	}

	if (!xRes)
		xRes = graphMgr->NextScreen()->Xdim();
	if (!yRes)
		yRes = graphMgr->NextScreen()->Ydim();

	audioSize = asize;
	hasAudio = audFlag;
	vPadSize[0] = vsize1;
	vPadSize[1] = vsize2;
	frameCount = fcount;

	if (strcmp(buffer, "SOL"))	
		msgMgr->Fatal("Resource is not Robot type!");

	if (version < 2  ||  version > 5)
		msgMgr->Fatal("Unsupported version of Robot resource");

	if (!hasAudio)  {
		LSeek(fd, audPrimerSize, SEEK_CUR);
	}
	else  {
		if (audPrimerSize)  {
			AudPrimerHdr audPrimerHdr;
			long pos = LSeek(fd, 0, SEEK_CUR);
			Read(fd, &audPrimerHdr, 14);

			if (audPrimerHdr.compressionType)
				msgMgr->Fatal("Unknown audio header compression type");

			if (audPrimerHdr.totalSampleCount)  {
				audioList.AddBlock(0, audPrimerHdr.evenSize);
				Read(fd, audioList.LastBlockAddr(), audPrimerHdr.evenSize);

				audioList.AddBlock(1, audPrimerHdr.oddSize);
				Read(fd, audioList.LastBlockAddr(), audPrimerHdr.oddSize);
			}

			if (audPrimerHdr.evenSize + audPrimerHdr.oddSize != audPrimerSize)  {
				LSeek(fd, pos + audPrimerSize, SEEK_SET);
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

	delete[] recordPosArray;
	recordPosArray = New ulong[frameCount];

	Read(fd, videoSizeArray, sizeof(*videoSizeArray) * frameCount);
	ushort* tempSize = New ushort[frameCount];
	if (version < 4)
		LSeek(fd, 8 * frameCount, SEEK_CUR);
	else
		Read(fd, tempSize, sizeof(ushort) * frameCount);

	if (version >= 3)  {
		Read(fd, cueTimeList, sizeof(*cueTimeList) * 256);
		Read(fd, cueValueList, sizeof(*cueValueList) * 256);

		//  Make sure they didn't use last frame or greater to cue with
		#ifdef DEBUG
		for (int ii = 0; ii < 256; ii++)  {
			if (cueTimeList[ii] != -1  &&  cueTimeList[ii] >= frameCount - 1)  {
				msgMgr->Fatal("Cue %d exceeds legal range!\n", cueTimeList[ii]);
			}
		}
		#endif
	}

	long pos = LSeek(fd, 0, SEEK_CUR);
	int remain = pos % 2048;
	if (remain)
		LSeek(fd, 2048 - remain, SEEK_CUR);
	fileDataStart = LSeek(fd, 0, SEEK_CUR);

	if (version >= 4)  {
		long total = fileDataStart;
		recordPosArray[0] = total;
		for (int i = 0; i < frameCount - 1; i++)  {
			total += tempSize[i];
			recordPosArray[i + 1] = total;
		}
	}


	if (version >= 4  &&  hasAudio)  {
		long usedEachFrame = 11025 / frameRate;
		canSkip = (audioSize / usedEachFrame) - 1;
		if (canSkip < 0)
			canSkip = 0;
	}

	delete tempSize;
	AudioPause(-3);			//  Robot audio pause
	framesSkipped = 0;
	maxFramesSeqSkipped = 0;
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
	assert(src);
	assert(dest);

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
Robot::LZDecode(uchar* dest, uchar* src, ulong srcCount, ulong destCount)
{
	assert(src);
	assert(dest);
	assert(destCount >= srcCount);

	int xdim = xsize;
	int ydim = (ysize * vertScaleFactor) / 100;
	Initcompress(lzScratch);
	int result = Decompress((char**) &src, (char**) &dest, &srcCount, &destCount, lzScratch);
	srcCount = 0;
	result = Decompress((char**) &src, (char**) &dest, &srcCount, &destCount, lzScratch);
	if (destCount > 1)
		msgMgr->Fatal("Decompression failure frame %d\n", frameNum); //Msg_CompressDestination);
}





void
Robot::ExpandData(uchar* dest, const uchar* src, int starty)
{
	assert(src);
	assert(dest);
	assert(starty >= 0);

	int ydim = (ysize * vertScaleFactor) / 100;
	assert(ydim);

	int numer = ysize;
	int denom = ydim;
	int total = (numer * starty) % denom;

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
	assert(ptr);
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
Robot::ToTicks(int frame)
{
	return (frame * 60) / frameRate;
}


int
Robot::ToFrame(int ticks)
{
	return (ticks * frameRate) / 60;
}


int
Robot::Info(SOL_Rect* rect)
{
	assertMsg(status != ROBOT_UNUSED, "Robot::Info - Called when robot uninitialized");
	assert(rect);

	SOL_Rect rectUnion;
	rectUnion.MakeEmpty();

	if (siCount)  {
		for (int i = 0; i < siCount; i++)  {
			SOL_Rect rect1;
			long x = graphMgr->NextScreen()->Planes().Search(robotSi[i]->PlaneId());
			robotSi[i]->NowSeen(graphMgr->NextScreen()->Planes()[x], rect1);
			if (!i)
				rectUnion = rect1;
			else
				rectUnion.Both(rect1);
		}
	}

	*rect = rectUnion;
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
      rGetCue,		//  returns cues to Robot SCI object
		rChanges,
		rPause,
		rFrameNum
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

			assert(argCount == 6);
			graphMgr->InitRobot(arg(2), arg(3), arg(4), arg(5), arg(6));
			pm.acc = False;
			break;

		case rDisplayFrame:
			assert(argCount == 2  || argCount == 4);
			if (argCount < 3)
				graphMgr->GRobot().DisplayFrame(arg(2));
			else
				graphMgr->GRobot().DisplayFrame(arg(2), arg(3), arg(4));
			break;

		case rFrameInfo:
			// Input -
			//              arg(1) = ArrayID to array for rect

			assert(argCount == 2);
			pm.acc = graphMgr->GRobot().Info(&rect);
			data = (SCIWord*) ((ArrayID) arg(2)).Data();
			data[0] = rect.A.x;
			data[1] = rect.A.y;
			data[2] = rect.B.x;
			data[3] = rect.B.y;
			break;


		case rPlay:
			assert(argCount == 1);
			graphMgr->GRobot().Resume();
			break;

		case rPause:
			assert(argCount == 1);
			graphMgr->GRobot().Pause();
			break;

		case rFrameNum:
			assert(argCount == 1);
			pm.acc = graphMgr->GRobot().FrameNum();
			break;


		case rHasEnded:
			// return true if waiting for terminating handshake
			//	------------------------------------------------
			// Input
			// 	  None
			// Output
			//		  pm.acc = True / False

			assert(argCount == 1);
			pm.acc = (graphMgr->RobotStatus() == ROBOT_FINAL);
			break;

		case rExists:
			// return whether robot sequence in progress (visible)
			// ---------------------------------------------------
			// Input
			//      None
			// Output
			//      pm.acc = True / False

			assert(argCount == 1);
			pm.acc = (graphMgr->RobotStatus() != ROBOT_UNUSED);
			break;

		case rTerminate:
			// Input
			//      None
			// Output

			assert(argCount == 1);
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

			assert(argCount == 2);
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
	assert(frame < frameCount);
	assert(frame >= 0);
	if (version < 4)  {
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
	else  {
		LSeek(fd, recordPosArray[frame], SEEK_SET);
	}
}

int
Robot::Open(ResNum num)
{
	return resMgr->Open(MemResRobot,num);
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

	assert(buf);
	assert(fileSpec);
	assert(rootName);
	//	if there are no wildcard characters in fileSpec, assume it's a dir and
	// slap on a backslash so _splitpath knows it, unless it's just a drive
	//	or the last char is already a backslash
	if (!strchr(fileSpec, '*') && fileSpec[strlen(fileSpec) - 1] != ':' &&
	    fileSpec[strlen(fileSpec) - 1] != '\\') {
		strcpy(fullName, fileSpec);
		strcat(fullName, "\\");
		fileSpec = fullName;
	}
	
#ifndef MACINTOSH
	//	get the drive, directory and extension
	_splitpath(fileSpec, drive, dir, 0, ext);
	
	//	and add the name and extension
	_makepath(buf, drive, dir, rootName, "rbt");
#endif
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
	return AudRobot(RobotAudInfo(memid, size, pos*2));
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
	suppressAudio = False;
}


#ifdef DEBUG
Bool
Robot::AudioList::Assert() const
{
	if (blockCount < 0  ||  blockCount > 10)
		return False;

	if (oldestBlockIndex < 0  ||  oldestBlockIndex > 9)
		return False;

	if (newestBlockIndex < 0  ||  newestBlockIndex > 9)
		return False;

	if (!(suppressAudio == True  ||  suppressAudio == False))
		return False;

	return True;
}
#endif

void
Robot::AudioList::Reset()
{
	assert(Assert());
	for (int i = 0; i < 10; i++)  {
		delete array[i];
		array[i] = NULL;
	}

	oldestBlockIndex = 0;
	newestBlockIndex = 0;
	blockCount = 0;
	suppressAudio = False;
}


Robot::AudioList::~AudioList()
{
	assert(Assert());
	for (int i = 0; i < 10; i++)  {
		delete array[i];
	}
}


Bool
Robot::AudioList::IsEmpty() const
{
	assert(Assert());
	return blockCount;
}

void
Robot::AudioList::StopAudio()
{
	assert(Assert());
	if (!suppressAudio)  {
		suppressAudio = True;
		AudRobot(RobotAudInfo(NULL, -1, -1));

		while (blockCount)  {
			delete array[oldestBlockIndex];
			array[oldestBlockIndex] = NULL;
			oldestBlockIndex++;
			if (oldestBlockIndex == 10)
				oldestBlockIndex = 0;
	
			blockCount--;
		}
	}
}

void
Robot::AudioList::StopAudioNow()
{
	assert(Assert());
	if (!suppressAudio)  {
		suppressAudio = True;
		AudRobot(RobotAudInfo(NULL, 0, 0));

		while (blockCount)  {
			delete array[oldestBlockIndex];
			array[oldestBlockIndex] = NULL;
			oldestBlockIndex++;
			if (oldestBlockIndex == 10)
				oldestBlockIndex = 0;

			blockCount--;
		}
	}
}

void
Robot::AudioList::AddBlock(int pos, int size)
{
	assert(Assert());
	assert(pos >= 0);
	assert(size > 0  &&  size < 64000);

	if (blockCount == 10)  {
		//DebugPrintf("Robot audio list full - Deleting oldest audio!\n");
		delete array[oldestBlockIndex];
		array[oldestBlockIndex] = NULL;
		oldestBlockIndex++;
		if (oldestBlockIndex == 10)
			oldestBlockIndex = 0;
		blockCount--;
	}

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
	assert(newestBlockIndex >= 0);
	assert(newestBlockIndex < 10);
	assert(blockCount);
	return **array[newestBlockIndex];
}



void
Robot::AudioList::SubmitDriverMax()
{
	assert(Assert());
	if (suppressAudio)  {
		while (blockCount)  {
			delete array[oldestBlockIndex];
			array[oldestBlockIndex] = NULL;
			oldestBlockIndex++;
			if (oldestBlockIndex == 10)
				oldestBlockIndex = 0;
	
			blockCount--;
		}
	}

	while (blockCount)  {
		if (!array[oldestBlockIndex]->Submit())  {
			return;
		}

		delete array[oldestBlockIndex];
		array[oldestBlockIndex] = NULL;
		oldestBlockIndex++;
		if (oldestBlockIndex == 10)
			oldestBlockIndex = 0;

		blockCount--;
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
Robot::DelayTime::Reset()
{
	for (int i = 0; i < 10; i++)  {
		timeStamp[i] = i;
		delays[i] = 0;
	}

	earliestTimeStamp = 0;
	latestTimeStamp = 9;
	startTime = -1;
}


#ifdef DEBUG
void
Robot::DelayTime::Dump() const
{
	msgMgr->Mono("Robot::DelayTime Dump\n");
	msgMgr->Mono(" earliestTimeStamp: %d  latestTimeStamp: %d  startTime: %d\n",
			earliestTimeStamp, latestTimeStamp, startTime);

	for (int i = 0; i < 10; i++)  {
		msgMgr->Mono("i: %d  timeStamp: %d  delay: %d\n", i, timeStamp[i], delays[i]);
	}
}
#endif

void
Robot::DelayTime::StartTiming()
{
	assert(startTime == -1);
	startTime = graphMgr->GRobot().TickCount();
}

void
Robot::DelayTime::EndTiming()
{
	assert(startTime != -1);
		
	int time = graphMgr->GRobot().TickCount() - startTime;
	for (int i = 0; i < 10; i++)  {
		if (timeStamp[i] == earliestTimeStamp)  {
			timeStamp[i] = ++latestTimeStamp;
			delays[i] = time;
			break;
		}
	}
	earliestTimeStamp++;
	startTime = -1;
	SortList();
}

Bool
Robot::DelayTime::TimingInProgress() const
{
	return startTime != -1;
}

void
Robot::DelayTime::SortList()
{
	assert(startTime == -1);

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
	assert(startTime == -1);
	return delays[5];
}


#ifdef DEBUG

void AssertMsgFail(const char* filename, int line, const char* str)
{
	msgMgr->Fatal("Assert failed: %s(%d) - %s\n", filename, line, str);
}

#endif


//  Version 5 robot file format doco
#if 0

byte		type;				//  0x3D
byte		Unused;			//  0x00
char[4]	"SOL\0"			//  ID
short		version			//  0x0004
short		audioSize		//  0K or 4K   [4408]
short		audioPadSize	//  2K or 0K   (pad to next X boundary)
short		minSpeed			//  150, 300 (K per sec)
short		frameCount		//  Frame count
short		palSize			//  size of palette (Minimum 1200 bytes)
short		audioInitSize	//  primer audio size (4K min)
short		xRes				//  X resolution (0 if same as game)
short		yRes				//  Y resolution (0 if same as game)
byte		paletteFlag
byte		audioFlag
short		cueCount
short		frameRate		//  frames per second (normally 10)
short		accessType		//  0 for now
short		safeFrameSkipAudio	//  Number of records skippable w/o audio dropout
short		maxCelsPerFrame;
long		maxsize0;		//  Used for preallocating memory for robot
long		maxsize1;		//  ..
long		maxsize2;		//  ..
long		maxsize3;
long		0					//  Reserved
long		0					//  Reserved

byte		audioInit[audioInitSize];	//  See below
byte		palette[palSize];
short		videoSizeArray[frameCount];	//  Total size of video block (inc. headers)
long		recordSizeArray[frameCount];	//  Size of each record
long		cueTimeList[256];
short		cueValueList[256];

Padding to next 2K bounds


Video Block (padSize - audioSize)
Audio Block (if any) (audioSize includes all audio headers)
Video Block
Audio Block




Video Block
	short	nCels;				//  1 or more

	byte	compressionType;	//  LZ, RLE, None
	byte	vertScaleFactor;	//  100 = No reduction
	short	xsize;				//  Hires XDim
	short	ysize;				//  Hires YDim
	short	xoff;					//  Hires origin
	short	yoff;					//  Hires origin
	short	x1;					//  Lores origin placement
	short	y1;					//  Lores origin placement
	short	size;					//  compressed size of cel
	short	checksum;			//  now used for chunk count in data block
	short	blockCount;
	short	0						//  Future expansion
	byte 	data[size]			//  videoData

		Contents of videoData block (1 or more chunks)
		This structure repeats chunk count (checksum) times:

		long	cmpSize			//  compressed size ( below 64K )
		long	destSize			//  size of block when uncompressed ( below 64K )
		short compType			//  compression type (LZ or UNCOMP)
		byte	cmpData[cmpSize]	//  actual data (usually LZ compressed)



	If 2 cels, etc
	byte	compressionType;	//  LZ, RLE, None
	byte	vertScaleFactor;	//  100 = No reduction
	short	xsize;				//  Hires XDim
	short	ysize;				//  Hires YDim
	short	xoff;					//  Hires origin
	short	yoff;					//  Hires origin
	short	x1;					//  Lores origin placement
	short	y1;					//  Lores origin placement
	short	size;					//  compressed size of cel
	short	checksum;			//  for block of raw videoData
	short	blockCount;
	short	0						//  Future expansion
	byte 	data[size]			//  videoData (see above)


AudioInit Block
	long	totalSampleCount;
	short	compressType;		//  None, LZ
	long	evenBlockSize;
	long	oddBlockSize;

	byte	evenBlock[evenBlockSize];
	byte	oddBlock[oddBlockSize];

#endif	
