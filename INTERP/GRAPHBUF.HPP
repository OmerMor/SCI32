#ifndef GRAPHBUF_HPP
#define GRAPHBUF_HPP

#ifndef GRAPHMGR_HPP
#include "graphmgr.hpp"
#endif

class GraphicsMgrBuffered : public GraphicsMgr
{
	public:
	GraphicsMgrBuffered(int xdim, int ydim);
	~GraphicsMgrBuffered()						{}

	int  				ScreenCount() const     {return 1;}
	const Screen&	VisibleScreen() const   {return last;}
	const Screen&	NextScreen() const		{return next;}

	Bool		IsOnMe(const ScreenItemDef&, const SOL_Point&, Bool checkSkip);
	void		UpdateScreen()							{last = next;}
	Bool		PlaneExistsLast(int planeId)		{return last.PlaneExists(planeId);}
	void		ClearLastScreen()						{last.Clear();}
	Screen*	CurrScreen()                  	{return &last;}

	protected:
	void		CalcLists(ScreenItemList** draw, IRectList** erase);

	Screen	last;
};

#endif

