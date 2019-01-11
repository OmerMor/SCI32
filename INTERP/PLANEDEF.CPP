#include <string.h>
#include "sol.hpp"
#include "planedef.hpp"
#include "selector.hpp"
#include "object.hpp"
#include "graphmgr.hpp"
#include "ratio.hpp"
#ifdef WIN32S
#include "shiftpal.hpp"
#endif

long PlaneDef::nextId = 20000;

/*
 *
 *
 *  Constructor
 *  ===========
 *
 *
 */

PlaneDef::PlaneDef(ObjectID ID):
	vanish(ID.GetProperty(s_vanishingX),
			 ID.GetProperty(s_vanishingY)
			)
{
	SOL_Rect rect;
	int xdim = graphMgr->Xdim();
	int ydim = graphMgr->Ydim();

	gameRect.x1 = ID.GetProperty(s_insetRectL);
	gameRect.y1 = ID.GetProperty(s_insetRectT);
	gameRect.x2 = ID.GetProperty(s_insetRectR); // + 1) * xdim) / LOWRESX) - 1;
	gameRect.y2 = ID.GetProperty(s_insetRectB); // + 1) * ydim) / LOWRESY) - 1;
	ConvGameRectToPlaneRect();

	planeId = ID;
#ifndef WIN32S
	color = ID.GetProperty(s_back);
#else
	color = (uchar)AddPalShift((int)ID.GetProperty(s_back));
#endif
	priority	= ID.GetProperty(s_priority);
	picResNum = ID.GetProperty(s_picture);
	SetType();
	picMirror = ID.GetProperty(s_mirrored);
}


PlaneDef::PlaneDef(int xdim, int ydim)
{
	planeId = nextId;
	nextId++;

	gameRect.Set(0, 0, xdim - 1, ydim - 1);
	ConvGameRectToPlaneRect();
	vanish.x = 0;
	vanish.y = 0;
	priority = 0;
	picResNum = -1;
	SetType();
	picMirror = False;
	color = SOL_BLACK;
}


PlaneDef::PlaneDef(const SOL_Rect& rect)
{
	planeId = nextId;
	nextId++;

	gameRect = rect;
	ConvGameRectToPlaneRect();

	vanish.x = 0;
	vanish.y = 0;
	priority = 0;
	picResNum = -1;
	SetType();
	picMirror = False;
	color = SOL_BLACK;
}



void
PlaneDef::Resize(int xdim, int ydim)
{
	gameRect.Set(gameRect.x1, gameRect.y1, gameRect.x1 + xdim - 1, gameRect.y1 + ydim - 1);
	ConvGameRectToPlaneRect();
}



/*
 *
 *
 *  operator== Method
 *  =================
 *
 *
 */

Bool
PlaneDef::operator==(const PlaneDef& src) const
{
	return (
		planeId == src.planeId  &&
		plane == src.plane  &&
		gameRect == src.gameRect  &&
		vanish == src.vanish  &&
		picResNum == src.picResNum  &&
		priority == src.priority  &&
		color == src.color  &&
		picMirror == src.picMirror
	);
}


PlaneDef&
PlaneDef::operator=(const PlaneDef& src)
{
	planeId = src.planeId;
	gameRect = src.gameRect;
	plane = src.plane;
	vanish = src.vanish;
	picResNum = src.picResNum;
	type = src.type;
	picMirror = src.picMirror;
	priority = src.priority;
	color = src.color;
	return *this;
}

void
PlaneDef::SetType() {
	switch (picResNum) {
		case -1:
			type = coloredPlane;
			break;
		case -2:
			type = transparentPlane;
			break;
		case -3:
			type = opaquePlane;
			break;
		default:
			type = picturePlane;
			break;
	}
}

void
PlaneDef::MoveTo(SOL_Point p)
{
	Ratio scaleX(graphMgr->Xdim(), LOWRESX);
	Ratio scaleY(graphMgr->Ydim(), LOWRESY);
	p.x = p.x * scaleX;
	p.y = p.y * scaleY;
	gameRect.MoveTo(p.x, p.y);
	ConvGameRectToPlaneRect();
}

void
PlaneDef::ConvGameRectToPlaneRect()
{
	plane = gameRect;
	Ratio scaleX(graphMgr->Xdim(), LOWRESX);
	Ratio scaleY(graphMgr->Ydim(), LOWRESY);
	plane.x1 = plane.x1 * scaleX;
	plane.y1 = plane.y1 * scaleY;
	plane.x2 = ((plane.x2 + 1) * scaleX) - 1;
	plane.y2 = ((plane.y2 + 1) * scaleY) - 1;
}
