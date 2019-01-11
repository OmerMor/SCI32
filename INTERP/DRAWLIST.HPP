#ifndef DRAWLIST_HPP
#define DRAWLIST_HPP

#ifndef SI_HPP
#include "si.hpp"
#endif

const MAXDRAW = 250;

class DrawItem
{
	public:
	DrawItem(ScreenItem* theSi,SOL_Rect& rect):onScreen(rect.A,rect.B)
		{si = theSi;} 

	void			Draw(Buffer& dest)	{si->Draw(dest,onScreen);}
	SOL_Rect&	OnScreen()				{return onScreen;}
	ScreenItem&	SI()						{return *si;}

	protected:

	ScreenItem*		si;
	SOL_Rect			onScreen;

	friend class	DrawList;
};

class DrawList
{
	public:
	//  Create & destroy object
	DrawList();
	~DrawList();

	//  Const Methods
	int	Length() const							{return length;}
	Bool	Exists(int index) const				{return (index < length) && (drawArray[index] != NULL);}

	//  Normal methods
	DrawItem& operator[](int index) 			{return *drawArray[index];}
	int	Add(ScreenItem*,SOL_Rect&);		// Added in priority order
	void	Delete(int index);
	void	Pack();
	void	Sort();

	protected:
	int			length;
	DrawItem* 	drawArray[MAXDRAW];
};

#endif

