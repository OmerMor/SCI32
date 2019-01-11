// shwstyle.cpp

#include "sol.hpp"

#include "shwstyle.hpp"
#include "event.hpp"
#include "buffer.hpp"
#include "graph.hpp"
#include "graphmgr.hpp"
#include "kernel.hpp"
#include "object.hpp"
#include "palmgr.hpp"
#include "pmachine.hpp"
#include "time.hpp"

#include <string.h>

char palMorphArray[256] = {
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void
KSetPalStyleRange(argList)
{
	int i = arg(1);
	int j = arg(2);

	for(int k=i;k<=j;k++)
		palMorphArray[k] = 0;


}

void
KSetScroll(argList)
{
	// Scroll for a plane
	//
	// Input
	// 	arg1-----planeID
	// 	arg2-----directionX
	// 	arg3-----directionY
	//    arg4-----pic number
	//		arg5-----True with animate
	//					False without animate
	//		arg6-----cycle speed	(in ticks)
	//		arg7-----mirror

	if (!arg(2) && !arg(3)) 
		// No motion
		return;

	// Make sure this plane is not already
	// in the scroll list.
	int planeID = arg(1);
	PlaneScroll* scroll = planeScrolls;
	while (scroll) {
		if (scroll->planeID == planeID)
			msgMgr->Fatal(SrcLoc,Msg_ScrollAlreadyInProgress,planeID);
		scroll = scroll->next;
	}

	if (arg(2) && arg(3))
			msgMgr->Fatal(SrcLoc,Msg_ScrollInXAndY,planeID);

	scroll = New PlaneScroll();
	scroll->xStep = arg(2);
	scroll->yStep = arg(3);
	scroll->newPic = arg(4);
	scroll->clock = timeMgr->GetTickCount();
	scroll->planeID = planeID;
	scroll->animate = arg(5);
	if (argCount > 5)
		scroll->speed = arg(6);

	// Add the new pic just off the plane
	scroll->yDim = 0;
	scroll->xDim = 0;

	int index = graphMgr->VisibleScreen().Planes().Search(planeID);
	if (index == -1)
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,index);
	SOL_Rect r = graphMgr->VisibleScreen().Planes()[index].GameRect();

	SOL_Point pt;
	if (scroll->xStep) {
 		if (scroll->xStep > 0) {
			pt.x = -r.Xlen();
			pt.y = 0;
			scroll->xDim = -r.Xlen();
 		}
		else {
			pt.x = r.Xlen();
			pt.y = 0;
			scroll->xDim = r.Xlen();
 		}
	}
	else {
 		if (scroll->yStep > 0) {
			pt.x = 0;
			pt.y = -r.Ylen();
			scroll->yDim = -r.Ylen();
 		}
		else {
			pt.x = 0;
			pt.y = r.Ylen();
			scroll->yDim = r.Ylen();
 		}
	}

	scroll->oldPic = graphMgr->AddPic(planeID,scroll->newPic,pt,arg(7));

	if (scroll->animate) {
		scroll->next = planeScrolls;
		planeScrolls = scroll;
	}
	else {
		Bool done = False;
		while (!done) {
			done = ProcessScroll(scroll);
			graphMgr->FrameOut();
			AsyncEventCheck();
		}
		delete scroll;
	}
}


void
ProcessScrolls()
{
	PlaneScroll* lastScroll = NULL;
	PlaneScroll* scroll = planeScrolls;
	while (scroll) {
		Bool done = ProcessScroll(scroll);
		if (done) {
			// Rechain styles
			if (lastScroll) {
				lastScroll->next = scroll->next;
				delete scroll;
				scroll = lastScroll->next;
			}
			else {
				planeScrolls = scroll->next;
				delete scroll;
				scroll = planeScrolls;
			}
		}
		else {
			lastScroll = scroll;
			scroll = scroll->next;
		}
	}
}

