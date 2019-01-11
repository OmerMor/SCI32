//	math.cpp

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sol.hpp"

#include "kernel.hpp"
#include "math.hpp"
#include "pmachine.hpp"
#include "point.hpp"
#include "savegame.hpp"

static ulong	lcgSeed;

static int getMajor(int,int,int,int);
static int prorateATan(long);

static long sinArray[]= {
	    0,
	  872,
	 1736,
	 2588,
	 3420,
	 4226,
	 5000,
	 5736,
	 6428,
	 7071,
	 7660,
	 8192,
	 8660,
	 9063,
	 9397,
	 9659,
	 9848,
	 9962,
	10000
};
		
static long atanArray[] = {
     875,
    1763,
    2679,
    3640,
    4663,
    5774,
    7002,
    8391,
   10000
};


const uint Generator = 0x015A7C4D;
const	Increment		= 1;

ushort firstGuess[] = 
   { 
      0x0001,  0x0002,  0x0003,  0x0004,  0x0005,  0x0007,  0x000A,  0x000E,
      0x0014,  0x001C,  0x0027,  0x0037,  0x004E,  0x006F,  0x009D,  0x00DE,
      0x0139,  0x01BB,  0x0273,  0x0376,  0x04E6,  0x06ED,  0x09CC,  0x0DDB,
      0x1398,  0x1BB6,  0x2731,  0x376C,  0x4E62,  0x6ED9,  0x9CC4,  0xFFFF
   };

// return the sqrt of a long int using two iterations of Newton-Raphson based
// on an educated intial guess.the guess for the largest range is too high 
// avoid any possible overflow.  This make the answer inacurate for high
// ranges.

// N-R:
//    guess(i+1) = guess(i) + n/guess(i)) / 2;

uint
sqrt(ulong n)
{

   uint guess, hold;
   int i;

   if (!n)
      return 0;

   hold = n;

   for(i = 0; i < 32; i++)
      if(!(n = n >> 1))
         break;

   n = hold;

   guess = firstGuess[i];
   guess = (guess + n/guess)/2;
   return (guess + n/guess)/2;
}

// return a pseudo random number via linear congruential generator
ulong
LCGRandom()
{
   while(!lcgSeed)
      time((time_t*) &lcgSeed);

   return(lcgSeed = (lcgSeed * Generator + Increment));
}

void
KAbs(argList)
{
	pm.acc = Abs(arg(1));
}

void
KSqrt(argList)
{
	pm.acc = (Acc) sqrt(Abs(arg(1)));
}

void
KGetAngle(argList)
{
	SOL_Point	sp;
	SOL_Point	dp;

	sp.x = arg(1);
	sp.y = arg(2);
	dp.x = arg(3);
	dp.y = arg(4);
	pm.acc = (Acc) RPtToAngle(&sp, &dp);
}

void
KGetDistance(argList)
	/*
	Compute distance between two points taking perspective into account
	perspective is optional fifth arg, default is zero
	perspective = departure from vertical along Y axis, in degrees
	*/
{
	long dx = Abs(arg(3) - arg(1));
	long dy = Abs(arg(4) - arg(2));

	if (argCount > 4)
		dy = CosDiv(arg(5), dy);

	pm.acc = (Acc) sqrt((dx * dx) + (dy * dy));
}

void
KSinMult(argList)
{
	pm.acc = (Acc) SinMult(arg(1), arg(2));
}

void
KCosMult(argList)
{
	pm.acc = (Acc) CosMult(arg(1), arg(2));
}

void
KSinDiv(argList)
{
	pm.acc = (Acc) SinDiv(arg(1), arg(2));
}

void
KCosDiv(argList)
{
	pm.acc = (Acc) CosDiv(arg(1), arg(2));
}

void
KATan(argList)
{
	pm.acc = (Acc) ATan(arg(1), arg(2), arg(3), arg(4));
}

