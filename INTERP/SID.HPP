
/*

		The purpose of the ScreenItemDef class is to provide the
		GraphManager user a way to specify one instance of a
		bitmap somewhere on a plane.

		The constructor of ScreenItemDef takes a PlaneDef argument
		(in order to associate the Screen item to that plane).

		ScreenItemDefs include:
			XY Position of screenitem (hotspot)
			View, Loop & Cel Number to use for bitmap
			User scaling information
			Inset if you only want to display part of the cel
			Fixed or floating priority.

		When a new ScreenItemDef is constructed, it receives a
		unique ID number.  (DAN:  We may want to take an optional
		parameter that lets the user specify a value to use for
		an ID (SCI OBJ ID).  Look at issues, remember Outpost).

		NOTES:
		Change from public interface access, use methods to change
		data.  We especially need a good way to specify scaling
		without the user needing to understand the scaling "norms" of 128.
		Also, fixed priority still seems confusing.

 */

#ifndef SID_HPP
#define SID_HPP

#ifndef RECT_HPP
#include "rect.hpp"
#endif

#ifndef CELOBJ_HPP
#include "celobj.hpp"
#endif

#ifndef MSG_HPP
#include "msg.hpp"
#endif

class ObjectID;
class PlaneDef;


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


class ScreenItemDef
{
	public:
	Bitmap		bitmap;		// The bitmap for this object
	SOL_Point	pos;			// User specified X, Y coordinates
	Coord			z;				// User specified Z coordinate
	ScaleInfo	scale;
	uchar			color;		// Used for lines and polygons


	//  Constructors, etc
	ScreenItemDef(ObjectID);
	ScreenItemDef(const ScreenItemDef&);
	ScreenItemDef(long plane, const Bitmap&);
	ScreenItemDef(long plane, Bitmap, SOL_Point, ScaleInfo, int id);
	ScreenItemDef(long plane, Bitmap, SOL_Rect&);
	ScreenItemDef& operator=(const ScreenItemDef&);
	virtual ~ScreenItemDef() {}

	//  Const Methods
	Bool	operator==(const ScreenItemDef&) const;
	Bool	operator!=(const ScreenItemDef&) const;
	long	Id() const							{return id;}
	int	PicResNum()							{return picResNum;}
	int	Priority() const					{return priority;}
	Bool	FixedPri() const					{return fixedPriFlag;}
	SOL_Rect&	InsetRect()					{return insetRect;}
	long	PlaneId() const					{return planeId;}
	Bool	UserMirror() const				{return userMirror;}

	//  Normal methods
	void	SetPicResNum(int pic)			{picResNum = pic;}
	void	SetPriority(int p)				{priority = p; fixedPriFlag = True;}
	void	SetFloatPriority()				{fixedPriFlag = False;}
	void	SetInsetRect(const SOL_Rect& r)	{insetRect = r; useInsetRect = True;}
	void	SetUserMirror(Bool f)			{userMirror = f;}
	void	ClearInsetRect()					{useInsetRect = False;}

	protected:
	static	long nextId;
	long		id;			// The object's ID (created by constructor)
	long		planeId;		// The plane for this object
	int		priority;  	// User specified priority relative to it's plane
	Bool		fixedPriFlag;
	Bool		useInsetRect;
	Bool		userMirror;
	SOL_Rect	insetRect;
	int		picResNum;		//  Picture this sid was derived from
};

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
