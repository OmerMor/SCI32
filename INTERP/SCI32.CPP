#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <process.h>
#include <conio.h>

#include "sol.hpp"
#include "sci.hpp"
#include "config.hpp"
#include "graphmd.hpp"
#include "sid.hpp"
#include "rect.hpp"
#include "planedef.hpp"
#include "error.hpp"
#include "font.hpp"
#include "memmgr.hpp"
#include "event.hpp"
#include "video.hpp"
#include "window.hpp"
#include "mouse.hpp"
#include "resource.hpp"



const int SICOUNT = 5;
int siVelocity [SICOUNT][2];


SaveVars sv;

void intVelocities()
{
	siVelocity[0][0] = 7;
	siVelocity[0][1] = 2;
	siVelocity[1][0] = 7;
	siVelocity[1][1] = -4;

	siVelocity[2][0] = -7;
	siVelocity[2][1] = 0;

	siVelocity[3][0] = -7;
	siVelocity[3][1] = 6;
	siVelocity[4][0] = 7;
	siVelocity[4][1] = 3;
}

void MoveSi(ScreenItemDef& si,int index,int xDim,int yDim)
{
	if (index < 0 || index > 5)
		Message("Index out of range: %d!\n", index);

	// Bump cel in loop
	si.bitmap.SetCel((si.bitmap.Cel() + 1) % 6);
	if
		(
		(si.pos.x <= 0) && (siVelocity[index][0] < 0)
		||
		(si.pos.x >= (xDim - si.bitmap.Cel()*5)) && (siVelocity[index][0] > 0)
		)
		siVelocity[index][0] = -siVelocity[index][0];

	if
		(
		(si.pos.y <= 0) && (siVelocity[index][1] < 0)
		||
		(si.pos.y >= (yDim - si.bitmap.Cel()*10)) && (siVelocity[index][1] > 0)
		)
		siVelocity[index][1] = -siVelocity[index][1];

	si.pos.x += siVelocity[index][0];
	si.pos.y += siVelocity[index][1];

	// Determine the loop
	if (siVelocity[index][0] > 0 ) 
		si.bitmap.SetLoop(0);
	else
		si.bitmap.SetLoop(1);
}