Bool 
ProcessScroll(PlaneScroll* scroll)
{
	Bool done = False;
	uint time = timeMgr->GetTickCount();
	if (scroll->clock >= time)
		return False;
	int xStep = scroll->xStep;
	int yStep = scroll->yStep;
	if (((scroll->xDim + xStep)*scroll->xDim) <= 0)
		xStep = - scroll->xDim;
	if (((scroll->yDim + yStep)*scroll->yDim) <= 0)
		yStep = - scroll->yDim;
	scroll->xDim += xStep;
	scroll->yDim += yStep;
	if ((scroll->xDim == 0) && (scroll->yDim == 0)) {
		// At the end of the scroll
		graphMgr->DeletePic(scroll->planeID,scroll->oldPic,scroll->newPic);
		done = True;
	}
	graphMgr->ScrollScreenItems(scroll->planeID,xStep,yStep);
	
	return done;
}

void
KShowStylePercent(argList)
{
	// Percent complete of a show style for a plane
	//
	// Input
	// 	arg1-----planeID
	// Output
	//    acc -----the percent complete of the plane's show style
	if (!showStyles)
		pm.acc = 100;
	else {
		PlaneShowStyle* style = showStyles;
		while (style) {
			if (style->planeID == arg(1)) {
				pm.acc = (100*style->percent)/style->divisions;
				return;
			}
			style = style->next;
		}
		pm.acc = 100;
	}
}

void
KSetShowStyle(argList)
{
	// Show style for a plane
	//
	// Input
	// 	arg1-----showStyle
	// 	arg2-----planeID
	//		arg3-----speed	(seconds to complete)
	// 	arg4-----color (-1 is skip color)
	//    arg5-----priority of show style
	//		arg6-----True with animate
	//					False without animate
	//		arg7-----Reference frame (may be needed to remove a screen item)
	//		arg8-----Number of divisions for a show style (optional)

	// Planes not allowed to move or be overlaped by a
	// higher priority plane while show style in progress.
	//
	// Make sure this plane is not already
	// in the showStyles list.

	Bool 		entry;
	uchar 	color;
	int		thisStyle;

	thisStyle = arg(1);
	if (arg(4) == -1)	{
		entry = True;
		color = SOL_BLACK;
	}
	else {
		entry = False;
		color = arg(4);
	}


	if (thisStyle > LASTSTYLE)
		msgMgr->Fatal(SrcLoc,"Illegal show style for plane ",arg(2));
	int planeID = arg(2);

	// Save current plane image
	int index = graphMgr->NextScreen()->Planes().Search(planeID);
	if (index == -1)
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,index);
	SOL_Rect planeRect = graphMgr->NextScreen()->Planes()[index].GameRect();

	int priority = arg(5);
	Bool newStyle = True;
	PlaneShowStyle* style = showStyles;
	while (style) {
		if (style->planeID == planeID) {
			if (
				(style->BMWidth == planeRect.Xlen())
				&&
				(style->BMHeight == planeRect.Ylen())
				&&
				(
				((argCount > 7) && (style->divisions == arg(8))) ||
				((argCount <= 7) && (style->divisions == showStyleDivisions[thisStyle]))
				)
				&&
				(style->form == showStyleForms[thisStyle])
				) {
					newStyle = False;
					// The reverse of the exit style
					style->percent = 0;
					entry = True;
			}
			else {
				// Need the color!
				entry = True;
				color = style->color;
				KillStyle(style);
				break;
			}
			break;
		}	
		style = style->next;
	}

//#ifndef WINDOWS
	if (thisStyle <= PLAINFORM)  return;
