/*
		The purpose of the RectList class is to keep a list
		of IRects that can have "unused" entries in it.  An
		unused entry is created when a list element is deleted.

		Method summary:

		Add  -  Adds copy of IRect to the bottom of the list, even
				  if there are unused entries elsewhere in the list.
				  This behavior is depended on so that looping from
				  beginning to end will process newly added items.

		Length- Returns maximum used IRect index + 1.  Deleting an
				  item will not reduce the length, even if its the
				  last item in the list.

		Exists- Reports whether this lists element has been deleted or not.

		oper [] Returns reference to specified IRect in list.


 */

#if !defined (RECTLIST_HPP)
#define RECTLIST_HPP

#ifndef RECT_HPP
#include	"rect.hpp"
#endif



const MAXIR = 200;
class RectList
{
	public:
	//  Create & destroy object
	RectList();
	~RectList();
	RectList(const RectList&);
	const RectList& operator=(const RectList&);

	//  Const Methods
	const SOL_Rect& operator[](int index) const	{return *rectArray[index];}
	int	Length() const									{return length;}

	//  Normal methods
	SOL_Rect&	operator[](int index)				{return *rectArray[index];}
	void	Add(const SOL_Rect&);
	void	Delete(int index);
	int	Pack();
	void	Clear();

	protected:
	SOL_Rect* rectArray[MAXIR];
	int	length;
};

#endif
