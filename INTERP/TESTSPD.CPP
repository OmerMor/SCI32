#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "sol.hpp"
#include "sci.hpp"
#include "graphmgr.hpp"
#include "resource.hpp"
#include "msg.hpp"
#include "event.hpp"
#include "mouse.hpp"
#include "mono.hpp"
#include "cursor.hpp"
#include "time.hpp"
#include "config.hpp"

#include "kernel.hpp"
#include "audio.hpp"
#include "event.hpp"


const int SICOUNT = 5;
int siVelocity [SICOUNT][2];
void intVelocities();
void MoveSi(ScreenItem&, int index, int xDim, int yDim);


void
Test()
{
	graphMgr->GCursor().Set(0, 0, 0);
	graphMgr->GCursor().SetDevice(*mouse);
	int robotNum = 300;
	if (configMgr->Get(configMgr->PlayAudio))
		robotNum = atoi(configMgr->Str(configMgr->PlayAudio));


	// Set up default plane
	Plane* defaultPlane = New Plane(SOL_Rect(0, 0, LOWRESX-1, LOWRESY-1));
	defaultPlane->SetColor(5);
	defaultPlane->SetPriority(2);
	defaultPlane->SetPicNum(490);
//	defaultPlane->SetPicMirror(True);
	graphMgr->AddPlane(defaultPlane);
	int defaultPlaneID = defaultPlane->Id();

	// Set up default plane
	Plane* defaultPlane2 = New Plane(SOL_Rect(0, 0, LOWRESX-1, LOWRESY-1));
	defaultPlane2->SetColor(5);
	defaultPlane2->SetPriority(1);
	graphMgr->AddPlane(defaultPlane2);
	graphMgr->DeletePlane(defaultPlane2);

	Plane* backPlane = New Plane(SOL_Rect(0, LOWRESY - 62, LOWRESX - 1, LOWRESY - 1));
//	Plane* backPlane = New Plane(SOL_Rect(0, 0, LOWRESX - 1, LOWRESY - 1));
	backPlane->SetColor(0);
	backPlane->SetPriority(0);
	graphMgr->AddPlane(backPlane);


	// Lets create some screen items
	Bitmap bitmap(1,2,0);
	ScaleInfo scaleInfo(0,0,0,0);
	ScreenItem*	si2 = New ScreenItem(defaultPlaneID, bitmap, SOL_Point(160,100), scaleInfo, 4);
	ScreenItem*	si3 = New ScreenItem(defaultPlaneID, bitmap, SOL_Point(160,140), scaleInfo, 3);
	ScreenItem*	si1 = New ScreenItem(defaultPlaneID, Bitmap(375, 1, 4), SOL_Point(76, 78), scaleInfo, 5);
	si1->SetPriority(200);

	// Set up the velocity vectors
	intVelocities();
	si3->scale.scaleType = SCALE_MANUAL;
	si3->scale.scaleX = 128;
	si3->scale.scaleY = 128;

	graphMgr->AddScreenItem(si1);
	graphMgr->AddScreenItem(si2);
	graphMgr->AddScreenItem(si3);

	long count = 0;
	int go = True;
	int fast = True; //False;
	int dirX = 1;
	int dirY = 0;

	long totalFrames = 0;
	long frameCount = 0;
	long startTick = timeMgr->GetTickCount();

	fast = True;
	go = True;
	count = 5000;
	graphMgr->FrameOut();

	while (True)
	{
		++totalFrames;
		graphMgr->FrameOut();

		SOL_Event event;
#if 0
		while (True)  {
			if (eventMgr->Get(&event))  {
				if (event.type == SOL_Event::MouseDown)
					if (graphMgr->IsOnMe(si1, event.where, True))
						exit(0);
			}
		}
#endif
		if (count) {
			count--;
			if (!count)  {
				msgMgr->Mono("Time was: %d\n", timeMgr->GetTickCount() - startTick);
				break;
			}
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
				if (ch == 'Q')  {
					graphMgr->KillRobot();
					break;		  // quit
				}
				if (ch == 'A') go = True;	  // automatic
				if (ch == 'M') go = False;	  // manual
				if (ch == 'F') fast = True;  // fast
				if (ch == 'S') fast = False; // slow
				if (ch == 'T') {				  // a timming run only
					msgMgr->Alert("Starting timing..");
					fast = True;
					go = True;
					count = 1000;
					startTick = timeMgr->GetTickCount();
				}
				if (ch == '8')	// increase y scaling
					si3->scale.scaleY += si3->scale.scaleY/10;
				if (ch == '2')	// decrease y scaling
					si3->scale.scaleY -= si3->scale.scaleY/10;
				if (ch == '6')	{ 
					// increase x scaling
					si3->scale.scaleX += si3->scale.scaleX/10;
					// Alter step size
					// Org step/Org scale = new step/new scale
					if (siVelocity[2][0] > 0)
						siVelocity[2][0] = si3->scale.scaleX*7/128;
					else
						siVelocity[2][0] = -si3->scale.scaleX*7/128;
				}
				if (ch == '4')	{ 
					// decrease x scaling
					si3->scale.scaleX -= si3->scale.scaleX/10;
					if (siVelocity[2][0] > 0)
						siVelocity[2][0] = si3->scale.scaleX*7/128;
					else
						siVelocity[2][0] = -si3->scale.scaleX*7/128;
				}
			}
		}
		// ******************************************************************

		// Alter the screen item postions
		MoveSi(*si1,0,320,200);
		MoveSi(*si2,1,320,200);
		MoveSi(*si3,2,320,200);

		graphMgr->UpdateScreenItem(si1);
		graphMgr->UpdateScreenItem(si2);
		graphMgr->UpdateScreenItem(si3);
	}
	graphMgr->DeleteScreenItem(si1);
	graphMgr->DeleteScreenItem(si2);
	graphMgr->DeleteScreenItem(si3);

	graphMgr->DeletePlane(defaultPlane);
	graphMgr->DeletePlane(backPlane);
}



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

void MoveSi(ScreenItem& si,int index,int xDim,int yDim)
{
	if (index < 0 || index > 5)
		msgMgr->Fatal(SrcLoc, "Index out of range: %d!\n", index);

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



