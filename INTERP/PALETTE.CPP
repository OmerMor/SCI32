//	palette.cpp

#include <stdlib.h>
#include <string.h>

#include "sol.hpp"

#include "msg.hpp"
#include "palette.hpp"
#include "memmgr.hpp"



inline int sqr(int i) {return i * i;}

/*
 *
 *  Constructor
 *  ===========
 *
 */

SOLPalette::SOLPalette(const HunkPalette& src)
{
	SOLPalette::operator=(src);
}


/*
 *
 *
 *  Constructor
 *  ===========
 *
 */

SOLPalette::SOLPalette(const SOLPalette& src)
{
	for (int i = 0; i < MaxColors; i++)
		colors[i] = src.colors[i];
}



/*
 *
 *
 *  Constructor
 *  ===========
 *
 */

SOLPalette::SOLPalette()
{
	Rgb24F unusedBlack(0, 0, 0, 0);

	for (int i = 0; i < MaxColors; i++)
		colors[i] = unusedBlack;
}



/*
 *
 *
 *  Assign from HunkPalette
 *  =======================
 *
 *
 */

SOLPalette&
SOLPalette::operator=(const HunkPalette& src)
{
	Rgb24F unusedBlack(0, 0, 0, 0);

	for (int i = 0; i < MaxColors; i++)
		colors[i] = unusedBlack;

	if (src.Count())  {
		Rgb24F rgb;
		HunkPalette::CompPal* pal = src.PalAddr();
		uchar* ptr = ((uchar*) pal) + sizeof(HunkPalette::CompPal);

		if (!pal->type)  {
			int end = pal->startOffset + pal->nColors;
			for (int i = pal->startOffset; i < end; i++)  {
				rgb.flag = *ptr++;
				rgb.red = *ptr++;
				rgb.green = *ptr++;
				rgb.blue = *ptr++;
				colors[i] = rgb;
			}
		}
		else  {
			rgb.flag = pal->def;
			int end = pal->startOffset + pal->nColors;
			for (int i = pal->startOffset; i < end; i++)  {
				rgb.red = *ptr++;
				rgb.green = *ptr++;
				rgb.blue = *ptr++;
				colors[i] = rgb;
			}
		}
	}
	return *this;
}



/*
 *
 *
 *  Assign from Palette
 *  ===================
 *
 */

SOLPalette&
SOLPalette::operator=(const SOLPalette& src)
{
	for (int i = 0; i < MaxColors; i++)
		colors[i] = src.colors[i];
	return *this;
}


/*
 *
 *
 *  Compare operator
 *  ================
 *
 */

int
SOLPalette::operator==(const SOLPalette& src) const
{
	for (int i = 0; i < MaxColors; i++)
		if (colors[i] != src.colors[i])
			break;

	if (i == MaxColors)
		return True;
	return False;
}


int
SOLPalette::Match(const Rgb24& rgb) const
{
	int diff = 0xfffff;
	int index = 0;
	int current;
	int red = rgb.red;
	int green = rgb.green;
	int blue = rgb.blue;

	//  Doesn't match to cycling colors
	for (int i = 0; i < 236; i++) {
		current = sqr((int)(colors[i].red) - red);
		if (diff > current) {
			current += sqr((int)(colors[i].green) - green);
			if (diff > current) {
				current += sqr((int)(colors[i].blue) - blue);
				if (diff > current) {
					diff = current;
					index = i;
				}
			}
		}
	}
	return index;
}

int
SOLPalette::Match(const Rgb24& rgb, int mindist, int* finalDist, const uchar* forbidden) const
{
	int diff = 0xfffff;
	int index = -1;
	int current = mindist;
	int red = rgb.red;
	int green = rgb.green;
	int blue = rgb.blue;

	for (int i = 0; i < 236; i++) {
		if (!forbidden[i])  {   
			current = sqr((int)(colors[i].red) - red);
			if (diff > current)  {
				current += sqr((int)(colors[i].green) - green);
				if (diff > current)  {
					current += sqr((int)(colors[i].blue) - blue);
					if (diff > current)  {
						diff = current;
						index = i;
					}
				}
			}
		}
	}

	*finalDist = current;
	return index;
}


