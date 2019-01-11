//	graphmd.cpp

#include <bios.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>

#include "sol.hpp"

#include "graphbuf.hpp"
#include "msg.hpp"
#include "si.hpp"




GraphicsMgrBuffered::GraphicsMgrBuffered(int xdim, int ydim) : GraphicsMgr(xdim, ydim),
	last(xdim, ydim) {}



void
GraphicsMgrBuffered::CalcLists(ScreenItemList** drawLists,IRectList** eraseLists)
{
	next.CalcLists(last,drawLists,eraseLists);
	last = next;
}

/* 
 *
 *  Point tested needs to be Plane-relative, simRes scale
 *
 */

Bool
GraphicsMgrBuffered::IsOnMe(const ScreenItemDef& sid, const SOL_Point& p, Bool checkSkip)
{
	int i = last.Planes().Search(sid.PlaneId());
	if (i == -1)
		msgMgr->Fatal(SrcLoc, Msg_CantFindPlaneID);

	int j = last.Planes()[i].SIList().Search(sid.Id());
	if (j == -1)
		return False;

	Ratio scaleX(graphMgr->Xdim(), SCIRESX);
	Ratio scaleY(graphMgr->Ydim(), SCIRESY);

	SOL_Point pScreen;
	pScreen.x = (p.x * scaleX) + last.Planes()[i].DefRect().x1;
	pScreen.y = (p.y * scaleY) + last.Planes()[i].DefRect().y1;

	ScreenItem si = last.Planes()[i].SIList()[j];
	if (!si.OnScreen().Intersect(pScreen.x, pScreen.y))
		return False;

	if (!checkSkip)
		return True;

	SOL_Point pCel;
	pCel.x = (pScreen.x / scaleX) - (si.X1() / scaleX);
	pCel.y = (pScreen.y / scaleY) - (si.Y1() / scaleY);

	//  pCel = "Un User Scaled" offset 
	si.DoUpdate(last.Planes()[i]);

	if (si.scale.scaleType != SCALE_OFF)  {
		if (si.scale.scaleX  &&  si.scale.scaleY)  {
			pCel.x = pCel.x / Ratio(si.scale.scaleX, 128);
			pCel.y = pCel.y / Ratio(si.scale.scaleY, 128);
		}
	}

	uchar result = si.ReadPix(pCel.x, pCel.y);
	return result != si.GetSkip();
}






