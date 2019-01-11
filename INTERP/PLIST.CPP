//	plist.cpp

#include "sol.hpp"

#include "msg.hpp"
#include "plane.hpp"
#include "plist.hpp"


PlaneList::PlaneList()
{
	length = 0;
}


PlaneList::~PlaneList()
{
	for (int i = 0; i < length; i++)
		delete planeArray[i];
}

PlaneList::PlaneList(const PlaneList& src)
{
	length = src.length;
	for (int i = 0; i < length; i++)
		planeArray[i] = New Plane(*src.planeArray[i]);
}

const PlaneList&
PlaneList::operator=(const PlaneList& src)
{
	Clear();
	length = src.length;
	for (int i = 0; i < length; i++)
		planeArray[i] = New Plane(*src.planeArray[i]);

	return *this;
}


int
PlaneList::Add(Plane* plane)
{
	if (length == MAXPLANES)
		msgMgr->Fatal(SrcLoc, Msg_TooManyPlanes);

	for (int i = 0; i < length; i++)  {
		if (planeArray[i]->Priority() >= plane->Priority())
			break;
	}

	for (int j = length; j > i; j--)
		planeArray[j] = planeArray[j - 1];

	planeArray[i] = plane;
	length++;
	return plane->Id();
}


void
PlaneList::Delete(int index)
{
	if (index >= length)
		msgMgr->Fatal(SrcLoc, Msg_InvalidPlaneIndex);

	planeArray[index]->SIList().Clear();
	delete planeArray[index];
	for (int i = index + 1; i < length; i++)
		planeArray[i - 1] = planeArray[i];
	length--;
}


int
PlaneList::Search(long id) const
{
	for (int i = 0; i < length; i++)  {
		if (planeArray[i]->Id() == id)
			return i;
	}
	return -1;
}

void
PlaneList::Clear()
{
	for (int i = 0; i < length; i++)
		delete planeArray[i];
	length = 0;
}


/*
 *
 *  Sort Method
 *  ===========
 *  Sorts PlaneList List from lo to high priority
 *
 *  Algorithm is modified bubble sort.   Feel free to replace
 *  with a better performance one, but remember that the list is
 *  usually very small (est. 5 elements or less).
 *
 */

void	
PlaneList::Sort()
{
	for (int i = length - 1; i > 0; i--)  {
		int swap = False;

		for (int j = 0; j < i; j++)  {
			if (
				(planeArray[j]->Priority() > planeArray[j+1]->Priority()) 
				||
				(
				(planeArray[j]->Priority() == planeArray[j+1]->Priority()) 
				&&
				(planeArray[j]->Id() > planeArray[j+1]->Id())
				)
				)  {
				Plane* temp = planeArray[j+1];
				planeArray[j+1] = planeArray[j];
				planeArray[j] = temp;
				swap = True;
			}
		}
		if (!swap)
			break;
	}
}

int
PlaneList::TopPlane()
{
	if (length)
		return planeArray[length-1]->Priority();
	else
		return 0;
}


int
PlaneList::TopSCIPlane()
{
   // This method returns the highest priority
   // for a non_Window plane (Window planes begin
   // at priority PLANEPRIORITYSTART.
   int topPri = 0;
	if (length) {
	   for (int i = 0;i < length;i++)  {
         if (planeArray[i]->Priority() < PLANEPRIORITYSTART)
            topPri = planeArray[i]->Priority();
         else
            break;
      }
   }
	return topPri;
}