//#endif

	if (newStyle) {
		style = New PlaneShowStyle();
		style->si = NULL;
		style->si = NULL;
		style->form = showStyleForms[thisStyle];
		style->BM = NULL;
		style->BMWidth = planeRect.Xlen();
		style->BMHeight = planeRect.Ylen();
		style->deleteMe = False;
		if (argCount > 7)
			style->divisions = arg(8);
		else
			style->divisions = showStyleDivisions[thisStyle];
		style->planeID = planeID;
	}
	style->entry = entry;
	style->color = color;
	style->clock = timeMgr->GetTickCount();
	style->style = thisStyle;
	style->animate = arg(6);

	// calculate the ticks per processing
	style->speed = (60*arg(3)+style->divisions-1)/style->divisions;
	if (style->speed == 0) {
		delete style;
		msgMgr->Alert(SrcLoc,Msg_ShowStyleSpeed);
		return;
	}

	if (arg(7)) {
		// Generate a reference frame
		graphMgr->FrameOut(False);
	}

	if (newStyle) {
		// Switch on show style
		switch (style->style) {
			case PLAINFORM:
//				PlainSetUp();
				break;
			case PIXELDISSOLVE:
				PixelDissolveSetUp(style,priority,planeRect);
				break;
			case DISSOLVE:
				DissolveSetUp(style,priority,planeRect);
				break;
			case IRISOUT:
			case IRISIN:
				IrisSetUp(style,priority); 
				break;
			case WIPERIGHT:
			case WIPELEFT:
				HWipeSetUp(style,priority);
				break;
			case WIPEDOWN:
			case WIPEUP:
				VWipeSetUp(style,priority);
				break;
			case HSHUTTEROUT:
			case HSHUTTERIN:
				HShutterSetUp(style,priority); 
				break;
			case VSHUTTEROUT:
			case VSHUTTERIN:
				VShutterSetUp(style,priority); 
				break;
			case FADEOUT:
				break;
			case FADEIN:
				break;
			case PALETTEMORPH:
				break;
		}
		// Store for processing durring frameout
		style->next = showStyles;
		showStyles = style;
	}
}

void
ProcessShowStyles()
{
	Bool notDone;
	Bool withOutAnimate;
	uint time = timeMgr->GetTickCount();
	do {
		notDone = False;
		withOutAnimate = False;
		PlaneShowStyle* lastStyle = NULL;
		PlaneShowStyle* style = showStyles;
		while (style) {
			if (!style->animate)	++withOutAnimate;
			Bool done = ProcessShowStyle(style,time);
			if (!done) notDone = True;
			if (done && style->deleteMe) {
				style = ShutDownStyle(style);
			}
			else {
				lastStyle = style;
				style = style->next;
			}
		}
		if (withOutAnimate ) {
			graphMgr->FrameOut();
		}
	} while (notDone && withOutAnimate);
}

Bool 
ProcessShowStyle(PlaneShowStyle* style,uint time)
{
	Bool done = False;
	
	if ((style->clock >= time) && style->animate)
		return False;

	// Switch on show style
	switch (style->style) {
		case PLAINFORM:
			done = Plain(style);
			break;
		case PIXELDISSOLVE:
			done = PixelDissolve(style);
			break;
		case DISSOLVE:
			done = Dissolve(style);
			break;
		case FADEOUT:
			done = Fade(-1,style);
			break;
		case FADEIN:
			done = Fade(1,style);
			break;
		case PALETTEMORPH:
			done = PalMorph(style);
			break;
		case HSHUTTEROUT:
		case IRISOUT:
		case VSHUTTEROUT:
		case WIPELEFT:
		case WIPEUP:
			done = GenericProcess(-1,style);
			break;
		case HSHUTTERIN:
		case IRISIN:
		case VSHUTTERIN:
		case WIPERIGHT:
		case WIPEDOWN:
			done = GenericProcess(1,style);
			break;
	}
	return (done);
}

Bool
PalMorph(PlaneShowStyle* style)
{
			graphMgr->PalMorphFrameOut(palMorphArray);
			style->deleteMe = True;
			return True;
}

Bool
GenericProcess(int direction,PlaneShowStyle* style)
{
	Bool noChange = True;
	uint percent;

	if (style->percent < style->divisions) {
		// calculate the starting percent
		if (direction > 0)
			percent = style->percent;
		else
			percent = style->divisions - style->percent - 1;

		int i = style->tilesPerFrame*percent;
		for (int j = 0;j < style->tilesPerFrame;++j) {
			if (style->entry)	{
				graphMgr->DeleteScreenItem(style->si[i]);
				style->si[i++] = NULL;
			}
			else {
				graphMgr->AddScreenItem(style->si[i++]);
			}
		}

		style->clock += style->speed;
		style->percent++;
		noChange	= False;
	}
	if ((style->percent >= style->divisions) && noChange)	{
		if (style->entry) style->deleteMe = True;
			return True;
	}
	else
		return False;
}

