//	rectlist.cpp

#include "sol.hpp"

#include "msg.hpp"
#include "rect.hpp"
#include "rectlist.hpp"



/*
 *
 *  Constructor
 *  ===========
 *
 */

RectList::RectList()
{
	length = 0;
}

/*
 *
 *  Destructor
 *  ==========
 *
 */

RectList::~RectList()
{
	for (int i = 0; i < length; i++)
		delete rectArray[i];
}

/*
 *
 *  Copy Init
 *  =========
 *
 */

RectList::RectList(const RectList& src)
{
	length = src.length;
	for (int i = 0; i < length; i++)  {
		if (src.rectArray[i])
			rectArray[i] = New SOL_Rect(*src.rectArray[i]);
		else
			rectArray[i] = NULL;
	}
}


/*
 *
 *  Assignment operator
 *  ===================
 *
 */

const RectList&
RectList::operator=(const RectList& src)
{
	for (int i = 0; i < length; i++)
		delete rectArray[i];

	length = src.length;
	for (i = 0; i < length; i++)  {
		if (src.rectArray[i])
			rectArray[i] = New SOL_Rect(*src.rectArray[i]);
		else
			rectArray[i] = NULL;
	}
	return *this;
}


/*
 *
 *  Add method
 *  ==========
 *
 */

void
RectList::Add(const SOL_Rect& src)
{
	if (length == MAXIR)
		msgMgr->Fatal(SrcLoc, Msg_RectListNoRoom);
	rectArray[length++] = New SOL_Rect(src);
}


/*
 *
 *  Delete method
 *  =============
 *
 */

void
RectList::Delete(int index)
{
	delete rectArray[index];
	rectArray[index] = NULL;
}


/*
 *
 *  Pack method
 *  ===========
 *
 */

int
RectList::Pack()
{
	SOL_Rect** src = rectArray;
	SOL_Rect** dest = rectArray;
	int newlen = 0;

	for (int i = 0; i < length; i++)  {
		if (*src == NULL)  {
			src++;
			continue;
		}
		newlen++;
		*dest++ = *src++;
	}
	length = newlen;
	return length;
}


/*
 *
 *  Clear method
 *  ============
 *
 */

void
RectList::Clear()
{
	for (int i = 0; i < length; i++)  {
		delete rectArray[i];
		rectArray[i] = NULL;
	}
	length = 0;
}