int
SOLPalette::Compare(const SOLPalette& src, uchar *diff) const
{
	int retVal = True;

	for (int i = 0; i < MaxColors; i++) {
		if (colors[i] != src.colors[i]) {
			*diff++ = 1;
         retVal = False;
		} else
      	*diff++ = 0;
   }

	return retVal;
}


/*
 *
 *
 *  Merge method
 *  ============
 *
 */

void
SOLPalette::Merge(const SOLPalette& src)
{
	for (int i = 0; i < MaxColors; i++)
		if (src.colors[i].flag)
			colors[i] = src.colors[i];
}


void
HunkPalette::Init(void* src1)
{
	PalHeader* src = (PalHeader*) src1;
	palInHunk = NULL;
	version = 0;
	count = src->palCount;

	if (count)  {
		palInHunk = src;
		version = PalAddr()->valid;
	}
}

void
HunkPalette::SetVersion(int ver)
{
	if (count != palInHunk->palCount)
		msgMgr->Fatal(SrcLoc, Msg_PalMemMoved);

	if (count)  {
		CompPal* pal = PalAddr();
		if (pal->valid != version)
			msgMgr->Fatal(SrcLoc, Msg_PalMemMoved);

		pal->valid = ver;
		version = ver;
#ifdef DEBUG
		memMgr->SetChecksum(palInHunk);
#endif
	}
}

////////////////////////////////////////////////////////////////////////////

#include "kernel.hpp"
#include "graphmgr.hpp"
#include "memid.hpp"
#include "resource.hpp"
#include "palmgr.hpp"
#include "rgb.hpp"
#include "pmachine.hpp"

void
KPalVary(argList)
{
	enum {   
		PALVARYSTART = 0,
		PALVARYREVERSE ,
		PALVARYINFO,
		PALVARYKILL,
		PALVARYTARGET,
		PALVARYNEWTIME,
		PALVARYPAUSE,
      PALVARYNEWTARGET,
      PALVARYNEWSOURCE,
      PALVARYMERGESOURCE
   };
	
	MemID palID;
	switch (arg(1)) {

		case PALVARYSTART:

         /* input:
					arg2 = target palette number (required)  
               arg3 = time to shift palette in seconds (required)
               if time = zero then palette changed immediately 
							 to destination percent
               arg4 = target percent for shift (not required unless arg5 wanted)
							 If not present 100 assumed to be target percent
					 arg5 = starting palette entry
					 arg6 = ending palette entry
			*/
         {
				palID = resMgr->Get(MemResPalette, arg(2));
				HunkPalette hpal;
				hpal.Init(*palID);
				SOLPalette pal(hpal);
				if (argCount < 3)
					graphMgr->GPalette().SetVary(pal, 100);
				else if (argCount < 4)
					graphMgr->GPalette().SetVary(pal, 100, arg(3) * 60);
				else if (argCount < 5)            
					graphMgr->GPalette().SetVary(pal, arg(4), arg(3) * 60);
				else 
					graphMgr->GPalette().SetVary(pal, arg(4), arg(3) * 60, arg(5), arg(6));
         }	
			break;

		case PALVARYREVERSE:

         /* input:
               target palette will be orginal start palette and start palette
               will be orignal target palette
					 
               arg2 = time to shift palette in seconds (not required unless arg3 wanted)
							 if time = zero then palette changed immediately
               arg3 = target percent for shift (not required unless arg4 wanted)
							 If not present 0 assumed to be target percent
							 to destination percent
			*/
			if (argCount < 2)
				graphMgr->GPalette().SetVaryPercent(0); 			 
			else if (argCount < 3)
				graphMgr->GPalette().SetVaryPercent(0, arg(2) * 60);
			else             
				graphMgr->GPalette().SetVaryPercent(arg(3), arg(2) * 60);
         break;

		case PALVARYINFO:

			/* return percent of palette change */
			pm.acc = graphMgr->GPalette().VaryPercent();
			break;


		case PALVARYNEWTIME:			// Change the time for reaching target

         /* input:
               arg2 = time to finish (required)
			*/
			graphMgr->GPalette().SetVaryTime(arg(2)*60);
			break;

		case PALVARYKILL:

         /* stop palette change */
			graphMgr->GPalette().VaryOff();
			break;

		case PALVARYTARGET:

         /* input:
               arg2 = target palette number (required)
         */
         /* insert custom colors into target palette */
         /* Note: custom colors are inserted into start
				palette by RSetPalette */ 
			{
				palID = resMgr->Get(MemResPalette, arg(2));
				HunkPalette hpal;
				hpal.Init(*palID);
				SOLPalette pal(hpal);
				graphMgr->GPalette().MergeTarget(pal);
				pm.acc = graphMgr->GPalette().VaryPercent();
         }
			break;

		case PALVARYPAUSE:

         /* input:
               arg2 = TRUE or FALSE (required)
               TRUE:  Pause the palette change.
               FALSE  Restart the palette change.
         */
         /* Note:
               If PalVary is paused n times with TRUE it will require
               n calls to PalVary pause with FALSE to restart the palette
               changing.
		  */
        	if (arg(2)) 
				graphMgr->GPalette().VaryPause();
			else
				graphMgr->GPalette().VaryOn();

			break;

      case PALVARYNEWTARGET:
			{
				palID = resMgr->Get(MemResPalette, arg(2));
				HunkPalette hpal;
				hpal.Init(*palID);
				SOLPalette pal(hpal);
				graphMgr->GPalette().SetTarget(pal);
				pm.acc = graphMgr->GPalette().VaryPercent();
         }
			break;

      case PALVARYNEWSOURCE:
			{
				palID = resMgr->Get(MemResPalette, arg(2));
				HunkPalette hpal;
				hpal.Init(*palID);
				SOLPalette pal(hpal);
				graphMgr->GPalette().SetStart(pal);
				pm.acc = graphMgr->GPalette().VaryPercent();
         }
			break;

      case PALVARYMERGESOURCE:
			{
				palID = resMgr->Get(MemResPalette, arg(2));
				HunkPalette hpal;
				hpal.Init(*palID);
				SOLPalette pal(hpal);
				graphMgr->GPalette().MergeStart(pal);
				pm.acc = graphMgr->GPalette().VaryPercent();
         }
			break;

	}

}