PlaneShowStyle*
ShutDownStyle(PlaneShowStyle* style)
{
	// Switch on show style
	switch (style->style) {
		case DISSOLVE:
		case PIXELDISSOLVE:
			DissolveShutDown(style);
			break;
		case HSHUTTEROUT:
		case HSHUTTERIN:
		case IRISOUT:
		case IRISIN:
		case VSHUTTEROUT:
		case VSHUTTERIN:
		case WIPERIGHT:
		case WIPELEFT:
		case WIPEDOWN:
		case WIPEUP:
			GenericShutDown(style);
			break;
		case PLAINFORM:
		case FADEOUT:
		case FADEIN:
			break;
	}
	// Rechain styles
	PlaneShowStyle* lastStyle = NULL;
	PlaneShowStyle* search = showStyles;
	while (search) {
		if (search == style) break;
		lastStyle = search;
		search = search->next;
	}

	if (lastStyle) {
		lastStyle->next = style->next;
		delete style;
		style = lastStyle->next;
	}
	else {
		showStyles = style->next;
		delete style;
		style = showStyles;
	}
	return style;
}

void
GenericShutDown(PlaneShowStyle* style) 
{
	// Remove all the screen item defs
	for (int i = 0;i < style->tilesPerFrame*style->divisions;++i) {
		if (style->si[i]) delete style->si[i];
	}
	if (style->si) delete style->si;
}

void
KillStyle(PlaneShowStyle* style)
{
	// Switch on show style
	switch (style->style) {
		case DISSOLVE:
		case PIXELDISSOLVE:
			DissolveShutDown(style);
			break;
		case HSHUTTEROUT:
		case HSHUTTERIN:
		case IRISOUT:
		case IRISIN:
		case VSHUTTEROUT:
		case VSHUTTERIN:
		case WIPERIGHT:
		case WIPELEFT:
		case WIPEDOWN:
		case WIPEUP:
			GenericKill(style);
			break;
		case PLAINFORM:
		case FADEOUT:
		case FADEIN:
			break;
	}
	// Rechain styles
	PlaneShowStyle* lastStyle = NULL;
	PlaneShowStyle* search = showStyles;
	while (search) {
		if (search == style) break;
		lastStyle = search;
		search = search->next;
	}

	if (lastStyle) {
		lastStyle->next = style->next;
		delete style;
		style = lastStyle->next;
	}
	else {
		showStyles = style->next;
		delete style;
		style = showStyles;
	}
}

void
GenericKill(PlaneShowStyle* style) 
{
	// Remove all the screen item defs
	for (int i = 0;i < style->tilesPerFrame*style->divisions;++i) {
		if (style->si[i]) {
			graphMgr->DeleteScreenItem(style->si[i]);
		}
	}
	if (style->si) delete style->si;
}

Bool
Plain(PlaneShowStyle* style)
{
	if (style->entry) {
		graphMgr->GPalette().SetFade(100);
	}
	else {
		graphMgr->GPalette().SetFade(0);
	}
	style->deleteMe = True;
	return True;
}

Bool
Fade(int direction,PlaneShowStyle* style)
{
	Bool noChange = True;
	uint percent;

	if (style->percent < style->divisions) {
		// calculate the starting percent
		if (direction > 0)
			percent = style->percent;
		else
			percent = style->divisions - style->percent - 1;

		graphMgr->GPalette().SetFade(percent*100/(style->divisions-1));

		style->clock += style->speed;
		style->percent++;
		noChange	= False;
	}
	if ((style->percent >= style->divisions) && noChange)	{
		if (direction > 0) style->deleteMe = True;
		return True;
	}
	else
		return False;
}

