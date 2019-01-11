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

/*
struct {
	ushort	count;
	ushort	func;
	ushort	arg2;
	ushort	arg3;
	ushort	arg4;
	ushort	arg5;
	ushort	arg6;
} static audArgs;
*/

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

	Plane* backPlane = New Plane(SOL_Rect(0, LOWRESY - 62, LOWRESX - 1, LOWRESY - 1));
	backPlane->SetColor(0);
	backPlane->SetPriority(0);
	graphMgr->AddPlane(backPlane);

	graphMgr->FrameOut();
	graphMgr->InitRobot(robotNum, defaultPlaneID);
	graphMgr->GRobot().Resume();

///	audArgs.count = 4;
///	audArgs.func = WPLAY;
///	audArgs.arg2 = 111;
///	audArgs.arg3 = 0;
///	KDoAudio ((kArgs) &audArgs);


	long startTick = timeMgr->GetTickCount();

	while (True)
	{
		graphMgr->FrameOut();
//		AsyncEventCheck();
		if (graphMgr->RobotStatus() == ROBOT_FINAL)  {

			msgMgr->Mono("Video Done!  TickCount is: %d\n", timeMgr->GetTickCount());
			msgMgr->Alert("Time was: %d  Skipped: %d  Total: %d  Max Skip: %d\n", timeMgr->GetTickCount() - startTick,
			   graphMgr->GRobot().FramesSkipped(),
				graphMgr->GRobot().FrameCount(),
				graphMgr->GRobot().MaxFramesSeqSkipped());
			graphMgr->KillRobot();
			graphMgr->FrameOut();

			graphMgr->InitRobot(robotNum, defaultPlaneID);
			graphMgr->GRobot().Resume();
			startTick = timeMgr->GetTickCount();
		}

		if (kbhit()) {
			int ch = toupper(getch());
			if (ch == 'Q')  {
				graphMgr->KillRobot();
				break;		  // quit
			}
		}
	}

	graphMgr->DeletePlane(defaultPlane);
	graphMgr->DeletePlane(backPlane);
}



