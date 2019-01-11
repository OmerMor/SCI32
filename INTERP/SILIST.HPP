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
	const ScreenItem& operator[](int index) const 		{return *siArray[index];}

	//  Normal methods
	ScreenItem& operator[](int index) 		{return *siArray[index];}
	int	Add(ScreenItem*);						//  Add in priority order // cant be const
	void	Delete(const ScreenItem*);
	void	Delete(int index);
	void	Clear();
	void	Sort();
	void	Pack();
	void	Unsort();

	protected:
	ScreenItem* siArray[MAXSI];
	int	order[MAXSI];
	int	length;
};

#endif