void
HShutterSetUp(PlaneShowStyle* style,int priority) 
{
	// Create the screen item defs needed.
	style->tilesPerFrame = 2;
	style->si = New ScreenItem* [style->tilesPerFrame*style->divisions];

	Bitmap bitmap(style->color);
	int i=0;
	int j=0;
	for (;i < style->tilesPerFrame*style->divisions;++i,++j) {
		SOL_Rect lftBand((style->BMWidth*j)/(2*style->divisions),
							  0,
							  (style->BMWidth+1)*(j+1)/(2*style->divisions) - 1,
							  style->BMHeight - 1);
		style->si[i] = New ScreenItem(style->planeID,bitmap,lftBand);
		style->si[i++]->SetPriority(priority);
		SOL_Rect rgtBand(style->BMWidth - lftBand.B.x - 1,
							  0,
							  style->BMWidth - lftBand.A.x - 1,
							  style->BMHeight - 1);
		style->si[i] = New ScreenItem(style->planeID,bitmap,rgtBand);
		style->si[i]->SetPriority(priority);
	}
	if (style->entry) {
		// must add all the tiles to the plane because
		// ProcessShowStyle will be removing the tiles.
		for (i = 0;i < style->tilesPerFrame*style->divisions;++i) {
			graphMgr->AddScreenItem(style->si[i]);
		}
	}
}

void
VShutterSetUp(PlaneShowStyle* style,int priority) 
{
	// Create the screen item defs needed.
	style->tilesPerFrame = 2;
	style->si = New ScreenItem* [style->tilesPerFrame*style->divisions];
	Bitmap bitmap(style->color);
	int i=0;
	int j=0;
	for (;i < style->tilesPerFrame*style->divisions;++i,++j) {
		SOL_Rect topBand(0,
							  (style->BMHeight*j)/(2*style->divisions),
							  style->BMWidth - 1,
							  (style->BMHeight+1)*(j+1)/(2*style->divisions) - 1);
		style->si[i] = New ScreenItem(style->planeID,bitmap,topBand);
		style->si[i++]->SetPriority(priority);
		SOL_Rect botBand(0,
							  style->BMHeight - topBand.B.y - 1,
							  style->BMWidth - 1,
							  style->BMHeight - topBand.A.y - 1);
		style->si[i] = New ScreenItem(style->planeID,bitmap,botBand);
		style->si[i]->SetPriority(priority);
	}
	if (style->entry) {
		// must add all the tiles to the plane because
		// ProcessShowStyle will be removing the tiles.
		for (i = 0;i < style->tilesPerFrame*style->divisions;++i) {
			graphMgr->AddScreenItem(style->si[i]);
		}
	}
}

void
HWipeSetUp(PlaneShowStyle* style,int priority) 
{
	// Create the screen item defs needed.
	style->tilesPerFrame = 1;
	style->si = New ScreenItem* [style->divisions];
	Bitmap bitmap(style->color);
	for (int i = 0;i < style->divisions;++i) {
		SOL_Rect lftBand(style->BMWidth*i/style->divisions,
							  0,
							  style->BMWidth*(i+1)/style->divisions - 1,
							  style->BMHeight - 1);
		style->si[i] = New ScreenItem(style->planeID,bitmap,lftBand);
		style->si[i]->SetPriority(priority);
	}
	if (style->entry) {
		// must add all the tiles to the plane because
		// ProcessShowStyle will be removing the tiles.
		for (i = 0;i < style->divisions;++i) {
			graphMgr->AddScreenItem(style->si[i]);
		}
	}
}

void
VWipeSetUp(PlaneShowStyle* style,int priority) 
{
	// Create the screen item defs needed.
	style->tilesPerFrame = 1;
	style->si = New ScreenItem* [style->divisions];
	Bitmap bitmap(style->color);
	for (int i = 0;i < style->divisions;++i) {
		SOL_Rect topBand(0,
							  style->BMHeight*i/style->divisions,
							  style->BMWidth - 1,
							  style->BMHeight*(i+1)/style->divisions - 1);
		style->si[i] = New ScreenItem(style->planeID,bitmap,topBand);
		style->si[i]->SetPriority(priority);
	}
	if (style->entry) {
		// must add all the tiles to the plane because
		// ProcessShowStyle will be removing the tiles.
		for (i = 0;i < style->divisions;++i) {
			graphMgr->AddScreenItem(style->si[i]);
		}
	}
}

