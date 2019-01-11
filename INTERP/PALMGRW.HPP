#ifndef PALMGRW_HPP
#define PALMGRW_HPP

#ifndef PALMGRI_HPP
#include "palmgri.hpp"
#endif

#ifndef PALMGRT_HPP
#include "palmgrt.hpp"
#endif

void InitPalette(void);
void TermPalette(void);
void UpdatePalette(SOLPalette &current, SOLPalette &next, int &gammaValue);


class PaletteMgrIWin : public PaletteMgrIndex
{
#define MAXPALETTE	256				/* largest logical palette */
#define USERPALETTE	236				/* largest user palette (20 reserved) */

	public:
	PaletteMgrIWin()				{InitPalette();}
	~PaletteMgrIWin()				{TermPalette();}
	void	UpdateHardware()		{UpdatePalette(current,next,gammaValue);}
};


class PaletteMgrTWin : public PaletteMgrTrue
{
#define MAXPALETTE	256				/* largest logical palette */
#define USERPALETTE	236				/* largest user palette (20 reserved) */

	public:
	PaletteMgrTWin()				{InitPalette();}
	~PaletteMgrTWin()				{TermPalette();}
	void	UpdateHardware()		{UpdatePalette(current,next,gammaValue);}
};

#endif
