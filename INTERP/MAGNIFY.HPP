
#ifndef MAGNIFY_HPP
#define MAGNIFY_HPP

#ifndef MEMID_HPP
#include "memid.hpp"
#endif
						
#ifndef OBJECT_HPP
#include "object.hpp" 
#endif
						
#ifndef POINT_HPP
#include "point.hpp"  
#endif

#ifndef RECT_HPP
#include "rect.hpp"   
#endif

#ifndef GRAPHMGR_HPP
#include "graphmgr.hpp"
#endif

class MagnifyView
{
	public:
	MagnifyView(ObjectID ID, MagnifyView* theNext);
	~MagnifyView();
	
	void FillData(SOL_Point pt);
	void Update();

	ObjectID id, magID;
	MemID BMData, FGData, BGData;
   int   BMWidth, BMHeight, BGWidth, BGHeight;
   int   view, loop, cel;  
   SOL_Point org;
   SOL_Rect boundry;
	uchar remapColor;
	uchar skip;
   int power;
	MagnifyView* next;
	ScreenItem* si;
};

#endif