void
DissolveSetUp(PlaneShowStyle* style,int priority,SOL_Rect& planeRect)
{
	style->BM = memMgr->Alloc(MOVEABLE,(style->BMWidth)*(style->BMHeight)+CELHEADERSIZE,0,0);

	// build the header for the BM
	BuildHeader(style->BM,style->BMWidth,style->BMHeight,SOL_SKIP);

	// Transfer the current VMAP bits for the given plane into the BM
	Buffer* buffer = graphMgr->CurrBuffer();
	char* target = ((char*) memMgr->GetAddr(style->BM)) + CELHEADERSIZE;
	int height = planeRect.Ylen();
	while (height--) {
		memcpy(target,
		 		buffer->Addr(planeRect.A.x,planeRect.A.y),
	 	 		style->BMWidth);
		target += style->BMWidth;
		planeRect.A.y++;
	}

	Bitmap bitmap(style->BM);
	SOL_Point pt(0,0);
	ScaleInfo scaleInfo;
	style->siPlane = New ScreenItem(style->planeID,bitmap,pt,scaleInfo,0);
	style->siPlane->SetPriority(priority);
	graphMgr->AddScreenItem(style->siPlane);
}

void
DissolveShutDown(PlaneShowStyle* style) 
{
	if ((int) style->BM)
		memMgr->Free(style->BM);
	if (style->siPlane)	{
		graphMgr->DeleteScreenItem(style->siPlane);
		delete style->siPlane;
	}
}

Bool 
Dissolve(PlaneShowStyle* style)
{
	Bool noChange = True;
	ulong index;

	char* celPtr = ((char*) memMgr->GetAddr(style->BM)) + CELHEADERSIZE;

	ulong	pixels = style->divisions*style->divisions;

	// Get the starting index
	if (style->percent == 0) {
		int i = 0;
		index = pixels;
		while (True) {
			index >>= 1;
			if (index == 1) break;
			i++;
		}
		style->pDMask = pixelDissolveTable[i];
		// Get the seed pixel
		index = 800;
		style->firstPixel = index;
		style->pixel = index;
	}
	else {
		index = style->pixel;
		while (True) {
			if (index & 1) {
				index = index >> 1;
				index ^= style->pDMask;
			}
			else {
				index = index >> 1;
			}
			if (index < pixels) break;
		} 
		if (index == style->firstPixel) index = 0;
	}

	int xWidth = (style->BMWidth + style->divisions - 1)/style->divisions;
	int yHeight =(style->BMHeight + style->divisions - 1)/style->divisions;
	
	if (style->percent <= style->divisions) {
		SOL_Rect p(0,0,style->BMWidth - 1,style->BMHeight - 1);

		for (int i=0;i<style->divisions;i++) {
			SOL_Rect r;

			int x = index%style->divisions;
			int y = index/style->divisions;
			r.A.x = x*xWidth;
			r.A.y = y*yHeight;
			r.B.x = x*xWidth + xWidth - 1;
			r.B.y = y*yHeight + yHeight - 1;
			r.Clip(p);
			FillRect(celPtr,r,style->color,style->BMWidth);

			// Next index
			while (True) {
				if (index &1) {
					index = index >> 1;
					index ^= style->pDMask;
				}
				else {
					index = index >> 1;
				}
				if (index < pixels) break;
			}
 
			if (style->percent == style->divisions) {
//			if (index == style->firstPixel) {
				// finish it off
				r.A.x = 0;
				r.A.y = 0;
				r.B.x = style->BMWidth - 1;
				r.B.y = style->BMHeight - 1;
				FillRect(celPtr,r,style->color,style->BMWidth);
				break;
			}
		}
		style->pixel = index;
		style->clock += style->speed;
		style->percent++;
		noChange	= False;
		style->siPlane->SetUpdateCount(graphMgr->ScreenCount());
	}
	if ((style->percent >= style->divisions) && noChange)	{
		if (style->entry) style->deleteMe = True;
			return True;
	}
	else
		return False;
}