void
KPalette(argList)
{     
	enum palFunc {
		PALNULLENUM, PALLoad, PALIntensity, PALMatch
	};
	MemID palID;
	switch (arg(1)){
		case PALLoad:		/* gets a palette resource number */
			{
				palID = resMgr->Get(MemResPalette, arg(2));
				HunkPalette hpal;
				hpal.Init(*palID);
				graphMgr->GPalette().Submit(hpal);
         }
			break;

		case PALIntensity:	/* gets a start, end and intensity */
			graphMgr->GPalette().SetFade(arg(4), arg(2), arg(3));
			break;

		case PALMatch:	/* given R/G/B values, returns the index of this color */
			{
				Rgb24 aGun((uchar) arg(2), (uchar) arg(3), (uchar) arg(4));
				pm.acc = graphMgr->GPalette().Current().Match(aGun);
         }
			break;
	}
}  

void
KPalCycle(argList)
{                               
	enum cycleFunc {
		CycleStart, DoCycle, CyclePause, CycleGo, CycleOff
	};
	switch (arg(1)) {

		case CycleStart:
			//Cycle the palette
			// arg(2) start
			// arg(3) end
         // arg(4) direction
         // arg(5) optional ticks
			if (argCount < 5)
				graphMgr->GPalette().SetCycle(arg(2), arg(3), arg(4));
			else
				graphMgr->GPalette().SetCycle(arg(2), arg(3), arg(4), arg(5));
			break;

		case DoCycle:
			//Cycle
			// arg(2) what is the start of the Cycle to cycle
			// arg(3) how many steps to cycle optional (default 1) 
			if (argCount < 3)
				graphMgr->GPalette().DoCycle(arg(2));
			else
				graphMgr->GPalette().DoCycle(arg(2), arg(3));
			break;

		case CyclePause:
			graphMgr->GPalette().CyclePause(arg(2)); 
			break;

		case CycleGo:
			graphMgr->GPalette().CycleOn(arg(2)); 
			break;

		case CycleOff: 
			if (argCount < 2)
				graphMgr->GPalette().CycleAllOff();
			else
				graphMgr->GPalette().CycleOff(arg(2));
			break;

	}
}


