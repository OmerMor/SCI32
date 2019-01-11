//	drawlist.cpp

#include "sol.hpp"

#include "msg.hpp"
#include "drawlist.hpp"


//  Constructor
//  ===========
DrawList::DrawList()
{
	length = 0;
}

//  Destructor
//  ==========
DrawList::~DrawList()
{
	for (int i = 0; i < length; i++)
		delete drawArray[i];
}

//  Add method
//  ==========
//  Add screenitem to list at end
int
DrawList::Add(ScreenItem* screenItem,SOL_Rect& drawRect)
{
	if (length == MAXDRAW)
		msgMgr->Fatal(SrcLoc,"Draw items list exceeds %d",MAXDRAW);

	drawArray[length] = New DrawItem(screenItem,drawRect);
	return (length++);
}

//  Pack Method
//  ===========
//  Packs the list so no NULLs are in 
//  first length entries of list.
void
DrawList::Pack()
{
	DrawItem** src = drawArray;
	DrawItem** dest = drawArray;
	int newlen = 0;

	for (int i = 0; i < length; i++, src++)  {
		if (*src)  {
			*dest++ = *src;
			newlen++;
		}
	}
	length = newlen;
}

//  Delete Method
//  =============
void
DrawList::Delete(int index)
{

	if ((index < 0) || (index > length) || (drawArray[index] == NULL) )
		msgMgr->Fatal(SrcLoc, Msg_InvalidScreenItemIndex);

	delete drawArray[index];
	drawArray[index] = NULL;
}

//  Sort Method
//  ===========
void
DrawList::Sort()
{
	//  Sorts Draw List from lo to high priority
	//  The list is packed before sorting.
	//
	//  Algorithm is modified bubble sort.   Feel free to replace
	//  with a better performance one, but remember that the list is
	//  usually very small (est. 100 elements or less).
	Pack();
	for (int i = length - 1; i > 0; i--)  {
		int swap = False;

		for (int j = 0; j < i; j++)  {
			if (
				   (drawArray[j]->si->Priority() > drawArray[j+1]->si->Priority()) 
				   ||
				   (
				      (drawArray[j]->si->Priority() == drawArray[j+1]->si->Priority()) 
				      &&
                  (
				         (drawArray[j]->si->pos.y + drawArray[j]->si->z > drawArray[j+1]->si->pos.y+ drawArray[j+1]->si->z)
				         ||
                     (
				            (drawArray[j]->si->pos.y + drawArray[j]->si->z == drawArray[j+1]->si->pos.y + drawArray[j+1]->si->z)
                        &&
                        (drawArray[j]->si->Id() > drawArray[j+1]->si->Id())
				         )
                  )
               )
				)  {
				DrawItem* temp = drawArray[j+1];
				drawArray[j+1] = drawArray[j];
				drawArray[j] = temp;
				swap = True;
			}
		}
		if (!swap)
			break;
	}
}