void
PixelDissolveSetUp(PlaneShowStyle* style,int priority,SOL_Rect& planeRect)
{
	style->BM = memMgr->Alloc(MOVEABLE,(style->BMWidth)*(style->BMHeight)+CELHEADERSIZE,0,0);

	// build the header for the BM
	BuildHeader(style->BM,style->BMWidth,style->BMHeight,SOL_SKIP);

	// Transfer the current VMAP bits for the given plane into the BM
	Buffer* buffer = graphMgr->CurrBuffer();
	char* target = ((char*) memMgr->GetAddr(style->BM)) + CELHEADERSIZE;
	int height = planeRect.Ylen();
	while (height--) {
		memcpy(target,
		 		buffer->Addr(planeRect.A.x,planeRect.A.y),
	 	 		style->BMWidth);
		target += style->BMWidth;
		planeRect.A.y++;
	}

	Bitmap bitmap(style->BM);
	SOL_Point pt(0,0);
	ScaleInfo scaleInfo;
	style->siPlane = New ScreenItem(style->planeID,bitmap,pt,scaleInfo,0);
	style->siPlane->SetPriority(priority);
	graphMgr->AddScreenItem(style->siPlane);
}

Bool 
PixelDissolve(PlaneShowStyle* style)
{
	Bool noChange = True;
	ulong index;

	char* celPtr = ((char*) memMgr->GetAddr(style->BM)) + CELHEADERSIZE;

	SOL_Rect p(0,0,style->BMWidth - 1,style->BMHeight - 1);
	ulong	pixels = p.Area();

	ulong PixelsPerPercent	= (pixels + style->divisions)/style->divisions;
	ulong HorizontalPixels	= p.Xlen();


	// Get the starting index
	if (style->percent == 0) {
		int i = 0;
		index = pixels;
		if (index != 1) {
			while (True) {
				index >>= 1;
				if (index == 1) break;
				i++;
			}
		}
		style->pDMask = pixelDissolveTable[i];
		// Get the seed pixel
		// index = LCGRandom() % pixels;
		index = 53427;	// fake out to be similar to SCI16 with SP = 0xff00

		style->firstPixel = index;
		style->pixel = index;
	}
	else {
		index = style->pixel;
		while (True) {
			if (index & 1) {
				index = index >> 1;
				index ^= style->pDMask;
			}
			else {
				index = index >> 1;
			}
			if (index < pixels) break;
		} 
		if (index == style->firstPixel) index = 0;
	}

	if (style->percent < style->divisions) {
		for (int i=0;i<PixelsPerPercent;i++) {
			SOL_Point pt(index%HorizontalPixels,
							 index/HorizontalPixels);

			FillPoint(celPtr,pt,style->color,style->BMWidth);

			// Next index
			while (True) {
				if (index &1) {
					index = index >> 1;
					index ^= style->pDMask;
				}
				else {
					index = index >> 1;
				}
				if (index < pixels) break;
			} 
			if (index == style->firstPixel) {
				// finish it off
				SOL_Rect r(0,
							  0,
							  style->BMWidth - 1,
							  style->BMHeight - 1);
				FillRect(celPtr,r,style->color,style->BMWidth);
				break;
			}
		}
		style->pixel = index;
		style->clock += style->speed;
		style->percent++;
		noChange	= False;
		style->siPlane->SetUpdateCount(graphMgr->ScreenCount());
	}
	if ((style->percent >= style->divisions) && noChange)	{
		if (style->entry) style->deleteMe = True;
			return True;
	}
	else
		return False;
}

