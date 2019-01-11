#ifndef SCREEN_HPP
#define SCREEN_HPP

#ifndef PLIST_HPP
#include "plist.hpp"
#endif

#ifndef RECT_HPP
#include "rect.hpp"
#endif

#ifndef RECTLIST_HPP
#include "rectlist.hpp"
#endif

class ScreenItem;
class ScreenItemList;
class Plane;

class Screen
{
	public:

	//  Constructors, etc
	Screen(int xdim, int ydim);
	Screen&	operator=(const Screen&);

	//  Const
	const	SOL_Rect& ScreenRect() 			{return screenRect;}
	int	TopPlane() 							{return planes.TopPlane();}
	int	TopSCIPlane() 						{return planes.TopSCIPlane();}
	int	TopScreenItem(int planeID);
	void	NowSeen(int id,int planeID,SOL_Rect& nowSeen);
	PlaneList& Planes() 						{return planes;}
	const PlaneList& Planes() const		{return planes;}
	void	PlaneRect(int planeID,SOL_Rect& planeRect);
	int	Xdim() const						{return xDim;}
	int	Ydim() const						{return yDim;}

	//  Normal methods
	void	AbortPlane(Plane*);
	void	AddScreenItem(ScreenItem*);
	void	AddPlane(Plane*);
	int	AddPic(long planeID,int thePic,SOL_Point& thePoint,int mirror);
	void	CalcLists(Screen& last,DrawList* drawLists[],RectList* eraseLists[]);
	void	Clear();
	void	DeletePic(long planeID,int oldPic,int newPic);
	Plane*
			GetPlane(int planeId);
	ScreenItem* 
			GetScreenItem(int planeId,long siId);
	void	NewScreenDim(int xdim, int ydim);
	void	RemapMarkRedraw();
	void	ScrollScreenItems(long planeID,int xStep,int yStep,Bool movePic);
	RectList& 
			ShowList() 							{return showList;}
	void	SortPlanes()						{planes.Sort();}

	protected:

	PlaneList	planes;
	SOL_Rect		screenRect;
	RectList		showList;		// List of rectangles put to the VMAP for this screen
	int			xDim, yDim;
};

inline
Screen::Screen(int newXdim, int newYdim)
{
	xDim = newXdim;
	yDim = newYdim;
	screenRect.Set(0, 0, xDim - 1, yDim - 1);
}


inline Screen&
Screen::operator=(const Screen& src)
{
	xDim = src.xDim;
	yDim = src.yDim;
	return *this;
}


#endif