void
KRandom(argList)		// low, high */
{
	unsigned	low;
	unsigned	high;
	unsigned	range;
	ulong	tmp;

	if (argCount == 2) {
		low = arg(1);
		high = arg(2);
		range = (unsigned) (high - low + 1);

		tmp = (LCGRandom() & 0x00ffff00L) >> 8;
		tmp *= range;
		tmp >>= 16;
		tmp += low;

		pm.acc = (Acc) tmp;

	} else if (argCount == 1) {
		// Set seed to argument
		lcgSeed = *(long*) arg(1);
		pm.acc = False;

	} else if (argCount == 3) {
		*(long*) arg(3) = lcgSeed;
		pm.acc = False;

	} else {
		// assume argCount == 0
		// Return seed value
		pm.acc = (Acc) lcgSeed;
	}
}

void
KReadNumber(argList)
{
	//	read a number from the input line

	pm.acc = (Acc) atoi((char*) arg(1));
}

int
RPtToAngle(SOL_Point* sp, SOL_Point* dp)
{
	// return pseudo angle 0-359

	return ATan(dp->y, sp->x, sp->y, dp->x);
}

long
ScaledSin(int a)
{
	unsigned	entry;
	long	entryValue;
	long	deltaValue;
	
	if (a <  0)
		return ScaledSin(a + 360);

	if (a <= 90) {									// 1st quadrant, interpolate
		entry = (unsigned) (a / TrigStep);
		entryValue = sinArray[entry];
		deltaValue = sinArray[entry+1] - entryValue;
		
		return (long)
	      (entryValue +
	      ((deltaValue*(long)(a%TrigStep) + ((long)TrigStep/(long)2))
		      / (long)TrigStep));
	}

	if (a <= 180)
		return ScaledSin(180 - a);		// 2nd
	if (a <= 270)
		return -ScaledSin(a - 180);	// 3rd
	if (a <= 360)
		return -ScaledSin(360 - a);	// 4th

	return ScaledSin(a - 360);
}

long
ScaledCos(int a)
{
	if (a < 0)
		return ScaledCos(a + 360);
			
	if (a <= 90)
		return ScaledSin(90 - a);		// 1st quadrant
	
	if (a <= 180)
		return -ScaledCos(180 - a);	// 2nd
	if (a <= 270)
		return -ScaledCos(a - 180);	// 3rd
	if (a <= 360)
		return ScaledCos(360 - a);		// 4th
	
	return ScaledCos(a - 360);
}

int
ATan(int x1, int y1, int x2, int y2)
{
	// returns heading from (x1,y1) to (x2,y2) in degrees (always in the range
	//	0-359)
	
	int major = getMajor(x1,y1,x2,y2);

   // 0 <= major <= 90 
   if (x2 < x1)
      if (y2 <= y1)
         major += 180;
      else
	      major = 180 - major;
   else
      if (y2 < y1)
      	major = 360 - major;
      if (major == 360)
      	major = 0;

   return major;
}

static int
getMajor(int x1, int y1, int x2, int y2)
{
	// returns angle in the range 0-90

	long	deltaX = Abs(x2 - x1);
	long	deltaY = Abs(y2 - y1);
   long	index;
	int major;

   // if (x1,y1) = (x2,y2) return 0
   if (!deltaX && !deltaY)
      return 0;

   if (deltaY <= deltaX) {
      index = TrigScale * deltaY / deltaX;
      if (index < 1000)
			major = (int) (((long)57*deltaY + (deltaX/(long)2)) /deltaX);
      else
         major = prorateATan(index);

   } else
      major = 90 - getMajor(y1,x1,y2,x2);

   return major;
}

static int
prorateATan(long index)
{
	for (int i = 0; atanArray[i] < index; i++)
		;

   return 5*i +
		(int) ((((long)5*(index - atanArray[i-1])) +
   		((atanArray[i] - atanArray[i-1])/2))
			/ (atanArray[i] - atanArray[i-1]));
}

///////////////////////////////////////////////////////////////////////////

MathMgr mathMgr;

void
MathMgr::SaveRestore(SaveGame* saveGame, Bool save)
{
	saveGame->SaveRestore(&lcgSeed, sizeof lcgSeed, save);
}
