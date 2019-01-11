#include <string.h>
#include "sol.hpp"
#include "sid.hpp"
#include "object.hpp"
#include "planedef.hpp"
#include "resource.hpp"
#include "selector.hpp"
#include "graph.hpp"

long ScreenItemDef::nextId = 20000;


ScreenItemDef::ScreenItemDef(ObjectID ID):
	pos(ID.GetIndexedProperty(actX),
		 ID.GetIndexedProperty(actY)
		),
	scale(ID.GetIndexedProperty(actScaleX),
			ID.GetIndexedProperty(actScaleY),
			ID.GetIndexedProperty(actMaxScale),
			ID.GetIndexedProperty(actScaleSignal) & 3
		  )
{
	id = ID;

	uint view = ID.GetIndexedProperty(actView);
	uint loop = ID.GetIndexedProperty(actLoop);
	uint cel = ID.GetIndexedProperty(actCel);

   if (view < (uint) -2) {
	   // Load the view into memory
	   MemID viewID = resMgr->Get(MemResView, view);
	   ViewHeader* viewPtr = (ViewHeader*) *viewID;

	   if (viewPtr->loopCount <= loop) {
         loop = viewPtr->loopCount - 1;
			ID.SetIndexedProperty(actLoop, loop);
      }

	   LoopHeader* loopPtr = (LoopHeader*) ((char*) viewPtr + viewPtr->loopHeaderSize * loop + viewPtr->viewHeaderSize + 2); 
      if (loopPtr->altLoop != -1) {       
 		   loopPtr = (LoopHeader*) ((char*) viewPtr + viewPtr->loopHeaderSize*loopPtr->altLoop + viewPtr->viewHeaderSize + 2);
      }

	   if (loopPtr->numCels <= cel) {
         cel = loopPtr->numCels - 1;
			ID.SetIndexedProperty(actCel, cel);
      }

   }    
   bitmap = Bitmap(view, loop, cel);
	
   // If there is a font bitmap use it
	MemID FBM = ID.GetIndexedProperty(actBitmap);
	if ((int) FBM) {
      bitmap.SetMemID(FBM);
      bitmap.SetType(Bitmap::tMem);
   }

	planeId = ID.GetIndexedProperty(actPlaneID);

//	scale.scaleType = ID.GetProperty(s_scale_signal) & 3;
	if (ID.GetIndexedProperty(actFixedPri)) {
		priority = ID.GetIndexedProperty(actPri);
		fixedPriFlag = True;
	}
	else {
		fixedPriFlag = False;
		ID.SetIndexedProperty(actPri, pos.y);
	}

	//  Adjust for Z property
	z = ID.GetIndexedProperty(actZ);
	pos.y -= z;

	if (ID.GetIndexedProperty(actUseInsetRect))	{
		insetRect.x1 =	ID.GetIndexedProperty(actInsetRectL);
		insetRect.y1 =	ID.GetIndexedProperty(actInsetRectT);
		insetRect.x2 =	ID.GetIndexedProperty(actInsetRectR);
		insetRect.y2 =	ID.GetIndexedProperty(actInsetRectB);
		useInsetRect = True;
	}
	else {
		useInsetRect = False;
	}

	userMirror = False;
	picResNum = -1;

	ID->ClearGraphicsUpdate();
}

ScreenItemDef::ScreenItemDef(const ScreenItemDef& src)
{
	id = src.id;
	bitmap = src.bitmap;
	pos = src.pos;
	z = src.z;
	color = src.color;
	scale = src.scale;
	planeId = src.planeId;
	priority = src.priority;
	fixedPriFlag = src.fixedPriFlag;
	insetRect = src.insetRect;
	useInsetRect = src.useInsetRect;
	userMirror = src.userMirror;
	picResNum = src.picResNum;
}


//ScreenItemDef::ScreenItemDef(const PlaneDef& planedef)
//{
//	id = nextId++;
//	planeId = planedef.Id();
//	userMirror = False;
//}


ScreenItemDef::ScreenItemDef(long plane, Bitmap cel, SOL_Point p, ScaleInfo scInfo,
									 int identifier)
{
	id = identifier;
	pos = p;
	z = 0;
	planeId = plane;
	bitmap = cel;
	scale = scInfo;
	useInsetRect = False;
	fixedPriFlag = False;
	userMirror = False;
	picResNum = -1;
}

ScreenItemDef::ScreenItemDef(long plane,Bitmap cel,SOL_Rect& r)
{
	id = nextId++;
	pos.x = r.x1;
	pos.y = r.y1;
	z = 0;
	insetRect.x1 = 0;
	insetRect.y1 = 0;
	insetRect.x2 = r.Xlen() - 1;
	insetRect.y2 = r.Ylen() - 1;
	planeId = plane;
	bitmap = cel;
	useInsetRect = True;
	fixedPriFlag = False;
	userMirror = False;
	picResNum = -1;
}

ScreenItemDef::ScreenItemDef(long plane, const Bitmap& bm)
{
	// This constructor is used for pictures
	id = nextId++;
	planeId = plane;
	bitmap = bm;
	useInsetRect = False;
	fixedPriFlag = False;
	userMirror = False;
	picResNum = -1;
	pos.x = 0;
	pos.y = 0;
	z = 0;
}


Bool
ScreenItemDef::operator==(const ScreenItemDef& src) const
{
	return (
		bitmap == src.bitmap  &&
		pos == src.pos  &&
		z == src.z &&
		scale == src.scale  &&
		id == src.id  &&
		planeId == src.planeId  &&
		priority == src.priority  &&
		fixedPriFlag == src.fixedPriFlag  &&
		insetRect == src.insetRect  &&
		userMirror == src.userMirror  &&
		useInsetRect == src.useInsetRect &&
		picResNum == src.picResNum
	);
}

Bool
ScreenItemDef::operator!=(const ScreenItemDef& src) const
{
	return !operator==(src);
}


/*
 *
 *  Assignment operator
 *  ===================
 *  I don't understand why the compiler wont generate a default
 *  one in this case.  (This method Explicitly refered to in ScreenItem)
 *
 */

ScreenItemDef&
ScreenItemDef::operator=(const ScreenItemDef& src)
{
	id = src.id;
	bitmap = src.bitmap;
	planeId = src.planeId;
	pos = src.pos;
	z = src.z;
	color = src.color;
	scale = src.scale;
	priority = src.priority;
	fixedPriFlag = src.fixedPriFlag;
	insetRect = src.insetRect;
	useInsetRect = src.useInsetRect;
	userMirror = src.userMirror;
	picResNum = src.picResNum;

	return *this;
}
