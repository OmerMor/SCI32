//	graphmgr.cpp

#include <stdlib.h>
#include <string.h>

#include "sol.hpp"

#include "graphmgr.hpp"
#include "palmgr.hpp"
#include "plane.hpp"
#include "rectlist.hpp"
#include "buffer.hpp"
#include "kernel.hpp"
#include "savegame.hpp"
#include "si.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include "cursor.hpp"
#include "shwstyle.hpp"
#include "video.hpp"

GraphicsMgr* graphMgr = NULL;

DrawList	*drawLists[100];
RectList	*eraseLists[100];

GraphicsMgr::~GraphicsMgr()
{
	delete robot;
}

GraphicsMgr::GraphicsMgr(int xdim,int ydim) : next(xdim,ydim)
{
   magHead = NULL;
	timingFlag = False;
	pixelsPerCall = 0;
	resizeFlag = False;
	vmapValid = False;
	robot = NULL;
	morphOn = 0;
}

Robot&
GraphicsMgr::GRobot()
{
	if (!robot)
		msgMgr->Fatal("GraphicsMgr::GRobot called with NULL");
	return *robot;
}

void
GraphicsMgr::UpdateScreenItem(ScreenItem* si)
{
	// Make sure we have a valid screen item
	if (si != graphMgr->NextScreen()->GetScreenItem(si->PlaneId(),si->Id()))
		msgMgr->Fatal(SrcLoc,Msg_ScreenItemNotExist);

	if (!si->AddCount())
		si->SetUpdateCount(graphMgr->ScreenCount());

	// If deleted and added without an intermediate
	// FrameOut then the add will result in an update
	// and the deleteCount must be cleared
	si->ClearDeleteCount();
	si->SetCel(NULL);
}

void
GraphicsMgr::DeleteScreenItem(ScreenItem* si, int planeId)
{
	// Assume we have a valid screen item

	if (si->AddCount()) {
		// This screen item has not be processed by a frameout
		// Just get rid of the screen item in the next screen
		Plane* plane = GetPlaneNext(planeId);
		plane->AbortScreenItem(si);
		plane->PackSiArray();
	}
	else {
		si->SetDeleteCount(graphMgr->ScreenCount());
		si->SetAddCount(0);
		si->SetUpdateCount(0);
	}
}

void
GraphicsMgr::DeleteScreenItem(ScreenItem* si)
{
	// Make sure we have a valid screen item
	int planeId = si->PlaneId();
	if (si != graphMgr->NextScreen()->GetScreenItem(planeId, si->Id()))
		msgMgr->Fatal(SrcLoc,Msg_ScreenItemNotExist);

	DeleteScreenItem(si, planeId);
}

void
GraphicsMgr::UpdatePlane(Plane* plane)
{
	if (plane != GetPlaneNext(plane->Id()))
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,plane->Id());
		
	Plane* currentPlane = VisiblePlane(plane->Id());

	plane->UpdatePlane(currentPlane);
	plane->OnScreen().Clip(next.ScreenRect());

	next.SortPlanes();
}

void
GraphicsMgr::DeletePlane(int planeId)
{
	Plane* plane = GetPlaneNext(planeId);
	if (plane == NULL)
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,plane->Id());

// FOR PQ4CD ONLY, COMMENT OUT THIS LINE:
//		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,plane->Id());
// AND ADD THIS LINE:
//		return;
	DeletePlane(plane);
}

void
GraphicsMgr::DeletePlane(Plane* plane)
{
	if (plane != GetPlaneNext(plane->Id()))
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,plane->Id());
		
	if (plane->AddCount())  {
		next.AbortPlane(plane);
	}
	else  {
		plane->SetDeleteCount(ScreenCount());
		plane->SetAddCount(0);
		plane->SetUpdateCount(0);
	}
}


void
GraphicsMgr::KillRobot()
{
	delete robot;
	robot = NULL;
}

