/*
		The purpose of the ScreenItem class is to hold all the
		information needed to represent one instance of a cel
		or line on a plane.

		When a ScreenItem is created, a plane is needed to help
		calculate the scaling and positioning.	Also, the resource 
		manager must be consulted to find out what the dimensions,
		resolution and hotspot is for a cel.
*/

#ifndef SI_HPP
#define SI_HPP

#ifndef RECT_HPP
#include "rect.hpp"
#endif

#ifndef RATIO_HPP
#include "ratio.hpp"
#endif

#ifndef SCALE_HPP
#include "scale.hpp"
#endif

#ifndef CELOBJ_HPP
#include "celobj.hpp"
#endif

class ObjectID;
class Plane;
class CelObj;

class ScreenItem
{
	enum {tRect, tLine};

	public:

	//  Constructors, etc
	ScreenItem(ScreenItem*);
	ScreenItem(ObjectID&);
	ScreenItem(long plane, Bitmap&);
	ScreenItem(long plane, Bitmap&, SOL_Rect&);
	ScreenItem(long plane, Bitmap&, SOL_Point, ScaleInfo);
	ScreenItem(long plane, Bitmap&, SOL_Point, ScaleInfo, int);
	~ScreenItem();
	ScreenItem& operator=(ObjectID&);
	ScreenItem& operator=(const ScreenItem&);

	// Given
	SOL_Point	pos;				// User specified X, Y coordinates
	Coord			z;					// User specified Z coordinate
	Bitmap		bitmap;			// User bitmap for this object
	ScaleInfo	scale;			// User scaling information
	uchar 		color;			// User color for color celobjs

	//  Const Methods
	Bool			FixedPri() const			{return fixedPriFlag;}
	long			Id() const					{return id;}
	SOL_Rect&	InsetRect()					{return insetRect;}
	void			NowSeen(Plane&, SOL_Rect& nowSeen);
	int			PicResNum()					{return picResNum;}
	long			PlaneId() const			{return planeId;}
	int			Priority() const			{return priority;}
	Ratio			ScaleX() const				{return finalScaleX;}
	Ratio			ScaleY() const				{return finalScaleY;}
	Bool			UserMirror() const		{return userMirror;}
	int			X1() const					{return x1;}
	int			Y1() const					{return y1;}
	const SOL_Rect& 	ToDraw() const		{return toDraw;}

	//  Normal methods
	int			AddCount() const			{return addCount;}
	void			ClearDeleteCount()		{deleteCount = 0;}
	void			ClearInsetRect()			{useInsetRect = False;}
	void			DecUpdateCount()			{updateCount--;}
	void			DecAddCount()				{addCount--;}
	void			DecDeleteCount()			{deleteCount--;}
	int			DeleteCount() const		{return deleteCount;}
	void			Draw(Buffer& dest, SOL_Rect& drawRect)
						{cel->Draw(dest,*this,drawRect,userMirror^cel->Mirrored());}
	void 			DoUpdate(Plane& plane)	
						{GetCelObj();Update(plane);}
	void 			GetCelObj();
	uchar			GetSkip()					{GetCelObj();return cel->Skip();}
	CelObj*		GetCel()						{return cel;}
	SOL_Rect& 	OnScreen()			{return onScreen;}
	uchar			ReadPix(int x, int y)			
						{GetCelObj(); return cel->ReadPix(x, y, userMirror ^ cel->Mirrored());}
	int			RemapState()  				{GetCelObj(); return cel->RemapState();}
	void			SetCel(CelObj* theCel)  {delete cel; cel = theCel;}
	void			SetUpdateCount(int val)	{updateCount = val;}
	void			SetAddCount(int val)		{addCount = val;}
	void			SetDeleteCount(int val)	{deleteCount = val;}
	void			SetInsetRect(const SOL_Rect& r)	
						{insetRect = r; useInsetRect = True;}
	void			SetOnScreen(const SOL_Rect& rect){onScreen = rect;}
	void			SetPicResNum(int pic)	{picResNum = pic;}
	void			SetPriority(int p)		{priority = p; fixedPriFlag = True;}
	void			SetFloatPriority()		{fixedPriFlag = False;}
	void			SetUserMirror(Bool f)	{userMirror = f;}
	void			SubmitPalette()			{GetCelObj(); cel->SubmitPalette();}
	void			SubmitPalette();
	int			UpdateCount() const		{return updateCount;}

	protected:
	static long nextId;			// For internal screen items and SCI lines
	long			id;				// The object's ID
	int			picResNum;		// Picture this si was derived from
	long			planeId;			// The plane's ID  for this object
	int			priority;  		// User specified priority relative to it's plane
	Bool			fixedPriFlag;	// User specified priority
	int			updateCount;
	int			addCount;
	int			deleteCount;
	CelObj*		cel;
	Bool			useInsetRect;
	Bool			userMirror;

	// Calculated
	int			x1, y1;			// Upper left coordinate (Scaled)(Screen relative)
	SOL_Rect		toDraw;			// Scaled (Plane relative)
	SOL_Rect		onScreen;		//	Scaled (Screen relative)
	Ratio			finalScaleX;
	Ratio			finalScaleY;
	SOL_Rect		insetRect;

	void			Update(Plane&);
	// Do not define
	ScreenItem(const ScreenItem&);
};

inline void
ScreenItem::GetCelObj()
{
	if (!cel) {
		switch (bitmap.Type()) {
			case Bitmap::tView:
				cel = New CelObjView(bitmap.View(),bitmap.Loop(),bitmap.Cel());
				break;
			case Bitmap::tPic:
				msgMgr->Fatal(SrcLoc,"Internal error, pic screen item with no cel.");
				break;
			case Bitmap::tMem:
				cel = New CelObjMem(bitmap.Handle());
				break;
			case Bitmap::tColor:
				cel = New CelObjColor(bitmap.Color(),insetRect.Xlen(),insetRect.Ylen());
				break;
		}
	}
}

#endif
