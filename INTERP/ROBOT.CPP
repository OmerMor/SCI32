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
#ifdef DEBUG	//  ----------------------------------

void AssertMsgFail(const char* filename, int line, const char* str);


class LogObj  {
	public:
	LogObj(const char* str);
	~LogObj();

	protected:
	static int tab;
	const char*	str;
};


static int LogObj::tab = 0;

//#define LOG(exp)	LogObj localLogObj(exp)
#define LOG(exp)
#define DebugPrintf	msgMgr->Mono
#define	assertMsg(exp, str) \
	if (exp) {}				\
	else						\
	AssertMsgFail(__FILE__, __LINE__, str)

#else		//   DEBUG  --------------------------------

inline void DebugPrintf(char*, ...) {}
#define	assertMsg(exp, str)
#define LOG(exp)

#endif


class ReadFile
{
	public:
	ReadFile(int fd);
	~ReadFile()				{}

	Bool		Read(void*, int count);
 	Bool		Close();
	Bool		LSeek(int pos, int type);
	Bool		Tell(long* pos);
	void		ClearErrorAccum()				{errorAccum = False;}
	Bool		DidErrorAccum() const		{return errorAccum;}

	protected:
	int		fd;
	Bool		fileOpenFlag;
	Bool		errorAccum;

};

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


//  BUGBUG - DF - 5/31/95 - Can't use LSEEK(n,n,SEEK_SET) if robot
//  opened through resource.



/**************************************************************
   Robot constructor


	Prepares to play a single robot.
	Accesses config manager for "adjustPerDAC" flag, which allows
	the user to shut off attempts to correct video/audio timing
	to account for small % differences in audio playback rate.


 **************************************************************/

Robot::Robot(int resNum, long planeId, int pri, int xoff, int yoff) :
	resNum(resNum),
	planeId(planeId), 
	xoff(xoff),
	yoff(yoff)
{
	for (int i = 0; i < SiArraySize; i++)
	{
		robotSi[i] = NULL;
		priority[i] = pri;
	}
	siCount = 0;
	framesSkipped = 0;
	maxFramesSeqSkipped = 0;
	fd = 0;
	cueForceShowFrame = -1;
	robotStartFrame = 0;

	hunkPalette = NULL;
	videoSizeArray = NULL;
	recordPosArray = NULL;
	celHandleArray = NULL;
	celHandleArraySize = 0;
	firstFrame = True;
	lastFrameNum = -1;
	status = ROBOT_UNUSED;
	lzScratch = New char[SCRATCHRAM_SIZE];
	cueTimeList = New long[CueArraySize];
	cueValueList = New ushort[CueArraySize];
	for (i = 0; i < CueArraySize; i++)
	{
		cueTimeList[i] = -1;
		cueValueList[i] = 0;
	}

	adjustPerDAC = True;
	char* cp = configMgr->Get("adjustPerDAC", 0);
	if (strlen(cp) && (*cp == 'F' || *cp == 'f'))
		adjustPerDAC = False;

	hasAudio = False;
	prevFrame = -1;

	#ifdef DEBUG
	videoSizeArrayCopy = NULL;
	recordPosArrayCopy = NULL;
	maxFrameSize = 0;
	#endif
	//  BUGBUG - DDF - 5/25/95  -  What if theres no DAC or no audio in robot?
}



/**************************************************************
   Robot destructor

   NOTE:   If robot is not deleted when graphMgr destroyed, 
   this destructor gets called during graphmgr destructor, which
   will reenter a graphmgr method, causing trouble

 **************************************************************/


Robot::~Robot()
{
	assert(Assert());
	delete[] videoSizeArray;
	delete[]	recordPosArray;
	delete[] lzScratch;
	delete[] cueTimeList;
	delete[] cueValueList;
	delete[] hunkPalette;

   //  Deallocate all FrameLifetime handles in cel handle array
	if (celHandleArray)
	{
		for (int i = 0; i < celHandleArraySize; i++)
		{
			if (celHandleArray[i].status == CelHandleInfo::FrameLifetime)
			{
				assert(celHandleArray[i].id.IsValid());
				celHandleArray[i].id.Free();
			}
		}
	}
   //  Deallocate cel handle array
	delete[] celHandleArray;
	celHandleArray = NULL;

   //  Deallocate intermediate buffer
	assert(!squashedCelHandle  ||  squashedCelHandle.IsValid());
	squashedCelHandle.Free();

   //  Deallocate all fixed Cel handles
	for (int i = 0; i < FixedCelArraySize; i++)
		fixedCelHandle[i].Free();

	#ifdef DEBUG
	delete[] videoSizeArrayCopy;
	delete[] recordPosArrayCopy;
	#endif

	status = ROBOT_UNUSED;					//  in case calls made to deleted robot
	if (fd)
		resMgr->Close(fd);

	for (i = 0; i < siCount; i++)
		graphMgr->DeleteScreenItem(robotSi[i]);

	if (hasAudio)
		audioList.StopAudioNow();
}

long
Robot::TickCount() const
{
	return timeMgr->GetTickCount();
}