void
GraphicsMgr::PalMorphFrameOut(char* palArray,PlaneShowStyle* style)
{
// First Decimate Original VMAP	and output to SCREEN

	int palIndex = -1;
	SOLPalette  myPal;
	SOLPalette  thisPal;
//BEW CHANGED	int s = pm.Global(g_prevRoomNum);
	int s = pmGlobal(g_prevRoomNum);
	myPal = GPalette().Next();

	AlterVMAP(&myPal,&myPal,palIndex,palArray);	
 	SOL_Rect	A(0,0,next.Xdim()-1,next.Ydim()-1);
	next.ShowList().Add(A);
	ShowBits();

// Second draw next image to VMAP

	int nplanes = next.Planes().Length();

	for (int i = 0;i < nplanes;i++)  {
		drawLists[i] = New DrawList;
		eraseLists[i] = New RectList;
	}

	if (Remap::remapCount && lastRemapFlag)
		RemapMarkRedraw();

	SOL_Rect dirtyRect(0,0,-1,-1);
	CalcLists(drawLists,eraseLists,dirtyRect);

	// The number of planes may have been reduced by CalcLists
	nplanes = next.Planes().Length();

	// Sort the draw lists
	for (i = 0; i < nplanes; i++)
		drawLists[i]->Sort();

	for (i = 0; i < nplanes; i++)  {
		int count = drawLists[i]->Length();
		DrawList& drawlist = *drawLists[i];
		for (int j = 0; j < count; j++) {
			drawlist[j].SI().SubmitPalette();
		}
	}

	lastRemapFlag = GPalette().UpdateForFrame();

	// draw to vmap
	vmapValid = False;
	for (i = 0; i < nplanes; i++)  {
		DrawEraseList(*eraseLists[i],next.Planes()[i]);
		DrawScreenItemList(*drawLists[i]);
	}

// Third rework palette to work for both palette reduced images

	thisPal = GPalette().Next();

	if (s<1000) {
	for(int k=0;k<256;k++){ 
			if ((palArray[k] == -1) || (palArray[k] == 0)) {
//			if ((palArray[k] == -1) || ((palArray[k] == 0)&& ((k>71)&&(k<104)))) {
// the above line is KQ7 specific
	  			(myPal)[k].red   = (thisPal)[k].red   ;
				(myPal)[k].green = (thisPal)[k].green  ;
				(myPal)[k].blue  = (thisPal)[k].blue 	;
				(myPal)[k].flag  = 1 	;
			}
		}
	}
	else
	for(int k=0;k<256;k++){ 
//			if ((palArray[k] == -1) || (palArray[k] == 0)) {
			if ((palArray[k] == -1) || ((palArray[k] == 0)&& ((k>71)&&(k<104)))) {
// the above line is KQ7 specific
	  			(myPal)[k].red   = (thisPal)[k].red   ;
				(myPal)[k].green = (thisPal)[k].green  ;
				(myPal)[k].blue  = (thisPal)[k].blue 	;
				(myPal)[k].flag  = 1 	;
			}
		}


	GPalette().Submit(myPal);
	GPalette().UpdateFFrame();
	GPalette().UpdateHardware();

// Fourth decimate new image in VMAP and output to screen

	palIndex = 1;
	AlterVMAP(&thisPal,&myPal,palIndex,palArray);	

	if ((style != NULL) && (style->style != PALETTEMORPH))
		ProcessEffects(style);
	else {
		ShowBits();
	}

	vmapValid = True;

// Finally let the graphics engine know everything should be redrawn
// For palette cycling 

	for (i = 0;i < nplanes;i++)	
	next.Planes()[i].UpdateRedrawAllCount();

	if (Remap::remapCount && lastRemapFlag)
		RemapMarkRedraw();

	CalcLists(drawLists,eraseLists,dirtyRect);

	// The number of planes may have been reduced by CalcLists
	nplanes = next.Planes().Length();

	// Sort the draw lists
	for (i = 0; i < nplanes; i++)
		drawLists[i]->Sort();

	for (i = 0; i < nplanes; i++)  {
		int count = drawLists[i]->Length();
		DrawList& drawlist = *drawLists[i];
		for (int j = 0; j < count; j++) {
			drawlist[j].SI().SubmitPalette();
		}
	}

	lastRemapFlag = GPalette().UpdateForFrame();

	// draw to vmap
	for (i = 0; i < nplanes; i++)  {
		vmapValid = False;
		DrawEraseList(*eraseLists[i],next.Planes()[i]);
		DrawScreenItemList(*drawLists[i]);
	}

	GPalette().Submit(thisPal);
	GPalette().UpdateFFrame();
	GPalette().UpdateHardware();

	ShowBits();
	vmapValid = True;

	//  Clean up and return
	for (i = 0;i < nplanes;i++)  {
		delete eraseLists[i];
		delete drawLists[i];
	}
}

