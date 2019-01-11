#ifndef SCALE_HPP
#define SCALE_HPP

#ifndef MSG_HPP
#include "msg.hpp"
#endif

struct ScaleInfo
{
	public:
	ScaleInfo(int x, int y, int max, int type);
	ScaleInfo();
	Bool	operator==(const ScaleInfo&) const;


	int scaleX;			// User specified scaling factor (128 means no scaling)
	int scaleY;       // User specified scaling factor (128 means no scaling)
	int maxScale;
	int scaleType;    // enum {OFF,MANUAL,VANISHING}
};

const SCALE_OFF = 0;		//  initializer, not for comparisons!
const SCALE_ON_BIT = 1;
const SCALE_MANUAL = 1;
const SCALE_VANISHING = 3;

inline
ScaleInfo::ScaleInfo(int x, int y, int max, int type)
{
	scaleX = x;
	scaleY = y;
	maxScale = max;
	scaleType = type;
	if ((scaleType & SCALE_MANUAL) && (!scaleX || !scaleY))
		msgMgr->Fatal("Instructed to scale to zero");
}

inline
ScaleInfo::ScaleInfo()
{
	scaleX = 128;
	scaleY = 128;
	maxScale = 100;
	scaleType = SCALE_OFF;
}

inline Bool
ScaleInfo::operator==(const ScaleInfo& src) const
{
	return (
		scaleX == src.scaleX  &&
		scaleY == src.scaleY  &&
		maxScale == src.maxScale &&
		scaleType == src.scaleType
	);
}

#endif

