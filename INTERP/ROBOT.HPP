#ifndef ROBOT_HPP
#define ROBOT_HPP

#ifndef SI_HPP
#include "si.hpp"
#endif

#ifndef RESTYPE_HPP
#include "restype.hpp"
#endif

enum {ROBOT_UNUSED, ROBOT_PLAYING, ROBOT_FINAL, ROBOT_PAUSED};

class Robot
{
	public:
	Robot(int resNum, long plane, int pri = -1, int xoff = 0, int yoff = 0);
	~Robot();
	int	Status() const				{return status;}
	long	GetCue();
	void	Init();
	MemID	GetFrameBM(int frame);
	void	DisplayFrame(int frame, int newxoff = 50000, int newyoff = 50000, int newpri = 50000);
	int	Info(SOL_Rect*);
	void	DoRobot();
	void	Pause();
	void	Resume();
	void	FrameAlmostVisible();
	void	FrameNowVisible();

	protected:
	void		KillRobot();
	void		StopAudio();
	void		StopAudioNow();
	void		SeekFrame(int frame);
	void		OffsetData(uchar*, char offset);
	void		CreateCel(uchar*, Bool usePalette = False);
	void		RLEDecode(uchar* dest, uchar* src);
	void		LZDecode(uchar* dest, uchar* src, ulong srcCount);
	void		ExpandData(uchar* dest, const uchar* src);
	void		DoVersion2();
	void		DoVersion3();


	ScreenItem*		robotSi;
	ushort*			videoSizeArray;
	char*				lzScratch;
	void*				hunkPalette;
	int				audioSize;
	int				vPadSize[2];
	int				frameCount;
	int				frameNum;
	int				resNum;
	short				xRes, yRes;		//  MUST BE SHORT!
	Bool				firstFrame;
	Bool				hasAudio;
	long				planeId;
	int				fd;
	int				priority;
	int				xoff, yoff;
	int				fileX1, fileY1;
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
	

	MemID				bmHandle;
	int				compressType;
	int				vertScaleFactor;
	int				x1, y1;
	int				xsize, ysize;


	class AudioList  {
		public:
		AudioList();
		~AudioList();
		Bool				IsEmpty() const;
		void				AddBlock(int pos, int size);
		uchar*			LastBlockAddr();
		void				SubmitDriverMax();

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
	};

	AudioList	audioList;



	class DelayTime  {
		public:
		DelayTime();
		void	StartTiming();
		void	EndTiming();
		ulong	PredictedTicks();

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