void
GraphicsMgr::FrameOut(Bool showBits, SOL_Rect dirtyRect)
{
	#ifdef DEBUG
	static int reenter = 0;		//  BUGBUG - DF - Won't work if mult graphmgrs
	
	reenter++;
	if (reenter == 2)
		msgMgr->Alert("Frameout reentered!");

	#endif

	msgMgr->Mono("FrameOut->showBits	%x\n", showBits);
	if (robot)
		robot->DoRobot();

	int nplanes = next.Planes().Length();
	msgMgr->Mono("FrameOut->nplanes1	%x\n", nplanes);

	for (int i = 0;i < nplanes;i++)  {
		drawLists[i] = New DrawList;
		eraseLists[i] = New RectList;
	}

	if (Remap::remapCount && lastRemapFlag)
		RemapMarkRedraw();

	CalcLists(drawLists,eraseLists,dirtyRect);

	// The number of planes may have been reduced by CalcLists
	nplanes = next.Planes().Length();
	msgMgr->Mono("FrameOut->nplanes2	%x\n", nplanes);

	// Sort the draw lists
	for (i = 0; i < nplanes; i++)
		drawLists[i]->Sort();

	for (i = 0; i < nplanes; i++)  {
		int count = drawLists[i]->Length();
		DrawList& drawlist = *drawLists[i];
		for (int j = 0; j < count; j++) {
			drawlist[j].SI().SubmitPalette();
		}
	}

	lastRemapFlag = GPalette().UpdateForFrame();
	msgMgr->Mono("FrameOut->lastRemapFlag	%x\n", lastRemapFlag);

	// draw to vmap
	for (i = 0; i < nplanes; i++)  {
		vmapValid = False;
		DrawEraseList(*eraseLists[i],next.Planes()[i]);
		DrawScreenItemList(*drawLists[i]);
	}

	// move rectangles to video memory
	if (robot)
		robot->FrameAlmostVisible();

	GPalette().UpdateHardware();

	// If not just a reference frame for a
	// show style output changes to hardware
	msgMgr->Alert("FrameOut->showBits	%x\n", showBits);
	if (showBits)
		ShowBits();

	vmapValid = True;

	if (robot)
		robot->FrameNowVisible();

	//  Clean up and return
	for (i = 0;i < nplanes;i++)  {
		delete eraseLists[i];
		delete drawLists[i];
	}

	#ifdef DEBUG
	reenter--;
	#endif
}

void
GraphicsMgr::DrawEraseList(const RectList& eraseList,const Plane& plane)
{
	//	Draws a monochrome rectangles and lines from a plane's
	//	erase list into the screen manager's screen.
	//	Adds to showList.

	// Go through the entire list of erase items
	if (plane.IsColored())  {
		int eraseListSize = eraseList.Length();
		for (int i = 0;i < eraseListSize;++i) {
			SOL_Rect rect = eraseList[i];

			// Add the rectangle to the showList
			MergeToShowList(rect,NextScreen()->ShowList(),pixelsPerCall);
			int hRun = rect.Xlen();
			int vRun = rect.Ylen();
			int drawSkip = CurrBuffer()->XDim();
			char color = plane.Color();
			for (int j = 0;j < vRun;++j) {
				uchar* startDraw = CurrBuffer()->Addr(rect.A.x, rect.A.y + j);
				memset(startDraw,color,hRun);
			}
		}
	}
}