void
IrisSetUp(PlaneShowStyle* style,int priority) 
{
	// Create the screen item defs needed.
	style->tilesPerFrame = 4;
	style->si = New ScreenItem* [style->tilesPerFrame*style->divisions];
	Bitmap bitmap(style->color);
	int i=0;
	int j=0;
	for (;i < style->tilesPerFrame*style->divisions;++i,++j) {
		SOL_Rect topBand((style->BMWidth*j)/(2*style->divisions),
							  (style->BMHeight*j)/(2*style->divisions),
							  0,
							  (style->BMHeight+1)*(j+1)/(2*style->divisions) - 1);
		topBand.B.x = style->BMWidth - topBand.A.x - 1;

		SOL_Rect botBand(topBand.A.x,
							  style->BMHeight - topBand.B.y - 1,	
							  topBand.B.x,
							  style->BMHeight - topBand.A.y - 1);	

		SOL_Rect lftBand(topBand.A.x,
							  topBand.B.y + 1,
							  (style->BMWidth+1)*(j+1)/(2*style->divisions) - 1,
							  botBand.A.y - 1);

		SOL_Rect rgtBand(style->BMWidth - lftBand.B.x - 1,
							  lftBand.A.y,
							  style->BMWidth - lftBand.A.x - 1,
							  lftBand.B.y);

		style->si[i] = New ScreenItem(style->planeID,bitmap,topBand);
		style->si[i++]->SetPriority(priority);

		style->si[i] = New ScreenItem(style->planeID,bitmap,botBand);
		style->si[i++]->SetPriority(priority);

		style->si[i] = New ScreenItem(style->planeID,bitmap,lftBand);
		style->si[i++]->SetPriority(priority);

		style->si[i] = New ScreenItem(style->planeID,bitmap,rgtBand);
		style->si[i]->SetPriority(priority);
	}
	if (style->entry) {
		// must add all the tiles to the plane because
		// ProcessShowStyle will be removing the tiles.
		for (i = 0;i < style->tilesPerFrame*style->divisions;++i) {
			graphMgr->AddScreenItem(style->si[i]);
		}
	}
}

void 
FillRect(char* celPtr,SOL_Rect& r,uchar color,int BMWidth)
{
	if (r.IsEmpty()) return;

	celPtr += r.A.y*BMWidth + r.A.x;
	int width = r.Xlen();
	int height = r.Ylen();

	while (height--) {
		memset(celPtr,color,width);
		celPtr += BMWidth;
	}
}

void 
FillPoint(char* celPtr,SOL_Point& pt,uchar color,int BMWidth)
{
	celPtr += pt.y*BMWidth + pt.x;
	*celPtr = color;
}

int pixelDissolveTable[15] =
	{0x0003,
	 0x0006,
	 0x000c,
	 0x0014,
	 0x0030,
	 0x0060,
	 0x00b8,
	 0x0110,
	 0x0240,
	 0x0500,
	 0x0ca0,
	 0x1b00,
	 0x3500,
	 0x6000,
	 0xb400};

int showStyleDivisions[16] =
	{1,		// PLAIN,		  -form PLAIN
	 20,		// HSHUTTEROUT	  -form HSHUTTER
	 20,		// HSHUTTERIN	  -form HSHUTTER	
	 20,		// VSHUTTEROUT	  -form VSHUTTER
	 20,		// VSHUTTERIN	  -form VSHUTTER	
	 20,		// WIPELEFT		  -form HWIPE
	 20,		// WIPERIGHT	  -form HWIPE	
	 20,		// WIPEUP		  -form VWIPE	
	 20,		// WIPEDOWN		  -form VWIPE
	 20,		// IRISOUT		  -form IRIS	
	 20,		// IRISIN		  -form IRIS	
	 40,		// DISSOLVE		  -form PLAIN
	 40,		// PIXELDISSOLVE -form PLAIN
	 101,		// FADEOUT		  -form PLAIN
	 101,		// FADEIN		  -form PLAIN
	  2};		// PALETTEMORPH

int showStyleForms[16] =
	{PLAINFORM,	 
	 HSHUTTER,
	 HSHUTTER,
	 VSHUTTER,
	 VSHUTTER,
	 HWIPE,	 
	 HWIPE,
	 VWIPE,
	 VWIPE,	 
	 IRIS,	 
	 IRIS,	 
	 PLAINFORM,	 
	 PLAINFORM,	 
	 PLAINFORM,	 
	 PLAINFORM,	 
	 PLAINFORM};	 


// List of planes that have a show style in progress
PlaneShowStyle* showStyles;

// List of scrolling planes
PlaneScroll* planeScrolls;