/**************************************************************
   Init Method


	Opens robot file, loads up primer, inits all main variables
	from file header.

	Sets robot audio in paused state so submissions will not play
	until 1st frame visible

	Robot resolution comes 1st from the robot file. If undefined,
	it will look for "hires=xxx,yyy" in the config file; otherwise,
	it	defaults	to 640x480.


 **************************************************************/

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
	assertMsg(status != ROBOT_KILLED, "Robot::Init called after robot killed!");

	fd = Open(resNum);
	if (critErrHandler->Tripped() ||  fd < 1)
		msgMgr->Fatal("Could not open Robot resource %d\n", resNum);

	firstFrame = True;
	lastFrameNum = -1;
	cueForceShowFrame = -1;
	frameNum = 0;
	status = ROBOT_PAUSED;

	ushort minrate = 300;

	ReadFile file(fd);

	file.Read(buffer, 4);
	file.Read(&version, 2);
	file.Read(&asize, 2);
	file.Read(&audPadSize, 2);
	file.Read(&minrate, 2);
	file.Read(&fcount, 2);
	file.Read(&palSize, 2);
	file.Read(&audPrimerSize, 2);
	file.Read(&xRes, 2);
	file.Read(&yRes, 2);
	file.Read(&palFlag, 1);
	file.Read(&audFlag, 1);
	file.Read(junk, 2);			//  cueCount, frameRate
	file.Read(&frameRate, 2);
	file.Read(&hiresCoords, 2);
	file.Read(&canSkip, 2);
	file.Read(&maxCelsPerFrame, 2);
	file.Read(&maxCelArea[0], 4);
	file.Read(&maxCelArea[1], 4);
	file.Read(&maxCelArea[2], 4);
	file.Read(&maxCelArea[3], 4);
	file.Read(junk, 8);

	assert(version == 5);
	assert(minrate >= 150  &&  minrate <= 300);

	if (file.DidErrorAccum())
		msgMgr->Fatal("Read error on robot: %d\n", resNum);

	normalFrameRate = frameRate;
	lowFrameRate = normalFrameRate - 1;
	highFrameRate = normalFrameRate + 1;

	//  If resolution undefined, use hires values, otherwise default
	if (xRes == 0  ||  yRes == 0)
	{
		if (*configMgr->Get("hires", 0))
		{
			xRes = atoi(configMgr->Get("hires", 0));
			yRes = atoi(configMgr->Get("hires", 1));
		}
		else
		{
			xRes = 640;
			yRes = 480;
		}
	}

	audioSize = asize;
	expectedAudCount = audioSize - 8;
	hasAudio = audFlag;
	vPadSize[0] = vsize1;
	vPadSize[1] = vsize2;
	frameCount = fcount;

	if (strcmp(buffer, "SOL"))	
		msgMgr->Fatal("Resource is not Robot type!");

	if (version != 5)
		msgMgr->Fatal("Unsupported version of Robot resource");

	if (!hasAudio)
		file.LSeek(audPrimerSize, SEEK_CUR);
	else
	{
		if (audPrimerSize)
		{
			AudPrimerHdr audPrimerHdr;
			long pos;
			file.Tell(&pos);
			file.Read(&audPrimerHdr, 14);

			if (audPrimerHdr.compressionType)
				msgMgr->Fatal("Unknown audio header compression type");

			if (audPrimerHdr.totalSampleCount)
			{
				audioList.AddBlock(0, audPrimerHdr.evenSize);
				file.Read(audioList.LastBlockAddr(), audPrimerHdr.evenSize);

				audioList.AddBlock(1, audPrimerHdr.oddSize);
				file.Read(audioList.LastBlockAddr(), audPrimerHdr.oddSize);
			}

			if (audPrimerHdr.evenSize + audPrimerHdr.oddSize != audPrimerSize)
				file.LSeek(pos + audPrimerSize, SEEK_SET);
		}
//cfs begin...
		else if (audPadSize)
		{
			audioList.AddBlock(0, 19922);
			memset(audioList.LastBlockAddr(), 0, 19922);
			audioList.AddBlock(1, 21024);
			memset(audioList.LastBlockAddr(), 0, 21024);
		}
//... cfs end
	}

	//  Read in palette if flag indicates one in file
	if (palFlag)
	{
		hunkPalette = (void*) New uchar[PALSIZE];
		file.Read(hunkPalette, palSize);
	}
	else
		file.LSeek(palSize, SEEK_CUR);

	delete[] videoSizeArray;
	videoSizeArray = New ushort[frameCount];

	delete[] recordPosArray;
	recordPosArray = New long[frameCount];

	file.Read(videoSizeArray, sizeof(*videoSizeArray) * frameCount);
	ushort* tempSize = New ushort[frameCount];
	file.Read(tempSize, sizeof(ushort) * frameCount);

	file.Read(cueTimeList, sizeof(*cueTimeList) * CueArraySize);
	file.Read(cueValueList, sizeof(*cueValueList) * CueArraySize);

	//  Make sure they didn't use last frame or greater to cue with
	#ifdef DEBUG
	for (int ii = 0; ii < CueArraySize; ii++)
	{
		if (cueTimeList[ii] != -1  &&  cueTimeList[ii] >= frameCount - 1)
			msgMgr->Fatal("Cue %d exceeds legal range!\n", cueTimeList[ii]);
	}
	#endif


	long pos;
	file.Tell(&pos);
	int remain = pos % CDSectorSize;
	if (remain)
		file.LSeek(CDSectorSize - remain, SEEK_CUR);

	file.Tell(&fileDataStart);

	#ifdef DEBUG
	maxFrameSize = (minrate * 1024L) / frameRate;
	for (ii = 0; ii < frameCount; ii++)
	{
		assert(tempSize[ii] <= maxFrameSize);
	}
	#endif

	long total = fileDataStart;
	recordPosArray[0] = total;
	for (int i = 0; i < frameCount - 1; i++)
	{
		total += tempSize[i];
		recordPosArray[i + 1] = total;
	}

	#ifdef DEBUG
	delete[] videoSizeArrayCopy;
	videoSizeArrayCopy = New ushort[frameCount];

	delete[] recordPosArrayCopy;
	recordPosArrayCopy = New long[frameCount];

	for (ii = 0; ii < frameCount; ii++)
	{
		recordPosArrayCopy[ii] = recordPosArray[ii];
		videoSizeArrayCopy[ii] = videoSizeArray[ii];
	}
	#endif

	if (hasAudio)
	{
		long usedEachFrame = 11025 / normalFrameRate;
		canSkip = (audioSize / usedEachFrame) - 1;
		if (canSkip < 0)
			canSkip = 0;
	}

	delete[] tempSize;
	AudioPause(ROBOTS_ONLY);			//  Robot audio pause
	framesSkipped = 0;
	maxFramesSeqSkipped = 0;
	prevFrame = -1;

	if (file.DidErrorAccum())
	{
		msgMgr->Fatal("Read error on robot: %d\n", resNum);
	}


	//  Allocate Fixed Cel buffers if version 5 or up
	//DebugPrintf("Max cels per frame: %d\n", (int) maxCelsPerFrame);
	int count = maxCelsPerFrame;
	if (count > FixedCelArraySize)
		count = FixedCelArraySize;

	for (i = 0; i < count; i++)
		fixedCelHandle[i].Get(MemBitmap, maxCelArea[i] + CELHEADERSIZE + PALSIZE, LOCKED);

	//  Allocate intermediate buffer
	if (version > 5)				//  BUGBUG - ?? check this
		squashedCelHandle.Get(MemBitmap, maxCelArea[0] + CELHEADERSIZE + PALSIZE, LOCKED);
	else
		squashedCelHandle.Get(MemBitmap, maxCelArea[0] + CELHEADERSIZE + PALSIZE);

	squashedCelArea = maxCelArea[0];
}






/**************************************************************
   DoRobot method

   Called from beginning of each FrameOut if robot exists.
	Responsible for calculating which frame to show, loading &
	decompressing it, and adding it to the graphMgr.


 **************************************************************/

