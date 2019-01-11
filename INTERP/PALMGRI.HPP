#ifndef PALMGRI_HPP
#define PALMGRI_HPP

#ifndef PALMGR_HPP
#include "palmgr.hpp"
#endif


#define MAXCYCLES 10

class PaletteMgrIndex : public PaletteMgr
{
	public:
	PaletteMgrIndex();
	~PaletteMgrIndex();
	const SOLPalette&	Current() const		{return current;}
	const SOLPalette&	Source() const			{return source;}
	const SOLPalette&	Next() const			{return next;}
	const uchar* 		CycleMap() const		{return cycleLoc;}

	void	Submit(HunkPalette&); 
	void	Submit(const SOLPalette&);
	Bool	UpdateForFrame();
	Bool	UpdateFFrame();



	void	SetCycle(int begin, int end, int direction = 1, int ticks = 0);
	void	CycleOn(int which);
	void	CyclePause(int which);
	void	CycleOff(int which);
	void	CycleAllOff();

	void	SetTarget(const SOLPalette&);
	void	MergeTarget(const SOLPalette&);
	void	SetStart(const SOLPalette&);
	void	MergeStart(const SOLPalette&);
	void	SetVary(const SOLPalette&, int percent = 100, int ticks = 0, int start = -1, int end = -1);
	void	SetVaryPercent(int percent, int ticks = 0, int start = -1, int end = -1); 
	void	SetVaryTime(int ticks);
	int	VaryPercent() const {return (varyDirection < 0)?-varyPercent:varyPercent;}
	void	VaryOff();
	void	VaryOn();
	void  VaryPause() {varyPause++; varyDirection = 0;} 
	                	
   void  SaveRestoreBefore(SaveGame* saveGame, Bool save);
	void	SaveRestore(SaveGame* saveGame, Bool save);

	void	SetFade(int percent, int begin = 0, int end = 255);
	void	FadeOff();

	protected:
	void	ApplyAll();
	void	ApplyCycles();
   void  ApplyVary();
   void	ApplyFade();
	void	DoCycle(int which, int steps = 1);

	long	version;
	int	varyDirection;
	int	varyPercent;
	int 	targetVaryPercent;
   int   varyStartPos;
   int   varyEndPos;
	Bool	varyPause;
	Bool	needUpdate;
   int	deltaTimeVary;
	int	intensity[256];
	uchar	cycleLoc[256];
	struct Cycle {
		int 	begin;
		int 	range;
		int 	current;
		Bool	forward;
		ulong	lastUpdate;
		int 	deltaTime;
      Bool	pause;
	}	*cycle[MAXCYCLES];
   ulong	timeLastVary;
   int gammaValue; 
		
	SOLPalette source, *start, *target, current, next;
};

extern const char gamma[4][256];

#endif