void
GraphicsMgr::DrawScreenItemList(DrawList& drawList)
{
	// Draws a rectangles and lines from cels
	//	into the screen manager's screen.
	//	Adds to showList

	// Go through the entire list of screenItems
	int drawListSize = drawList.Length();
	for (int i = 0;i < drawListSize;++i) {
		MergeToShowList(drawList[i].OnScreen(),NextScreen()->ShowList(),pixelsPerCall);
		drawList[i].Draw(*CurrBuffer());
	}
}

void
GraphicsMgr::NewScreenDim(int xdim, int ydim)
{
	next.NewScreenDim(xdim, ydim);
	ClearLastScreen();
	resizeFlag = True;
}

void
GraphicsMgr::InitRobot(int resNum, long planeID, int priority, int xoff, int yoff)
{
	delete robot;
	robot = New Robot(resNum, planeID, priority, xoff, yoff);
	robot->Init();
}


int
GraphicsMgr::RobotStatus() const
{
	if (robot)
		return robot->Status();
	else
		return ROBOT_UNUSED;
}

Bool
GraphicsMgr::IsOnMe(ScreenItem& si, const Plane& plane, const SOL_Point& p, Bool checkSkip)
{
	Ratio scaleX(graphMgr->Xdim(), SCIRESX);
	Ratio scaleY(graphMgr->Ydim(), SCIRESY);

	SOL_Point pScreen;
	pScreen.x = (p.x * scaleX) + plane.DefRect().A.x;
	pScreen.y = (p.y * scaleY) + plane.DefRect().A.y;

	if (!si.OnScreen().Intersect(pScreen))
		return False;

	if (!checkSkip)
		return True;

	//  pCel = "Un User Scaled" offset 
	si.GetCelObj();

	SOL_Point pCel;

	scaleX.numer =	si.GetCel()->ResX();
	scaleX.denom =	graphMgr->Xdim();
	scaleY.numer =	si.GetCel()->ResY();
	scaleY.denom =	graphMgr->Ydim();

	pCel.x = (pScreen.x - si.X1())*scaleX;
	pCel.y = (pScreen.y - si.Y1())*scaleY;

	if (si.scale.scaleType != SCALE_OFF)  {
		if (si.scale.scaleX  &&  si.scale.scaleY)  {
			pCel.x = pCel.x / Ratio(si.scale.scaleX, 128);
			pCel.y = pCel.y / Ratio(si.scale.scaleY, 128);
		}
	}

	Bool result = si.ReadPix(pCel.x,pCel.y) != si.GetSkip();

	return result;
}

void
GraphicsMgr::SaveRestore(SaveGame*, Bool)
{
}

void	
GraphicsMgr::SaveRestoreDo(Saveable::Func func, SaveGame* saveGame, Bool save)
{
   GPalette().SaveRestoreDo(func, saveGame, save);
   GRemapper().SaveRestoreDo(func, saveGame, save);
	GCursor().SaveRestoreDo(func, saveGame, save);
   Saveable::SaveRestoreDo(func, saveGame, save);
}