void
Robot::DoRobot()
{
	LOG("DoRobot\n");
	assertMsg(status != ROBOT_UNUSED, "Robot::DoRobot - Called when robot uninitialized");
	assertMsg(status != ROBOT_KILLED, "Robot::DoRobot - Called after robot killed");
	AsyncEventCheck();
	assert(Assert());

	if (status != ROBOT_PLAYING)
		return;

	if (!firstFrame)
	{
		if (cueForceShowFrame != -1)
		{
			frameNum = cueForceShowFrame;
			cueForceShowFrame = -1;
		}
		else
		{
			int calcFrameNum = CalcFrameNum(delayTime.PredictedTicks());
			if (calcFrameNum < frameNum)
			{
				//msgMgr->Mono("Calc: %d  Actual: %d\n", calcFrameNum, frameNum);
				return;
			}
			frameNum = calcFrameNum;
			//frameNum++;
		}
	}

	//  If last frame already shown, set final flag and return
	if (frameNum >= frameCount)
	{
		if (lastFrameNum == frameCount - 1)
		{
			status = ROBOT_FINAL;
			if (hasAudio)
			{
				audioList.StopAudio();
				frameRate = normalFrameRate;
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

	if (hasAudio)
	{
		uchar scratch[20];
		ReadFile file(fd);
		for (long tmp = lastFrameNum + canSkip + 1; tmp < frameNum; tmp += canSkip + 1)
		{
			file.LSeek(recordPosArray[tmp] + videoSizeArray[tmp], SEEK_SET);
			audioList.SubmitDriverMax();
			file.Read(scratch, 8);
			long audCount = *((long*) &scratch[4]);
			long audPos = *((long*) scratch);

			if (audCount != expectedAudCount)
			{
				audioList.AddBlock(audPos, ZeroCompressCount + audCount);
				memset(audioList.LastBlockAddr(), 0, ZeroCompressCount);
				file.Read(audioList.LastBlockAddr() + ZeroCompressCount, audCount);
				audCount += ZeroCompressCount;
			}
			else
			{
				audioList.AddBlock(audPos, audCount);
				file.Read(audioList.LastBlockAddr(), audCount);
			}

			#ifdef DEBUG
			//if (file.DidErrorAccum())
			//	msgMgr->Mono("Read error");
			#endif
		}
	}

	audioList.SubmitDriverMax();
	delayTime.StartTiming();
	SeekFrame(frameNum);
	if (version == 5)
		DoVersion5();
	else
		msgMgr->Fatal("Robot Version %d not supported in this interpreter", version);

	audioList.SubmitDriverMax();



	//  Set flag, return ok
	if (frameNum != lastFrameNum + 1)
	{
		int skipped = frameNum - lastFrameNum - 1;
		framesSkipped += skipped;
		if (skipped > maxFramesSeqSkipped)
			maxFramesSeqSkipped = skipped;
	}

	lastFrameNum = frameNum;
}


/**************************************************************
   FrameAlmostVisible method

	Called from FrameOut just before ShowBits.
	Doesn't do much yet.


 **************************************************************/

void
Robot::FrameAlmostVisible()
{
	AsyncEventCheck();
	if (!firstFrame)
	{
		//  Do delay if we are ahead of schedule
	}
}



/**************************************************************
   FrameNowVisible method

	Called from FrameOut just after ShowBits (even if robot frame
	didn't change).  

   Used to sync initial video frame with beginning of audio.  Also
	attempts to keep variations in audio card rates in sync with video.


 **************************************************************/


void
Robot::FrameNowVisible()
{
	LOG("FrameNowVisible\n");

	if (firstFrame  &&  status == ROBOT_PLAYING)
	{
		robotStartTime = TickCount();
		robotStartFrame = frameNum;			// 0;
		if (frameNum == 0)
			AudioResume(ROBOTS_ONLY);						//  Robot audio resume
		else
			audioList.Suppress();

		firstFrame = False;
		if (hasAudio)
			checkAudioSyncTime = robotStartTime + 300; // 5 seconds
	}

	if (status == ROBOT_PLAYING)
	{
		if (delayTime.TimingInProgress())
			delayTime.EndTiming();

		audioList.SubmitDriverMax();

		//  Make sure we aren't going to return before time to show this frame is here
		//  Important for faster machines to keep motion overall fluid

		if (prevFrame != frameNum)
		{
			while (CalcFrameNum() < frameNum)
			{
				AsyncEventCheck();
			}
			prevFrame = frameNum;
		}


		//  Try to keep audio and video in sync (for audio cards with bad clock rates)
		if (adjustPerDAC  &&  !firstFrame  &&  hasAudio  &&  TickCount() >= checkAudioSyncTime)
		{
			RobotAudStatus astat;
			Bool worked = QueryAudRobot(&astat);

			#ifdef DEBUG
			//if (!worked)
			//	msgMgr->Mono("Audio Query failed!\n");
			#endif

			if (worked)
			{
				assert(astat.bytesPlayed >= 0);
				assert(astat.bytesPlaying >= astat.bytesPlayed);
				assert(astat.bytesSubmitted >= astat.bytesPlaying);

				int bytesPerFrame = astat.rate / normalFrameRate;
				if (astat.bits == 16)
					bytesPerFrame *= 2;

				checkAudioSyncTime = TickCount() + 20;			//  check again in 1 sec
				int videoFrameNow = CalcFrameNum();
				int audioFrameNow = astat.bytesPlaying / bytesPerFrame;

				//  Make sure we haven't had an unusual delay (alert boxes, etc)
				if (videoFrameNow < frameCount  &&  audioFrameNow < frameCount)
				{
					//		msgMgr->Mono("audioFrame: %d  videoFrame: %d\n", audioFrameNow, videoFrameNow);
	
					if (audioFrameNow < videoFrameNow - 1  &&  frameRate != lowFrameRate)
					{
						frameRate = lowFrameRate;

						//  We don't want to go backwards in time
						if (audioFrameNow < frameNum)
							SetRobotTime(frameNum);
						else
							SetRobotTime(audioFrameNow);

						//FrameRateChanged();
						//msgMgr->Mono("-");
					}
					else if (audioFrameNow > videoFrameNow + 1  &&  frameRate != highFrameRate)
					{
						frameRate = highFrameRate;

						//  We don't want to go backwards in time
						if (audioFrameNow < frameNum)
							SetRobotTime(frameNum);
						else
							SetRobotTime(audioFrameNow);

						//FrameRateChanged();
					  	//msgMgr->Mono("+");
					}
					else if (frameRate != normalFrameRate)
					{
						frameRate = normalFrameRate;

						//  We don't want to go backwards in time
						if (audioFrameNow < frameNum)
							SetRobotTime(frameNum);
						else
							SetRobotTime(audioFrameNow);

						//FrameRateChanged();
						//msgMgr->Mono(".");
					}
				}
			}
		}
	}

	AsyncEventCheck();
}


/**************************************************************
   Pause method

   Note that pause/resume will not do anything if we are at FINAL.
	Pause will *always* terminate the audio for the remaining duration
	of the robot!


 **************************************************************/

void
Robot::Pause()
{
	LOG("Pause\n");
	assertMsg(status != ROBOT_UNUSED, "Robot::Pause - Called when robot uninitialized");
	assertMsg(status != ROBOT_KILLED, "Robot::Pause - Called after robot killed");

	if (status == ROBOT_PLAYING)
	{
		audioList.StopAudioNow();
		status = ROBOT_PAUSED;
		frameRate = normalFrameRate;
	}
}



/**************************************************************
   Resume method

   Note that pause/resume will not do anything if we are at FINAL


 **************************************************************/

void
Robot::Resume()
{	
	LOG("Resume\n");
	assertMsg(status != ROBOT_UNUSED, "Robot::Resume - Called when robot uninitialized");
	assertMsg(status != ROBOT_KILLED, "Robot::Resume - Called after robot killed");

	if (status == ROBOT_PAUSED)
	{
		status = ROBOT_PLAYING;

		if (!firstFrame  ||  frameNum != 0)
			SetRobotTime(frameNum);
	}
}




/**************************************************************
   Assert method


 **************************************************************/
#ifdef DEBUG
Bool
Robot::Assert() const
{
	if (videoSizeArrayCopy)
	{
		for (int i = 0; i < frameCount; i++)
		{
			if (videoSizeArray[i] != videoSizeArrayCopy[i])
			{
				DebugPrintf("videoSizeArray corrupt");
				return False;
			} 
			if (recordPosArray[i] != recordPosArrayCopy[i])
			{
				DebugPrintf("recordPosArray corrupt");
				return False;
			}
		}
	}
	return audioList.Assert();
}
#endif


void
Robot::DoVersion5()
{
	LOG("DoVersion5\n");

	int oldSiCount = siCount;
	ushort videoSize = videoSizeArray[frameNum];
	uchar* scratch = New uchar[videoSize];

	ReadFile file(fd);
	if (!file.Read(scratch, videoSize))
	{
		#ifdef DEBUG
		//msgMgr->Mono("Read error!\n");
		#endif

		delete[] scratch;
		return;
	}

	siCount = * (short*) scratch;
	assert(siCount > 0);
	assert(siCount <= SiArraySize);

	if (hasAudio)
	{
		//  Get audio header
		uchar audioTmp[16];
		if (!file.Read(audioTmp, 8))
		{
			#ifdef DEBUG
			//msgMgr->Mono("Read error!\n");
			#endif

			delete[] scratch;
			return;
		}

		long audCount = *((long*) &audioTmp[4]);
		long audPos = *((long*) audioTmp);

		if (audCount != expectedAudCount)
		{
			audioList.AddBlock(audPos, ZeroCompressCount + audCount);
			memset(audioList.LastBlockAddr(), 0, ZeroCompressCount);
			if (!file.Read(audioList.LastBlockAddr() + ZeroCompressCount, audCount))
			{
				#ifdef DEBUG
				//msgMgr->Mono("Read error!\n");
				#endif

				delete[] scratch;
				return;
			}
			audCount += ZeroCompressCount;
		}
		else
		{
			audioList.AddBlock(audPos, audCount);
			if (!file.Read(audioList.LastBlockAddr(), audCount))
			{
				#ifdef DEBUG
				//msgMgr->Mono("Read error!\n");
				#endif

				delete[] scratch;
				return;
			}
		}
	}

	CreateCel5(scratch + 2, siCount, True);

	for (int i = 0; i < siCount; i++)
	{
		if (!robotSi[i])
		{
			robotSi[i] = New ScreenItem(planeId, Bitmap(celHandleArray[i].id));
			robotSi[i]->pos = SOL_Point(x1[i], y1[i]);
			if (priority[i] == -1)
				robotSi[i]->SetFloatPriority();
			else
				robotSi[i]->SetPriority(priority[i]);
			graphMgr->AddScreenItem(robotSi[i]);
		}
		else
		{
			robotSi[i]->bitmap = celHandleArray[i].id;
			robotSi[i]->pos = SOL_Point(x1[i], y1[i]);
			if (priority[i] == -1)
				robotSi[i]->SetFloatPriority();
			else
				robotSi[i]->SetPriority(priority[i]);
			graphMgr->UpdateScreenItem(robotSi[i]);
		}
	}

	for (i = siCount; i < oldSiCount; i++)
	{
		graphMgr->DeleteScreenItem(robotSi[i]);
		robotSi[i] = NULL;
	}

	delete[] scratch;
}



/**************************************************************
   DisplayFrame method

   Used only from an SCI kernel call.  It cancels the realtime
	aspect of the robot, entering a paused state which kills the audio
	for the remainder of the robot.
	
	If DisplayFrame is repeatedly	called to show the same frame, 
	it will NOT be reloaded from the disk each time.


 **************************************************************/

void
Robot::DisplayFrame(int frame, int newXOff, int newYOff, int newPri)
{
	LOG("DisplayFrame\n");
	assertMsg(status != ROBOT_UNUSED, "Robot::DisplayFrame - Called when robot uninitialized");
	assertMsg(status != ROBOT_KILLED, "Robot::DisplayFrame - Called after robot killed");
	assertMsg(frame >= 0  &&  frame < frameCount, "Robot::DisplayFrame - Frame out of range!\n");

	//  Update xoff, yoff, priority if override wanted
	if (newXOff != Unspecified)
		xoff = newXOff;
	if (newYOff != Unspecified)
		yoff = newYOff;
	if (newPri != Unspecified)
	{
		for (int i = 0; i < SiArraySize; i++)
			priority[i] = newPri;
	}

	frameNum = frame;
	Pause();

	if (frame != lastFrameNum)
	{
		SeekFrame(frameNum);
		if (version == 5)
			DoVersion5();
		else
			msgMgr->Fatal("Unsupported robot version: %d\n", (int) version);
	}
	else
	{
		//  In case xoff, yoff was changed this call
		assert(siCount <= SiArraySize);
		for (int i = 0; i < siCount; i++)
		{
			if (!hiresCoords)
			{
		 		x1[i] = fileX1[i] + xoff;
				y1[i] = fileY1[i] + yoff;
			}
			else
			{
				CelHeader* celHeader;
				assert(celHandleArray[i].id.IsValid());
				celHeader = (CelHeader*) *celHandleArray[i].id;

				xsize = celHeader->xDim;
				ysize = celHeader->yDim;

				int gameResX = graphMgr->Xdim();
				int gameResY = graphMgr->Ydim();

				#ifndef HI_RES
				int highResX1 = fileX1[i] + ((xoff * gameResX) / LOWRESX);
				int highResY1 = fileY1[i] + ((yoff * gameResY) / LOWRESY);
				int highResY2 = highResY1 + ysize - 1;

				int lowResX1 = (highResX1 * LOWRESX) / gameResX;
				int lowResY1 = (highResY1 * LOWRESY) / gameResY;
				int lowResY2 = (highResY2 * LOWRESY) / gameResY;

				int xorg = (highResX1 - ((lowResX1 * 63) / 32)) * -1;	// BUGBUG - DF - Depends on Phantas res!
				int yorg = ((lowResY2 * 9) / 4) - highResY1;				// BUGBUG - DF - Depends on Phantas res!
				celHeader->xHot = xorg;
				celHeader->yHot = yorg;
				x1[i] = lowResX1;
				y1[i] = lowResY2;
				#else
				int highResX1 = fileX1[i] + xoff;
				int highResY1 = fileY1[i] + yoff;
				int highResY2 = highResY1 + ysize - 1;

				int xorg = 0;
				int yorg = ysize - 1;
				celHeader->xHot = xorg;
				celHeader->yHot = yorg;
				x1[i] = highResX1;
				y1[i] = highResY2;
				#endif
			}

			MemID id;
			id = celHandleArray[i].id;
			assert(id.IsValid());

			if (!robotSi[i])
			{
				robotSi[i] = New ScreenItem(planeId, Bitmap(id));
				robotSi[i]->pos = SOL_Point(x1[i], y1[i]);
				if (priority[i] == -1)
					robotSi[i]->SetFloatPriority();
				else
					robotSi[i]->SetPriority(priority[i]);
				graphMgr->AddScreenItem(robotSi[i]);
			}
			else
			{
				robotSi[i]->bitmap = id;
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

/**************************************************************
   KillRobot method

	Used only from Kernel Call.  Stops the robot and audio, next
	frameout will remove the robot from the screen.


 **************************************************************/

void
Robot::KillRobot()
{
	if (fd)
	{
		resMgr->Close(fd);
		fd = 0;
	}

	for (int i = 0; i < siCount; i++)
	{
		graphMgr->DeleteScreenItem(robotSi[i]);
		robotSi[i] = NULL;
	}
	siCount = 0;

	if (hasAudio)
	{
		audioList.StopAudioNow();
		frameRate = normalFrameRate;
		hasAudio = False;
	}
	status = ROBOT_KILLED;
}




/**************************************************************
   GetCue method

	Used only from Kernel Call.  
	
	Can be called at any time.

	Returns 0 if no cue, cue # if one found.
	If a cue is found, the cue'd frame will be shown on the next
	frameout, irregardless of timing.  This is necessary to sync
	the appearance / disappearance of objects.


 **************************************************************/

long
Robot::GetCue()
{
	//  This if statement compensates for bad timing code in Phantasmagoria#1
	//  It should become a Warning or Fatal message that stops game play
	assert(status != ROBOT_UNUSED);
	assert(status != ROBOT_KILLED);
	assert(status != ROBOT_PAUSED);

	if (status == ROBOT_UNUSED  ||  status == ROBOT_KILLED  ||  status == ROBOT_PAUSED)
		return 0;

	//  If we are waiting for handshake, return -1
	if (graphMgr->RobotStatus() == ROBOT_FINAL)
		return -1;

	//  Nothing displayed yet, so ignore cue request
	if (firstFrame)
		return 0;

	//  Search for cue time <= estimated next current frame number
	int estNextFrameNum = CalcFrameNum(delayTime.PredictedTicks());

	assert(estNextFrameNum >= 0);
	assert(frameCount > 0);

	if (estNextFrameNum >= frameCount)
		estNextFrameNum = frameCount - 1;

	//  Cannot have a cue on the last frame, because we force the next frame
	//  to show.  This condition should be screened during init.
	//  Also, how does cueTimeList work on non-10-per-sec robots?

	for (int i = 0; i < CueArraySize; i++)
	{
		if (cueTimeList[i] != -1  &&  cueTimeList[i] <= estNextFrameNum)
		{
			if (cueTimeList[i] >= lastFrameNum)
				cueForceShowFrame = cueTimeList[i] + 1;

			cueTimeList[i] = -1;
			return cueValueList[i];
		}
	}

	//  No cues found for this time
	return 0;
}

Bool
Robot::IsRobotRealTime() const
{
	if (status == ROBOT_PLAYING  &&  !firstFrame  &&  fd)
		return True;
	return False;
}

void
Robot::SetRobotTime(int myFrame)
{
	assert(myFrame >= 0  &&  myFrame < frameCount);
	assert(myFrame >= frameNum);
	robotStartTime = TickCount();
	robotStartFrame = myFrame;
}


void
Robot::FrameRateChanged()
{
	robotStartTime = TickCount();
	robotStartFrame = frameNum;
}


int
Robot::CalcFrameNum(int extraTicks)
{
	assert(extraTicks >= 0);
	int calcFrame = ToFrame(TickCount() + extraTicks - robotStartTime) + robotStartFrame;
	assert(calcFrame >= 0);
	return calcFrame;
}



/*************************************************************************
 PreAllocCelMemory method

 Allocates, fills celHandleArray with MemIDs to minimum sized blocks of
 memory for each cel in this frame.  It also handles deallocating previous
 blocks (if needed), before starting the new allocations.

 Also maintains SquashedCel data for intermediate decoding of cel.

 *************************************************************************/

void
Robot::PreAllocCelMemory(const uchar* video, int celCount)
{
	assert(video);
	assert(celCount > 0  &&  celCount <= SiArraySize);
	assert(celHandleArraySize <= SiArraySize);

   //  Deallocate previous dynamically allocated cels (marked as FrameLifeTime)
	if (celHandleArray)
	{
		for (int i = 0; i < celHandleArraySize; i++)
		{
			if (celHandleArray[i].status == CelHandleInfo::FrameLifetime)
			{
				celHandleArray[i].id.Free();
				celHandleArray[i].id = 0;
				celHandleArray[i].status = CelHandleInfo::Invalid;
				celHandleArray[i].area = 0;
			}
		}
	}

   //  Deallocate handle array
	delete[] celHandleArray;

   //  Alloc a handle array, inited to zero, marked as Invalid
	celHandleArray = New CelHandleInfo[celCount];
	celHandleArraySize = celCount;

   //  Copy fixed mem handles to first section of handle array, mark as RobotLifeTime
	int count = celCount;
	if (count > FixedCelArraySize)
		count = FixedCelArraySize;

	for (int i = 0; i < count; i++)
	{
		//  If version 5 robots, max cels per frame is wrong, so consider
		if (!celHandleArray[i].id)
			break;

		celHandleArray[i].id = fixedCelHandle[i];
		celHandleArray[i].status = CelHandleInfo::RobotLifetime;
		celHandleArray[i].area = maxCelArea[i];
	}


	long maxAreaThisFrame = 0;
	for (i = 0; i < celCount; i++)
	{
	   //  Keep running total of max size required for this frame
		const VideoHeader4* vh = (const VideoHeader4*) video;
		long area = vh->xsize * vh->ysize;
		if (area > maxAreaThisFrame)
			maxAreaThisFrame = area;

		//  If memory prealloced, check that size is sufficient to hold the cel
		if (celHandleArray[i].status == CelHandleInfo::RobotLifetime)
		{
			assert(i < FixedCelArraySize);

			//  If insufficient, allocate one with a frame lifetime for this cel
			if (maxCelArea[i] < area)
			{
				//DebugPrintf("Allocating frame lifetime memory!\n");
				celHandleArray[i].id.Get(MemBitmap, area + CELHEADERSIZE + PALSIZE);
				celHandleArray[i].area = area;
				celHandleArray[i].status = CelHandleInfo::FrameLifetime;				
			}
		}
		//  If memory not allocated, do so
		else if (celHandleArray[i].status == CelHandleInfo::Invalid)
		{
			//DebugPrintf("Allocating additional frame lifetime memory!\n");
			celHandleArray[i].id.Get(MemBitmap, area + CELHEADERSIZE + PALSIZE);
			celHandleArray[i].area = area;
			celHandleArray[i].status = CelHandleInfo::FrameLifetime;				
		}
		else
			msgMgr->Fatal("Cel Handle has bad status");

		video += sizeof(VideoHeader4);
		video += vh->size;
	}

   //  test max size required against intermediate buffer storage size
   //  if too small, dealloc old intermediate area and realloc new
	if (maxAreaThisFrame > squashedCelArea)
	{
		//DebugPrintf("Allocating squash memory!\n");
		squashedCelHandle.Free();
		squashedCelHandle.Get(MemBitmap, maxAreaThisFrame);
		squashedCelArea = maxAreaThisFrame;
		assert(version <= 5);
	}

	#ifdef DEBUG
	assert(celHandleArray);
	assert(celHandleArraySize == celCount);
	assert(squashedCelHandle.IsValid());
	assert(squashedCelArea > 0);

	for (i = 0; i < celCount; i++)
	{
		assert(celHandleArray[i].id.IsValid());		
		assert(celHandleArray[i].status == CelHandleInfo::FrameLifetime  ||
				 celHandleArray[i].status == CelHandleInfo::RobotLifetime);
		assert(celHandleArray[i].area > 0);
	}
	#endif
}

void
Robot::CreateCel5(const uchar* video, int celCount, Bool usePalette)
{
	assert(video);
	assert(celCount > 0  &&  celCount <= SiArraySize);

	PreAllocCelMemory(video, celCount);
	for (int i = 0; i < celCount; i++)
	{
		Create1Cel5(video, i, usePalette);
		const VideoHeader4* vh = (const VideoHeader4*) video;
		int size = vh->size;
		video += sizeof(VideoHeader4);
		video += size;
	}
}


void
Robot::Create1Cel5(const uchar* video, int index, Bool usePalette)
{
	assert(video);
	assert(index >= 0  &&  index < SiArraySize);

	VideoHeader4* vh = (VideoHeader4*) video;
	video += sizeof(VideoHeader4);
	vertScaleFactor = vh->vertScaleFactor;

	xsize = vh->xsize;
	ysize = vh->ysize;
	int xorg, yorg;
	int gameResX = graphMgr->Xdim();
	int gameResY = graphMgr->Ydim();

	#ifndef HI_RES
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
	#else

	//  BUGBUG - DDF - 6/29/95 - Robot shifts slightly if resource.cfg
	//  contains "hires=630,450" statement.  This is because we are not
	//  compensating for gameRes values.  Do we ever want to define hires
	//  differently than the SCI coordinate system?
	//  Let's remove "hires=" from the interpreter after Phantas ships,
	//  and always create the vmap at 640x480

	int highResX1 = vh->x1 + xoff;
	int highResY1 = vh->y1 + yoff;
	int highResY2 = highResY1 + ysize - 1;

	xorg = 0;
	yorg = ysize - 1;
	x1[index] = highResX1;
	y1[index] = highResY2;
	#endif


	fileX1[index] = vh->x1;
	fileY1[index] = vh->y1;

	assert(celHandleArray[index].area >= xsize * ysize);
	assert(celHandleArray[index].id.IsValid());
	BuildHeader(celHandleArray[index].id, xsize, ysize, 255, xorg, yorg, xRes, yRes, usePalette);

	uchar* destBuff = NULL;
	if (vertScaleFactor == 100)
		destBuff = (uchar*) *celHandleArray[index].id + CELHEADERSIZE;
	else
	{
		assert(squashedCelArea >= xsize * ((ysize * vertScaleFactor) / 100));
 		destBuff = (uchar*) *squashedCelHandle;
	}

	//  Loop through proper number of sub-blocks
	uchar* ptr = destBuff;
	for (int i = 0; i < vh->checksum; i++)
	{
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


		switch (compType)
		{
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

	if (vertScaleFactor != 100)
	{
		ExpandData((uchar*) *celHandleArray[index].id + CELHEADERSIZE, ptr);
	}

	if (usePalette)
	{
		memcpy((uchar*) *celHandleArray[index].id + CELHEADERSIZE + (xsize * ysize), hunkPalette, PALSIZE);
	}
}





void
Robot::LZDecode(uchar* dest, const uchar* src, long srcCount1)
{
	assert(src);
	assert(dest);
	assert(srcCount1 > 0);

	int xdim = xsize;
	int ydim = (ysize * vertScaleFactor) / 100;
	ulong destCount = xdim * ydim;
	long tmpDestCount = destCount;
	uchar* tmpDest = dest;

	Initcompress(lzScratch);

	ulong srcCount = srcCount1;
	int result = Decompress((char**) &src, (char**) &dest, &srcCount, &destCount, lzScratch);
//	if (srcCount  ||  result != 1)

	srcCount = 0;
	result = Decompress((char**) &src, (char**) &dest, &srcCount, &destCount, lzScratch);
	if (destCount > 1)
	{
		assert(destCount == 0);					//  Will fail here if debug on
		memset(tmpDest, 255, tmpDestCount);	//  otherwise fill with skip
	}
}





void
Robot::LZDecode(uchar* dest, const uchar* src, long srcCount1, long destCount1)
{
	assert(src);
	assert(dest);
	assert(srcCount1 > 0);
	assert(destCount1 >= srcCount1);

	ulong srcCount = srcCount1;
	ulong destCount = destCount1;
	uchar* tmpDest = dest;

	Initcompress(lzScratch);
	int result = Decompress((char**) &src, (char**) &dest, &srcCount, &destCount, lzScratch);
	srcCount = 0;
	result = Decompress((char**) &src, (char**) &dest, &srcCount, &destCount, lzScratch);
	if (destCount > 1)
	{
		assert(destCount == 0);					//  Will fail here if debug on
		memset(tmpDest, 255, destCount1);	//  otherwise fill with skip
	}
}





void
Robot::ExpandData(uchar* dest, const uchar* src)
{
	assert(src);
	assert(dest);

	int ydim = (ysize * vertScaleFactor) / 100;
	assert(ydim);

	int numer = ysize;
	int denom = ydim;
	int total = 0;

	for (int y = ydim - 1; y >= 0; y--)
	{
		total += numer;
		int count = total / denom;
		total %= denom;

		while (count--)
		{
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
	for (int y = 0; y < ydim; y++)
	{
		for (int x = 0; x < xsize; x++)
		{
			if (*ptr < 246)
				*ptr += offset;
			ptr++;
		}
	}
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
	assertMsg(status != ROBOT_KILLED, "Robot::Info - Called after robot killed");
	assert(rect);

	SOL_Rect rectUnion;
	rectUnion.MakeEmpty();

	if (siCount)
	{
		for (int i = 0; i < siCount; i++)
		{
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
	ArrayID id;
	SCIWord* data;

	switch (function)
	{
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
			id = (ArrayID) arg(2);
			assert(id.IsValid());
			assert(id.Size() >= 8);

			pm.acc = graphMgr->GRobot().Info(&rect);
			data = (SCIWord*) id.Data();
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

		case rGetCue:
		{
			// Input
			//      arg2-----robot object
			// Output
         //    cue values returned
         //       0  no ques and not at end of play
         //       -1 play over last frame still on screen
         //       que value of oldest cue encountered that has not been returned 

			assert(argCount == 2);
	      ObjectID robotObject = (ObjectID) arg(2);
			assert(robotObject.IsValid());

			robotObject.SetIndexedProperty(sndSignal, (Property) graphMgr->GRobot().GetCue());
			break;
		}

		default:
			msgMgr->Alert("Robot Kernel function not implemented");
			break;
	}
}


Bool
Robot::SeekFrame(int frame)
{
	assert(frame < frameCount);
	assert(frame >= 0);

	ReadFile file(fd);
	file.LSeek(recordPosArray[frame], SEEK_SET);
	assert(file.DidErrorAccum() == False);
	return !file.DidErrorAccum();
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
	    fileSpec[strlen(fileSpec) - 1] != '\\')
	{	    
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
	assert(size > 0);
	memid.Get(MemResAudio, size);
	memid.SetNotDiscardable();
	assert(memid);
}


Robot::AudioList::AudioBlock::~AudioBlock()
{
	assert(memid);
	memid.Free();
}

Bool
Robot::AudioList::AudioBlock::Submit()
{
	LOG("Submit\n");
	return AudRobot(RobotAudInfo(memid, size, pos*2));
}

#ifdef DEBUG
Bool
Robot::AudioList::AudioBlock::Assert() const
{
	if (!memid.IsValid())
	{
		DebugPrintf("Memid invalid");
		return False;
	}

	if (pos < 0)
	{
		DebugPrintf("pos negative");
		return False;
	}

	if (size <= 0  || size > 65536L)
	{
		DebugPrintf("size range error");
		return False;
	}

	return True;
}
#endif
		
uchar*
Robot::AudioList::AudioBlock::operator*()
{
	return (uchar*) *memid;
}


Robot::AudioList::AudioList()
{
	for (int i = 0; i < AudioListSize; i++)
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
	if (blockCount < 0  ||  blockCount > AudioListSize)
	{
		DebugPrintf("BlockCount out of range");
		return False;
	}

	if (oldestBlockIndex < 0  ||  oldestBlockIndex >= AudioListSize)
	{
		DebugPrintf("oldestBlockIndex out of range");
		return False;
	}

	if (newestBlockIndex < 0  ||  newestBlockIndex >= AudioListSize)
	{
		DebugPrintf("newestBlockIndex out of range");
		return False;
	}

	if (!(suppressAudio == True  ||  suppressAudio == False))
	{
		DebugPrintf("supressAudioFlag corrupt");
		return False;
	}

	int i = oldestBlockIndex;
	for (int count = blockCount; count--; )
	{
		if (!array[i])
		{
			DebugPrintf("Array element null");
			return False;
		}

		if (!array[i]->Assert())
		{
			DebugPrintf("element assert failed");
			return False;
		}
		i++;
		if (i == AudioListSize)
			i = 0;
	}

	return True;
}
#endif

Robot::AudioList::~AudioList()
{
	assert(Assert());
	for (int i = 0; i < AudioListSize; i++)
		delete array[i];
}


void
Robot::AudioList::StopAudio()
{
	LOG("StopAudio\n");
	assert(Assert());
	if (!suppressAudio)
	{
		suppressAudio = True;
		AudRobot(RobotAudInfo(NULL, -1, -1));

		while (blockCount)
		{
			delete array[oldestBlockIndex];
			array[oldestBlockIndex] = NULL;
			oldestBlockIndex++;
			if (oldestBlockIndex == AudioListSize)
				oldestBlockIndex = 0;
	
			blockCount--;
		}
	}
}

void
Robot::AudioList::StopAudioNow()
{
	LOG("StopAudioNow\n");
	assert(Assert());

	if (!suppressAudio)
	{
		suppressAudio = True;
		AudRobot(RobotAudInfo(NULL, 0, 0));

		while (blockCount)
		{
			delete array[oldestBlockIndex];
			array[oldestBlockIndex] = NULL;
			oldestBlockIndex++;
			if (oldestBlockIndex == AudioListSize)
				oldestBlockIndex = 0;

			blockCount--;
		}
	}
}


void
Robot::AudioList::Suppress()
{
	LOG("Suppress\n");
	assert(Assert());

	if (!suppressAudio)
	{
		suppressAudio = True;
		while (blockCount)
		{
			delete array[oldestBlockIndex];
			array[oldestBlockIndex] = NULL;
			oldestBlockIndex++;
			if (oldestBlockIndex == AudioListSize)
				oldestBlockIndex = 0;

			blockCount--;
		}
	}
}



void
Robot::AudioList::AddBlock(int pos, int size)
{
	LOG("AddBlock\n");
 	assert(Assert());
	assert(pos >= 0);
	assert(size > 0  &&  size < 64000);

	if (blockCount == AudioListSize)
	{
		//DebugPrintf("Robot audio list full - Deleting oldest audio!\n");
		delete array[oldestBlockIndex];
		array[oldestBlockIndex] = NULL;
		oldestBlockIndex++;
		if (oldestBlockIndex == AudioListSize)
			oldestBlockIndex = 0;
		blockCount--;
	}

	//  If first block, init variables properly
	if (!blockCount)
		oldestBlockIndex = newestBlockIndex = 0;
	else
	{
		newestBlockIndex++;
		if (newestBlockIndex == AudioListSize)
			newestBlockIndex = 0;
	}

	array[newestBlockIndex] = New AudioBlock(pos, size);
	blockCount++;
}


uchar*
Robot::AudioList::LastBlockAddr()
{
	assert(newestBlockIndex >= 0);
	assert(newestBlockIndex < AudioListSize);
	assert(blockCount);
	uchar* ret = **array[newestBlockIndex];
	assert(ret);
	return ret;
}



void
Robot::AudioList::SubmitDriverMax()
{
	LOG("SubmitDriverMax\n");
	assert(Assert());

	if (suppressAudio)
	{
		while (blockCount)
		{
			delete array[oldestBlockIndex];
			array[oldestBlockIndex] = NULL;
			oldestBlockIndex++;
			if (oldestBlockIndex == AudioListSize)
				oldestBlockIndex = 0;
	
			blockCount--;
		}
	}

	while (blockCount)
	{
		if (!array[oldestBlockIndex]->Submit())
			return;

		delete array[oldestBlockIndex];
		array[oldestBlockIndex] = NULL;
		oldestBlockIndex++;
		if (oldestBlockIndex == AudioListSize)
			oldestBlockIndex = 0;

		blockCount--;
	}
}



Robot::DelayTime::DelayTime()
{
	for (int i = 0; i < DelayArraySize; i++)
	{
		timeStamp[i] = i;
		delays[i] = 0;
	}

	earliestTimeStamp = 0;
	latestTimeStamp = DelayArraySize - 1;
	startTime = -1;
}

#ifdef DEBUG
void
Robot::DelayTime::Dump() const
{
	DebugPrintf("Robot::DelayTime Dump\n");
	DebugPrintf(" earliestTimeStamp: %d  latestTimeStamp: %d  startTime: %d\n",
			earliestTimeStamp, latestTimeStamp, startTime);

	for (int i = 0; i < DelayArraySize; i++)
	{
		DebugPrintf("i: %d  timeStamp: %d  delay: %d\n", i, timeStamp[i], delays[i]);
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
	for (int i = 0; i < DelayArraySize; i++)
	{
		if (timeStamp[i] == earliestTimeStamp)
		{
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

	for (int i = 0; i < DelayArraySize - 1; i++)
	{
		long smallDelay = delays[i];
		int smallIndex = i;

		//  Search for smallest remaining in list
		for (int j = i + 1; j < DelayArraySize; j++)
		{
			if (delays[j] < smallDelay)
			{
				smallDelay = delays[j];
				smallIndex = j;
			}
		}

		//  If smaller found, swap it to the top of the list
		if (smallIndex != i)
		{
			long tmpDelay = delays[i];
			int tmpTimeStamp = timeStamp[i];

			delays[i] = smallDelay;
			timeStamp[i] = timeStamp[smallIndex];

			delays[smallIndex] = tmpDelay;
			timeStamp[smallIndex] = tmpTimeStamp;
		}
	}
}


long
Robot::DelayTime::PredictedTicks() const
{
	assert(startTime == -1);
	return delays[DelayArraySize / 2];
}


#ifdef DEBUG

void AssertMsgFail(const char* filename, int line, const char* str)
{
	msgMgr->Fatal("Assert failed: %s(%d) - %s\n", filename, line, str);
}


LogObj::LogObj(const char* str1)
{
	str = str1;
	for (int i = 0; i < tab; i++)
		DebugPrintf(" ");
	DebugPrintf((char*) str1);
	tab += 2;
}

LogObj::~LogObj()
{
	tab -= 2;
}

#endif


/***********************************************************************
 ***********************************************************************

  File Stuff






 ***********************************************************************
 ***********************************************************************/



ReadFile::ReadFile(int fd1)
{
	fileOpenFlag = True;
	errorAccum = False;
	fd = fd1;
}


Bool
ReadFile::Read(void* buff, int count)
{
	assert(fileOpenFlag);
	assert(count > 0);

	#ifdef DEBUG
	memset(buff, 0xFE, count);
	#endif

	int ret = ::Read(fd, buff, count);
	if (ret != count)
		errorAccum = True;

	return !errorAccum;
}

Bool
ReadFile::LSeek(int pos, int type)
{
	assert(fileOpenFlag);
	assert(pos >= 0);
	assert(type == SEEK_SET  ||  type == SEEK_CUR  ||  type == SEEK_END);

	int ret = ::LSeek(fd, pos, type);
	if (ret == -1)
		errorAccum = True;

	return errorAccum;
}

Bool
ReadFile::Tell(long* loc)
{
	assert(fileOpenFlag);
	assert(loc);

	int ret = ::LSeek(fd, 0, SEEK_CUR);
	if (ret == -1)
		errorAccum = True;
	else
		*loc = ret;

	return errorAccum;
}

Bool
ReadFile::Close()
{
	assert(fileOpenFlag);
	
	int ret = ::Close(fd);
	if (ret == -1)
		errorAccum = True;

	fileOpenFlag = False;
	fd = -1;
	return errorAccum;
}


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