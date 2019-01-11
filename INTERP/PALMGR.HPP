#ifndef PALMGR_HPP
#define PALMGR_HPP

#ifndef PALETTE_HPP
#include "palette.hpp"
#endif

#ifndef SAVEABLE_HPP
#include "saveable.hpp"
#endif

class PaletteMgr : public Saveable
{
	public:
	virtual	~PaletteMgr() {}

	virtual  void	Submit(HunkPalette&) = 0;
	virtual  void	Submit(const SOLPalette&) = 0;
	virtual	const SOLPalette&	Current() const = 0;
	virtual	const SOLPalette&	Source() const = 0;
	virtual	const SOLPalette&	Next() const = 0;
	virtual	const uchar* 		CycleMap() const = 0;
	virtual	Bool	UpdateForFrame() = 0;
	virtual	Bool	UpdateFFrame() = 0;


	virtual	void	SetCycle(int begin, int end, int direction = 1, int ticks = 0) = 0;
	virtual	void	DoCycle(int which, int steps = 1) = 0;
	virtual	void	CycleOn(int which) = 0;
	virtual	void	CyclePause(int which) = 0;
	virtual	void	CycleOff(int which) = 0;
	virtual	void	CycleAllOff() = 0;

	virtual	void	SetTarget(const SOLPalette&) = 0;  
	virtual	void	MergeTarget(const SOLPalette&) = 0;
	virtual	void	SetStart(const SOLPalette&) = 0;
	virtual	void	MergeStart(const SOLPalette&) = 0;
	virtual	void	SetVary(const SOLPalette&, int percent = 100, int ticks = 0, int start = -1, int end = -1) = 0;
	virtual	void	SetVaryPercent(int percent, int ticks = 0, int start = -1, int end = -1) = 0;
	virtual	void	SetVaryTime(int ticks) = 0;
	virtual	int	VaryPercent() const = 0;
	virtual	void	VaryOff() = 0;
	virtual	void	VaryOn() = 0;
	virtual	void  VaryPause() = 0;
	virtual	void	SetFade(int percent, int begin = 0, int end = 255) = 0;
	virtual	void	FadeOff() = 0;
	
	virtual	void	SaveRestore(SaveGame* saveGame, Bool save) = 0;
	virtual	void	UpdateHardware() = 0;
};


#endif

