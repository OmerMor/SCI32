//	math.hpp

#ifndef	MATH_HPP
#define	MATH_HPP

#ifndef SAVEABLE_HPP
#include "saveable.hpp"
#endif

struct MathMgr : Saveable {
	void	SaveRestore(SaveGame*, Bool save);
} extern mathMgr;

#define Sign(t)	((t) > 0 ? 1 : (t) < 0 ? -1 : 0)
#define Abs(t)		((t) < 0 ? -(t) : (t))

/*	n will be under 1000
	n*TrigScale must be less than largest long int (2,000,000,000)
	so TrigScale<2,000,000
*/
const long	TrigScale	= 10000;
const 		TrigStep	= 5;

class SOL_Point;

uint		sqrt(ulong);
ulong		LCGRandom();
int		RPtToAngle(SOL_Point*, SOL_Point*);
long		ScaledSin(int);
long		ScaledCos(int);
int		ATan(int,int,int,int);

inline int	SinMult(int a, long n)
	{ return ((ScaledSin(a) * n) / TrigScale); }

inline int CosMult(int a, long n)
	{ return ((ScaledCos(a) * n) / TrigScale); }

inline int SinDiv(int a, long n)
	{ return ((TrigScale * n) / ScaledSin(a)); }

inline int CosDiv(int a, long n)
	{ return ((TrigScale * n) / ScaledCos(a)); }

#endif
