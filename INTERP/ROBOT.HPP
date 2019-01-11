#ifndef ROBOT_HPP
#define ROBOT_HPP

#ifndef SI_HPP
#include "si.hpp"
#endif

#ifndef RESTYPE_HPP
#include "restype.hpp"
#endif

enum {ROBOT_UNUSED, ROBOT_PLAYING, ROBOT_FINAL, ROBOT_PAUSED};

const SiArraySize = 10;

class Robot
{
	public:
	Robot(int resNum, long plane, int pri = -1, int xoff = 0, int yoff = 0);
	~Robot();
	int	Status() const				{return status;}
	int	FrameNum() const			{return frameNum;}
	int	FrameCount() const		{return frameCount;}
	int	FramesSkipped() const	{return framesSkipped;}
	int	MaxFramesSeqSkipped() const {return maxFramesSeqSkipped;}
	long	TickCount() const;

	long	GetCue();
	void	Init();
	void	DisplayFrame(int frame, int newxoff = 50000, int newyoff = 50000, int newpri = 50000);
	int	Info(SOL_Rect*);
	void	DoRobot();
	void	Pause();
	void	Resume();
	void	FrameAlmostVisible();
	void	FrameNowVisible();

	protected:
	void		KillRobot();
	void		SeekFrame(int frame);
	void		OffsetData(uchar*, char offset);
	void		CreateCel(uchar*, Bool usePalette = False);
	void		CreateCel4(uchar*, int celCount, Bool usePalette = False);
	void		Create1Cel4(uchar* video, int index, Bool usePalette);
	void		CreateCel5(uchar* video, int celCount, Bool usePalette);
	void		Create1Cel5(uchar* video, int index, Bool usePalette);
	void		LZDecode(uchar* dest, uchar* src, ulong srcCount);
	void		LZDecode(uchar* dest, uchar* src, ulong srcCount, ulong destCount);
	void		RLEDecode(uchar* dest, uchar* src);
	void		ExpandData(uchar* dest, const uchar* src, int starty);
	int		ToTicks(int frame);
	int		ToFrame(int ticks);
	void		DoVersion2();
	void		DoVersion3();
	void		DoVersion4();
	void		DoVersion5();

	ScreenItem*		robotSi[SiArraySize];
	int				siCount;
	ushort*			videoSizeArray;
	ulong*			recordPosArray;
	char*				lzScratch;
	void*				hunkPalette;
	int				audioSize;
	int				vPadSize[2];
	int				frameCount;
	int				frameNum;
	short				frameRate;		//  Frames per second
	short				accessType;
	int				resNum;
	short				xRes, yRes;		//  MUST BE SHORT!
	Bool				firstFrame;
	Bool				hasAudio;
	long				planeId;
	int				fd;
	int				priority[SiArraySize];
	int				xoff, yoff;
	int				fileX1[SiArraySize], fileY1[SiArraySize];
	int				status;
	
	//	 *** NEW ***
	ulong				robotStartTime;
	ulong				displayFrameTime;
	ulong				lastFrameReadTime;
	ulong				fileDataStart;
	int				lastFrameNum;
	long*				cueTimeList;
	ushort*			cueValueList;
	short				version;
	int				framesSkipped;
	int				maxFramesSeqSkipped;
	int				cueForceShowFrame;
	
	short				audPadSize;
	short				canSkip;
	short				maxCelsPerFrame;
	long				maxsize0, maxsize1, maxsize2, maxsize3;

	MemID				bmHandle[SiArraySize];
	int				compressType;
	int				vertScaleFactor;
	int				x1[SiArraySize], y1[SiArraySize];
	int				xsize, ysize;


	class AudioList  {
		public:
		AudioList();
		~AudioList();
		Bool		IsEmpty() const;
		Bool		Assert() const;
		void		AddBlock(int pos, int size);
		uchar*	LastBlockAddr();
		void		SubmitDriverMax();
		void		StopAudio();
		void		StopAudioNow();

		protected:
		class AudioBlock  {
			public:
			AudioBlock(int pos, int size);
			~AudioBlock();
			Bool		Submit();
			uchar*	operator*();

			protected:
			MemID		memid;
			int		pos;
			int		size;
		};

		protected:
		AudioBlock* array[10];
		int			oldestBlockIndex;
		int			newestBlockIndex;
		int			blockCount;
		Bool			suppressAudio;
	};

	AudioList	audioList;



	class DelayTime  {
		public:
		DelayTime();
		Bool	TimingInProgress() const;
		void	StartTiming();
		void	EndTiming();
		ulong	PredictedTicks();
		void	Dump() const;

		protected:
		void	SortList();
		void	AddToList(ulong);

		ulong		delays[10], timeStamp[10];
		ulong		earliestTimeStamp;
		ulong		latestTimeStamp;
		long		startTime;
	};

	DelayTime	delayTime;



	private:
	int		Open(ResNum);
	void		MakeName(char*, char* fileSpec, ResNum);
	void		MakeName(char*, char* fileSpec, char* rootName);
};

#endif
