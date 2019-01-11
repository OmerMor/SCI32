#ifndef PLIST_HPP
#define PLIST_HPP

#ifndef PLANE_HPP
#include "plane.hpp"
#endif

#include <stdio.h>

const MAXPLANES = 100;

class PlaneList
{
	public:
	//  Create & destroy object
	PlaneList();
	~PlaneList();
	PlaneList(const PlaneList&);
	const PlaneList& operator=(const PlaneList&);

	//  Const Methods
	int	Length() const								{return length;}
	int	Search(long id) const;				//  ID BASED
	int	TopPlane() const;
	int	TopSCIPlane() const;
	const Plane& operator[](int index) const	{return *planeArray[index];}

	//  Normal methods
	Plane& operator[](int index) 					{return *planeArray[index];}
	Plane* Pointer(int index)						{return planeArray[index];}
	int	Add(Plane*);				//   cant be const
	void	Delete(int index);
	void	Clear();
	void	Sort();

	protected:
	Plane* planeArray[MAXPLANES];
	int	length;
};


#endif