void
GraphicsMgr::MergeToShowList(const SOL_Rect& r,RectList& rectList,int callOverhead)
{
/*
	Go through the rectangles merging the given rectangle when
	it is more efficient to draw one large rectangle rather than
	multiple small ones. i.e. which of the following examples
	is quicker to draw?



	 ____________________				    	 ______________________________
	|                    |				    	|                     			 |
	|	                  |				    	|	                   			 |
	|							|				    	|							 			 |
	|							|				    	|							 			 |
	|			  ____________________	    	|			                      |
	|			 |         				 |	    	|			           				 |
	|			 |                    |	    	|			                      |
	|			 |							 |	    	|			  							 |
	|			 |							 |	    	|			  							 |
	|			 |							 |	    	|			  							 |
	|			 |							 |	    	|			  							 |
	|			 |							 |	    	|			  							 |
	|_________|							 |	    	|                   				 |
				 |							 |			|		  							  	 |
				 |							 |			|		  							  	 |
				 |							 |			|		  							  	 |
				 |							 |			|		   						  	 |
				 |____________________|			|______________________________|

*/
	SOL_Rect merged;
	SOL_Rect rect1 = r;
	Bool doOver = True;
	int listSize = rectList.Length();
	while (doOver)  {
		doOver = False;
		for (int i = 0;i < listSize;++i) {
			SOL_Rect rect2  = rectList[i];
			// If 
			// (area merged - (area rect1 + area rect2)) < pixelsPerCall 
			// then use merged rectangle
			// else use both rectangles
			merged.A.x = Min(rect1.A.x,rect2.A.x);
			merged.B.x = Max(rect1.B.x,rect2.B.x);
			merged.A.y = Min(rect1.A.y,rect2.A.y);
			merged.B.y = Max(rect1.B.y,rect2.B.y);

			// Overlap calculate the pixel difference
			int pixelDif =  merged.Area();
			pixelDif -= rect1.Area();
			pixelDif -= rect2.Area();

			if (pixelDif <= callOverhead) {
				rect1 = merged;
				rectList.Delete(i);     
				// Start over again
				doOver = True;
			}
		}
		listSize = rectList.Pack();
	}
	rectList.Add(rect1);
}

void
GraphicsMgr::MovieFrameOut(uchar*	bmp,
										int		bitmapWidth,
										int 		bitmapX1,
										int 		bitmapY1,
										int 		bitmapX2,
										int 		bitmapY2,
										int 		screenX1,
										int 		screenY1,
										Bool		vmdDouble,
										Bool		vmdBlackLines)
{
	// Draw from bitmap into frame buffer
	SOL_Rect rect(bitmapX1,bitmapY1,bitmapX2,bitmapY2);
	SOL_Point point(screenX1,screenY1);
	SOL_Rect showRect(bitmapX1,bitmapY1,bitmapX2,bitmapY2);
	if (vmdDouble) {
		showRect.A.x = showRect.A.x<<1;
		showRect.A.y = showRect.A.y<<1;
		showRect.B.x = showRect.B.x<<1;
		showRect.B.y = showRect.B.y<<1;
	}
	showRect.Offset(point.x,point.y);
	vmapValid = False;

	if (vmdDouble) {
		if (vmdBlackLines)
			PutMovieRectDblBlack(CurrBuffer()->start,bmp,bitmapWidth,&rect,&point);
		else
			PutMovieRectDbl(CurrBuffer()->start,bmp,bitmapWidth,&rect,&point);
	}
	else {
		if (vmdBlackLines)
			PutMovieRectBlack(CurrBuffer()->start,bmp,bitmapWidth,&rect,&point);
		else
			PutMovieRect(CurrBuffer()->start,bmp,bitmapWidth,&rect,&point);
	}

	// Put to hardware
	next.ShowList().Add(showRect);
	ShowBits();
	vmapValid = True;
}

Bool
GraphicsMgr::NowSeen(int id, int planeID, SOL_Rect& nowSeen) 
{
	// This is actually comming from the next screenItem.
	// There is not enough information kept in the last 
	// screenItem to properly calculate the now seen
	// rectangle. Since it works fine this way and to add
	// the information needed in the last screenItem would
	// be a slow down	in the graphics engine I'm not going
	// to change it. See the code below.
	int index = next.Planes().Search(planeID);
	if (index == -1)
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,planeID);
	return next.Planes()[index].NowSeen(id,nowSeen);
}


//Bool
//GraphicsMgr::NowSeen(int id, int planeID, SOL_Rect& nowSeen) 
//{
//	Screen& last =	VisibleScreen();
//
//	int index = last.Planes().Search(planeID);
//	if (index == -1) {
//		index = next.Planes().Search(planeID);
//		if (index == -1)
//			msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,planeID);
//		next.Planes()[index].NowSeen(id,nowSeen);
//		return False;
//	}
//	return last.Planes()[index].NowSeen(id,nowSeen);
//}