void
main(int argc, char* argv[])
{
	memMgr = 		New MemoryMgr(200000);
	configMgr = 	New ConfigMgr(argc,argv);
	resMgr = 		New ResourceMgr();
	eventMgr = 		New EventMgr();
	graphMgr = 		New GraphicsMgrDOS(320, 200);

	graphMgr->GCursor().Set(0, 0, 0);
	SOL_Mouse* mouse = New SOL_Mouse(True, 319, 199);
	graphMgr->GCursor().SetDevice(*mouse);

	// Set up default plane
	PlaneDef	dPlane(SOL_Rect(0, 0, 319, 199));
	dPlane.SetResolution(320, 200);
	dPlane.SetColor(0);
	dPlane.SetPriority(2);
	graphMgr->Add(defaultPlane);

	// Set up default plane
	PlaneDef	defaultPlane(SOL_Rect(0, 0, 319, 189));
	defaultPlane.SetResolution(320,200);
	defaultPlane.SetColor(5);
	defaultPlane.SetPriority(2);
	defaultPlane.SetPicNum(490);
//	defaultPlane.SetPicMirror(True);
	int defaultPlaneID = graphMgr->Add(defaultPlane);


	// A second plane
	PlaneDef	otherPlane(SOL_Rect(0, 0, 99, 79));
//	PlaneDef	otherPlane(SOL_Rect(20, 20, 119, 99));
	otherPlane.SetColor(10);
	otherPlane.SetPriority(3);
	int otherPlaneID = graphMgr->Add(otherPlane);


	PlaneDef	backPlane(SOL_Rect(0, 190, 319, 199));
	backPlane.SetResolution(320, 200);
	backPlane.SetColor(5);
	backPlane.SetPriority(30);
	graphMgr->Add(backPlane);


	// Lets create some screen items
	Bitmap bitmap(1,2,0);
	ScaleInfo scaleInfo(0,0,0,0);
//	ScreenItemDef	si1(defaultPlaneID, bitmap, SOL_Point(160,100), scaleInfo, 5);
	ScreenItemDef	si2(defaultPlaneID, bitmap, SOL_Point(160,100), scaleInfo, 4);
	ScreenItemDef	si3(defaultPlaneID, bitmap, SOL_Point(160,40), scaleInfo, 3);
//	ScreenItemDef	si4(defaultPlaneID, bitmap, SOL_Point(160,100), scaleInfo, 2);
	ScreenItemDef	si4(defaultPlaneID, Bitmap(375, 1, 4), SOL_Point(76, 78), scaleInfo, 2);
	ScreenItemDef	si5(otherPlaneID, bitmap, SOL_Point(5,10), scaleInfo, 1);
	ScreenItemDef	si1(defaultPlaneID, Bitmap(375, 1, 4), SOL_Point(76, 78), scaleInfo, 5);
	si4.SetPriority(2000);
	si1.SetPriority(200);

	// Set up the velocity vectors
	intVelocities();

	// One frameout per second
	// Get the time
	si3.scale.scaleType = SCALE_MANUAL;
	si3.scale.scaleX = 128;
	si3.scale.scaleY = 128;

	graphMgr->Add(si1);
	graphMgr->Add(si2);
	graphMgr->Add(si3);
	graphMgr->Add(si4);
	graphMgr->Add(si5);

	long count = 0;
	int go = False;
	int fast = True; //False;
	int dirX = 1;
	int dirY = 0;

	long totalFrames = 0;
	long frameCount = 0;

	while (True)
	{
		++totalFrames;
		graphMgr->FrameOut();

//		getch();
		if (count) {
			printf("Count was %d!\n", count);
			count--;
		}
		else {
			int newInput = False;
			int ch =0;

			if (go) {
				if (kbhit()) {
					ch = getch();
					newInput = True;
				}
			}
			else {
				ch = getch();
				newInput = True;
			}
			if (newInput) {
				newInput = False;
				ch = toupper(ch);
				if (ch == 'Q') break;		  // quit
				if (ch == 'A') go = True;	  // automatic
				if (ch == 'M') go = False;	  // manual
				if (ch == 'F') fast = True;  // fast
				if (ch == 'S') fast = False; // slow
				if (ch == 'T') {				  // a timming run only
					fast = True;
					go = True;
					count = 1000;
				}
				if (ch == 'P') {
					// Reverse the priority between the two planes
					if (otherPlane.Priority() == 3)
						otherPlane.SetPriority(1);
					else
						otherPlane.SetPriority(3);
				}
				if (ch == '8')	// increase y scaling
					si3.scale.scaleY += si3.scale.scaleY/10;
				if (ch == '2')	// decrease y scaling
					si3.scale.scaleY -= si3.scale.scaleY/10;
				if (ch == '6')	{ 
					// increase x scaling
					si3.scale.scaleX += si3.scale.scaleX/10;
					// Alter step size
					// Org step/Org scale = new step/new scale
					if (siVelocity[2][0] > 0)
						siVelocity[2][0] = si3.scale.scaleX*7/128;
					else
						siVelocity[2][0] = -si3.scale.scaleX*7/128;
				}
				if (ch == '4')	{ 
					// decrease x scaling
					si3.scale.scaleX -= si3.scale.scaleX/10;
					if (siVelocity[2][0] > 0)
						siVelocity[2][0] = si3.scale.scaleX*7/128;
					else
						siVelocity[2][0] = -si3.scale.scaleX*7/128;
				}

			}
		}
		// ******************************************************************

		// Alter the screen item postions

		MoveSi(si1,0,320,200);
		MoveSi(si2,1,320,200);
		MoveSi(si3,2,320,200);
		MoveSi(si4,3,320,200);
		MoveSi(si5,4,100,80);

		graphMgr->Update(si1);
		graphMgr->Update(si2);
		graphMgr->Update(si3);
//		graphMgr->Update(si4);
		graphMgr->Update(si5);

		// move the other plane
		SOL_Rect r = otherPlane.DefRect();
		if ((dirX == 1)	&& (r.x1 == 220))
			{
			dirX = 0;
			dirY = 1;
			}
		if ((dirX == -1)	&& (r.x1 == 0))
			{
			dirX = 0;
			dirY = -1;
			}
		if ((dirY == 1)	&& (r.y1 == 120))
			{
			dirX = -1;
			dirY = 0;
			}
		if ((dirY == -1)	&& (r.y1 == 0))
			{
			dirX = 1;
			dirY = 0;
			}
		if (otherPlane.Priority() == 3)  {
			otherPlane.MoveDelta(dirX*4,dirY*5);
		}
		graphMgr->Update(otherPlane);
	}


	delete mouse;
	delete graphMgr;
	delete eventMgr;
	delete resMgr;
	delete configMgr;
	delete memMgr;
}







/*
//	delete mouse;
	delete graphMgr;
	delete eventMgr;
	delete resMgr;
	delete configMgr;
	delete memMgr;
	return;
*/
