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
	const	SOL_Rect& ScreenRect() const	{return screenRect;}
	int	TopPlane() const					{return planes.TopPlane();}
	int	TopSCIPlane() const				{return planes.TopSCIPlane();}
	int	TopScreenItem(int planeID) const;
	int	Xdim() const						{return xDim;}
	int	Ydim() const						{return yDim;}
	const PlaneList& Planes() const		{return planes;}

	PlaneList& Planes() 						{return planes;}
	void	PlaneRect(int planeID, SOL_Rect& planeRect) const;

	//  Normal methods
	void	AbortPlane(const Plane*);
	void	AddScreenItem(ScreenItem*);			//  Cant be const
	void	AddPlane(Plane*);							//  Cant be const
	int	AddPic(long planeID, int thePic, const SOL_Point&, int mirror);
	void	CalcLists(Screen& last,DrawList* drawLists[],RectList* eraseLists[], const SOL_Rect& dirtyRect);
	void	Clear();
#ifdef MACINTOSH
	void	ClearForRestoreGame();
#endif
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