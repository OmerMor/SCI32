#ifndef SILIST_HPP
#define SILIST_HPP

#ifndef SI_HPP
#include "si.hpp"
#endif


const MAXSI = 250;
class ScreenItemList
{
	public:
	//  Create & destroy object
	ScreenItemList();
	~ScreenItemList();
	ScreenItemList(const ScreenItemList&);
	const ScreenItemList& operator=(const ScreenItemList&);

	//  Const Methods
	int	Length() const							{return length;}
	Bool	Exists(int index) const				{return (index < length) && (siArray[index] != NULL);}
	int	Search(const ScreenItem*) const;	//  ID BASED
	int	Search(long) const;					//  ID BASED

	//  Normal methods
	ScreenItem& operator[](int index) 		{return *siArray[index];}
	int	Add(ScreenItem*);						//  Add in priority order
	void	Delete(const ScreenItem*);
	void	Delete(int index);
	void	Clear();
	void	Sort();
	void	Pack();

	protected:
	ScreenItem* siArray[MAXSI];
	int	length;
};

#endif
