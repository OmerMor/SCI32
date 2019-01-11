//	silist.cpp

#include "sol.hpp"

#include "msg.hpp"
#include "silist.hpp"


//  Constructor
//  ===========
ScreenItemList::ScreenItemList()
{
	length = 0;
}

//  Destructor
//  ==========
ScreenItemList::~ScreenItemList()
{
	for (int i = 0; i < length; i++)	{
		delete siArray[i];
	}
}

//  Assignment operator
//  ===================

const ScreenItemList&
ScreenItemList::operator=(const ScreenItemList& src)
{
	for (int i = 0; i < length; i++)  {
		delete siArray[i];
		siArray[i] = NULL;
	}

	length = src.length;
	for (i = 0; i < length; i++)  {
		if (src.siArray[i]) {
			siArray[i] = New ScreenItem(src.siArray[i]);
		}
		else
			siArray[i] = NULL;
	}

	return *this;
}

//  Add method
//  ==========
//  Add screenitem to list at end
int
ScreenItemList::Add(ScreenItem* si)
{
	if (length == MAXSI)
		msgMgr->Fatal(SrcLoc, Msg_TooManyScreenItems);
	siArray[length] = si;
	return length++;
}

//  Clear Method
//  ============
//  Resets the list to no entries
void
ScreenItemList::Clear()
{
	for (int i = 0; i < length; i++)  {
		delete siArray[i];
		siArray[i] = NULL;
	}
	length = 0;
}

//  Pack Method
//  ===========
//  Packs the list so no NULLs are in 
//  first length entries of list.
void
ScreenItemList::Pack()
{
	ScreenItem** src = siArray;
	ScreenItem** dest = siArray;
	int newlen = 0;

	for (int i = 0; i < length; i++, src++)  {
		if (*src)  {
			*dest++ = *src;
			newlen++;
		}
	}
	length = newlen;
}

//  Search Method
//  =============
//  Searches for ScreenItem with same ID as arg object
//  Returns -1 if Id not found in list
int
ScreenItemList::Search(const ScreenItem* si) const
{
	long id = si->Id();
	for (int i = 0; i < length; i++)  {
		if (siArray[i]  &&  siArray[i]->Id() == id)
			return i;
	}
	return -1;
}

int
ScreenItemList::Search(long sItemID) const
{
	for (int i = 0; i < length; i++)  {
		if (siArray[i]  &&  siArray[i]->Id() == sItemID)
			return i;
	}
	return -1;
}

//  Delete Method
//  =============
void
ScreenItemList::Delete(const ScreenItem* si)
{
	int index = Search(si);
	if (index == -1)
		msgMgr->Fatal(SrcLoc, Msg_CantFindSI);
	delete siArray[index];
	siArray[index] = NULL;
}

//  Delete Method
//  =============
void
ScreenItemList::Delete(int index)
{

	if ((index < 0) || (index > length) || (siArray[index] == NULL) )
		msgMgr->Fatal(SrcLoc, Msg_InvalidScreenItemIndex);
	delete siArray[index];
	siArray[index] = NULL;
}

//  Sort Method
//  ===========
void
ScreenItemList::Sort()
{
	//  Sorts ScreenItem List from lo to high priority
	//  The list is packed before sorting.
	//
	//  Algorithm is modified bubble sort.   Feel free to replace
	//  with a better performance one, but remember that the list is
	//  usually very small (est. 100 elements or less).

	for (int i=0;i<length;++i)
		order[i] = i;	
	for (i = length - 1;i > 0;i--)  {
		int swap = False;

		for (int j = 0; j < i; j++)  {
			if (
					(siArray[j] == NULL)
					||
				   (siArray[j]->Priority() > siArray[j+1]->Priority()) 
				   ||
				   (
				      (siArray[j]->Priority() == siArray[j+1]->Priority()) 
				      &&
                  (
				         (siArray[j]->pos.y + siArray[j]->z > siArray[j+1]->pos.y+ siArray[j+1]->z)
				         ||
                     (
				            (siArray[j]->pos.y + siArray[j]->z == siArray[j+1]->pos.y + siArray[j+1]->z)
                        &&
                        (siArray[j]->Id() > siArray[j+1]->Id())
				         )
                  )
               )
				)  {
				ScreenItem* temp = siArray[j+1];
				siArray[j+1] = siArray[j];
				siArray[j] = temp;
				// Keep track of the unsorted order
				int tmp = order[j+1];
				order[j+1] = order[j];
				order[j] = tmp;
				swap = True;
			}
		}
		if (!swap)
			break;
	}
}

void
ScreenItemList::Unsort()
{
	// Put the list back to it's original order
	for (int i=0;i<length;++i) {
		while (order[i] != i) {
			ScreenItem* temp = siArray[order[i]];
			int tmp = order[order[i]];

			siArray[order[i]] = siArray[i];
			order[order[i]] = order[i];

			siArray[i] = temp;
			order[i] = tmp;
		}
	}
}
